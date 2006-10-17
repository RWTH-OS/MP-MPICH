#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "configure.h"
#include "plugin.h"

ConfContext *actContext;

void initContext(ConfContext* Context) {
    int port;
    Context->Debug = FALSE;
    Context->Verbose = FALSE;
    port = rand() % 63535 + 2000;
    sprintf(Context->Port,"%d",port);
}

static void GetDlgData(HWND dlg, ConfContext* C) {
    C->Verbose = (SendDlgItemMessage(dlg,IDC_Verbose,BM_GETCHECK,0,0) == BST_CHECKED);
    C->Debug = (SendDlgItemMessage(dlg,IDC_Debug,BM_GETSTATE,0,0) == BST_CHECKED);
    if(C->Global) {
	if(!SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_GETTEXT,7,(LPARAM)C->Port)) 
	    C->Port[0]=0;
    }
}

static void SetDlgData(HWND dlg, ConfContext* C) {
    int index=0;
    SendDlgItemMessage(dlg,IDC_Debug,BM_SETCHECK,C->Debug,0);
    SendDlgItemMessage(dlg,IDC_Verbose,BM_SETCHECK,C->Verbose,0);
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_SETTEXT,0,(LONG)C->Port);

    if(!C->Global) {
	EnableWindow(GetDlgItem(dlg,IDC_EDIT_PORT),FALSE);
	EnableWindow(GetDlgItem(dlg,IDC_PORT_LABEL),FALSE);
    }    
}


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;
    
    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
	SetDlgData(dlg,Context);
	SetFocus(GetDlgItem(dlg,IDC_Debug));
	return FALSE;
	break;
    case WM_COMMAND:
	switch LOWORD(wParam) {
	case IDOK:
	    GetDlgData(dlg,Context);
	    EndDialog(dlg,IDOK);
	    return TRUE;
	case IDCANCEL:
	    EndDialog(dlg,IDCANCEL);
	    return TRUE;
	    break;
	}
    }
    return FALSE;
}

