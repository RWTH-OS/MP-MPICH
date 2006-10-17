/*
    $Id$
*/


#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "r_mpe.h"
#include "internal.h"
#include "Dragging.h"
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

static BOOL Registered=FALSE;
static TCHAR szAppName[] = TEXT("WIN_MPE_WINDOW");
DisplayInfo DInfo = {0,0};

typedef struct {
    MPE_Context *Context;
    int x,y,w,h;
    unsigned char *title;
} ThreadParam;


static void PopFileInitialize(HWND hwnd,OPENFILENAME &ofn)
{
	static TCHAR szFilter[] = TEXT("Bitmaps (*.bmp)\0*.bmp\0")\
				  TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName= NULL;
} 

static BOOL GetFileName(HWND hwnd,PTSTR pstrFileName,PTSTR pstrTitleName) 
{	static OPENFILENAME ofn;
	PopFileInitialize(hwnd,ofn);
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrTitle = pstrTitleName;
	ofn.Flags |= OFN_HIDEREADONLY;

	return(GetSaveFileName(&ofn));
}

inline void win_draw_rect(HDC hDC,MPE_Context *Context,Rect_type *rect) {
    HBRUSH newBrush;
    
    
    if(!rect->fill) {
	newBrush = CreateSolidBrush(rect->c);
	FrameRect(hDC,(RECT*)&rect->x1,newBrush);
	DeleteObject(newBrush);
    } else {
	Rectangle(hDC,rect->x1,rect->y1,rect->x2,rect->y2);
    }
    UnionRect(&Context->UpdateRect,(const RECT*)&rect->x1,&Context->UpdateRect);    
}


inline void win_draw_circle(HDC hDC,MPE_Context *Context,Circle_type *circle) {
    HBRUSH oldBrush;
    RECT R = {circle->x-circle->radius,circle->y-circle->radius,
	      circle->x+circle->radius,circle->y+circle->radius};
    if(!circle->fill) {
	oldBrush = (HBRUSH)SelectObject(hDC,GetStockObject(NULL_BRUSH));
    } 
    
    Ellipse(hDC,R.left,R.top,R.right,R.bottom);
    UnionRect(&Context->UpdateRect,&R,&Context->UpdateRect);
    
    if(!circle->fill) {
	SelectObject(hDC,oldBrush);
    }
}



inline void win_draw_line(HDC hDC,MPE_Context *Context,Line_type *line) {
    HPEN oldPen,newPen;
    LOGBRUSH lb;
    RECT R = {line->x1-Context->LineWidth,line->y1-Context->LineWidth,
	      line->x2+Context->LineWidth,line->y2+Context->LineWidth};

    if(Context->LineStyle != PS_SOLID && Context->LineWidth >1) {
	lb.lbStyle = BS_SOLID; 
        lb.lbColor = line->c;
        lb.lbHatch = 0; 
	newPen = ExtCreatePen(PS_GEOMETRIC | Context->LineStyle, 
                              Context->LineWidth, &lb, 0, NULL);
    } else newPen = CreatePen(Context->LineStyle,Context->LineWidth,line->c);
    oldPen = (HPEN)SelectObject(hDC,newPen);
    MoveToEx(hDC,line->x1,line->y1,0);
    LineTo(hDC,line->x2,line->y2);
    UnionRect(&Context->UpdateRect,&R,&Context->UpdateRect);
    DeleteObject(SelectObject(hDC,oldPen));
}


void win_draw_text(HDC hDC,MPE_Context *Context, Text_type *txt) {
    int len;
    COLORREF oldTxtCol;
    RECT R;
    oldTxtCol = SetTextColor(hDC,txt->color);
    len = strlen(txt->txt);
    TextOut(hDC,txt->x,txt->y,txt->txt,len);
    SetTextColor(hDC,oldTxtCol);
    R.top = txt->y;
    R.left = txt->x;
    R.bottom = txt->y+DrawTextEx(hDC,txt->txt,len,&R,DT_CALCRECT|DT_SINGLELINE|DT_LEFT,0);
    
    InvalidateRect(Context->hWnd,&R,FALSE);
}




