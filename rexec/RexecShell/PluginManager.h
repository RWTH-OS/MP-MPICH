//---------------------------------------------------------------------------
#ifndef PluginManagerH
#define PluginManagerH
//---------------------------------------------------------------------------

#include <vcl\system.hpp>
#include <vcl\classes.hpp>

#pragma hdrstop
#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"

#define PM_CHANGE (WM_USER+100)

class CPluginRef: public TObject {
public:
	CPluginRef() {data=0;}
    CPluginRef(PlgDesc &S) {data=&S;}
    CPluginRef(PlgDesc *S) {data=S;}
	__fastcall ~CPluginRef() {
        if(data->hLib) FreeLibrary(data->hLib);
    	if(data) delete data; data=0;}
    operator PlgDesc*() {return data;}
	PlgDesc &operator*() {return *data;}
	PlgDesc *operator->() {return data;}
    PlgDesc *operator=(PlgDesc *S) {data=S; return S;}
    PlgDesc &operator=(PlgDesc &S) {data=&S; return S;}
protected:
	PlgDesc* data;
};

class TPluginManager {
public:
	TPluginManager();
	~TPluginManager();
    PlgDesc *GetActualPlugin() {
    	if(ActIndex>=0)
        	return *(CPluginRef*)Plugins->Objects[ActIndex];
        else return 0;
    }
    TStrings *GetPluginList() {return Plugins;}
    void SetActualPlugin(int id);
    BOOL SetActualPlugin(char *Name);
    void LoadPlugins();
    int GetActualIndex() {return ActIndex;}
	BOOL RegisterClientWindow(HWND client);
    BOOL RemoveClientWindow(HWND client);
private:
	void SendNotifications(BOOL refresh);
	int ActIndex;
    HWND Clients[20];//List with handles of all client windows
    DWORD numClients;
    TStringList *Plugins;
};


extern TPluginManager PluginManager;

#endif
