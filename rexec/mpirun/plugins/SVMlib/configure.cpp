#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "Configure.h"
#include "Plugin.h"


void initContext(ConfContext* Context) {
    Context->Port[0]=0;
    Context->AutoIp = BST_CHECKED;
    Context->ActIp[0]=0;
    Context->Display[0] = 0;
    Context->LogFormat = 0;
    Context->StartServer = 0;
}


static void GetGlobalData(HWND dlg,ConfContext *C) {
    int i;
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    if(!SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_GETTEXT,7,(LPARAM)C->Port)) 
	C->Port[0]=0;
    if(SendDlgItemMessage(dlg,IDC_START_SERVER,BM_GETCHECK,0,0) == BST_CHECKED) {
	C->StartServer = 1;
	C->Display[0]=0;
    } else {
	C->StartServer = 0;
	SendDlgItemMessage(dlg,IDC_EDIT_DISPLAY,WM_GETTEXT,255,(LPARAM)C->Display);
    }
    for(i=0;i<3;++i) {
	if(SendDlgItemMessage(dlg,LOG[i],BM_GETCHECK,0,0) == BST_CHECKED) {
	    C->LogFormat = i;
	    break;
	}
    }
}

static void GetLocalData(HWND dlg,ConfContext *C) {
    C->AutoIp = SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_GETCHECK,0,0);
    if(C->AutoIp == BST_CHECKED || !SendDlgItemMessage(dlg,IDC_IPCOMBO,WM_GETTEXT,20,(LPARAM)C->ActIp))
	C->ActIp[0]=0;
    
}

static void SetGlobalData(HWND dlg,ConfContext *C) {
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_SETTEXT,0,(LONG)C->Port);
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,EM_SETLIMITTEXT,5,0);
    if(!C->StartServer) {
	SendDlgItemMessage(dlg,IDC_EDIT_DISPLAY,WM_SETTEXT,0,(LONG)C->Display);
    } else {
	SendDlgItemMessage(dlg,IDC_START_SERVER,BM_SETCHECK,BST_CHECKED,0);
	EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),FALSE);
    }

    SendDlgItemMessage(dlg,LOG[C->LogFormat],BM_SETCHECK,BST_CHECKED,0);
}

static void SetLocalData(HWND dlg,ConfContext *C) {
    int index=0;
    SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_SETCHECK,C->AutoIp,0);
    if(C->AutoIp == BST_CHECKED) {
	index = -1;
	EnableWindow(GetDlgItem(dlg,IDC_IPCOMBO),FALSE);
    } else {
        if(C->ActIp[0]!=0) {
    	    index = SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_FINDSTRINGEXACT,-1,(LPARAM)C->ActIp);
	    if(index == CB_ERR) index = 0;	    
	}	
	SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_SETCURSEL,index,0);    
    }
}

void InsertAddresses(HWND dlg,HostData *Host) {
    DWORD i;
    char Addr[20];
    R_MACHINE_INFO *Info;

    if(!Host->State||!Host->State->Configuration) return;

    Info = Host->State->Configuration;
 
    for(i=0;i<Info->IP.NumEntries;++i) {
	if(ANET(Info->IP.IPS[i]) == 127 &&
	   BNET(Info->IP.IPS[i]) == 0 &&
	   CNET(Info->IP.IPS[i]) == 0 &&
	   HPART(Info->IP.IPS[i]) == 1) continue;
	sprintf(Addr,"%d.%d.%d.%d",ANET(Info->IP.IPS[i]),
        	                   BNET(Info->IP.IPS[i]),
            	                   CNET(Info->IP.IPS[i]),
				   HPART(Info->IP.IPS[i]));
	SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_ADDSTRING,0,(LPARAM)Addr);

    }
}

BOOL CALLBACK ConfigureLocalProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;
    static DWORD LastSelected;
    
    DWORD state;
    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
	InsertAddresses(dlg,(HostData*)lParam);
	SetLocalData(dlg,Context);
	LastSelected = 0;
	SetFocus(GetDlgItem(dlg,IDC_AUTOCHECK));
	SendMessage(dlg,WM_SETTEXT,0,(LPARAM)VISUAL_NAME);
	return FALSE;
	break;
    case WM_COMMAND:
	switch LOWORD(wParam) {
	case IDOK:
	    GetLocalData(dlg,Context);
	    EndDialog(dlg,IDOK);
	    return TRUE;
	case IDCANCEL:
	    EndDialog(dlg,IDCANCEL);
	    return TRUE;
	    break;
	case IDC_AUTOCHECK:
	    if(HIWORD(wParam)==BN_CLICKED) {
		state = SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_GETCHECK,0,0);
		if(state ==BST_CHECKED) {
		    LastSelected = SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_GETCURSEL,0,0);
		    SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_SETCURSEL,-1,0);
		    EnableWindow(GetDlgItem(dlg,IDC_IPCOMBO),FALSE);
		} else {
		    EnableWindow(GetDlgItem(dlg,IDC_IPCOMBO),TRUE);
		    SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_SETCURSEL,LastSelected,0);
		}
	    }
	    return TRUE;
	}
    }
    return FALSE;
}

BOOL CALLBACK ConfigureGlobalProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;
    DWORD state;

    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
	SetGlobalData(dlg,Context);
	SetFocus(GetDlgItem(dlg,IDC_EDIT_PORT));
	SendMessage(dlg,WM_SETTEXT,0,(LPARAM)VISUAL_NAME);
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
	case IDC_START_SERVER:
	    if(HIWORD(wParam)==BN_CLICKED) {
	    	state = SendDlgItemMessage(dlg,IDC_START_SERVER,BM_GETCHECK,0,0);
		if(state==BST_CHECKED) {
		    EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),FALSE);
		} else {
		    EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),TRUE);
		}
	    }
	}
    }
    return FALSE;
}