void win_draw_generic(HDC hDC,MPE_Context *Context, Graphic_request *requs,WORD numRequs) {
    WORD i;

    Color_type curr_col=0;
    HBRUSH oldB=0,actB=0;
    HPEN oldP=0,actP=0;
    RECT R;

    oldB = (HBRUSH)SelectObject(hDC,GetStockObject(BLACK_BRUSH));
    oldP = (HPEN)SelectObject(hDC,GetStockObject(BLACK_PEN));
    for(i=0;i<numRequs;++i) {
	
	switch(requs[i].type) {
	case REQU_POINT: SetPixelV(hDC,requs[i].data.point.x,
			               requs[i].data.point.y,
				       requs[i].data.point.c);
			 R.left = requs[i].data.point.x;
			 R.top = requs[i].data.point.y;
			 R.bottom = requs[i].data.point.y+1;
			 R.right = requs[i].data.point.x+1;
			 UnionRect(&Context->UpdateRect,&Context->UpdateRect,&R);
			 break;
	case REQU_LINE: win_draw_line(hDC,Context,&requs[i].data.line); break;
	case REQU_RECT:
	    if(curr_col != requs[i].data.rect.c && requs[i].data.rect.fill) {
		curr_col = requs[i].data.rect.c;
		actP = CreatePen(PS_SOLID,1,requs[i].data.rect.c);
		DeleteObject(SelectObject(hDC,actP));
		actB = CreateSolidBrush(requs[i].data.rect.c);
		DeleteObject(SelectObject(hDC,actB));		
	    }
	    win_draw_rect(hDC,Context,&requs[i].data.rect); 

	    break;
	case REQU_CIRCLE: 
	    if(curr_col != requs[i].data.circle.c) {
		curr_col = requs[i].data.circle.c;
		actP = CreatePen(PS_SOLID,1,requs[i].data.circle.c);
		DeleteObject(SelectObject(hDC,actP));
		actB = CreateSolidBrush(requs[i].data.circle.c);
		DeleteObject(SelectObject(hDC,actB));		
	    }
	    win_draw_circle(hDC,Context,&requs[i].data.circle); 
	    break;
	default: break;
	}
    }
    if(oldP) DeleteObject(SelectObject(hDC,oldP));
    if(oldB) DeleteObject(SelectObject(hDC,oldB));
}

DWORD GetBitmapImage(HDC hDC, HBITMAP hBmp,DWORD *PixelStart, BYTE *image,DWORD *size) {
    int NumScanLines;
    BITMAPINFO bi;
    DWORD PalSize,NeededSize;
   
    memset(&bi,0,sizeof(bi));
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biCompression = BI_RGB;

    NumScanLines = GetDIBits(hDC,hBmp,0,0,0,&bi,DIB_RGB_COLORS);
    
    if(!NumScanLines) {
	*size = 0;
	return 0;
    }

   if(bi.bmiHeader.biBitCount>24) {
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	NumScanLines = GetDIBits(hDC,hBmp,0,0,0,&bi,DIB_RGB_COLORS);
	if(!NumScanLines) {
	    *size = 0;
	    return 0;
	}
    }

    if(!bi.bmiHeader.biSizeImage)
	bi.bmiHeader.biSizeImage = 
	((((bi.bmiHeader.biWidth * bi.bmiHeader.biBitCount) + 31) & ~31) >> 3) 
	* bi.bmiHeader.biHeight;

    if(bi.bmiHeader.biBitCount <16) {
	PalSize = ((2<<(bi.bmiHeader.biBitCount-1))-1)*sizeof(RGBQUAD);
    } else if(bi.bmiHeader.biCompression == BI_BITFIELDS) PalSize = 12;
    else PalSize = 0;

    NeededSize = bi.bmiHeader.biSize+PalSize+bi.bmiHeader.biSizeImage;
    
    if(*size < NeededSize) {
	*size = NeededSize;
	return 0;
    }
    *PixelStart = bi.bmiHeader.biSize+PalSize;
    *size = NeededSize;
    memcpy(image,&bi.bmiHeader,bi.bmiHeader.biSize);
    NumScanLines = GetDIBits(hDC,hBmp,0,bi.bmiHeader.biHeight,image+bi.bmiHeader.biSize+PalSize,
	                     (PBITMAPINFO)image,DIB_RGB_COLORS);


    return NumScanLines;
}

DWORD SaveBitmap(MPE_Context *Context,char *name) {
    static char Filename[MAX_PATH]="mpe.bmp";
    BYTE *memory=0;
    DWORD memsize=0,written,error;
    HANDLE file;
    HBITMAP hbmp;
    BITMAPFILEHEADER bfh = {0x4d42,0,0,0,0};
    
    if(!name) {
	if(!GetFileName(Context->hWnd,Filename,"")) return ERROR_CANCELLED;	
    } else strcpy(Filename,name);
    

    hbmp = (HBITMAP)SelectObject(Context->memDC,Context->OldBmp);
    GetBitmapImage(Context->memDC,hbmp,&bfh.bfOffBits,memory,&memsize);
    if(!memsize) {
	error = GetLastError();
	SelectObject(Context->memDC,hbmp);
	if(!name)
	    MessageBox(Context->hWnd,"Could not read bitmap bits","Error",MB_OK|MB_ICONERROR);
	return error;
    }
   
    memory = (BYTE*)malloc(memsize);
    
    GetBitmapImage(Context->memDC,hbmp,&bfh.bfOffBits,memory,&memsize);
    if(!memsize) {
	error = GetLastError();
	SelectObject(Context->memDC,hbmp);
	if(memory) free(memory);
	if(!name) 
	    MessageBox(Context->hWnd,"Could not read bitmap bits","Error",MB_OK|MB_ICONERROR);
	return error;
    }
    
    file = CreateFile(Filename,GENERIC_WRITE,0,0,CREATE_ALWAYS,
	              FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,0);
    if(file == INVALID_HANDLE_VALUE) {
	error = GetLastError();
	if(!name)
	    MessageBox(Context->hWnd,"Could not create file","Error",MB_OK|MB_ICONERROR);
	SelectObject(Context->memDC,hbmp);
	if(memory) free(memory);
	return error;
    }
    bfh.bfSize = sizeof(bfh)+memsize;
    bfh.bfOffBits += sizeof(bfh);
    if(!WriteFile(file,&bfh,sizeof(bfh),&written,0))
	error = GetLastError();
    else
	if(!WriteFile(file,memory,memsize,&memsize,0))
	    error = GetLastError();
	else error = ERROR_SUCCESS;
    CloseHandle(file);
    SelectObject(Context->memDC,hbmp);
    free(memory);
    return error;

}

LRESULT CALLBACK WndProc(HWND wnd,UINT Msg, WPARAM wParam,LPARAM lParam) {
    MPE_Context *Context;
    HDC hDC;
    PAINTSTRUCT PS;
    RECT R;
    HMENU hMenu;
    HBITMAP hbmp;
    DIB_params_t *params;
    DWORD res;
    
    
	Context = (MPE_Context*)GetWindowLongPtr(wnd, GWLP_USERDATA);
	// old WIN32 stuff
    //Context = (MPE_Context*)GetWindowLong(wnd, GWL_USERDATA);
    switch(Msg) {
    case WM_ERASEBKGND : return 0;
    case WM_TIMER:
	hDC=GetDC(wnd);
	GetClipBox(hDC,&R);
	IntersectRect(&R,&R,&Context->UpdateRect);
	if(Context->hPalette) {
	    SelectPalette(hDC,Context->hPalette,FALSE);
	    RealizePalette(hDC);
	}
	BitBlt(hDC,R.left,R.top,R.right-R.left,R.bottom-R.top,Context->memDC,R.left,R.top,SRCCOPY);
	ReleaseDC(wnd,hDC);
	SetRectEmpty(&Context->UpdateRect);
	if(Context->TimerRunning) {
	    Context->TimerRunning = FALSE;
	    KillTimer(wnd,wParam);
	}
	return 0;
    case WM_PAINT:
	hDC = BeginPaint(wnd,&PS);
	if(Context->hPalette) {
	    SelectPalette(hDC,Context->hPalette,FALSE);
	    RealizePalette(hDC);
	}
	BitBlt(hDC,PS.rcPaint.left,PS.rcPaint.top,PS.rcPaint.right-PS.rcPaint.left,
	           PS.rcPaint.bottom-PS.rcPaint.top,Context->memDC,
		   PS.rcPaint.left,PS.rcPaint.top,SRCCOPY);
	
	EndPaint(wnd,&PS);
	return 0;
	
    case WM_CAPTURECHANGED: 
	EnterCriticalSection(&Context->winCS);
	Context->Dragging = FALSE;
	DeleteObject(SelectObject(Context->dragDC,GetStockObject(BLACK_PEN)));
	ReleaseDC(Context->hWnd,Context->dragDC);
	PulseEvent(Context->hClickEvent);
	LeaveCriticalSection(&Context->winCS);
	return 0;
    case WM_QUERYNEWPALETTE:
#ifdef _DEBUG
	fprintf(stderr,"Got QUERYNEWPALETTE\n");
#endif
	if(!Context || !Context->hPalette) return FALSE;
	hDC = GetDC(wnd);
	SelectPalette(hDC,Context->hPalette,FALSE);
	RealizePalette(hDC);
	InvalidateRect(wnd,NULL,FALSE);
	ReleaseDC(wnd,hDC);
#ifdef _DEBUG
	fprintf(stderr,"Handeled\n");
#endif

	return TRUE;
    case WM_PALETTECHANGED:
	if(!Context || !Context->hPalette || (HWND)wParam == wnd) 
	    break;
	hDC = GetDC(wnd);
	SelectPalette(hDC,Context->hPalette,FALSE);
	RealizePalette(hDC);
	UpdateColors(hDC);
	ReleaseDC(wnd,hDC);
	break;
    case WM_MOUSEMOVE:
	if(Context->Dragging) {
	    DrawDragVisual(Context);
	    Context->DragCoords.right = LOWORD(lParam);
	    Context->DragCoords.bottom = HIWORD(lParam);
	    DrawDragVisual(Context);
	}
	return 0;
    case WM_LBUTTONDOWN:
	if(Context->CurrentVisual != MPE_NO_DRAG) {
	    SetCapture(wnd);
	    Context->Dragging = TRUE;
	    Context->dragDC = GetDC(wnd);
	    SetROP2(Context->dragDC,R2_XORPEN);
	    SelectObject(Context->dragDC,CreatePen(PS_SOLID,1,RGB(255,255,255)));
	    SelectObject(Context->dragDC,GetStockObject(NULL_BRUSH));
	    Context->DragCoords.left = LOWORD(lParam);
	    Context->DragCoords.top = HIWORD(lParam);
	    Context->DragCoords.right = LOWORD(lParam)+1;
	    Context->DragCoords.bottom = HIWORD(lParam)+1;
	    DrawDragVisual(Context);
	}
	return 0;
    case WM_RBUTTONDOWN:
	if(Context->Dragging) {
	    DrawDragVisual(Context);
	    ReleaseCapture();
	}
	return 0;
	
    case WM_LBUTTONUP:
	if(Context->Dragging) {
	    EnterCriticalSection(&Context->winCS);	
	    DrawDragVisual(Context);
	    Context->DragCoords.right = LOWORD(lParam);
	    Context->DragCoords.bottom = HIWORD(lParam);
	    Context->CurrentVisual = MPE_NO_DRAG;
	    ReleaseCapture(); 	
	    LeaveCriticalSection(&Context->winCS);	
	    return 0;
	}
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
	GetClientRect(wnd,&R);
	EnterCriticalSection(&Context->winCS);	
	Context->LastClick.x = R.right-LOWORD(lParam);
	Context->LastClick.y = HIWORD(lParam);
	Context->Button = ((Msg==WM_LBUTTONUP)?1:((Msg==WM_RBUTTONUP)?2:3));
	PulseEvent(Context->hClickEvent);
	Context->Clicked = TRUE;
	LeaveCriticalSection(&Context->winCS);
	return 0;
    case WM_DESTROY: 
	EnterCriticalSection(&Context->winCS);
	Context->Open = FALSE;
	SetEvent(Context->hClickEvent);
	LeaveCriticalSection(&Context->winCS);
	PostQuitMessage(0);
	return 0;
    case MPM_DRAW:
	hDC = Context->memDC;
	win_draw_generic(hDC,Context,(Graphic_request*)lParam,wParam);
	if(!Context->Delay) SendMessage(wnd,WM_TIMER,123,0);
	else if(!Context->TimerRunning) {
	    SetTimer(wnd,123,Context->Delay,0);
	    Context->TimerRunning = TRUE;
	}	
	return 0;
    case MPM_GET_DIB:
	params = (DIB_params_t*)lParam;
	hbmp = (HBITMAP)SelectObject(Context->memDC,Context->OldBmp);
	res = GetBitmapImage(Context->memDC,hbmp,
	    &params->PixelOffset,
	    params->buffer,
	    &params->size);
	SelectObject(Context->memDC,hbmp);
	return res?ERROR_SUCCESS:RPC_S_CALL_FAILED;
    case MPM_SAVE_DIB:
	return SaveBitmap(Context,(char*)lParam);	   
	
    case MPM_DRAW_TEXT:
	hDC = Context->memDC;
	win_draw_text(hDC,Context,(Text_type*)lParam);
	return 0;
    case WM_COMMAND:
	// The menu events:
	if(wParam==ID_IMAGE_SAVETODISK) {
	    SaveBitmap(Context,0);
	    return 0;
	}
	if(wParam==IDCLOSE) {SendMessage(wnd,WM_CLOSE,0,0); return 0;}
	DWORD index;
	hMenu = GetSubMenu(GetMenu(wnd),1);
	switch(Context->Delay) {
	case 0: index = 0; break;
	case 50: index = 1; break;
	case 100: index = 2; break;
	case 200: index = 3; break;
	case 500: index = 4; break;
	default: index = 0; break;
	}
	CheckMenuItem(hMenu,index,MF_UNCHECKED|MF_BYPOSITION);
	switch(wParam) {
	case ID_REFRESHRATE_NODELAY: Context->Delay = 0; index = 0; break;
	case ID_REFRESHRATE_50: Context->Delay = 50; index = 1; break;
	case ID_REFRESHRATE_100: Context->Delay = 100; index = 2; break;
	case ID_REFRESHRATE_200: Context->Delay = 200; index = 3; break;
	case ID_REFRESHRATE_500: Context->Delay = 500; index = 4; break;
	default: index=0;
	}
	CheckMenuItem(hMenu,index,MF_CHECKED|MF_BYPOSITION);
	return 0;
    }
    return DefWindowProc(wnd,Msg,wParam,lParam);
}


DWORD RegisterWindowClass(HINSTANCE dllInstance) {
    
    WNDCLASS wndclass;

    if(Registered) return ERROR_SUCCESS;

    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = GetModuleHandle(0);
    wndclass.hIcon = LoadIcon(dllInstance,MAKEINTRESOURCE(IDI_ICON));
    wndclass.hCursor = LoadCursor(0,IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if(!RegisterClass(&wndclass)) 
	return GetLastError();

    Registered = TRUE;
    return ERROR_SUCCESS;
}

static DWORD __stdcall WindowLoop(void *Param) {
    ThreadParam *P=(ThreadParam*)Param;
    HWND hWnd; 
    MSG msg;
    HBITMAP hBitmap;
    RECT CR,WR;
    HMENU hMenu ;

    hMenu = LoadMenu(hModule,"MAIN_MENU");
	if(!hMenu) printf("Could not load menu: %d\n",GetLastError());
    P->Context->hWnd = hWnd = 
	CreateWindow(szAppName,TEXT((const char*)P->title),
		     WS_OVERLAPPED|WS_MINIMIZEBOX|WS_BORDER|WS_CAPTION|WS_SYSMENU,
		     P->x,P->y,P->w,P->h,0,hMenu,GetModuleHandle(0),P->Context);

    if(!P->Context->hWnd) 
	ExitThread(GetLastError());

    P->Context->Id = (LONG)hWnd;

    P->Context->memDC = CreateCompatibleDC(NULL);
    if(!P->Context->memDC) {
	free(P);
	ExitThread(ERROR_OUTOFMEMORY);
    }
    
    
    GetClientRect(hWnd,&CR);
    if(CR.right!=P->w || CR.bottom != P->h) {
	int offx,offy;
        GetWindowRect(hWnd,&WR);
	
	offx = P->w-CR.right;
	offy = P->h-CR.bottom;
	WR.bottom -= WR.top-offy;
	WR.right -= WR.left-offx;	
        SetWindowPos(hWnd,HWND_TOP,0,0,WR.right,WR.bottom,SWP_NOCOPYBITS|SWP_NOMOVE);
	GetClientRect(hWnd,&CR);
    }

    hBitmap = CreateBitmap(CR.right, CR.bottom, 
	                   GetDeviceCaps(P->Context->memDC, PLANES), 
			   GetDeviceCaps(P->Context->memDC, BITSPIXEL), 
			   NULL);
    if(!hBitmap) {
	DeleteDC(P->Context->memDC);
	free(P);
	ExitThread(ERROR_OUTOFMEMORY);
    }
    
    if(!DInfo.Maxcolors) {
	DInfo.Palette = P->Context->Palette = (RC_PALETTE & GetDeviceCaps(P->Context->memDC,RASTERCAPS));
	if(DInfo.Palette) 
	    DInfo.Maxcolors = GetDeviceCaps(P->Context->memDC,NUMCOLORS);
	else DInfo.Maxcolors = (DWORD)-1;
    }
    SetBkMode(P->Context->memDC,TRANSPARENT);
    
    P->Context->OldBmp = (HBITMAP)SelectObject(P->Context->memDC, hBitmap);
	SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)P->Context);
	// old WIN32 stuff
    //SetWindowLong(hWnd,GWL_USERDATA,(LONG)P->Context);
    Rectangle(P->Context->memDC,0,0,CR.right,CR.bottom);
    
    ShowWindow(hWnd,SW_SHOW);
    SetActiveWindow(hWnd);
    P->Context->Open = TRUE;
    
    while(GetMessage(&msg,hWnd,0,0) > 0) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    // Restore the original bitmap in the memDC and delete the new one.
    DeleteObject(SelectObject(P->Context->memDC, P->Context->OldBmp));
    DeleteDC(P->Context->memDC);
    if(P->Context->hPalette) DeleteObject(P->Context->hPalette);
    free(P);
    ExitThread(ERROR_SUCCESS);
    return 0;
}


DWORD OpenWindow(MPE_Context* Context,int x,int y,int w,int h,unsigned char *title) {
    ThreadParam *Param;
    DWORD ret,id;
    Param = (ThreadParam*)malloc(sizeof(ThreadParam));
    if(!Param) return RPC_S_OUT_OF_MEMORY;
    Param->Context = Context;
    Param->x = x;
    Param->y = y;
    Param->w = w;
    Param->h = h;
    Param->title = title;
    Context->hThread = CreateThread(0,0,WindowLoop,Param,0,&id);
    if(!Context->hThread) return GetLastError();
    while(!Context->Open) {
	if(WaitForSingleObject(Context->hThread,10) == WAIT_OBJECT_0) {
	    GetExitCodeThread(Context->hThread,&ret);
#ifdef _DEBUG
	    fprintf(stderr,"Thread failed to start: %u\n",ret);
#endif
	    CloseHandle(Context->hThread);
	    Context->hThread = 0;
	    return ret;
	}
    }
    return ERROR_SUCCESS;

}


#ifdef __cplusplus
}
#endif


