//---------------------------------------------------------------------------
#ifndef NetStateH
#define NetStateH
//---------------------------------------------------------------------------
//#include <vcl/dstring.h>

#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"

#define ENUM_START (WM_USER+13)
#define PAR_DATA (WM_USER+14)
#define ENUM_FINISH (WM_USER+15)
#define REFRESH_FINISH (WM_USER+16)
#define REFRESH_START (WM_USER+17)



class CServers {
//friend int __fastcall ThreadMain(void *);
public:
	CServers(BOOL Backup);
    ~CServers();
    void StopProcessing();
    BOOL RegisterClientWindow(HWND client);
    BOOL RemoveClientWindow(HWND client);
    void Lock() {EnterCriticalSection(&CS);}
    void Unlock() {LeaveCriticalSection(&CS);}
    int IndexOf(char *Name);
    int Add(HostData* Host);
    void Delete(int index);
    void Delete(char *Host);
    void Clear();
    BOOL Refresh();
    void WaitForEnd() {if(hThread) WaitForSingleObject(hThread,INFINITE);}
    HostData *Refresh(char *Host);
    BOOL EnumData(HWND client);
    HostData** GetList() {return Hosts;}
    int Count() {return NumHosts;}
    HostData * operator[](int index) { return Hosts[index];}
    HostData * operator[](char *name) { int i=IndexOf(name); if(i<0) return 0; return Hosts[i];}
    bool Refreshing;
    HostData **Hosts;
    BOOL useMachinestxt;  //si
    void RefreshOne(char * name) ; //si
    void ThreadFunc();
    void TestFunc(int n);
   
protected:
    //void ThreadFunc();
    int NumHosts;
    int numClients;
    int Arraysize;
    HWND Clients[20]; //List with handles of all client windows
    CRITICAL_SECTION CS,clientCS;
    HANDLE hThread;
    BOOL B,Finish;
    BOOL AskedForuseMachinestxt;  //si
    
};

extern CServers *Servers;


#endif
