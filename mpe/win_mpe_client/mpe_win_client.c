/*
    $Id$
*/

#include <windows.h>
#include <rpc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpe_win_graphics.h"
#include "r_mpe.h"

#if (_MSC_VER >=1400)
#pragma warning (disable:4996)
#endif

#define BUFSIZE 2000
#define NUMCOL 16


#define MPE_RGB PALETTERGB

typedef struct {
    void *RPCContext;
    char *CaptureFileName;
    BOOL CaptureOnServer;
    DWORD CaptureFrequ;
    DWORD LastCapture;
    Graphic_request Requests[BUFSIZE];
    DWORD numRequests;
    DisplayInfo DInfo;
    DWORD numColors;
    DWORD *Colortable;
} ContextHandle_t;

static DWORD colors[NUMCOL] = {
    MPE_RGB(255,255,255),
    MPE_RGB(0,0,0),
    MPE_RGB(255,0,0),
    MPE_RGB(255,255,0),
    MPE_RGB(0,255,0),
    MPE_RGB(0,255,255),
    MPE_RGB(0,0,255),
    MPE_RGB(255,0,255),
    MPE_RGB(127,255,212),
    MPE_RGB(34,139,34),
    MPE_RGB(255,165,0),
    MPE_RGB(176,48,96),
    MPE_RGB(255,64,64),
    MPE_RGB(255,181,197),
    MPE_RGB(255,114,86),
    MPE_RGB(127,127,127)
};
 
LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           error,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        sprintf( lpszBuf, TEXT(" %u"), error );
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        sprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, error );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}

#define NUM_PROTOCOLS 3
static char *Protocols[] = { "ncalrpc",
                             "ncacn_ip_tcp",
                             "ncacn_nb_tcp"
			   };

static 
DWORD CreateBinding(char *Host,RPC_BINDING_HANDLE *Binding) {
    
    
    unsigned char *stringBinding=0;
    RPC_STATUS status;
    int i;

    
    for(i=0;i<NUM_PROTOCOLS;++i) {
	status = RpcStringBindingCompose(0,
	    (unsigned char*)Protocols[i],
	    (unsigned char*)Host,
	    0,
	    0,
	    (unsigned char**)&stringBinding);
	if (status != RPC_S_OK) return(status);
	
	status = RpcBindingFromStringBinding(stringBinding, Binding);
	RpcStringFree(&stringBinding);
	
	if (status != RPC_S_OK) return(status);	
	status =
	    RpcBindingSetAuthInfo(*Binding,
	    0,
	    RPC_C_AUTHN_LEVEL_NONE,
	    RPC_C_AUTHN_WINNT,
	    0,
	    0
	    );
	
	if (status != RPC_S_OK) break;

	status = Ping(*Binding);
	if(status == RPC_S_OK) {
	    break;
	}
    }

    if(status != RPC_S_OK) {
	RpcBindingFree(Binding);
        *Binding = 0;
    }

    return status;
}

void __RPC_FAR * __RPC_API midl_user_allocate(size_t cBytes) 
{ 
    return(malloc(cBytes)); 
} 

void __RPC_API midl_user_free(void __RPC_FAR * p) 
{ 
    free(p); 
} 

