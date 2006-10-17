/*
* $Id$ 
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning (disable : 4786)

#include <windows.h>
#include <stdlib.h>
#include <map>
#include "mpe_server.h"
#include "r_mpe.h"
#include "internal.h"
#include "Dragging.h"

typedef
std::map < int,MPE_Context * >CContexts;

static CContexts Contexts;
static CRITICAL_SECTION CS;

HINSTANCE hModule;

#ifdef __cplusplus
extern
"C" {
#endif

HWND g_hNotifyWindow;
HANDLE hMainThread = 0;

BOOL APIENTRY DllMain(HINSTANCE Module, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
	hModule = Module;
	DisableThreadLibraryCalls(Module);
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:break;
    }
    return TRUE;
}

DWORD __stdcall
ThreadStart(void *NotifyWindow) {
    dwErr = RegisterWindowClass((HINSTANCE) hModule);
    if (dwErr == ERROR_SUCCESS)
	ServiceStart((HWND) NotifyWindow);
    ExitThread(dwErr);
    return 0; //If you use an outdated SDK
} 

MPE_SERVER_API DWORD
Start_MPE_Server(HWND NotifyWindow) {
    DWORD
	Id;

    if (!hMainThread) {
	g_hNotifyWindow = NotifyWindow;
	if (!NotifyWindow)
	    StartEvent = CreateEvent(0, TRUE, FALSE, 0);
	InitializeCriticalSection(&CS);
	hMainThread =
	    CreateThread(0, 0, ThreadStart, NotifyWindow, 0, &Id);
	if (!hMainThread) {
	    return GetLastError();
	}
	if (!NotifyWindow) {
	    WaitForSingleObject(StartEvent, INFINITE);
	    CloseHandle(StartEvent);
	}
    }
    return ERROR_SUCCESS;

}

MPE_SERVER_API VOID
Stop_MPE_Server(HWND NotifyWindow) {
    // Stops the server, wakes the main thread.
    if (hMainThread) {
	g_hNotifyWindow = NotifyWindow;
	if (!g_hNotifyWindow)
	    PostMessage(NotifyWindow, MPM_SERVER_STOPPING, 0, 0);
	RpcMgmtStopServerListening(0);
	if (!g_hNotifyWindow)
	    WaitForSingleObject(hMainThread, INFINITE);
	CloseHandle(hMainThread);
	hMainThread = 0;
	DeleteCriticalSection(&CS);
    }
} 

error_status_t
Ping(handle_t Binding) {
    return RPC_S_OK;
} 

error_status_t
R_Create_window(handle_t Binding,
		PPCONTEXT_HANDLE_TYPE pphContext,
		int x,int y,int w,int h,
		unsigned char __RPC_FAR * title,
		int __RPC_FAR * WindowId,
		DisplayInfo __RPC_FAR * info) {

    MPE_Context *Context;

    DWORD res;
    Context = (MPE_Context *) malloc(sizeof(MPE_Context));
    *pphContext = Context;
    if (!Context)
	return RPC_S_OUT_OF_MEMORY;

    Context->hClickEvent = CreateEvent(0, TRUE, FALSE, 0);
    if (!Context->hClickEvent) {
	free(Context);
	*WindowId = 0;
	*pphContext = 0;
    }

    if (x == -1)
	x = CW_USEDEFAULT;

    if (y == -1)
	y = CW_USEDEFAULT;
    if (w == -1)
	w = CW_USEDEFAULT;
    if (h == -1)
	h = CW_USEDEFAULT;
    Context->Clicked = FALSE;
    Context->LineWidth = 1;
    Context->LineStyle = PS_SOLID;
    Context->Open = FALSE;
    Context->RefCount = 1;
    Context->Dragging = FALSE;
    Context->CurrentVisual = MPE_NO_DRAG;
    Context->Ratio = 1.0;
    Context->TimerRunning = FALSE;
    Context->Delay = 0;
    Context->hPalette = 0;
    Context->NumColors = 0;
    InitializeCriticalSection(&Context->winCS);

    EnterCriticalSection(&CS);
    res = OpenWindow(Context, x, y, w, h, title);
    if (Context->Open) {
	Contexts.insert(CContexts::value_type(Context->Id, Context));
	*WindowId = Context->Id;
    } else {
	R_Close_window((PPCONTEXT_HANDLE_TYPE) & Context);
	*WindowId = 0;
	*pphContext = 0;
    }
#ifdef _DEBUG
    printf("New Window created...\n");
#endif
    LeaveCriticalSection(&CS);

    *info = DInfo;
    return res;
}

error_status_t
R_Connect_window(handle_t Binding,		
		 PPCONTEXT_HANDLE_TYPE pphContext,
		 int WindowId,
		 DisplayInfo __RPC_FAR * info) {
    CContexts::iterator i;
    EnterCriticalSection(&CS);
    i = Contexts.find(WindowId);
    if (i == Contexts.end()) {
	LeaveCriticalSection(&CS);
	*pphContext = 0;
	return RPC_S_INVALID_ARG;
    }

    ++(*i).second->RefCount;

    *pphContext = (*i).second;
#ifdef _DEBUG
    printf("Accepted new connection. Number now: %d\n",
	   (*i).second->RefCount);
#endif
    LeaveCriticalSection(&CS);
    *info = DInfo;
    return RPC_S_OK;
}

error_status_t
R_Close_window(PPCONTEXT_HANDLE_TYPE pphContext) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    EnterCriticalSection(&CS);
#ifdef _DEBUG
    fprintf(stderr, "removing client. %d remaining...\n",
	    Context->RefCount - 1);
#endif

    if (!--Context->RefCount) {
	if (Context->Open)
	    PostMessage(Context->hWnd, WM_QUIT, 0, 0);
	Contexts.erase(Context->Id);
	LeaveCriticalSection(&CS);
	WaitForSingleObject(Context->hThread, INFINITE);
	CloseHandle(Context->hThread);
	CloseHandle(Context->hClickEvent);
	DeleteCriticalSection(&Context->winCS);
	free(Context);
#ifdef _DEBUG
	fprintf(stderr, "Window closed...\n");
#endif

    } else
	LeaveCriticalSection(&CS);

    *pphContext = 0;
    return RPC_S_OK;
}

error_status_t
R_Draw(PPCONTEXT_HANDLE_TYPE pphContext,
       Graphic_request __RPC_FAR * requs,
       int nr) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    SendMessage(Context->hWnd, MPM_DRAW, (WPARAM) nr, (LPARAM) requs);
    return RPC_S_OK;

} 

error_status_t
R_Draw_string(PPCONTEXT_HANDLE_TYPE pphContext,
	      int x,
	      int y,
	      Color_type color,
	      unsigned char __RPC_FAR * txt) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    Text_type
	t = {
	(char *) txt, x, y, color
    };
    SendMessage(Context->hWnd, MPM_DRAW_TEXT, 0, (LPARAM) & t);

    return RPC_S_OK;
} 

error_status_t
R_Draw_logic(PPCONTEXT_HANDLE_TYPE pphContext,
	     int ROp) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    SetROP2(Context->memDC, ROp);
    return RPC_S_OK;
} 

error_status_t
R_Line_thickness(PPCONTEXT_HANDLE_TYPE
		 pphContext,
		 int num_pixels) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    Context->LineWidth = num_pixels;
    return RPC_S_OK;
} 

error_status_t
R_Draw_dashes(
		 /*
		  * * [in] 
		  */ PPCONTEXT_HANDLE_TYPE pphContext,
		 /*
		  * * [in] 
		  */ int dashlen) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    if (dashlen)
	Context->LineStyle = PS_DASH;
    else
	Context->LineStyle = PS_SOLID;
    return RPC_S_OK;
} 

