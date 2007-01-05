#include <windows.h>
#include <Commctrl.h>
#include <cpl.h>
#include <stdio.h>
#include "rcluma_config.h"
#include "resource.h"


#define STARTSTRING "&Start server"
#define STOPSTRING "&Stop server"
#define KEY "System\\CurrentControlSet\\Services\\rcluma\\Parameters"
#define SERVICE_CONTROL_RECONFIG 128


#ifdef __cplusplus 
extern "C" {            /* Assume C declarations for C++ */ 
#endif/* __cplusplus */ 

static HINSTANCE hinst;
static CONFIGURE_T Context;
static SC_HANDLE hService;


void CheckState(HWND dlg,CONFIGURE_T *C) {
    SERVICE_STATUS status;
    if(C->OpenResult != ERROR_SUCCESS)
		return;
    if(!QueryServiceStatus(C->hService,&status)) 
		return;

    if(status.dwCurrentState == SERVICE_STOPPED) {
        C->Running = FALSE;
		SetTimer(dlg,1,3000,NULL);
    } 
	else 
		if(status.dwCurrentState == SERVICE_RUNNING) {
			C->Running = TRUE;
			SetTimer(dlg,1,3000,NULL);
		} 
		else 
		if(status.dwWaitHint>0)
			SetTimer(dlg,1,status.dwWaitHint,NULL);
}

LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           error,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
		if(dwSize>16) 
			sprintf(lpszBuf,TEXT("Error: %d"),error);
        else 
			lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        sprintf( lpszBuf, TEXT("%s (%d)"), lpszTemp, error );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}


DWORD GetServiceHandle(SC_HANDLE *schService) {
        
    SC_HANDLE   schSCManager;
    DWORD err = ERROR_SUCCESS;
    
    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_CONNECT|
			SC_MANAGER_ENUMERATE_SERVICE|
			SC_MANAGER_QUERY_LOCK_STATUS    // access required
                        );
    
    if ( schSCManager != NULL ) {
        *schService = OpenService(schSCManager,"rcluma",
	    SERVICE_QUERY_STATUS|
	    SERVICE_START|
	    SERVICE_STOP|
	    SERVICE_USER_DEFINED_CONTROL);
		if(*schService == NULL) {
			err = GetLastError();
		}
		CloseServiceHandle(schSCManager);
    } 
	else {
		err = GetLastError();
    }
    return err;
}



DWORD StartStopServer(HWND dlg,CONFIGURE_T *C) {
    DWORD FINAL,lastCheck,loop=0,res=ERROR_SUCCESS;
    SERVICE_STATUS status;
    if(C->OpenResult != ERROR_SUCCESS)
		return C->OpenResult;

    if(C->Running) {
		if(!ControlService(C->hService,SERVICE_CONTROL_STOP,&status)) 
			res = GetLastError();
		FINAL = SERVICE_STOPPED;	
    } 
	else {
		if(!StartService(C->hService,0,NULL)) 
			res = GetLastError();
		FINAL = SERVICE_RUNNING;
    }

 /*
    if(!QueryServiceStatus(C->hService,&status)) 
	return GetLastError();

    lastCheck = status.dwCheckPoint;
    do {
	if(status.dwCurrentState != FINAL && res == ERROR_SUCCESS) {
	    Sleep(status.dwWaitHint); 
	    if(!QueryServiceStatus(C->hService,&status)) {
		res = GetLastError();
		break;
	    }
    	    if(lastCheck == status.dwCheckPoint) ++loop;
	}
    } while(loop <=5 && status.dwCurrentState != FINAL);

    if(status.dwCurrentState == SERVICE_STOPPED)
        C->Running = FALSE;
    else
        C->Running = TRUE;
    */
    PostMessage(dlg,WM_TIMER,1,0);
    return res;
}