static DWORD SaveToFile(ContextHandle_t *Context) {
    static DWORD memsize=0;
    BYTE *memory=0;
    DWORD written=0,res;
    HANDLE file;
    BITMAPFILEHEADER bfh = {0x4d42,0,0,0,0};
    char Filename[MAX_PATH];
    
    sprintf(Filename,"%s%.3d.bmp",Context->CaptureFileName,
	Context->LastCapture / Context->CaptureFrequ);
    
    if(Context->CaptureOnServer) {
	return R_Save_bitmap(&Context->RPCContext,Filename);
    } else {
	if(memsize) {
	    memory = (BYTE*)malloc(memsize);
	    if(!memory) return ERROR_OUTOFMEMORY;
	}
	res = R_Get_DIB(&Context->RPCContext,&bfh.bfOffBits,memory,&memsize,&written);
#if DEBUG
	fprintf(stderr,"Called R_Get_DIB, result is %d size is %d\n",res,memsize);
#endif
	if(!written && memsize) {
	    memory = realloc(memory,memsize);
	    if(!memory) return ERROR_OUTOFMEMORY;
	    res = R_Get_DIB(&Context->RPCContext,&bfh.bfOffBits,memory,&memsize,&written);
#if DEBUG
	    fprintf(stderr,"Called R_Get_DIB again, result is %d size is %d\n",res,memsize);
#endif
	    
	} else if(!memsize) {
	    free(memory);
	    return res;
	}
	
	file = CreateFile(Filename,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(file == INVALID_HANDLE_VALUE) {
	    return GetLastError();
	}
        
	bfh.bfSize = sizeof(bfh)+memsize;
	bfh.bfOffBits += sizeof(bfh);
	WriteFile(file,&bfh,sizeof(bfh),&written,0);
	WriteFile(file,memory,memsize,&written,0);
	res = GetLastError();
	CloseHandle(file);
	free(memory);
    }
    return res;
}

int MPE_Open_graphics(MPE_XGraph *handle, MPI_Comm comm, 
		      char *display, int x, int y, int w, int h, int is_collective ) 
{
    ContextHandle_t *newHandle;
#ifndef MPE_NOMPI
    int        numprocs, namelen;
#endif
    int        myid,res,winId;
    char *pos;
    char title[MAX_PATH];
    BOOL freeDisplay=FALSE;
    RPC_BINDING_HANDLE Binding;
    
    
    myid = 0;		     /* for the single processor version */
    *handle            = 0;    /* In case of errors */
    newHandle = (ContextHandle_t*)malloc(sizeof(ContextHandle_t));
    if(!newHandle) return ERROR_OUTOFMEMORY;
    newHandle->numRequests = 0;
    newHandle->RPCContext = NULL;
    newHandle->CaptureFileName = NULL;
    newHandle->CaptureFrequ = 0;
    newHandle->LastCapture = 0;
    
#ifndef MPE_NOMPI
    if (is_collective) {
	/* Not supported; just use individual connections */
	is_collective = 0;
    }
    
    MPI_Comm_size(comm,&numprocs);
    MPI_Comm_rank(comm,&myid);
#endif
   
    if (!display) {
#ifndef MPE_NOMPI
	int str_len;
#endif
	
#if DEBUG
	fprintf( stderr, "[%d] Guessing at display name.\n", myid );
#endif
	
	if (myid == 0) {
	    display = getenv( "DISPLAY" );
	    
#if DEBUG
	    fprintf( stderr, "$DISPLAY = %s\n", display );
#endif
	    
	    if (!display || display[0] == ':') {
		/* Replace display with hostname */
		freeDisplay = TRUE;
#ifdef MPE_NOMPI
		display = (char *)malloc( 100 );
		MPE_GetHostName( display, 100 );
#else
		/* This is not correct, since there is no guarentee that this
		is the "correct" network name */
		display = (char *)malloc( MPI_MAX_PROCESSOR_NAME );
		MPI_Get_processor_name( display, &namelen );
#endif
		
#if DEBUG
		fprintf( stderr, "Process 0 is: %s\n", display );
#endif
	    }
	    
#ifndef MPE_NOMPI
	    str_len = strlen( display ) + 1;
	    MPI_Bcast( &str_len, 1, MPI_INT, 0, comm );
#endif
	    
	} 
	
#ifndef MPE_NOMPI
	else {
	    MPI_Bcast( &str_len, 1, MPI_INT, 0, comm );
	    display = (char *) malloc( sizeof( char ) * str_len );
	    freeDisplay = TRUE;
	}
	MPI_Bcast( display, str_len, MPI_CHAR, 0, comm );
#endif
	
    }
    
    pos = strchr(display,':');
    if(pos) *pos = 0;
    
#if DEBUG
    fprintf( stderr, "[%d] trying to open %s\n", myid, display );
#endif
    
    res = CreateBinding(display,&Binding);
    
    winId = 0;    
    if (0 == myid) {
	if(res == RPC_S_OK) {
	    pos = strrchr(__argv[0],'\\');
	    if(pos) ++pos;
	    else pos = __argv[0];
	    strcpy(title,pos);
	    pos = strrchr(title,'.');
	    if(pos) *pos = 0;
	    res = R_Create_window(Binding,&newHandle->RPCContext,x,y,w,h,title,&winId,&newHandle->DInfo);
	    RpcBindingFree(&Binding);
	}
	
#ifndef MPE_NOMPI
	MPI_Bcast( &winId, 1, MPI_UNSIGNED_LONG, 0, comm );
#endif /* ifndef MPE_NOMPI */
	
    }
#ifndef MPE_NOMPI
    else {
	MPI_Bcast( &winId, 1, MPI_UNSIGNED_LONG, 0, comm );
	if (winId && res == RPC_S_OK) {  /* if process 0 connected */
	    res = R_Connect_window(Binding,&newHandle->RPCContext,winId,&newHandle->DInfo);
	    RpcBindingFree(&Binding);
	}
    }
#endif /* ifndef MPE_NOMPI */
    
#if DEBUG
    fprintf( stderr, "%s to %s from process %d.\n",
	(res==RPC_S_OK) ? "Successfully connected" : "Failed to connect",
	display, myid );
#endif
    
    
    
    if (res != RPC_S_OK) {
#ifndef MPE_NOMPI
	char myname[MPI_MAX_PROCESSOR_NAME];
	int namelen;
	MPI_Get_processor_name( myname, &namelen );
	fprintf( stderr, "Failed to connect to %s from %s\n", 
	    display, myname );
#endif
	*handle = (MPE_XGraph) 0;
	if(freeDisplay) free(display);
	return res;
    } else {
	if(freeDisplay) free(display);
	*handle = newHandle;
	newHandle->Colortable = (DWORD*)malloc(NUMCOL*sizeof(DWORD));
	memcpy(newHandle->Colortable,colors,NUMCOL*sizeof(DWORD));
	if(!myid && newHandle->DInfo.Palette)
	    R_Set_colortable(&newHandle->RPCContext,NUMCOL,newHandle->Colortable);
	newHandle->numColors = NUMCOL;
	return ERROR_SUCCESS;
    }
}


int MPE_Close_graphics (MPE_XGraph *handle ) {
    int res;
    ContextHandle_t *Context = (ContextHandle_t*)*handle;
    if(!Context || !Context->RPCContext) return ERROR_SUCCESS;
    if(Context->CaptureFrequ) {
	MPE_Update(handle);
	if((Context->LastCapture % Context->CaptureFrequ) != 0) {
	    Context->LastCapture += Context->CaptureFrequ;
	    SaveToFile(Context);
	}
    }
    MPE_CaptureFile(*handle,NULL,0);
    res = R_Close_window(&Context->RPCContext);
    if(res != ERROR_SUCCESS) {
	MPE_Xerror(res,"R_CloseWindow failed");
	
    }
    free(Context->Colortable);
    free(*handle);
    *handle = 0;
    return res; 
}

int MPE_Draw_point( MPE_XGraph handle, int x, int y,MPE_Color color ) {
    DWORD idx;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    idx = Context->numRequests;

    Context->Requests[idx].type = REQU_POINT;
    Context->Requests[idx].data.point.x = x;
    Context->Requests[idx].data.point.y = y;
    Context->Requests[idx].data.point.c = Context->Colortable[color];
    
    if(++Context->numRequests == BUFSIZE) 
	return MPE_Update(handle);
    return ERROR_SUCCESS;
}

int MPE_Draw_points(MPE_XGraph handle, MPE_Point *points, int numPoints) {
    
    int i,res;
    for(i=0;i<numPoints;++i) {
	res = MPE_Draw_point(handle,points[i].x,points[i].y,points[i].c);
	if(res != ERROR_SUCCESS) return res;
    }
    return ERROR_SUCCESS;
}


int MPE_Draw_line( MPE_XGraph handle, int x1, int y1,int x2, int y2, MPE_Color color ) {
    DWORD idx;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    idx = Context->numRequests;

    Context->Requests[idx].type = REQU_LINE;
    Context->Requests[idx].data.line.x1 = x1;
    Context->Requests[idx].data.line.x2 = x2;
    Context->Requests[idx].data.line.y1 = y1;
    Context->Requests[idx].data.line.y2 = y2;
    Context->Requests[idx].data.line.c = Context->Colortable[color];
    
    if(++Context->numRequests == BUFSIZE) 
	return MPE_Update(handle);
    return ERROR_SUCCESS;
}

static int Draw_circle( MPE_XGraph handle, int x, int y, int radius, MPE_Color color,BOOL fill) {
    DWORD idx;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    idx = Context->numRequests;

    Context->Requests[idx].type = REQU_CIRCLE;
    Context->Requests[idx].data.circle.fill = fill;
    Context->Requests[idx].data.circle.x = x;
    Context->Requests[idx].data.circle.y = y;
    Context->Requests[idx].data.circle.radius = radius;
    Context->Requests[idx].data.circle.c = Context->Colortable[color];
    
    if(++Context->numRequests == BUFSIZE) 
	return MPE_Update(handle);
    return ERROR_SUCCESS;
}

int MPE_Draw_circle( MPE_XGraph handle, int x, int y, int radius, MPE_Color color) {
    return Draw_circle(handle,x,y,radius,color,FALSE);
}

int MPE_Fill_circle(MPE_XGraph handle, int x, int y, int radius, MPE_Color color) {
    return Draw_circle(handle,x,y,radius,color,TRUE);
}


int MPE_Draw_string(MPE_XGraph handle, int x, int y, MPE_Color color,char *txt ) {
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    DWORD res;

    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;
    return R_Draw_string(&Context->RPCContext,x,y,Context->Colortable[color],(unsigned char*)txt);
}

static int Draw_rectangle(MPE_XGraph handle, int x, int y,int w, int h, MPE_Color color,BOOL fill) {

    DWORD idx;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    idx = Context->numRequests;

    Context->Requests[idx].type = REQU_RECT;
    Context->Requests[idx].data.rect.fill = fill;
    Context->Requests[idx].data.rect.x1 = x;
    Context->Requests[idx].data.rect.y1 = y;
    Context->Requests[idx].data.rect.x2 = x+w;
    Context->Requests[idx].data.rect.y2 = y+h;
    Context->Requests[idx].data.rect.c = Context->Colortable[color];

    if(++Context->numRequests == BUFSIZE) 
	return MPE_Update(handle);
    return ERROR_SUCCESS;
}

int MPE_Fill_rectangle(MPE_XGraph handle, int x, int y,int w, int h, MPE_Color color) {
    return Draw_rectangle(handle,x,y,w,h,color,TRUE);
}

int MPE_Draw_rectangle(MPE_XGraph handle, int x, int y,int w, int h, MPE_Color color) {
    return Draw_rectangle(handle,x,y,w,h,color,FALSE);
}


int MPE_Update(MPE_XGraph handle) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    if(Context->numRequests) {
	res = R_Draw(&Context->RPCContext,Context->Requests,Context->numRequests);
	Context->numRequests = 0;
	if(res == RPC_S_OK && Context->CaptureFrequ) {
#if DEBUG
	    fprintf(stderr,"Checking capture. Last is %d, Frequ is %d\n",Context->LastCapture,Context->CaptureFrequ);
#endif
	    ++Context->LastCapture;
	    if((Context->LastCapture % Context->CaptureFrequ) == 0) {
		res = SaveToFile(Context);
#if DEBUG
		fprintf(stderr,"Called SaveToFile\n");
#endif
		if(res != ERROR_SUCCESS) 
		    MPE_Xerror(res,"SaveBitmap result");
	    }
	}
    } else res = ERROR_SUCCESS;
    return res;
}

