#include <windows.h>
#include <stdio.h>
#include "Plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

HWND hEdit,hStatic;

void WINAPI _PlgDescription(struct PlgDesc* pDesc);

BOOL WINAPI SVMlibCreateComandline(HostData** sppHostAll,
								   DWORD SizeAll, 
								   HostData*** spppHostSelected, 
								   DWORD *pSizeSelected);


void WINAPI SVMlibAttach(HWND App);
void WINAPI SVMlibDetach(void);
BOOL WINAPI SVMlibCallback(HWND Dialog, WORD msg,WPARAM WParam,LPARAM LParam);
BOOL WINAPI SCICreateUserInfo(TStateData *spState, char *output, DWORD *pSize);
BOOL WINAPI SCIIsValid(HostData* spHost);

HostData actHost;

void WINAPI _PlgDescription(struct PlgDesc* pDesc) {
	pDesc->PluginVersionNumber = VersionPlugin_h;
	strcpy(pDesc->VisualName,"SVMlib+SCI");
	strcpy(pDesc->DllName,"SCIExt.dll");
	//pDesc->DllName[0]=0;
	pDesc->Attach=SVMlibAttach;
	pDesc->Detach=SVMlibDetach;
	pDesc->NewData=SCIIsValid;
	pDesc->SelectedChange=NULL;
	pDesc->DlgClose=SVMlibCreateComandline;
	pDesc->RefreshEnd=NULL;
	pDesc->DlgCallback=SVMlibCallback;
	pDesc->Convert=SCICreateUserInfo;
}


BOOL WINAPI SVMlibCreateComandline(HostData** sppHostAll,DWORD SizeAll, 
								   HostData*** spppHostSelected, DWORD *pSizeSelected)  
{
	DWORD i;
	char Name[128];
	DWORD size=128;
	HostData *MyHost;
	BOOL Found=FALSE;
	char Port[5];
	
	GetComputerName(Name,&size);

	// Liste umsortieren, damit der lokale Rechner oben steht.
	for (i=0;i<(*pSizeSelected);i++) {
		if(!stricmp(Name,(*spppHostSelected)[i]->Name)) {
			MyHost=(*spppHostSelected)[i];
			(*spppHostSelected)[i]=(*spppHostSelected)[0];
			(*spppHostSelected)[0]=MyHost;
			break;
		}
	}
	
	// Welcher Port?
	if(!SendMessage(hEdit,WM_GETTEXT,5,(LONG)Port)) strcpy(Port,"2002");
	
	for (i=0;i<(*pSizeSelected);i++) {
		if(i>0&&!stricmp((*spppHostSelected)[0]->Name,(*spppHostSelected)[i]->Name))
			sprintf(((*spppHostSelected)[i])->Commandline,"-n%d -mlocalhost -p%s -- ",*pSizeSelected,Port);
		else sprintf(((*spppHostSelected)[i])->Commandline,"-n%d -m%s -p%s -- ",*pSizeSelected,((*spppHostSelected)[0])->Name,Port);
	}

	if (*pSizeSelected>0) 
		return(TRUE);
	else
		return(FALSE);
}

void WINAPI SVMlibAttach(HWND App) {
	HINSTANCE hI;
	HFONT Font;
	hI=GetModuleHandle(0);
	Font=(HFONT)SendMessage(Dialog,WM_GETFONT,0,0);
	
	hStatic=CreateWindow("STATIC","Port",WS_CHILD|WS_VISIBLE,400,215,41,15,Dialog,0,hI,0);
	hEdit=CreateWindowEx(WS_EX_CLIENTEDGE ,"EDIT","2002",WS_BORDER|WS_CHILD|WS_VISIBLE|ES_NUMBER,400,230,41,21,Dialog,0,
		hI,0);

	if(Font) {	
		SendMessage(hEdit,WM_SETFONT,(WPARAM)Font,(LPARAM)1);
		SendMessage(hStatic,WM_SETFONT,(WPARAM)Font,(LPARAM)1);
	} 
}

void WINAPI SVMlibDetach(void) {
	DestroyWindow(hEdit);
	DestroyWindow(hStatic);
	
}

BOOL WINAPI SVMlibCallback(HWND Dialog, WORD msg,WPARAM WParam,LPARAM LParam) {
	return TRUE;
}

BOOL WINAPI SCICreateUserInfo(TStateData *spState, char *output, DWORD *pSize)
{
	actHost.State=spState;
	
	if(pSize==NULL) return FALSE;
	
	if (spState==NULL) {
		*pSize=0;
		return(FALSE);
	}
	
	if ((*pSize<20)) {
		*pSize=20;
		return (FALSE);
	}
	
	if (SCIIsValid(&actHost)) {
		sprintf(output,"SCI ID: %d",*((int*)spState->UserData));
	}
	else {
		*pSize=0;
		*output=0;
	}

	//*pSize=strlen(output);
	return(TRUE);
}

BOOL WINAPI SCIIsValid(HostData* spHost)
{
	if(!spHost) return FALSE;
	if(spHost->State==NULL) return FALSE;

	if (( spHost->State->UserDataSize == sizeof(int) )&&
		(*((int*)(spHost->State->UserData))>=0)) {
			return(TRUE);
	}
	else {
		return(FALSE);
	}
}


#ifdef __cplusplus
}
#endif
