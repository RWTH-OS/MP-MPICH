#ifndef __REXEC_CLIENT_H__
#define __REXEC_CLIENT_H__

#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"

#define KILL_FAILED	(WM_USER+9)
#define INIT_FINISH (WM_USER+10)
#define IN_DATA (WM_USER+11)
#define ERR_DATA (WM_USER+12)
#define RC_PROC (WM_USER+13)
#define RC_PROC_START (WM_USER+14)
#define RC_PROC_END (WM_USER+15)


#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD (WINAPI *MessageCallback)(char *, BOOL Error);
typedef BOOL (WINAPI *AccountEnumCallback)(char*,char*,char*,char*,void*);

typedef struct _RSI {
	char *Commandline;
	char *WorkingDir;
    char *Environment;
    DWORD EnvSize;
    DWORD CreationFlags;
	HWND Window;
        
} RemoteStartupInfo;

typedef struct {
	SEC_WINNT_AUTH_IDENTITY Ident;
    RPC_BINDING_HANDLE Binding;
} TServerHandle;

typedef struct {
	DWORD ID;
    char *Name;
    char *Owner;
} Enum_t;

void InitializeRexecClient();
void ShutdownRexecClient();
DWORD CreateRemoteProcess(HostData *Server,RemoteStartupInfo *SI,HANDLE *hProcess);
DWORD KillRemoteProcess(HANDLE hProcess);
DWORD WaitForRemoteEnd(HANDLE hProcess,DWORD Timeout);
void CloseRemoteHandle(HANDLE hProcess);
int ReadData(HANDLE hRemoteProc,char *buf,DWORD len,BOOL err);
//int SendInput(HANDLE hRemoteProc,char* buf,DWORD len);  //si already defined in winuser.h -> rename
int SendInputKey(HANDLE hRemoteProc,char* buf,DWORD len);
HANDLE GetRemoteThread(HANDLE hRemoteProc);
DWORD SetHostAccount(HostData *Server,TAccount *Creds);
char *GetHostAccountName(HostData *Server,char *buffer,DWORD size);
DWORD OpenHost(HostData *Server);
DWORD CloseHost(HostData *Server);
DWORD GetHostStatus(HostData *Server, char *ServerDllName);
void FreeHostStatus(HostData *Server);
TStateData *CopyStateData(HostData *dst,HostData *src);
HostData* CopyHost(HostData *dst,HostData *src);
void FreeHost(HostData *Server);

DWORD LockServer(HostData *Server);
DWORD UnlockServer(HostData *Server);
BOOL IsServerAlive(char *ServerName);
DWORD GetHostProcs(HWND client,HostData *Server);
DWORD KillProcess(HostData *Server,int id);

BOOL EnumStoredAccounts(AccountEnumCallback Callback,void *param);
BOOL StoreAccount(char *AccountName,char *UserName,char *Domain,char *Password);
BOOL LoadAccount(char *AccountName,char *UserName,char *Domain,char *Password);
BOOL DeleteAccount(char *Account);
LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize );

DWORD ShutDown(HostData *Server,BOOL restart); //si //10.6.
void SetRPCEncryption(BOOL Sec_enabled);  //si 4.8.03//

#ifdef __cplusplus 
}
#endif

#endif