int MPE_Num_colors ( MPE_XGraph handle, int *nc ) {
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    return Context->numColors;
    //return R_Num_colors(&Context->RPCContext,nc);
}

// fraction is a part of the rainbow (0.0 - 1.0) = (Red-Yellow-Green-Cyan-Blue-Magenta-Red)
// intensity (0.0 - 1.0) 0 = black, 1 = full color, 2 = white
static MPE_Color getColor(double fraction, double intensity)
{
    double red, green, blue;
    int r,g,b;
    
    double dtemp;
    fraction = fabs(modf(fraction, &dtemp));
    
    if (intensity > 2.0)
	intensity = 2.0;
    if (intensity < 0.0)
	intensity = 0.0;
    
    dtemp = 1.0/6.0;
    
    if (fraction < 1.0/6.0)
    {
	red = 1.0;
	green = fraction / dtemp;
	blue = 0.0;
    }
    else
	if (fraction < 1.0/3.0)
	{
	    red = 1.0 - ((fraction - dtemp) / dtemp);
	    green = 1.0;
	    blue = 0.0;
	}
	else
	    if (fraction < 0.5)
	    {
		red = 0.0;
		green = 1.0;
		blue = (fraction - (dtemp*2.0)) / dtemp;
	    }
	    else
		if (fraction < 2.0/3.0)
		{
		    red = 0.0;
		    green = 1.0 - ((fraction - (dtemp*3.0)) / dtemp);
		    blue = 1.0;
		}
		else
		    if (fraction < 5.0/6.0)
		    {
			red = (fraction - (dtemp*4.0)) / dtemp;
			green = 0.0;
			blue = 1.0;
		    }
		    else
		    {
			red = 1.0;
			green = 0.0;
			blue = 1.0 - ((fraction - (dtemp*5.0)) / dtemp);
		    }
		    
    if (intensity > 1)
    {
	intensity = intensity - 1.0;
	red = red + ((1.0 - red) * intensity);
	green = green + ((1.0 - green) * intensity);
	blue = blue + ((1.0 - blue) * intensity);
    }
    else
    {
	red = red * intensity;
	green = green * intensity;
	blue = blue * intensity;
    }
    
     
    r = (int)(red * 255.0);
    g = (int)(green * 255.0);
    b = (int)(blue * 255.0);
    
    return MPE_RGB(r,g,b);
}

int MPE_Make_color_array(MPE_XGraph handle, int num_colors, MPE_Color colors[])
{
    double fraction;
    int i,res = ERROR_SUCCESS;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    if(Context->numColors<(unsigned int)num_colors+2) {
	Context->Colortable = (DWORD*)realloc(Context->Colortable,(num_colors+2)*sizeof(DWORD));
    }
    Context->numColors = num_colors+2;
    for (i=0; i<num_colors; ++i)
    {
	fraction = (double)i / (double)num_colors;
	colors[i] = i+2;
	Context->Colortable[i+2] = getColor(fraction, 1.0);
    }
    if(Context->DInfo.Palette) 
	res = R_Set_colortable(&Context->RPCContext,Context->numColors,Context->Colortable);
    return res;
}



int MPE_CaptureFile(MPE_XGraph handle, char *name, int frequ ) {
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    char *pos;

    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    Context->CaptureFrequ = frequ;
#if DEBUG
	fprintf(stderr,"Entering MPE_CaptureFile(%d). Name is %x\n",frequ,Context->CaptureFileName);
#endif

    if(Context->CaptureFileName) {
        free(Context->CaptureFileName);
        Context->CaptureFileName = 0;
    }
    if(!name) {
	Context->CaptureFrequ = 0;
	return ERROR_SUCCESS;
    }
    pos = strchr(name,':');
    if(pos) {
	if(!strnicmp(name,"server:",7)) {
	    name = pos+1;
	    Context->CaptureOnServer = TRUE;
	} else 
	    Context->CaptureOnServer = FALSE;

    }
    Context->CaptureFileName = (char*)malloc((strlen(name)+1)*sizeof(char));
    if(!Context->CaptureFileName) return ERROR_OUTOFMEMORY;
    strcpy(Context->CaptureFileName,name);
    return ERROR_SUCCESS;
}

