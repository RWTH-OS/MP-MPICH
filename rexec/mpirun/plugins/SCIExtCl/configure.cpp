#include <windows.h>
#include "dialog2.h"
#include "resource.h"
#include "Plugin.h"
#include "configure.h"


void initContext(ConfContext* Context) {
	char szHelp[256];
	itoa(CH_SMI_BASE_PORT, szHelp, 10);
    strcpy(Context->Port,szHelp);
    Context->Debug = FALSE;
    Context->Verbose = 0;
#ifdef MPI
    Context->Conffile[0]=0;
    Context->Display[0] = 0;
    Context->LogFormat = 0;
    Context->StartServer = 0;
#endif

}


static void PopFileInitialize(HWND hwnd,OPENFILENAME &ofn)
{
	static TCHAR szFilter[] = TEXT("Configfiles (*.conf)\0*.conf\0")\
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

static BOOL PopFileOpenDlg(HWND hwnd,PTSTR pstrFileName,PTSTR pstrTitleName) 
{	static OPENFILENAME ofn;
	PopFileInitialize(hwnd,ofn);
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrTitle = pstrTitleName;
	ofn.Flags |= OFN_HIDEREADONLY;

	return(GetOpenFileName(&ofn));
}

static void GetLocalData(HWND dlg,ConfContext *C) {
	char szHelp[256];
	itoa(CH_SMI_BASE_PORT, szHelp, 10);

    C->Debug = (SendDlgItemMessage(dlg,IDC_CHECK_DEBUG,BM_GETCHECK,0,0) == BST_CHECKED);
    C->Verbose = (SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_GETSTATE,0,0) == BST_CHECKED);
    if(!SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_GETTEXT,5,(LPARAM)C->Port)) 
	strcpy(C->Port,szHelp);
#ifdef MPI
    if(!SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_GETTEXT,255,(LPARAM)C->Conffile))
	C->Conffile[0]=0;
#endif	
}

static void SetLocalData(HWND dlg,ConfContext *C) {
    SendDlgItemMessage(dlg,IDC_CHECK_DEBUG,BM_SETCHECK,C->Debug,0);
    SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_SETCHECK,C->Verbose,0);

#ifdef MPI
    SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_SETTEXT,0,(LPARAM)C->Conffile);
#endif
}


#ifdef MPI
static void GetGlobalData(HWND dlg,ConfContext *C) {
    int i;
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    if(!SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_GETTEXT,7,(LPARAM)C->Port)) 
	C->Port[0]=0;
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
    if(!SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_GETTEXT,255,(LPARAM)C->Conffile))
	C->Conffile[0]=0;
    C->Debug = (SendDlgItemMessage(dlg,IDC_CHECK_DEBUG,BM_GETCHECK,0,0) == BST_CHECKED);
    C->Verbose = (SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_GETSTATE,0,0) == BST_CHECKED);

}


static void SetGlobalData(HWND dlg,ConfContext *C) {
    const int LOG[3] = {IDC_SLOG,IDC_CLOG,IDC_ALOG};
    
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,WM_SETTEXT,0,(LONG)C->Port);
    if(!C->StartServer) {
	SendDlgItemMessage(dlg,IDC_EDIT_DISPLAY,WM_SETTEXT,0,(LONG)C->Display);
    } else {
	SendDlgItemMessage(dlg,IDC_START_SERVER,BM_SETCHECK,BST_CHECKED,0);
	EnableWindow(GetDlgItem(dlg,IDC_EDIT_DISPLAY),FALSE);
    }

    SendDlgItemMessage(dlg,LOG[C->LogFormat],BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_SETTEXT,0,(LPARAM)C->Conffile);
    SendDlgItemMessage(dlg,IDC_EDIT_PORT,EM_SETLIMITTEXT,5,0);
    SendDlgItemMessage(dlg,IDC_CHECK_DEBUG,BM_SETCHECK,C->Debug,0);
    SendDlgItemMessage(dlg,IDC_CHECK_VERBOSE,BM_SETCHECK,C->Verbose,0);
}
#endif


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    static ConfContext *Context;
    char szFile[256];
    DWORD state;

    
    switch(message) {
    case WM_INITDIALOG :
	Context = (ConfContext*)((HostData*)lParam)->ProcData->Context;
#ifdef MPI
	if(Context->Global)
	    SetGlobalData(dlg,Context);
	else
#endif
	    SetLocalData(dlg,Context);
	//SetFocus(GetDlgItem(dlg,IDC_CHECK_VERBOSE));
	return FALSE;
	break;
    case WM_COMMAND:
	switch LOWORD(wParam) {
	case IDOK:
#ifdef MPI
	if(Context->Global)
	    GetGlobalData(dlg,Context);
	else
#endif
	    GetLocalData(dlg,Context);
	    EndDialog(dlg,IDOK);
	    return TRUE;
	case IDCANCEL:
	    EndDialog(dlg,IDCANCEL);
	    return TRUE;
	    break;
	case IDC_BUTTON_SELFILE:
	    if(!SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_GETTEXT,255,(LPARAM)szFile)) 
		strcpy(szFile,"ch_smi.conf");
	    if(PopFileOpenDlg(dlg,szFile,"SELECT MPICH configuration file"))
		SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_SETTEXT,0,(LPARAM)szFile);
	    return TRUE;
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

