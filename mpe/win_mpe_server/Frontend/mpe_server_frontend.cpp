/*
    $Id$
*/
#include <windows.h>
#include <shellapi.h>

#include "mpe_server.h"
#include "resource.h"
#include "CommCtrl.h"

HINSTANCE g_hInstance;

enum STATES {ENABLED=0,DISABLED=1};

static TCHAR szAppName[] = TEXT("LFBS_MPE_SERVER_WINDOW");
static STATES State;
static HWND hDialog;

#define TRAY_NOTIFICATION (WM_USER+11)
#define ICON_ID 2

static DWORD IDS[2]= {IDI_ICON1,IDI_ICON2};
static char *Hints[2] = {"MPE Server running","MPE Server stopped"};

BOOL TaskBarSetIcon(HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip,DWORD MSG) 
{ 
    BOOL res; 
    NOTIFYICONDATA tnid; 
 
    tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = hwnd; 
    tnid.uID = uID; 
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnid.uCallbackMessage = TRAY_NOTIFICATION; 
    tnid.hIcon = hicon; 
    if (lpszTip) 
        lstrcpyn(tnid.szTip, lpszTip, sizeof(tnid.szTip)); 
    else 
        tnid.szTip[0] = '\0'; 
 
    res = Shell_NotifyIcon(MSG, &tnid); 
 
    if (hicon) 
        DestroyIcon(hicon); 
 
    return res; 
} 

BOOL TaskBarDeleteIcon(HWND hwnd, UINT uID) 
{ 
    BOOL res; 
    NOTIFYICONDATA tnid; 
 
    tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = hwnd; 
    tnid.uID = uID; 
         
    res = Shell_NotifyIcon(NIM_DELETE, &tnid); 
    
    return res; 
} 

VOID APIENTRY HandlePopupMenu (
        HWND   hwnd,
        POINT point)

{
    HMENU hMenu;
    HMENU hMenuTrackPopup;

    /* Get the menu for the popup from the resource file. */
    hMenu = LoadMenu (g_hInstance, MAKEINTRESOURCE(IDR_Context));
    if (!hMenu)
        return;

    /* Get the first menu in it which we will use for the call to
     * TrackPopup(). This could also have been created on the fly using
     * CreatePopupMenu and then we could have used InsertMenu() or
     * AppendMenu.
     */
    hMenuTrackPopup = GetSubMenu (hMenu, 0);

    EnableMenuItem(hMenuTrackPopup,State,MF_BYPOSITION|MF_ENABLED);
    /* Draw and track the "floating" popup */
    SetForegroundWindow(hwnd);
    TrackPopupMenu (hMenuTrackPopup, TPM_BOTTOMALIGN|TPM_RIGHTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);

    /* Destroy the menu since were are done with it. */
    DestroyMenu (hMenu);
    SendMessage(hwnd,WM_NULL,0,0);
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	    if(lParam == ID_POPUP_STOPSERVER)
		SendDlgItemMessage(hDlg,IDC_Message,WM_SETTEXT,0,(LPARAM)"MPE Server stopping");
	    else 
		SendDlgItemMessage(hDlg,IDC_Message,WM_SETTEXT,0,(LPARAM)"MPE Server starting");
	    SendDlgItemMessage(hDlg,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0,80));  
	    return TRUE;
	case MPM_SERVER_PENDING:
	    SendDlgItemMessage(hDlg,IDC_PROGRESS,PBM_STEPIT,0,0);
	    return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND wnd,UINT Msg, WPARAM wParam,LPARAM lParam) {
    
    static UINT reload;
    HICON hIcon;
    
    switch(Msg) {
    case WM_CREATE:
	reload = RegisterWindowMessage(TEXT("TaskbarCreated"));
	SendMessage(wnd,reload,0,0);
	return 0;
    case WM_DESTROY: 
	TaskBarDeleteIcon(wnd,ICON_ID);
	PostQuitMessage(0); 
	return 0;
    case MPM_SERVER_PENDING:
	SendMessage(hDialog,Msg,wParam,lParam);
	return 0;
    case MPM_SERVER_STARTING:
    case MPM_SERVER_STOPPING:	
	hDialog = CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_ServerDialog),wnd, (DLGPROC) DlgProc,Msg);
	ShowWindow(hDialog,SW_SHOW);
	return 0;
    case MPM_SERVER_STARTED:
    case MPM_SERVER_STOPPED:
	DestroyWindow(hDialog);
	State = ((Msg == MPM_SERVER_STARTED)?ENABLED:DISABLED);
	hIcon=LoadIcon(g_hInstance,MAKEINTRESOURCE(IDS[State]));
	if(hIcon) TaskBarSetIcon(wnd,ICON_ID,hIcon,Hints[State],NIM_MODIFY);
	hDialog=0;
	if(lParam != ERROR_SUCCESS)
	    MessageBox(wnd,"Operation failed","Error",MB_ICONERROR|MB_OK);
	return 0;
    case WM_COMMAND: 
	switch(LOWORD(wParam)) {
	case (IDCLOSE): 
	    DestroyWindow(wnd);
	    return 0;
	case ID_POPUP_STOPSERVER:
	case ID_POPUP_STARTSERVER:
	    if(LOWORD(wParam) == ID_POPUP_STOPSERVER) Stop_MPE_Server(wnd);
    	    else if(Start_MPE_Server(wnd) != ERROR_SUCCESS) 
		MessageBox(wnd,"Cannot start Server","Error",MB_ICONERROR|MB_OK);
	    	
	    return 0;
	}
	break;
    
    case TRAY_NOTIFICATION:
	switch(lParam) {
	case WM_RBUTTONUP:
	    POINT P;	    
	    GetCursorPos(&P);
	    HandlePopupMenu(wnd,P);
	    return 0;
	}
	return 0;
    default: 
	if(Msg == reload) {
	    hIcon=LoadIcon(g_hInstance,MAKEINTRESOURCE(IDS[State]));
	    if(hIcon) {
		TaskBarSetIcon(wnd,ICON_ID,hIcon,Hints[State],NIM_ADD);
	    }
	    return 0;
	}
    }
    return DefWindowProc(wnd,Msg,wParam,lParam);

}

DWORD RegisterWindowClass() {
    
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = g_hInstance;
    wndclass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(0,IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if(!RegisterClass(&wndclass)) 
	return GetLastError();

    return ERROR_SUCCESS;    
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
    HWND hWnd;
    MSG msg;
    g_hInstance = hInstance;
    InitCommonControls();

    
    State = DISABLED;
    if(RegisterWindowClass() != ERROR_SUCCESS) 
	return 1;

    hWnd=CreateWindow(szAppName,"MPE SERVER",0,
		 CW_USEDEFAULT,CW_USEDEFAULT,
		 CW_USEDEFAULT,CW_USEDEFAULT,
		 0,0,hInstance,0);


    if(!hWnd) {
	MessageBox(0,"Could not create window","Error",MB_ICONERROR|MB_OK);
	return 1;
    }

    hDialog = 0;

    if(Start_MPE_Server(hWnd) != ERROR_SUCCESS) {
	MessageBox(0,"Could not start MPE server","Error",MB_ICONERROR|MB_OK);
	return 1;
    }
    
    while(GetMessage(&msg,hWnd,0,0) > 0) {
	if(!hDialog || !IsDialogMessage(hDialog,&msg)) { 
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	} 
    }

    Stop_MPE_Server(0);
    return 0;
}