error_status_t
R_Dash_offset(
		 /*
		  * * [in] 
		  */ PPCONTEXT_HANDLE_TYPE pphContext,
		 /*
		  * * [in] 
		  */ int offset) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    if (offset)
	Context->LineStyle = PS_DASH;
    else
	Context->LineStyle = PS_SOLID;
    return RPC_S_OK;
} 

error_status_t
R_Set_colortable(PPCONTEXT_HANDLE_TYPE pphContext,
		 unsigned long NumColors,
		 unsigned long __RPC_FAR * colors) {
    
    LOGPALETTE *plp;
    HPALETTE newPal;
    int i;
    MPE_Context *Context = (MPE_Context *) * pphContext;
    

    EnterCriticalSection(&Context->winCS);
#ifdef _DEBUG
    fprintf(stderr,"Entering R_Set_palette\n");
#endif

    if (!Context->Palette) {
	LeaveCriticalSection(&Context->winCS);
	return ERROR_SUCCESS;
    }

    if (NumColors > 256)
	NumColors = 256;

    plp =
	(LOGPALETTE *) malloc(sizeof(LOGPALETTE) +
			      (NumColors - 1) * sizeof(PALETTEENTRY));
    if (!plp) {
	LeaveCriticalSection(&Context->winCS);
	return ERROR_OUTOFMEMORY;
    }
    for (i = 0; i < NumColors; ++i) {
	plp->palPalEntry[i].peRed = (BYTE) (colors[i] & 0x0000FF);
	plp->palPalEntry[i].peGreen =
	    (BYTE) ((colors[i] & 0x00FF00) >> 8);
	plp->palPalEntry[i].peBlue = (BYTE) ((colors[i] & 0xFF0000) >> 16);
	plp->palPalEntry[i].peFlags = 0;
    }

    plp->palNumEntries = NumColors;
    plp->palVersion = 0x0300;
    newPal = CreatePalette(plp);
    SelectPalette(Context->memDC, newPal, FALSE);
    RealizePalette(Context->memDC);
    if (Context->hPalette) {
	DeleteObject(Context->hPalette);
    }
    Context->hPalette = newPal;
    Context->NumColors = NumColors;
    free(plp);
    LeaveCriticalSection(&Context->winCS);
    return ERROR_SUCCESS;
}

