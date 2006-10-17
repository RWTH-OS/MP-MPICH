#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "configure.h"
#include "plugin.h"

ConfContext *actContext;

void initContext(ConfContext* Context) {
    Context->Singlethreaded = FALSE;
    Context->Polling = FALSE;
    Context->Verbose = FALSE;
    strcpy(Context->Port,"23143");
    Context->AutoIp=BST_CHECKED;
    Context->ActIp[0]=0;
}

static void GetDlgData(HWND dlg, ConfContext* C) {
    C->Polling = (SendDlgItemMessage(dlg,IDC_POLLING,BM_GETCHECK,0,0) == BST_CHECKED);
    C->Singlethreaded = (SendDlgItemMessage(dlg,IDC_SINGLE,BM_GETSTATE,0,0) == BST_CHECKED);
    C->Verbose = (SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_GETSTATE,0,0) == BST_CHECKED);
    if(!SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_GETTEXT,7,(LPARAM)C->Port)) 
	C->Port[0]=0;
    if(!C->Global) {
	C->AutoIp = SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_GETCHECK,0,0);
	if(C->AutoIp == BST_CHECKED || !SendDlgItemMessage(dlg,IDC_IPCOMBO,WM_GETTEXT,20,(LPARAM)C->ActIp))
	    C->ActIp[0]=0;
    }
    
}

static void SetDlgData(HWND dlg, ConfContext* C) {
    int index=0;
    SendDlgItemMessage(dlg,IDC_SINGLE,BM_SETCHECK,C->Singlethreaded,0);
    SendDlgItemMessage(dlg,IDC_POLLING,BM_SETCHECK,C->Polling,0);
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_SETTEXT,0,(LONG)C->Port);
    SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_SETCHECK,C->Verbose,0);
    
    if(!C->Global) {
	SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_SETCHECK,C->AutoIp,0);
	EnableWindow(GetDlgItem(dlg,IDC_AUTOCHECK),TRUE);	
	EnableWindow(GetDlgItem(dlg,IDC_EDIT_PORT),FALSE);
	EnableWindow(GetDlgItem(dlg,IDC_PORT_LABEL),FALSE);
    } else {
	SendDlgItemMessage(dlg,IDC_AUTOCHECK,BM_SETCHECK,BST_UNCHECKED,0);
	EnableWindow(GetDlgItem(dlg,IDC_AUTOCHECK),FALSE);	
    }
    if(C->AutoIp == BST_CHECKED || C->Global)
	EnableWindow(GetDlgItem(dlg,IDC_IPCOMBO),FALSE);
    else {
	EnableWindow(GetDlgItem(dlg,IDC_IPCOMBO),TRUE);
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
	sprintf(Addr,"%d.%d.%d.%d",ANET(Info->IP.IPS[i]),
        	                   BNET(Info->IP.IPS[i]),
            	                   CNET(Info->IP.IPS[i]),
				   HPART(Info->IP.IPS[i]));
	SendDlgItemMessage(dlg,IDC_IPCOMBO,CB_ADDSTRING,0,(LPARAM)Addr);

    }
}

BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;
    static DWORD LastSelected;

    DWORD state;
    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
	InsertAddresses(dlg,(HostData*)lParam);
	SetDlgData(dlg,Context);
	SetFocus(GetDlgItem(dlg,IDC_SINGLE));
	LastSelected = 0;
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
	    break;
	}

    }
    return FALSE;
}