int MPE_Draw_logic( MPE_XGraph handle, int type) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;

    return R_Draw_logic(&Context->RPCContext,type);
    
}

int MPE_Line_thickness(MPE_XGraph handle, int pixels) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;
    
    return R_Line_thickness(&Context->RPCContext,pixels);
    
}

int MPE_Draw_dashes( MPE_XGraph handle, int dashlen) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;
    return R_Draw_dashes(&Context->RPCContext,dashlen);
}

int MPE_Dash_offset(MPE_XGraph handle, int offset) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;
    return R_Dash_offset(&Context->RPCContext,offset);
}

int MPE_Add_RGB_color( MPE_XGraph handle, int r, int g, int b, MPE_Color *CL ) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    Context->Colortable = (DWORD*)realloc(Context->Colortable,(Context->numColors+1)*sizeof(DWORD));
    Context->Colortable[Context->numColors] = MPE_RGB(r,g,b);
    if(Context->DInfo.Palette)
	res = R_Add_color(&Context->RPCContext,Context->Colortable[Context->numColors]);
    else res = ERROR_SUCCESS;
    *CL = Context->numColors++;
  
    return res;
}

int MPE_Xerror( int error, char *msg ) {
    char message[256];
    if(error != ERROR_SUCCESS) {
	GetLastErrorText(error,message,256);
	fprintf(stderr,"%s: %s\n",msg,message);
    }
    return error;
}

/* xmouse */
int MPE_Get_mouse_press( MPE_XGraph handle, int *x, int *y, int *button ) {
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;
    
    return R_Get_mouse_press(&Context->RPCContext,x,y,button);
}

int MPE_Iget_mouse_press( MPE_XGraph handle, int *x, int *y, int *button,int *was_pressed ) {
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    return R_Iget_mouse_press(&Context->RPCContext,x,y,button,was_pressed);
}

int MPE_Get_drag_region(MPE_XGraph handle, int button, int dragVisual,
			int *pressx, int *pressy, int *releasex, int *releasey )
{
    DWORD res;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;

    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;

    return R_Get_drag_region(&Context->RPCContext,MPE_DRAG_FIXED_RECT,1.0,
	pressx,pressy,releasex,releasey);
}


int MPE_Get_drag_region_fixratio(MPE_XGraph handle, int button, double ratio,
				 int *pressx,int *pressy, 
				 int *releasex, int *releasey ) 
{
    int res,width,height;
    ContextHandle_t *Context = (ContextHandle_t*)handle;
    if(!Context || !Context->RPCContext) return ERROR_INVALID_HANDLE;
    
    res = MPE_Update(handle);
    if(res != ERROR_SUCCESS) return res;

    res = R_Get_drag_region(&Context->RPCContext,MPE_DRAG_FIXED_RECT,ratio,
	pressx,pressy,releasex,releasey);

    if(res != RPC_S_OK) return res;
    
    if (*pressx < *releasex) {
	width = *releasex - *pressx;
    } else {
	width = *pressx - *releasex;
    }
    if (*pressy < *releasey) {
	height = *releasey - *pressy;
    } else {
	height = *pressy - *releasey;
    }
    
    if (width*ratio > height) {
	/* width is limiting factor */
	height = (int) (width * ratio);
	if (*releasey > *pressy) {
	    *releasey = *pressy + height;
	} else {
	    *releasey = *pressy - height;
	}
    } else {
	width = (int) (height / ratio);
	if (*releasex > *pressx) {
	    *releasex = *pressx + width;
	} else {
	    *releasex = *pressx - width;
	}
    }
    return res;
}
