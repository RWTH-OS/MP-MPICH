#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include "Plugin.h"
#include "resource.h"
#include "configure.h"
#include <args.h>

#ifdef __cplusplus
extern "C" {
#endif


#define VISUAL_NAME "MPI/Pro"

EnvFuncs EProcs = {0,0,0};
HINSTANCE hInst;

void WINAPI _PlgDescription(struct _PlgDesc* pDesc);

BOOL WINAPI CreateComandline(HostData** sppHostAll,
	    DWORD SizeAll, 
	    HostData** sppHostSelected, 
	    DWORD *pSizeSelected,
	    HostData *globalData);


void WINAPI Attach(HWND App);
void WINAPI Detach(void);
void WINAPI Configure(HWND,HWND,HostData *,BOOL);
void WINAPI Parse(int* argc,char **argv,HostData *actHost);

//HostData actHost;

void WINAPI _PlgDescription(struct _PlgDesc* pDesc) {
	pDesc->PluginVersionNumber = VersionPlugin_h;
	strcpy(pDesc->VisualName,VISUAL_NAME);
	pDesc->DllName[0]=0;
	pDesc->Attach=Attach;
	pDesc->Detach=Detach;
	pDesc->NewData=NULL;
	pDesc->SelectedChange=NULL;
	pDesc->DlgClose=CreateComandline;
	pDesc->RefreshEnd=NULL;
	pDesc->Convert=NULL;
	pDesc->Configure=Configure;
	pDesc->ParseCommandline = Parse;
}

void WINAPI Configure(HWND hApplication,HWND hDialog,HostData *actHost,BOOL global) {
    char txt[255];
    ConfContext *Context;
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

    Context->Global = global;
    int res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_DIALOG1),hDialog,
	                     ConfigureProc,(LPARAM)actHost);
    if(res<0) {
	sprintf(txt,"Error %d",GetLastError());
	MessageBox(0,txt,"ERROR",MB_ICONERROR|MB_OK);
    }
}


BOOL WINAPI CreateComandline(HostData** sppHostAll,DWORD SizeAll, 
			     HostData** sppHostSelected, DWORD *pSizeSelected,
			     HostData *globalData)  
{
	DWORD i,j,localLow;
	
	char SMP_hosts[255],iproc[10],nproc[10],TCP_hosts[255],rootport[30];
	char low[10],high[10],*roothost,*nodename;
	char Name[128];
	DWORD size = 128,Start=0,End=0;
	bool found = false;
	HostData **help;

	ConfContext* actContext;

	if(!*pSizeSelected) return FALSE;


	GetComputerName(Name,&size);	
	for (i=0;i<*pSizeSelected;++i) {
	    if(!stricmp(Name,sppHostSelected[i]->Name)) {
		if(!found) {
		    Start = i;
		    found = true;
		}
	    } else if(found) break;
	}

	if(found&&Start>0) {
	    End = i;
	    help = (HostData**)alloca((End-Start)*sizeof(HostData*));
	    memcpy(help,sppHostSelected+Start,(End-Start)*sizeof(HostData*));
	    memmove(sppHostSelected+(End-Start),sppHostSelected,Start*sizeof(HostData*));
	    memcpy(sppHostSelected,help,(End-Start)*sizeof(HostData*));
	}

	if(!sppHostSelected[0]->State||!sppHostSelected[0]->State->Configuration) 
	    roothost = sppHostSelected[0]->Name;	    
	else {
	    actContext = (ConfContext*)EProcs.GetContext(sppHostSelected[0],0);
	    roothost = (char*)&(sppHostSelected[0]->State->Configuration->IP.Hostname_ip[0]);
	}

	sprintf(nproc,"%d",*pSizeSelected);

	if(!globalData->ProcData || !globalData->ProcData->Context) {
	    actContext = (ConfContext*)EProcs.GetContext(globalData,sizeof(ConfContext));
	    initContext(actContext);
	} else actContext = (ConfContext*)globalData->ProcData->Context;
	if(actContext->Port[0]) strcpy(rootport,actContext->Port);
	else strcpy(rootport,"26379");

	localLow=0;

	for (i=0;i<=(*pSizeSelected);++i) {
	    if(i==*pSizeSelected || stricmp(sppHostSelected[localLow]->Name,sppHostSelected[i]->Name))
	    {
		sprintf(low,"%d",localLow);
		sprintf(high,"%d",i-1);
		SMP_hosts[0]=0;
		TCP_hosts[0]=0;
		for(j=localLow;j<i;++j) 
		    sprintf(SMP_hosts,"%s %d",SMP_hosts,j);

		for(j=0;j<localLow;++j) 
		    sprintf(TCP_hosts,"%s %d",TCP_hosts,j);
		for(j=i;j<(*pSizeSelected);++j) 
		    sprintf(TCP_hosts,"%s %d",TCP_hosts,j);

		for(j=localLow;j<i;++j) {		    
		    if(!sppHostSelected[j]->ProcData || !sppHostSelected[j]->ProcData->Context) {
			actContext = (ConfContext*)globalData->ProcData->Context;
		    } else actContext = (ConfContext*)sppHostSelected[j]->ProcData->Context;
		    
		   
		    if(!sppHostSelected[0]->State||!sppHostSelected[0]->State->Configuration) {
			nodename = sppHostSelected[j]->Name;	    
		    } else {
			nodename = (char*)&(sppHostSelected[j]->State->Configuration->IP.Hostname_ip[0]);
		    }

		    EProcs.PutString(sppHostSelected[j],"MSTI_SIZE",nproc);
		    EProcs.PutString(sppHostSelected[j],"MSTI_ROOT_NAME",roothost);
		    EProcs.PutString(sppHostSelected[j],"MSTI_NODE_NAME",nodename);
		    EProcs.PutString(sppHostSelected[j],"MSTI_PORT",rootport);
		    sprintf(iproc,"%d",j);
		    EProcs.PutString(sppHostSelected[j],"MSTI_RANK",iproc);
		    EProcs.PutString(sppHostSelected[j],"MSTI_SMP_LIST",SMP_hosts);
		    if(strlen(TCP_hosts))
			EProcs.PutString(sppHostSelected[j],"MSTI_TCP_LIST",TCP_hosts);
		    if(actContext->Verbose) 
			EProcs.PutString(sppHostSelected[j],"MSTI_VERBOSE","1");
		    else 
			EProcs.PutString(sppHostSelected[j],"MSTI_VERBOSE","0");
		    if(actContext->Debug) 
			EProcs.PutString(sppHostSelected[j],"MSTI_DEBUG","1");
		    else 
			EProcs.PutString(sppHostSelected[j],"MSTI_DEBUG","0");
		}
		localLow = i;
	    }		   
	}

	if (*pSizeSelected>0) 
		return(TRUE);
	else
		return(FALSE);
}


void WINAPI Parse(int* argc,char **argv,HostData *actHost) {    
    char *port=0;
    ConfContext *Context=0;
    
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;


    Context->Debug = IsArgPresent(argc,argv,"-mpi_debug");
    Context->Verbose = IsArgPresent(argc,argv,"-mpi_verbose");
    GetStringArg(argc,argv,"-port",&port);
    
    
    if(port) {
	strncpy(Context->Port,port,7);
	Context->Port[7] = 0;
    }
}

void WINAPI Attach(HWND App) {
    srand( (unsigned)time( NULL ) );
}

void WINAPI Detach(void) {

	
}


BOOL __stdcall DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID) {
    hInst = hinstDLL;
    HMODULE hProc = GetModuleHandle(NULL);

    if(fdwReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(hInst);
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
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif
