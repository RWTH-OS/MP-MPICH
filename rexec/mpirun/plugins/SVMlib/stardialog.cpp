#include <windows.h>
#include <stdio.h>
#include "resource.h"

#define MPM_SERVER_STARTING (WM_USER+1)
#define MPM_SERVER_STOPPING (WM_USER+2)
#define MPM_SERVER_STARTED  (WM_USER+3)
#define MPM_SERVER_STOPPED  (WM_USER+4)
#define MPM_SERVER_PENDING  (WM_USER+5)

extern "C" {

extern HINSTANCE hInst;
HINSTANCE hMPElib=0;
BOOL ServerRunning = FALSE;

DWORD (*Start_MPE_Server)(HWND NotifyWindow);
VOID (*Stop_MPE_Server)(HWND NotifyWindow);



BOOL CALLBACK StartDlgProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    switch(message) {
    case WM_INITDIALOG :
	if(Start_MPE_Server) 
	    Start_MPE_Server(dlg);
	return FALSE;
    case MPM_SERVER_STARTED:
	ServerRunning = TRUE;
	EndDialog(dlg,IDOK);
	return TRUE;
    case MPM_SERVER_STOPPED:
	ServerRunning = FALSE;
	EndDialog(dlg,lParam);
	return TRUE;
    }
    return FALSE;
}



BOOL StopMPEServer() {
    if(ServerRunning && Stop_MPE_Server) {
	Stop_MPE_Server(0);
	ServerRunning = FALSE;
    }
    return TRUE;
}

BOOL StartMPEServer() {
    char ErrorMessage[255];
    int res;
    if(ServerRunning) return TRUE;
    if(!hMPElib) {
	hMPElib = LoadLibrary("mpe_server.dll");
	if(!hMPElib) {
	    sprintf(ErrorMessage,"Could not load mpe_server.dll (%d)",GetLastError());
	    MessageBox(0,ErrorMessage,"Error",MB_OK|MB_ICONERROR);
	    return FALSE;
	}

	Start_MPE_Server = (DWORD (*)(HWND))GetProcAddress(hMPElib,"Start_MPE_Server");
	if(!Start_MPE_Server) {
	    sprintf(ErrorMessage,"Could not find Start_MPE_Server (%d)",GetLastError());
	    MessageBox(0,ErrorMessage,"Error",MB_OK|MB_ICONERROR);
	    FreeLibrary(hMPElib);
	    hMPElib = 0;
	    return FALSE;
	}

	Stop_MPE_Server = (void (*)(HWND))GetProcAddress(hMPElib,"Stop_MPE_Server");
	if(!Stop_MPE_Server) {
	    sprintf(ErrorMessage,"Could not find Stop_MPE_Server (%d)",GetLastError());
	    MessageBox(0,ErrorMessage,"Error",MB_OK|MB_ICONERROR);
	    FreeLibrary(hMPElib);
	    hMPElib = 0;
	    return FALSE;
	}
    }
    res = DialogBox(hInst,MAKEINTRESOURCE(IDD_START_DIALOG),0,StartDlgProc);
    if(res == IDOK) return TRUE;
    if(res != -1) SetLastError(res);
    return FALSE;

}

} 