error_status_t
    R_Add_color(PPCONTEXT_HANDLE_TYPE pphContext, unsigned long color) {
    PALETTEENTRY pe;
    int i;
    DWORD res;
    MPE_Context *Context = (MPE_Context *) * pphContext;

    if (!Context->Palette)
	return ERROR_SUCCESS;

    EnterCriticalSection(&Context->winCS);
    if (!Context->Palette || Context->NumColors ==256) {
	LeaveCriticalSection(&Context->winCS);
	return ERROR_SUCCESS;
    }

    if(!Context->hPalette) {
	R_Set_colortable(pphContext,1,&color);
	LeaveCriticalSection(&Context->winCS);
	return ERROR_SUCCESS;
    }

    i = GetNearestPaletteIndex(Context->hPalette,color);
    if(i==CLR_INVALID) {
	res = GetLastError();
	LeaveCriticalSection(&Context->winCS);
	return res;
    }
    if(!GetPaletteEntries(Context->hPalette,i,1,&pe)) {
	res = GetLastError();
	LeaveCriticalSection(&Context->winCS);
	return res;
    }
    if((pe.peBlue == (BYTE) ((color & 0xFF0000) >> 16) ) &&
       (pe.peGreen == (BYTE) ((color & 0x00FF00) >> 8)) &&
       (pe.peRed == (BYTE) (color & 0x0000FF))) {
        LeaveCriticalSection(&Context->winCS);
        return ERROR_SUCCESS;
    }
    

    ResizePalette(Context->hPalette,Context->NumColors+1);
    pe.peRed = (BYTE) (color & 0x0000FF);
    pe.peGreen =(BYTE) ((color & 0x00FF00) >> 8);
    pe.peBlue = (BYTE) ((color & 0xFF0000)>> 16);
    pe.peFlags = 0;
    SetPaletteEntries(Context->hPalette,Context->NumColors++,1,&pe);
    RealizePalette(Context->memDC);
    LeaveCriticalSection(&Context->winCS);
    return ERROR_SUCCESS;

}

error_status_t
R_Get_mouse_press(PPCONTEXT_HANDLE_TYPE
		  pphContext,
		  int __RPC_FAR * x,
		  int __RPC_FAR * y,
		  int __RPC_FAR * Button) {
    
    DWORD res;
    MPE_Context *Context = (MPE_Context *) * pphContext;

    EnterCriticalSection(&Context->winCS);
    if (!Context->Open) {
	LeaveCriticalSection(&Context->winCS);
	return ERROR_INVALID_PARAMETER;
    }
    SetForegroundWindow(Context->hWnd);

    LeaveCriticalSection(&Context->winCS);
    res = WaitForSingleObject(Context->hClickEvent, INFINITE);
    *x = Context->LastClick.x;
    *y = Context->LastClick.y;
    *Button = Context->Button;
    Context->Clicked = FALSE;
    return Context->Open ? RPC_S_OK : RPC_S_INVALID_ARG;
}

error_status_t
    R_Iget_mouse_press(PPCONTEXT_HANDLE_TYPE pphContext,
		       int __RPC_FAR * x,
		       int __RPC_FAR * y,
		       int __RPC_FAR * Button,
		       int __RPC_FAR * was_pressed) {

    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;

    EnterCriticalSection(&Context->winCS);
    *x = Context->LastClick.x;
    *y = Context->LastClick.y;
    *Button = Context->Button;
    *was_pressed = Context->Clicked;
    Context->Clicked = FALSE;
    LeaveCriticalSection(&Context->winCS);
    return RPC_S_OK;
}

error_status_t
    R_Get_drag_region(PPCONTEXT_HANDLE_TYPE pphContext,
		      int dragVisual,
		      double ratio,
		      int __RPC_FAR * x1,
		      int __RPC_FAR * y1,
		      int __RPC_FAR * x2, int __RPC_FAR * y2) {
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (dragVisual != MPE_DRAG_NONE &&
	dragVisual != MPE_DRAG_LINE &&
	dragVisual != MPE_DRAG_RECT &&
	dragVisual != MPE_DRAG_SQUARE &&
	dragVisual != MPE_DRAG_CIRCLE_RADIUS &&
	dragVisual != MPE_DRAG_CIRCLE_DIAMETER &&
	dragVisual != MPE_DRAG_CIRCLE_BBOX &&
	dragVisual != MPE_DRAG_OVAL_BBOX
	&& dragVisual != MPE_DRAG_FIXED_RECT) {
	dragVisual = MPE_DRAG_RECT;
    }

    EnterCriticalSection(&Context->winCS);

    if (!Context->Open) {
	LeaveCriticalSection(&Context->winCS);
	return ERROR_INVALID_PARAMETER;
    }
    SetForegroundWindow(Context->hWnd);
    Context->CurrentVisual = dragVisual;
    Context->Ratio = ratio;
    SetRect(&Context->DragCoords, -1, -1, -1, -1);

    LeaveCriticalSection(&Context->winCS);
    while (
	   (Context->DragCoords.bottom < 0
	    || Context->CurrentVisual != MPE_NO_DRAG) && Context->Open)
	WaitForSingleObject(Context->hClickEvent, INFINITE);
    ConvertCoords(dragVisual, Context->DragCoords.left,
		  Context->DragCoords.top, Context->DragCoords.right,
		  Context->DragCoords.bottom, ratio, x1, y1, x2, y2);
    *x2 += *x1;
    *y2 += *y1;

    return Context->Open ? RPC_S_OK : RPC_S_INVALID_ARG;
}

error_status_t
R_Get_DIB(PPCONTEXT_HANDLE_TYPE pphContext,
	  unsigned long __RPC_FAR * PixelStart,
	  byte __RPC_FAR * image,
	  unsigned long __RPC_FAR * size,
	  unsigned long __RPC_FAR * allocated) {

    DWORD
	res;
    DIB_params_t
	params = {
	*size, 0, image
    };
    MPE_Context *
	Context = (MPE_Context *) * pphContext;

    if (!Context->Open)
	return ERROR_INVALID_PARAMETER;
    res = SendMessage(Context->hWnd, MPM_GET_DIB, 0, (LPARAM) & params);

    if (*size < params.size) {
	res = RPC_S_BUFFER_TOO_SMALL;
	*allocated = 0;
    } else {
	*allocated = params.size;
    }
    *PixelStart = params.PixelOffset;
    *size = params.size;
    return res;
}

error_status_t
R_Save_bitmap(PPCONTEXT_HANDLE_TYPE pphContext,
	  unsigned char __RPC_FAR * Filename) {
MPE_Context *
    Context = (MPE_Context *) * pphContext;

if (!Context->Open)
    return ERROR_INVALID_PARAMETER;

return SendMessage(Context->hWnd, MPM_SAVE_DIB, 0, (LPARAM) Filename);
} 

void __RPC_USER
PCONTEXT_HANDLE_TYPE_rundown(PCONTEXT_HANDLE_TYPE handle) {
#ifdef _DEBUG
    fprintf(stderr, "Client crashed...\n");
#endif
    R_Close_window(&handle);
} void __RPC_FAR *
__RPC_API midl_user_allocate(size_t cBytes) {
    return (malloc(cBytes));
}
void
  __RPC_API
midl_user_free(void __RPC_FAR * p) {
    free(p);
}
#ifdef __cplusplus
}
#endif
