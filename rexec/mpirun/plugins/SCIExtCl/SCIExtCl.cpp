#include <windows.h>
#include <stdio.h>
#include "Plugin.h"
#include "dialog2.h"
#include "resource.h"
#include "configure.h"
#include <args.h>
#include <startserver.h>
#include <SCIExt.h>

#ifdef __cplusplus
extern "C" {
#endif



#ifdef MPI
#define VISUAL_NAME "ch_smi (SCI)"
#define LIBNAME "ch_smi.dll"
#else 
#define VISUAL_NAME "Shared memory interface"
#define LIBNAME "SCIExtCl.dll"
#endif
 
HINSTANCE hInst;
EnvFuncs EProcs = {0,0,0,0}; 

__declspec(dllexport) void WINAPI _PlgDescription(struct _PlgDesc*);
BOOL WINAPI SCIExtIsValid(HostData*);
BOOL WINAPI SCIExtCreateComandline(HostData**,DWORD, HostData**,DWORD *,HostData *);
BOOL WINAPI SCIExtCreateUserInfo(TStateData *spState, char *output, DWORD *pSize);

BOOL isSmp(HostData** sppHostSelected, int SizeSelected);
BOOL WINAPI SCIExtSelectedChange(HostData** All,DWORD AllSize,HostData** SelHosts,DWORD* SelSize,HostData *globalData);
void WINAPI SCIExtDetach(void);
void WINAPI SCIExtConfigure(HWND,HWND,HostData*, BOOL);
void WINAPI SCIExtParse(int*,char**,HostData *);


static HostData _HostData;
static bool selected=false;
static bool SCISelected=false;

 
static HostData actHost;

__declspec(dllexport) void WINAPI _PlgDescription(struct _PlgDesc* pDesc) {
	pDesc->PluginVersionNumber = VersionPlugin_h;
	strcpy(pDesc->VisualName,VISUAL_NAME);
	strcpy(pDesc->DllName,"SCIExt.dll");
	pDesc->Attach=NULL;
	pDesc->Detach=SCIExtDetach;
	pDesc->NewData=SCIExtIsValid;
	pDesc->SelectedChange=SCIExtSelectedChange;
	pDesc->DlgClose=SCIExtCreateComandline;
	pDesc->RefreshEnd=NULL;
	pDesc->Convert=SCIExtCreateUserInfo;
	pDesc->Configure = SCIExtConfigure;
	pDesc->ParseCommandline = SCIExtParse;
}



void WINAPI SCIExtDetach(void) {
    StopMPEServer();
    selected = false;
}


bool WINAPI SCIExtIsSCIHost(HostData* spHost)
{
	if(!spHost || !spHost->State) return false;
	if(spHost->State->UserDataSize < sizeof(int)) return false;

	/* Ovveride check for SCI *PATCH* */
	return TRUE;

	if((*(int*)(spHost->State->UserData))>0) {
		return(true);
	} else {
		return(false);
	}
}


BOOL WINAPI SCIExtIsValid(HostData* actualHost) {
	if(!selected) return TRUE;
	if(!actualHost) return FALSE;
	
	/* Ovveride check for SCI *PATCH* */
	return TRUE;

	if(!SCISelected)
		return (strcmp(actualHost->Name,_HostData.Name)==0) ;
	else 
		return (SCIExtIsSCIHost(actualHost)?TRUE:FALSE);
}

// returns RingId of first SCI-Adapter only
// this has to be changed if multiple Adapter support is needed
DWORD WINAPI SCIExtSciRing(HostData* spHost) {
	NodeData* spNode;
	
	if(!selected) return 0;
	if(!spHost) return 0;
	if(!SCISelected) return 0; 

	// determine RingId only if host has provided the current info struct
	if (spHost->State->UserDataSize == sizeof(NodeData)) {
		spNode = (NodeData*)spHost->State->UserData;
		return(spNode->Subnets[0]);	
	}

	return 0;
}



BOOL WINAPI SCIExtSelectedChange(HostData** All,DWORD AllSize,HostData** SelHosts,
				 DWORD* SelSize,HostData *globalData) {
    DWORD i;
	DWORD iRingId = 0;

	/* Ovveride check for SCI *PATCH* */
    return TRUE;

	SCISelected = FALSE;

    if(*SelSize<=0) selected=false;
    else {
		selected=true;
		SCISelected = SCIExtIsSCIHost(*SelHosts);
		strcpy(_HostData.Name,(*SelHosts)->Name);
    }
    
	// the determination whether a node can be reached or not
	// only depends on the SciRinId of the adapter found first
	// untill now.
	// If you are using multiple adapters connecting 2 different sci-rings
	// claim all to be in the same SCI-ring using the SCIExt.ini File

	// get the Ring id of first Selected Host
	if (selected)
		iRingId = SCIExtSciRing(SelHosts[0]);

    for (i=0;i<AllSize;i++) {
		if(All[i] && All[i]->State) {
			All[i]->State->Valid=SCIExtIsValid(All[i]);
			// disable host, if RingId does noch match
				if ((selected)&&(All[i]->State->Valid)) {
					if (iRingId != SCIExtSciRing(All[i]))
					All[i]->State->Valid = FALSE;
				}
		}
	}

    i=0;
    while(i<*SelSize) {
		if(!(SelHosts)[i]->State || !SelHosts[i]->State->Valid) {
			(*SelSize)--;
			if(i<*SelSize)
			memmove(SelHosts+i,SelHosts+i+1,(*SelSize-i)*sizeof(HostData*));
		} 
		else i++;
    }
    
    return TRUE;
}


BOOL WINAPI SCIExtCreateComandline(HostData** sppHostAll,DWORD SizeAll, 
				   HostData** sppHostSelected, DWORD *pSizeSelected,
				   HostData *globalData)  
{
	DWORD i;
	int NrOfHosts;
	BOOL SMP;
	int iRandom = rand();
	char szDebug[8] = "";
	char szVerbose[8] = "";
	char Commandline[2048];
	char *szPort;
	char szHelp[6];//si
	char *SyncHost;
	DWORD LogFormat=0;
#ifdef MPI
	DWORD size = 256;
	char szConf[260] ="";
	char *Formats[] = {"SLOG","CLOG","ALOG"};
	char *Display=0;
#endif
	ConfContext *actContext;
	NrOfHosts=*pSizeSelected;
	
	/*
	strcpy(szSyncHost,sppHostSelected[0]->Name);
	for(i=0;i<strlen(szSyncHost);i++)
		szSyncHost[i]=tolower(szSyncHost[i]);
	*/

	actContext = (ConfContext*)EProcs.GetContext(globalData,0);
	if(actContext) {
#ifdef MPI
	    if(actContext->StartServer) {
		if(!StartMPEServer()) {
		    sprintf(Commandline,"Startup failed (%x)",GetLastError());
		    MessageBox(0,Commandline,"Error",MB_OK|MB_ICONERROR);
		}
		GetComputerName(actContext->Display,&size);
	    } else StopMPEServer();
	    LogFormat = actContext->LogFormat;
	    Display = actContext->Display;
#endif
	    szPort = actContext->Port;
	  
	} else 
	{
		//si: use random port to avoid conflicts
		itoa(CH_SMI_BASE_PORT+(iRandom%1000),szHelp,10);
		szPort=szHelp;
		/*MessageBox(0,szPort,szHelp,MB_OK|MB_ICONERROR);*/
		/*szPort = "5000";*/
	}

	if(!NrOfHosts) return FALSE;

	SMP=isSmp(sppHostSelected,*pSizeSelected);
	SyncHost = (char*)&(sppHostSelected[0]->State->Configuration->IP.Hostname_ip[0]);

	for (i=0;i<(*pSizeSelected);i++) {
	    *szDebug = 0;
	    *szVerbose = 0;
#ifdef MPI
	    *szConf = 0;
#endif
	    actContext = (ConfContext*)EProcs.GetContext(sppHostSelected[i],0);
	    if(actContext) {
		if(actContext->Debug) strcpy(szDebug,"-s");
		if(actContext->Verbose) strcpy(szVerbose,"-v");
#ifdef MPI
		if(Display && Display[0])
    		    EProcs.PutString(sppHostSelected[i],"DISPLAY",Display);

		if(strlen(actContext->Conffile)) sprintf(szConf,"-d %s",actContext->Conffile);
	    }
	    sprintf(Commandline,
				"-h %s -r %d -n %d -m %d -p %s %s %s %s",
				SyncHost,i,NrOfHosts,iRandom,szPort,szConf,szDebug,szVerbose);
#else
	    }
    	    sprintf(Commandline,
				"-h %s -r %d -n %d -m %d -p %s %s %s",
				SyncHost,i,NrOfHosts,iRandom,szPort,szDebug,szVerbose);

#endif
	    /*MessageBox(0,Commandline,"Commandline",MB_OK|MB_ICONERROR);*/
		if(SMP) strcat(Commandline," -l -- ");
		else	strcat(Commandline," -- ");
		EProcs.SetCommandline(sppHostSelected[i],Commandline);
#ifdef MPI
		EProcs.PutString(sppHostSelected[i],"MPE_LOG_FORMAT",Formats[LogFormat]);
#endif
	}

	if (*pSizeSelected>0) 
		return(TRUE);
	else
		return(FALSE);
}

void WINAPI SCIExtParse(int* argc,char **argv,HostData *actHost) {    
    char *Conffile=0;
    char *Port=0,*Display=0;
    ConfContext *Context=0;
    
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

#ifdef MPI
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
    GetStringArg(argc,argv,"-devconf",&Conffile);
#endif
    GetStringArg(argc,argv,"-port",&Port);
    
    Context->Debug = IsArgPresent(argc,argv,"-smidebug");
    Context->Verbose = IsArgPresent(argc,argv,"-v");
    

    if(Port)
	strcpy(Context->Port,Port);
#ifdef MPI
    if(Display)
	strcpy(Context->Display,Display);
    
    if(Conffile)
	strcpy(Context->Conffile,Conffile);
#endif
}

BOOL WINAPI SCIExtCreateUserInfo(TStateData *spState, char *output, DWORD *pSize)
{
	actHost.State=spState;
	int uSize;
	NodeData *Data;
	char txt[200];

	if (pSize==NULL)
		return(FALSE);
	
	if ((*pSize<210)||(spState==NULL)) {
		*pSize=210;
		return (FALSE);
	}
	
	

	if (SCIExtIsSCIHost(&actHost)) {
		Data = (NodeData*)spState->UserData;
		uSize = spState->UserDataSize;
		if(uSize > sizeof(int) && Data->NumAdapters < MAXADAPTERS) {
		    *output = 0;
		    sprintf(txt,"%s\n",Data->Driver.sc_revision_string);
		    strcat(output,txt);
		    for(DWORD i=0;i<Data->NumAdapters;++i) {
				if (spState->UserDataSize == sizeof(NodeData))
					sprintf(txt,"ID[%d]: %d, Ring %d\n",i,Data->Ids[i],Data->Subnets[i]);
				else
					sprintf(txt,"ID[%d]: %d, Ring %d\n",i,Data->Ids[i],0);
				strcat(output,txt);
		    }
		    
		} else 
		    sprintf(output,"ID: %d\n",*(DWORD*)Data);
	}
	else {
		sprintf(output,"No SCI device installed!\n");
	}

	//*pSize=strlen(output);
	return(TRUE);
}

BOOL isSmp (HostData** sppHostSelected, int SizeSelected)
{
	char *FirstHost;
	int i;

	FirstHost=sppHostSelected[0]->Name;

	for(i=1;i<SizeSelected;i++)
		if (strcmp(FirstHost,sppHostSelected[i]->Name)!=0)
			return(FALSE);

	return(TRUE);
}


void WINAPI SCIExtConfigure(HWND hApplication,HWND hDialog, HostData *actHost,BOOL global) {
    char txt[255];
    int res;
    ConfContext *Context;
    if(!actHost->ProcData || !actHost->ProcData->Context) {
	Context = (ConfContext*)EProcs.GetContext(actHost,sizeof(ConfContext));
	initContext(Context);
    } else Context = (ConfContext*)actHost->ProcData->Context;

    Context->Global = global;
    if(global)
	res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_GLOBAL_DIALOG),hDialog,ConfigureProc,(LPARAM)actHost);
    else
	res = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_LOCAL_DIALOG),hDialog,ConfigureProc,(LPARAM)actHost);
    if(res<0) {
	sprintf(txt,"Error %d",GetLastError());
	MessageBox(0,txt,"ERROR",MB_ICONERROR|MB_OK);
    }
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
	
    } else if(fdwReason == DLL_PROCESS_ATTACH)
	StopMPEServer();

    return TRUE;
}

#ifdef __cplusplus
}
#endif
