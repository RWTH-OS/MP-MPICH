#include <windows.h>
#include <stdio.h>
#include "Plugin.h"
#include "Configure.h"
#include "Resource.h"
#include "startserver.h"
#include "args.h"

#ifdef __cplusplus
extern "C" {
#endif


EnvFuncs EProcs = {0,0,0};
HINSTANCE hInst;

extern BOOL CALLBACK StartDlgProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
extern DWORD (*Start_MPE_Server)(HWND NotifyWindow);
extern VOID (*Stop_MPE_Server)(HWND NotifyWindow);


void WINAPI _PlgDescription(struct _PlgDesc* pDesc);
void WINAPI SVMlibDetach(void);
BOOL WINAPI SVMlibCreateComandline(HostData** sppHostAll,
				   DWORD SizeAll, 
				   HostData** sppHostSelected, 
				   DWORD *pSizeSelected,
				   HostData *globalData);


void WINAPI SVMlibConfigure(HWND,HWND,HostData *,BOOL);
void WINAPI SVMlibParse(int*,char**,HostData *);

//HostData actHost;

void WINAPI _PlgDescription(struct _PlgDesc* pDesc) {
	pDesc->PluginVersionNumber = VersionPlugin_h;
	strcpy(pDesc->VisualName,VISUAL_NAME);
	pDesc->DllName[0]=0;
	pDesc->Attach=0;
	pDesc->Detach=SVMlibDetach;
	pDesc->NewData=0;
	pDesc->SelectedChange=0;
	pDesc->DlgClose=SVMlibCreateComandline;
	pDesc->RefreshEnd=0;
	pDesc->Convert=0;
	pDesc->Configure=SVMlibConfigure;
	pDesc->ParseCommandline = SVMlibParse;
}

void WINAPI SVMlibDetach(void) {
    StopMPEServer();
}

static char *CreateString(DWORD Count,const char *master, const char *port,const char *Nic) {
    static char Commandline[2048];

    Commandline[0]=0;
    
    if(Count) {
	sprintf(Commandline,"-n %d",Count);
    }

    if(master && master[0]) {
	strcat(Commandline," -m ");
	strcat(Commandline,master);
    }

    if(port) {
	strcat(Commandline," -p ");
	strcat(Commandline,port);
    }
    if(Nic) {
	strcat(Commandline," -b ");
	strcat(Commandline,Nic);
    }

    strcat(Commandline," -- ");

    return Commandline;


}
 


BOOL WINAPI SVMlibCreateComandline(HostData** sppHostAll,DWORD SizeAll, 
				   HostData** sppHostSelected, 
				   DWORD *pSizeSelected,
				   HostData *globalData)  
{
    DWORD i;
    char Name[128],*rootport,*roothost,*Nic,*Display,*LogFormat;
    DWORD size=128;
    HostData *MyHost;
    BOOL Found=FALSE;
    char *Cml;
    ConfContext *actContext;
    char *Formats[] = {"SLOG","CLOG","ALOG"};
    char ErrorMessage[255];

    if (*pSizeSelected==0) return FALSE;

    GetComputerName(Name,&size);
    
    for (i=0;i<(*pSizeSelected);i++) {
	if(!stricmp(Name,sppHostSelected[i]->Name)) {
	    MyHost=sppHostSelected[i];
	    sppHostSelected[i]=sppHostSelected[0];
	    sppHostSelected[0]=MyHost;
	    break;
	}
    }
    

    actContext = (ConfContext*)EProcs.GetContext(globalData,0);
    if(actContext) {
      if(actContext->StartServer) {
	   if(!StartMPEServer()) {
		sprintf(ErrorMessage,"MPE Startup failed (%x)",GetLastError());
		MessageBox(0,ErrorMessage,"Error",MB_OK|MB_ICONERROR);
	    }
	   strcpy(actContext->Display,Name);
	  } else StopMPEServer();
	  rootport = actContext->Port;
	  if(!*rootport) rootport = 0;
	  Display = actContext->Display;
	  LogFormat = Formats[actContext->LogFormat];
    } else {
	  rootport = 0;
	  Display = 0;
	  LogFormat = Formats[0];
    }

    

    actContext = (ConfContext*)EProcs.GetContext(sppHostSelected[0],0);
    if(!actContext || actContext->AutoIp == BST_CHECKED) {
	roothost = (char*)&(sppHostSelected[0]->State->Configuration->IP.Hostname_ip[0]);
	Nic = 0;
    } else {
	roothost = actContext->ActIp;
	Nic = actContext->ActIp;
	if(!*Nic) Nic = 0;
    }
    
    /*
    EProcs.SetCommandline(sppHostSelected[0],
	CreateString(*pSizeSelected,
		    (char*)&sppHostSelected[0]->State->Configuration->IP.Hostname_ip[0],
	            rootport,Nic
	)
    );
    */

    EProcs.SetCommandline(sppHostSelected[0],CreateString(*pSizeSelected,0,rootport,Nic));

    if(Display && Display[0]) EProcs.PutString(sppHostSelected[0],"DISPLAY",Display);
    EProcs.PutString(sppHostSelected[0],"MPE_LOG_FORMAT",LogFormat);

    for (i=1;i<(*pSizeSelected);i++) {
	actContext = (ConfContext*)EProcs.GetContext(sppHostSelected[i],0);
	if(!actContext || actContext->AutoIp == BST_CHECKED)
	    Nic = 0;
	else 
	    Nic = actContext->ActIp;

	if(!stricmp(sppHostSelected[0]->Name,sppHostSelected[i]->Name))
	 // Cml = CreateString(*pSizeSelected,"127.0.0.1",rootport,Nic);
	  Cml = CreateString(0,"127.0.0.1",rootport,Nic);
	else 
	    //Cml = CreateString(*pSizeSelected,roothost,rootport,Nic);
	    Cml = CreateString(0,roothost,rootport,Nic);
	EProcs.SetCommandline(sppHostSelected[i],Cml);
        if(Display && Display[0])
    	    EProcs.PutString(sppHostSelected[i],"DISPLAY",Display);
	EProcs.PutString(sppHostSelected[i],"MPE_LOG_FORMAT",LogFormat);
    }
    

    if (*pSizeSelected>0) 
	return(TRUE);
    else
	return(FALSE);
}

void WINAPI SVMlibParse(int* argc,char **argv,HostData *actHost) {
    char *Port=0,*Display=0;
    ConfContext *Context=0;
    
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

    Context->StartServer = IsArgPresent(argc,argv,"-mpe");

    if(Context->StartServer) {
 	DWORD size = 256;
	GetComputerName(Context->Display,&size);
    }

    if(IsArgPresent(argc,argv,"-alog"))
	Context->LogFormat = 2;

    if(IsArgPresent(argc,argv,"-clog"))
	Context->LogFormat = 1;

    if(IsArgPresent(argc,argv,"-slog"))
	Context->LogFormat = 0;

    GetStringArg(argc,argv,"-display",&Display);
    GetStringArg(argc,argv,"-port",&Port);
    
    if(Port)
	strcpy(Context->Port,Port);

    if(Display)
	strcpy(Context->Display,Display);
}


void WINAPI SVMlibConfigure(HWND hApplication,HWND hDialog,HostData *actHost,BOOL global) {
    char txt[255];
    int res;
    ConfContext *Context;
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

    Context->Global = global;
    if(global)
	res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_DIALOG1),hDialog,
	                         ConfigureGlobalProc,(LPARAM)actHost);    
    else 
	res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_DIALOG2),hDialog,
	                         ConfigureLocalProc,(LPARAM)actHost);    
    if(res<0) {
	sprintf(txt,"Error %d",GetLastError());
	MessageBox(0,txt,"ERROR",MB_ICONERROR|MB_OK);
    }
}


