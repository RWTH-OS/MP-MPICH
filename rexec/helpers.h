#ifndef __HELPERS_H__
#define  __HELPERS_H__

#include <wtypes.h>
//#include <iostream>
//#include <stdio.h>
#include <malloc.h>
#include <aclapi.h>
#include <accctrl.h>
#include <winnls.h>
#include <lm.h>

#define SERVER_STRING "Cluster Manager"
#define VERSION 2
#ifdef _DEBUG
#define SUBVERSION "17R - DEBUG"
#else
#define SUBVERSION "17R"
#endif
//////////////////////////////////////////////////////////////////////////////
//
// rcluma versions

//version 2.14R: added remote shutdown
//version 2.15R: added PRC-protocols for test purposes
//version 2.16R: error correction at debug output 
//version 2.17R: support of RPC calls without user authentication 
//version and subversion have to be changed in SERVICE.H, too!
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
#if!defined(DBG)
#include <fstream>
extern std::ofstream *o;
#define DBG(m) if(debug_flag && o) (*o)<<m<<std::endl<<std::flush;
#endif
extern "C" {
#else 
#define DBG(m)
#endif

//additionial output in logfile if compiled as debug version
#ifdef _DEBUG
#define DBGOUT(m) if(debug_flag && o) (*o)<<m<<std::endl<<std::flush;
#else
#define DBGOUT(m)
#endif

extern BOOL debug_flag;
extern PSID LockSID;
extern PSID LocalAdmins,DomainAdmins;
extern BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
extern BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *,DWORD, LPDWORD );
extern DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD );      


void LogLogMessage(DWORD ID,PSID Sid,DWORD numInserts,LPTSTR *lpStrings);

BOOL CompareUser(PSID sid,DWORD *error);
BOOL CheckGroupMembership(PSID GroupSid, DWORD *error);
void NormalizeDomainname(char *Domain);
BOOL IsUserAdmin(WCHAR *wszDCName);
PSID CreateSidFromRid(LPWSTR TargetComputer,DWORD Rid);
PSID LookupAliasFromRid(DWORD Rid);
BOOL ConvertSid(PSID pSid, LPTSTR pszSidText, LPDWORD dwBufferLen);
TOKEN_USER *GetLoggedOnUserToken(HANDLE *hProc);
TOKEN_USER *GetActualUserToken(DWORD *error);
BOOL InjectExitProc(HANDLE hProcess,DWORD ExitCode);
BOOL GetProcessOwner(HANDLE hProcess,char *buf,DWORD *size);
DWORD GetPrimaryToken( HANDLE *newToken);

BOOL AllowAccessToDesktop();

error_status_t ShutDown(BOOL restart);
char * GetDateTimeString(char * DateTimeString);

#ifdef __cplusplus
}
#endif

#endif