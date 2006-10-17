#include <windows.h>
#include <Commctrl.h>
#include "resource.h"
#include "Plugin.h"
#include "configure.h"


void initContext(ConfContext* Context) {
    strcpy(Context->Num,"2");
    Context->Display[0] = 0;
    Context->LogFormat = 0;
    Context->StartServer = 0;
}


static void GetGlobalData(HWND dlg,ConfContext *C) {
    int i;
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    if(!SendDlgItemMessage(dlg,IDC_EDIT_NUM,WM_GETTEXT,2,(LPARAM)C->Num)) 
	strcpy(C->Num,"1");
    if(SendDlgItemMessage(dlg,IDC_START_SERVER,BM_GETCHECK,0,0) == BST_CHECKED) {
	i=256;
	C->StartServer = 1;
	C->Display[0]=0;
    } else
	SendDlgItemMessage(dlg,IDC_EDIT_DISPLAY,WM_GETTEXT,255,(LPARAM)C->Display);
    for(i=0;i<3;++i) {
	if(SendDlgItemMessage(dlg,LOG[i],BM_GETCHECK,0,0) == BST_CHECKED) {
	    C->LogFormat = i;
	    break;
	}
    }
}


static void SetGlobalData(HWND dlg,ConfContext *C) {
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    
    SendDlgItemMessage(dlg,IDC_EDIT_NUM,WM_SETTEXT,0,(LONG)C->Num);
    if(!C->StartServer) {
	SendDlgItemMessage(dlg,IDC_EDIT_DISPLAY,WM_SETTEXT,0,(LONG)C->Display);
    } else {
	SendDlgItemMessage(dlg,IDC_START_SERVER,BM_SETCHECK,BST_CHECKED,0);
	EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),FALSE);
    }

    SendDlgItemMessage(dlg,LOG[C->LogFormat],BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(dlg,IDC_EDIT_NUM,EM_LIMITTEXT,2,0);
    SendDlgItemMessage(dlg,IDC_SPIN,UDM_SETRANGE,0,MAKELONG(32, 1));
}



BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;

    DWORD state;
    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
	SetGlobalData(dlg,Context);
	return FALSE;
	break;
    case WM_COMMAND:
	switch LOWORD(wParam) {
	case IDOK:
	    GetGlobalData(dlg,Context);
	    EndDialog(dlg,IDOK);
	    return TRUE;
	case IDCANCEL:
	    EndDialog(dlg,IDCANCEL);
	    return TRUE;
	    break;
	case IDC_START_SERVER:
	    if(HIWORD(wParam)==BN_CLICKED) {
	    	state = SendDlgItemMessage(dlg,IDC_START_SERVER,BM_GETCHECK,0,0);
		if(state==BST_CHECKED) {
		    EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),FALSE);
		} else {
		    EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),TRUE);
		}
		return TRUE;
	    }
	}

    }
    return FALSE;
}

