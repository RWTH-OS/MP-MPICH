//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#include <mbstring.h>
#include <dos.h>
#pragma hdrstop

#include "PluginManager.h"
#include "NetState.h"
#if ((BCBVER > 1))
	//CBuilder5
	#include <stdio.h>
#endif
//---------------------------------------------------------------------------

TPluginManager PluginManager;

TPluginManager::TPluginManager() {
	PlgDesc *Plugin;
 	ActIndex = 0;
    numClients=0;
  	Plugins = new TStringList;
    Plugins->Sorted = false;
	Plugin=new PlgDesc;
    ZeroMemory(Plugin,sizeof(PlgDesc));
    strcpy(Plugin->VisualName,"None");
    Plugins->AddObject(Plugin->VisualName,new CPluginRef(Plugin));
}

TPluginManager::~TPluginManager() {
	CPluginRef *ref;
    PlgDesc *plg;

    plg = GetActualPlugin();
    if(plg->Detach) plg->Detach();
    for(int i=0;i<Plugins->Count;++i) {
    	ref = (CPluginRef*)(Plugins->Objects[i]);
        delete ref;
    }
    Plugins->Clear();
    delete Plugins;
    Plugins=0;
}

void TPluginManager::SendNotifications(BOOL refresh) {
	for(DWORD i=0;i<numClients;++i)
    	SendMessage(Clients[i],PM_CHANGE,refresh,(LPARAM)ActIndex);
}

void TPluginManager::SetActualPlugin(int id) {
	PlgDesc *plg;
    BOOL refresh = FALSE;
    if(id != ActIndex && ActIndex<Plugins->Count) {
		if(ActIndex >=0) {
		    plg = GetActualPlugin();
		     if(plg->Detach != 0)
		    	plg->Detach();
             if(plg->DllName[0]) refresh = true;
        }
	   	ActIndex=id;
		if(ActIndex >=0) {
    		plg = GetActualPlugin();
			if(plg->Attach)
		    	plg->Attach(Application->Handle);
            refresh |= (plg->DllName[0]!=0);
        }
        SendNotifications(refresh);
    }
}

BOOL TPluginManager::SetActualPlugin(char *Name) {
	int id;
    BOOL found=TRUE;
    id = Plugins->IndexOf(Name);
    if(id<0) {
    	id = 0;
        found = FALSE;
    }
	SetActualPlugin(id);
    return found;
}

void TPluginManager::LoadPlugins() {
char *pos;
    char dir[MAX_PATH],DLL[MAX_PATH], helpstr[255];
    HANDLE hSearch;
    WIN32_FIND_DATAA FileData;
    HINSTANCE hLib;
    CHAR *Name;
    InitFunc IF;
    PlgDesc *Description;
    pos=strrchr(_argv[0],'\\');
    if(!pos) return;
    pos++;
    *pos=0;
    sprintf(dir,"%sPlugins\\*.dll",_argv[0]);

    hSearch=FindFirstFileA(dir,&FileData);
    if(hSearch==INVALID_HANDLE_VALUE) return;
    do {
		if(!FileData.cFileName[0]) Name=FileData.cAlternateFileName;
        else Name=FileData.cFileName;
        sprintf(DLL,"%sPlugins\\%s",_argv[0],Name);
        //Application->MessageBox(DLL,"Plugin",MB_OK);
    	hLib=LoadLibraryA(DLL);
        if(!hLib) {
        	//Application->MessageBox("Load failed",Name,MB_OK|MB_ICONWARNING);
            Application->MessageBox("Load failed",Name,MB_OK|MB_ICONWARNING);
        	continue;
        }
        IF=(InitFunc)GetProcAddress(hLib,"_PlgDescription");
        if(!IF) {
        	FreeLibrary(hLib);
            continue;
        }
        Description= new PlgDesc;
        ZeroMemory(Description,sizeof(PlgDesc));
        IF(Description); /* call _PlgDescription exported by the dll*/
        /* Si compare plugin version with version of plugin.h used by RexecShell*/
        /* please modify  '#define VersionPlugin_h' in Plugin.h on larger changes */
        if (VersionPlugin_h != Description->PluginVersionNumber)
        {
           sprintf(helpstr,"Pluginfile '%s' is not up to date",Name);
           Application->MessageBox(helpstr,"WARNING",MB_OK);
        }
        Description->Init=IF;
        Description->hLib=hLib;
		Plugins->AddObject(Description->VisualName,new CPluginRef(Description));
    } while(FindNextFileA(hSearch,&FileData));
    FindClose(hSearch);
    SendNotifications(FALSE);
}

// RegisterClientWindow is called by client window OnCreate
BOOL TPluginManager::RegisterClientWindow(HWND client) {
    if(numClients>=20) {
    	return FALSE;
    }
    // if client window is already registered return true
    for(DWORD i=0;i<numClients;++i)
    	if(Clients[i]==client) {
        	return TRUE;
        }
    // else add handle to list, increase number of clients
    Clients[numClients++]=client;
    return TRUE;
}

BOOL TPluginManager::RemoveClientWindow(HWND client) {
	for(DWORD i=numClients-1;i>=0 ;--i)
    	if(Clients[i]==client) {
         memmove(Clients+i,Clients+i+1,(numClients-i-1)*sizeof(HWND));
         numClients--;
         break;
        }
    return TRUE;
}
