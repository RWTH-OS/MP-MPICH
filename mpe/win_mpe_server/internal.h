#ifndef __MPE_SERVER_INTERNAL__
#define __MPE_SERVER_INTERNAL__

#define MPM_DRAW (WM_USER+12)
#define MPM_DRAW_TEXT (MPM_DRAW+1)
#define MPM_GET_DIB (MPM_DRAW+2)
#define MPM_SAVE_DIB (MPM_DRAW+3)

#ifdef __cplusplus
extern "C" {
#endif

extern DisplayInfo DInfo;

typedef struct {
    BOOL Open;
    LONG Id;
    DWORD RefCount;
    HWND hWnd;
    HDC memDC;
    HBITMAP OldBmp;
    HANDLE hThread;
    int LineStyle;
    int LineWidth;

    POINT LastClick;
    BOOL Clicked;
    int Button;

    CRITICAL_SECTION winCS;
    HANDLE hClickEvent;
    
    BOOL Dragging;
    int CurrentVisual;
    double Ratio;
    RECT DragCoords;
    HDC dragDC;

    RECT UpdateRect;
    BOOL TimerRunning;
    DWORD Delay;

    BOOL Palette;
    DWORD NumColors;
    HPALETTE hPalette;

} MPE_Context;

typedef struct {
    char *txt;
    int x,y;
    DWORD color;
} Text_type;

typedef struct {
    DWORD size;
    DWORD PixelOffset;
    byte *buffer;
} DIB_params_t;

extern VOID ServiceStart (HWND NotifyWindow);
extern DWORD RegisterWindowClass(HINSTANCE dllInstance);
extern DWORD OpenWindow(MPE_Context*,int,int,int,int,unsigned char*);
extern DWORD GetBitmapImage(HDC, HBITMAP,DWORD *, BYTE *,DWORD *);

extern DWORD dwErr;
extern HANDLE StartEvent;
extern HINSTANCE hModule;

#ifdef __cplusplus
}
#endif


#endif