void InitContext(CONFIGURE_T *C) {
    SERVICE_STATUS status;
    HKEY hKey;
    LONG lError;
    DWORD ValType,ValSize;

    C->Changed = FALSE;
    C->OpenResult = GetServiceHandle(&C->hService);
    if(C->OpenResult == ERROR_SUCCESS) {
		QueryServiceStatus(C->hService,&status);
		if(status.dwCurrentState == SERVICE_STOPPED)
			C->Running = FALSE;
		else
			C->Running = TRUE;
    } 
	else 
		C->Running = FALSE;
    
    
    lError=RegOpenKeyEx(HKEY_LOCAL_MACHINE,KEY,0,KEY_READ,&hKey);
    
    if(lError!=ERROR_SUCCESS) {
		C->LogFile = (char*)malloc(sizeof(char)*16);
		strcpy(C->LogFile,"C:\\rcluma.log");
		C->Logging = FALSE;
		C->NumThreads = 5;
		return;
    }
    
    ValSize=0;
    lError=RegQueryValueEx(hKey,"LOG_FILE",0,&ValType,(LPBYTE) C->LogFile,&ValSize);
    if(lError !=ERROR_SUCCESS && lError != ERROR_MORE_DATA) {
		C->LogFile = (char*)malloc(sizeof(char)*18);
		strcpy(C->LogFile,"C:\\rcluma.log");
    } 
	else {
		C->LogFile = (char*)malloc(sizeof(char)*(ValSize+1));
		lError=RegQueryValueEx(hKey,"LOG_FILE",0,&ValType,(LPBYTE) C->LogFile,&ValSize);
		if(lError !=ERROR_SUCCESS) {
			C->LogFile[0]=0;
		}
    }
    ValSize = sizeof(BOOL);
    lError=RegQueryValueEx(hKey,"LOGGING",0,&ValType,(LPBYTE) &C->Logging,&ValSize);
    if(lError !=ERROR_SUCCESS) {
		C->Logging = FALSE;
    }
	ValSize = sizeof(BOOL);
    lError=RegQueryValueEx(hKey,"NOUSER",0,&ValType,(LPBYTE) &C->NoUser,&ValSize);
    if(lError !=ERROR_SUCCESS) {
		C->NoUser = FALSE;
    }
    ValSize = sizeof(DWORD);
    lError=RegQueryValueEx(hKey,"NUM_THREADS",0,&ValType,(LPBYTE) &C->NumThreads,&ValSize);
    if(lError !=ERROR_SUCCESS) {
		C->NumThreads = 5;
    }
    
    RegCloseKey(hKey);
} 

/* settings are stored in registry, rclumad service will check registry to retrieve settings */

DWORD StoreContext(HWND dlg,CONFIGURE_T *C) {
    // Store the values into the registry...
    HKEY hKey;
    LONG lError;
    DWORD disp;
    SERVICE_STATUS status;
    char error[256];
    
    if(!C->Changed) 
		return ERROR_SUCCESS;
    lError=RegCreateKeyEx(HKEY_LOCAL_MACHINE,KEY,0,"",
	REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,0,&hKey,&disp);
    
    if(lError!=ERROR_SUCCESS) {
		return lError;
    }
    if(C->LogFile)
		lError=RegSetValueEx(hKey,"LOG_FILE",0,REG_EXPAND_SZ,(PBYTE) C->LogFile,strlen(C->LogFile)+1);
    else
		lError=RegSetValueEx(hKey,"LOG_FILE",0,REG_EXPAND_SZ,(PBYTE) "",1);
    lError=RegSetValueEx(hKey,"LOGGING",0,REG_DWORD,(PBYTE) &C->Logging,sizeof(C->Logging));
	lError=RegSetValueEx(hKey,"NOUSER",0,REG_DWORD,(PBYTE) &C->NoUser,sizeof(C->NoUser));
    lError=RegSetValueEx(hKey,"NUM_THREADS",0,REG_DWORD,(PBYTE) &C->NumThreads,sizeof(C->NumThreads));
    
    RegCloseKey(hKey);
    
	// send a message to the service; service will read config information from registry
    if(Context.OpenResult == ERROR_SUCCESS && Context.Running) {
		if(!ControlService(C->hService,SERVICE_CONTROL_RECONFIG,&status)) {
			lError = GetLastError();
			MessageBox(dlg,GetLastErrorText(lError,error,256),"ControlService",MB_ICONWARNING|MB_OK);
		}
    }

    return lError;
}

void SetDialogData(HWND dlg,CONFIGURE_T *C) {
    char Threads[16];
    SendDlgItemMessage(dlg,IDC_LOGGING,BM_SETCHECK,C->Logging,0);
	SendDlgItemMessage(dlg,IDC_NOUSER,BM_SETCHECK,C->NoUser,0);
    SendDlgItemMessage(dlg,IDC_EDIT_THREADS,EM_LIMITTEXT,3,0);
    SendDlgItemMessage(dlg,IDC_SPIN1,UDM_SETRANGE,0,MAKELONG(100, 1));
    sprintf(Threads,"%d",C->NumThreads);
    SendDlgItemMessage(dlg,IDC_EDIT_THREADS,WM_SETTEXT,0,(LONG)Threads);
    if(C->OpenResult != ERROR_SUCCESS) 
		EnableWindow(GetDlgItem(dlg,IDC_START),FALSE);
    if(!C->Running) 
		SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STARTSTRING);
    else 
		SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STOPSTRING);
    
    /* enable or disable edit of logfile */
    if(!C->Logging) {
		EnableWindow(GetDlgItem(dlg,IDC_EDIT_FILE),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_LOG_LABEL),FALSE);
    } 
	else {
		EnableWindow(GetDlgItem(dlg,IDC_EDIT_FILE),TRUE);
		EnableWindow(GetDlgItem(dlg,IDC_LOG_LABEL),TRUE);
    }
    if(C->LogFile)
		SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_SETTEXT,0,(LONG)C->LogFile);
    C->Changed = FALSE;
    EnableWindow(GetDlgItem(dlg,IDC_APPLY),FALSE);
}

void GetDialogData(HWND dlg,CONFIGURE_T *C) {
    char Threads[16];
    DWORD len;
    C->Logging = ( SendDlgItemMessage(dlg,IDC_LOGGING,BM_GETCHECK,0,0) == BST_CHECKED);
	C->NoUser = ( SendDlgItemMessage(dlg,IDC_NOUSER,BM_GETCHECK,0,0) == BST_CHECKED);
    if(SendDlgItemMessage(dlg,IDC_EDIT_THREADS,WM_GETTEXT,16,(LPARAM)Threads)) 
		C->NumThreads = atoi(Threads);
    else C->NumThreads = 1;
    len = SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_GETTEXTLENGTH,0,0);
    if(C->LogFile) 
		free(C->LogFile);
    C->LogFile = (char*)malloc(sizeof(char)*(len+1));
    SendDlgItemMessage(dlg,IDC_EDIT_FILE,WM_GETTEXT,len+1,(LPARAM)C->LogFile);     
}

void FreeResources(CONFIGURE_T *C) {
    if(Context.OpenResult == ERROR_SUCCESS) {	
		CloseServiceHandle(C->hService);
    }

    if(C->LogFile) 
		free(C->LogFile);
    C->LogFile = 0;
}

BOOL CALLBACK DialogFunction(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam) {
    
    DWORD State,res;
    char error[256];
    switch(message) {
      case WM_TIMER:
	    CheckState(dlg,&Context);
		if(!Context.Running) 
			SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STARTSTRING);
	    else 
			SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STOPSTRING);
	  break;
      case WM_INITDIALOG :
		InitContext(&Context);
		SetDialogData(dlg,&Context);
		SetFocus(GetDlgItem(dlg,IDC_EDIT_THREADS));
		if(Context.OpenResult != ERROR_SUCCESS) {
	      MessageBox(dlg,GetLastErrorText(Context.OpenResult,error,256),
	               "OpenService",MB_ICONERROR|MB_OK);
		} 
		SetTimer(dlg,1,3000,0);
		return FALSE;
	  break;
      case WM_COMMAND:
		switch LOWORD(wParam) {
		  case IDOK:
			KillTimer(dlg,1);
			GetDialogData(dlg,&Context);
			StoreContext(dlg,&Context);
			FreeResources(&Context);
			EndDialog(dlg,IDOK);
			return TRUE;
		  case IDCANCEL:
			KillTimer(dlg,1);
			FreeResources(&Context);
			EndDialog(dlg,IDCANCEL);
			return TRUE;
		  break;
	      case IDC_LOGGING:
	        State = SendDlgItemMessage(dlg,IDC_LOGGING,BM_GETCHECK,0,0);
			EnableWindow(GetDlgItem(dlg,IDC_EDIT_FILE),State);
			EnableWindow(GetDlgItem(dlg,IDC_LOG_LABEL),State);
			Context.Changed = TRUE;
			EnableWindow(GetDlgItem(dlg,IDC_APPLY),TRUE);
	      break;
		  case IDC_NOUSER:
	        State = SendDlgItemMessage(dlg,IDC_NOUSER,BM_GETCHECK,0,0);
			Context.Changed = TRUE;
			EnableWindow(GetDlgItem(dlg,IDC_APPLY),TRUE);
	      break;
		  case IDC_APPLY:
			GetDialogData(dlg,&Context);
			StoreContext(dlg,&Context);
	      break;
	      case IDC_START:
			res=StartStopServer(dlg,&Context);
			if(res != ERROR_SUCCESS) {
				MessageBox(dlg,GetLastErrorText(res,error,256),
	               "Start/Stop server",MB_ICONERROR|MB_OK);
			}
	    /*
            if(!Context.Running) 
		SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STARTSTRING);
	    else 
		SendDlgItemMessage(dlg,IDC_START,WM_SETTEXT,0,(LONG)STOPSTRING);
	    */
		  break;
		}
	
		if(HIWORD(wParam) == EN_CHANGE) {
			Context.Changed = TRUE;
			EnableWindow(GetDlgItem(dlg,IDC_APPLY),TRUE);
		}
	  break;
      default: break;
    }//switch(message)
    return FALSE;
}


/*
	CPlApplet: Application Entry-Point Function
	This function receives requests in the form of Control Panel (CPL) messages and then 
	carries out the requested work - initializing the application, displaying and managing 
	the dialog box(es), and closing the application. 

*/


LONG APIENTRY CPlApplet(HWND hwndCPL, UINT uMsg,LPARAM lParam1, LPARAM lParam2) 
{ 
    int i; 
    LPCPLINFO lpCPlInfo; 
    i = (int) lParam1; 
 
    switch (uMsg) { 
        case CPL_INIT:      // first message, sent once 
            hinst = GetModuleHandle("rcluma.cpl");
			
            return TRUE; 
 
        case CPL_GETCOUNT:  // second message, sent once 
            return 1; 
            break; 
 
        case CPL_INQUIRE: // third message, sent once per application 
            
			lpCPlInfo = (LPCPLINFO) lParam2; 
            lpCPlInfo->lData = 0; 
			lpCPlInfo->idIcon = IDI_ICON1;
            lpCPlInfo->idName = IDS_NAME;
            lpCPlInfo->idInfo = IDS_DESCRIPTION;
            break; 

        case CPL_DBLCLK:    // application icon double-clicked 	    
            DialogBox(hinst, 
                MAKEINTRESOURCE(IDD_DIALOG1), 
                hwndCPL, DialogFunction); 
            break; 
 
        case CPL_STOP:      // sent once per application before CPL_EXIT 
            break; 
 
        case CPL_EXIT:    // sent once before FreeLibrary is called 	    
        default: 
            break; 
    } 
    return 0; 
} 

#ifdef __cplusplus 
}          
#endif/* __cplusplus */ 