BOOL __stdcall DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID) {
    HMODULE hProc = GetModuleHandle(NULL);/* get handle to calling exe-file */
    hInst = hinstDLL;
    if(fdwReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(hInst);
	/* 
	the following functions are defined in the exe-file using this dll, e.g. RexecShell.exe
	*/
	EProcs.PutString = (void (*)(HostData*,char*,char*)) GetProcAddress(hProc,"_PutEnvString");
	if(!EProcs.PutString)
	    EProcs.PutString = (void (*)(HostData*,char*,char*)) GetProcAddress(hProc,"PutEnvString");

	EProcs.GetString = (char *(*)(HostData*,char*,char*,DWORD *)) GetProcAddress(hProc,"_GetEnvString");
	if(!EProcs.GetString)
	    EProcs.GetString = (char *(*)(HostData*,char*,char*,DWORD *)) GetProcAddress(hProc,"GetEnvString");
	
	EProcs.SetCommandline = (void (*)(HostData*,char*))GetProcAddress(hProc,"_SetCommandline");
	if(!EProcs.SetCommandline)
	    EProcs.SetCommandline = (void (*)(HostData*,char*))GetProcAddress(hProc,"SetCommandline");

	EProcs.GetContext = (void* (*)(HostData*,DWORD))GetProcAddress(hProc,"_GetContext");
	if(!EProcs.GetContext)
	    EProcs.GetContext = (void* (*)(HostData*,DWORD))GetProcAddress(hProc,"GetContext");

	if(!EProcs.SetCommandline || !EProcs.GetString || !EProcs.PutString || !EProcs.GetContext) 
	    return FALSE;
	
    } else if(fdwReason == DLL_PROCESS_DETACH) {
	StopMPEServer();
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif
