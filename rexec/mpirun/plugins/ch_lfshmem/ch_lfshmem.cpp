#include <windows.h>
#include <stdio.h>
#include <Commctrl.h>
#include "Plugin.h"
#include "configure.h"
#include "resource.h"
#include <startserver.h>
#include <args.h>

#ifdef __cplusplus
extern "C" {
#endif

HINSTANCE hInst;

#define VISUAL_NAME "ch_ntshmem"

HWND hEdit,hStatic;
EnvFuncs EProcs = {0,0,0};

void WINAPI _PlgDescription(struct _PlgDesc* pDesc);

BOOL WINAPI LFSHMEMCreateCommandline(HostData** sppHostAll,
				     DWORD SizeAll, 
				     HostData** sppHostSelected, 
				     DWORD *pSizeSelected,
				     HostData *globalData);


void WINAPI LFSHMEMAttach(HWND App);
void WINAPI LFSHMEMDetach(void);
BOOL WINAPI LFSHMEMSelectedChange(HostData**,DWORD,HostData**,DWORD*,HostData *);
BOOL WINAPI LFSHMEMIsValid(HostData* actualHost);
void WINAPI LFSHMEMConfigure(HWND,HWND,HostData*,BOOL);
void WINAPI LFSHMEMParse(int* argc,char **argv,HostData *actHost);

static HostData _HostData;
static bool selected=false;

void WINAPI _PlgDescription(struct _PlgDesc* pDesc) {
	InitCommonControls();
	pDesc->PluginVersionNumber = VersionPlugin_h;
	strcpy(pDesc->VisualName,VISUAL_NAME);
	pDesc->DllName[0]=0;
	pDesc->Attach=LFSHMEMAttach;
	pDesc->Detach=LFSHMEMDetach;
	pDesc->NewData=LFSHMEMIsValid;
	pDesc->SelectedChange=LFSHMEMSelectedChange;
	pDesc->DlgClose=LFSHMEMCreateCommandline;
	pDesc->RefreshEnd=NULL;
	pDesc->Convert=NULL;
	pDesc->Configure = LFSHMEMConfigure;
	pDesc->ParseCommandline = LFSHMEMParse;
}


BOOL WINAPI LFSHMEMIsValid(HostData* actualHost) {
	if(!selected) return TRUE;
	
	return (strcmp(actualHost->Name,_HostData.Name)==0) ;
}

BOOL WINAPI LFSHMEMCreateCommandline(HostData** sppHostAll,DWORD SizeAll, 
				     HostData** sppHostSelected, DWORD *pSizeSelected,
				     HostData *globalData)  
{
    
    char Commandline[10],ErrorMessage[256];
    ConfContext *actContext,dummy;
    char *Formats[] = {"SLOG","CLOG","ALOG"};
    DWORD size = 256;
    
    if (!*pSizeSelected) return FALSE;
    actContext = (ConfContext*)EProcs.GetContext(globalData,0);
    if(!actContext) {
	initContext(&dummy);
	actContext=&dummy;
    }
    if(actContext) {
	if(actContext->StartServer) {
	    if(!StartMPEServer()) {
		sprintf(ErrorMessage,"Startup failed (%x)",GetLastError());
		MessageBox(0,ErrorMessage,"Error",MB_OK|MB_ICONERROR);
	    }
	    GetComputerName(actContext->Display,&size);
	} else StopMPEServer();

	if(actContext->Display[0])
	    EProcs.PutString(sppHostSelected[0],"DISPLAY",actContext->Display);
	EProcs.PutString(sppHostSelected[0],"MPE_LOG_FORMAT",Formats[actContext->LogFormat]);
	sprintf(Commandline,"-np %s ",actContext->Num);
	EProcs.SetCommandline(sppHostSelected[0],Commandline);	
    } else return FALSE;    

    if (*pSizeSelected>0) 
	return(TRUE);
    else
	return(FALSE);
}

void WINAPI LFSHMEMParse(int* argc,char **argv,HostData *actHost) {    
    char *num=0,*Display=0;
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
    GetStringArg(argc,argv,"-num",&num);
    if(num) {
	strncpy(Context->Num,num,7);
	num[7]=0;
    }
    
    if(Display)
	strcpy(Context->Display,Display);
    

}

void WINAPI LFSHMEMConfigure(HWND hApplication,HWND hDialog,HostData *actHost,BOOL global) {
    char txt[255];
    int res;
    ConfContext *Context;

    
    if(!global || !actHost) return;
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

    Context->Global = global;
    
    res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_GLOBAL_DIALOG),hDialog,
                         ConfigureProc,(LPARAM)actHost);    
    if(res<0) {
	sprintf(txt,"Error %d",GetLastError());
	MessageBox(0,txt,"ERROR",MB_ICONERROR|MB_OK);
    }
}

BOOL WINAPI LFSHMEMSelectedChange(HostData** All,DWORD AllSize,HostData** SelHosts,DWORD* SelSize,HostData *globalData) {
    DWORD i;
    
    if(*SelSize<=0) selected=false;
    else {
	selected=true;
	strcpy(_HostData.Name,(*SelHosts)->Name);
    }
    
    for (i=0;i<AllSize;i++) {
	if(All[i]->State)
	    All[i]->State->Valid=LFSHMEMIsValid(All[i]);
    }
    if(*SelSize > 1) *SelSize = 1;
    return TRUE;
}


void WINAPI LFSHMEMAttach(HWND App) {
	selected=false;
}

void WINAPI LFSHMEMDetach(void) {
    StopMPEServer();	
    DestroyWindow(hEdit);
    DestroyWindow(hStatic);
    selected = false;
	
}

BOOL __stdcall DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID) {
 
    HMODULE hProc = GetModuleHandle(NULL);

    hInst = hinstDLL;

    if(fdwReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(hinstDLL);
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
    } else if(fdwReason == DLL_PROCESS_ATTACH)
	StopMPEServer();
    return TRUE;
}

#ifdef __cplusplus
}
#endif
