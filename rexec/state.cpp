#define WIN32_LEAN_AND_MEAN

#include <wtypes.h>
#include <Iphlpapi.h>
#include <winsock2.h>
//#include <Iprtrmib.h>

//#include <Ws2tcpip.h>
#include <wtypes.h>

#include <stdio.h>
#include <malloc.h>
#include <aclapi.h>
#include <accctrl.h>
#include <winnls.h>
#include <lm.h>

#include <rpcdce.h>


#include <Iprtrmib.h>

#include <iostream>
#include <functional>
#include <list>

#include "cluma.h"
#include "service.h"
#include "helpers.h"
#include "messages.h"

BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *,DWORD, LPDWORD );
DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD ); 

extern "C" {
    BOOL (WINAPI *ProcessId2SessionId)(DWORD,DWORD *);
}

HINSTANCE hInstLib;

static CRITICAL_SECTION lockCS;

typedef BOOL (WINAPI *UserDataFunc)(void*,DWORD*);

PSID LockSID=0;
PSID LocalAdmins=0,DomainAdmins=0;
int LockCount = 0;

R_MACHINE_INFO MachineInfo;

WCHAR *wszDCName=0;          // Unicode DC name

struct UserDll {
    char Name[MAX_PATH];
    HINSTANCE hLib;
    UserDataFunc QueryUserData;
};

typedef std::list<UserDll> DllQueue;
DllQueue Plugins;

extern "C" {

    void GetLoginStatus(char *Username) {
        DWORD UserSize=256,DomainSize=256;
        TOKEN_USER *pTokenUser;
        char DomainName[256],UserName[256];
        SID_NAME_USE NameUse;
        HANDLE Proc;

        DBG("Entering GetLoginStatus")
            pTokenUser = GetLoggedOnUserToken(&Proc);
        if(Proc) CloseHandle(Proc);


        if(!pTokenUser || !IsValidSid(pTokenUser->User.Sid)) {
            DBG("GetLoginStatus: GetLoggedOnUserToken failed")
                free(pTokenUser);
            return ;
        }

        if(!LookupAccountSid(0,pTokenUser->User.Sid,UserName,&UserSize,DomainName,&DomainSize,&NameUse)) {
            DBG("GetLoginStatus: LookupAccountSid failed "<<GetLastError())
                UserSize=MAX_SIZE;
            ConvertSid(pTokenUser->User.Sid,Username,&UserSize);
            free(pTokenUser);
            return ;
        } else {
            NormalizeDomainname(DomainName);
            if(strlen(DomainName)+strlen(UserName)+3>MAX_SIZE)
                sprintf(Username,"%s",UserName);
            else
                sprintf(Username,"%s/%s",DomainName,UserName);
            free(pTokenUser);
            DBG("GetLoginStatus returns "<<Username)
                return ;
        }
    }

    error_status_t R_GetConsoleUser( 
        /* [in] */ handle_t Binding,
        /* [string][out] */ unsigned char __RPC_FAR User[ 255 ]) {

        User[0]=0;
    //DBG("Entering R_GetConsoleUser")
    GetLoginStatus((char*)User);
    //DBG("Leaving R_GetConsoleUser")
    return RPC_S_OK;
        }


        error_status_t GetData(char *dll,void *buffer,DWORD *Size) {
            error_status_t eStatus=FALSE;
            BOOL result=FALSE;
            DllQueue::iterator it;
            UserDll Pl;

            if(!dll[0]) {
                *Size=0;
                return TRUE;
            }

            for(it=Plugins.begin();it!=Plugins.end();it++) 
                if(!stricmp(dll,(*it).Name)) break;

            if(it == Plugins.end()) {
                strcpy(Pl.Name,dll);
                Pl.hLib=LoadLibraryA(dll);
                if(!(Pl.hLib)) {
                    AddToMessageLog(IDM_LOADLIBRARY,GetLastError(),Pl.Name);
                    DBG("LoadLibrary("<<Pl.Name<<")failed")
                        *Size=0;
                    return RPC_S_INVALID_ARG;
                }

                Pl.QueryUserData=(UserDataFunc)GetProcAddress(Pl.hLib,"_QueryUserData");
                if(!Pl.QueryUserData) {
                    AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"_QueryUserData");
                    DBG("GetProcAddress(\"_QueryUserData\") failed")
                        *Size=0;
                    FreeLibrary(Pl.hLib);
                    return RPC_S_INVALID_ARG;
                }
                Plugins.push_front(Pl);
                it=Plugins.begin();
            } 
            __try {
                result=(*it).QueryUserData(buffer,Size);
            } __finally{
                if(result) eStatus= RPC_S_OK;
            }
            if (eStatus != RPC_S_OK)
                eStatus = RPC_S_BUFFER_TOO_SMALL;

			/* Error occurred in RexecShell. Buffer was Null, Size was set, but return value was NO_ERROR 
			-> additional check if bufder == NULL && size > 0*/
			if (!buffer && Size)
                eStatus = RPC_S_BUFFER_TOO_SMALL;
            return eStatus;
        }

        error_status_t R_GetUserData( 
            /* [in] */ handle_t Binding,
            /* [string][in] */ unsigned char __RPC_FAR *DllName,
            /* [size_is][unique][out][in] */ byte __RPC_FAR result[  ],
            /* [in] */ long BufferSize,
            /* [ref][out] */ long __RPC_FAR *ResultSize) {

            error_status_t res;
        RPC_STATUS status;

        if(!DllName||!ResultSize) return ERROR_INVALID_PARAMETER;

        status = RpcImpersonateClient(Binding);

        if (status != RPC_S_OK) {
            return(RPC_S_ACCESS_DENIED);
        }

        *ResultSize=BufferSize;
        EnterCriticalSection(&lockCS);
		DBG("GetData from library "<<(char*)DllName)
        res=GetData((char*)DllName,result,(unsigned long*)ResultSize);
        LeaveCriticalSection(&lockCS);

        RpcRevertToSelf();
        return res;
            }


            error_status_t R_GetLockStatus( 
                /* [in] */ handle_t Binding,
                /* [ref][out] */ int __RPC_FAR *locked,
                /* [string][out] */ unsigned char __RPC_FAR User[ MAX_SIZE ]) {
                DWORD UserSize=MAX_SIZE,DomainSize=255;
            char DomainName[256],UserName[256];
            SID_NAME_USE NameUse;

            EnterCriticalSection(&lockCS);
            __try {
                //DBG("Entering R_GetLockStatus")
                *locked = LockCount;
                if(!LockSID) {
                    User[0]=0;
                    DBG("Node is free")
                } else {
                    if(!LookupAccountSid(0,LockSID,UserName,&UserSize,DomainName,&DomainSize,&NameUse)) {
                        DBG("R_GetLockStatus: LookupAccountSid failed "<<GetLastError())
                            UserSize=MAX_SIZE;
                        if(!ConvertSid(LockSID,(char*)User,&UserSize)) sprintf((char*)User,"UNKNOWN");
                    } else {
                        if(strlen(UserName)+strlen(DomainName)+2>MAX_SIZE)
                            sprintf((char*)User,"%s",UserName);
                        else sprintf((char*)User,"%s/%s",DomainName,UserName);
                        DBG("Node is locked by "<<User)
                    }
                }
            } __finally {
                LeaveCriticalSection(&lockCS);
            }
            //DBG("Leaving R_GetConsoleUser")
            return RPC_S_OK;
                }


                error_status_t R_LockServer(handle_t h) {
                    RPC_STATUS status;
                    TOKEN_USER *pTokenUser;
                    DWORD dwActSize;
                    DWORD error;

                    //DBG("Entering R_LockServer")
                    status = RpcImpersonateClient(h);

                    if (status != RPC_S_OK) {
                        DBG("Impersonation failed ("<<status<<")")
                            return(status);
                    }

                    pTokenUser = GetActualUserToken(&error);
                    if(!pTokenUser) {
                        RpcRevertToSelf();
                        DBG("R_LockServer: GetActualUserToken failed "<<error)
                            return error;
                    }

                    if(!IsValidSid(pTokenUser->User.Sid)) {
                        free(pTokenUser);
                        RpcRevertToSelf();  
                        return RPC_S_INVALID_SECURITY_DESC;
                    }

                    EnterCriticalSection(&lockCS);
                    if(LockSID) {
                        if(!EqualSid(pTokenUser->User.Sid,LockSID)) {
                            status=RPC_S_ACCESS_DENIED;
                            DBG("Node is already locked by different user")
                        }
                        else {
                            DBG("Node is already locked "<<LockCount<<" times by user")
                                status=RPC_S_OK;
                            ++LockCount;
                        }
                    } else {
                        dwActSize=GetLengthSid(pTokenUser->User.Sid);
                        LockSID=(PSID)malloc(dwActSize);
                        CopySid(dwActSize,LockSID,pTokenUser->User.Sid);
                        LockCount = 1;
                        status=RPC_S_OK;
                        DBG("Locked node for user")
                    }
                    LeaveCriticalSection(&lockCS);

                    free(pTokenUser);
                    RpcRevertToSelf();
                    DBG("Leaving R_LockServer with "<<status)
                        return(status);
                }


                error_status_t R_UnlockServer(handle_t h) {
                    RPC_STATUS status;
                    int Locked;
                    char UserName[255];

                    DWORD size = 255;

                    DBG("Entering R_UnlockServer")
                        EnterCriticalSection(&lockCS);
                    R_GetLockStatus(0,&Locked,(unsigned char*)UserName);
                    if(!Locked) {
                        DBG("Server is not locked")
                            LeaveCriticalSection(&lockCS);
                        return RPC_S_OK;
                    }

                    status = RpcImpersonateClient(h);

                    if (status != RPC_S_OK) {
                        LeaveCriticalSection(&lockCS);
                        DBG("Impersonation failed ("<<status<<")")
                            return status;
                    }

                    if(!CompareUser(LockSID,(DWORD*)&status) && status==RPC_S_OK && !IsUserAdmin(wszDCName)) {
                        status = RPC_S_ACCESS_DENIED;
                        DBG("Server is locked by different user")
                    } else {
                        if(!--LockCount) {
                            free(LockSID);
                            LockSID=0;
                        }
                        status = RPC_S_OK;
                        DBG("Unlocked server. LockCount is: "<<LockCount)
                    }
                    LeaveCriticalSection(&lockCS);
                    RpcRevertToSelf();  
                    DBG("Leaving R_UnlockServer with "<<status)
                        return status;
                }


                error_status_t R_GetProcs( /* [in] */ handle_t Binding) {
                    DWORD *procs=0;
                    DWORD dwSize=0,dwActSize,dwIndex;
                    char szFileName[MAX_PATH],Owner[255],*pos;
                    bool finish = false;
                    HANDLE hProcess;
                    HINSTANCE hMod;

					DBG("R_GetProcs called");

                    /* We want to get all processes.
                    So don't impersonate client
                    status = RpcImpersonateClient(Binding);

                    if (status != RPC_S_OK) {
                    return(status);
                    }
                    */
                    procs=(DWORD*)malloc(100*sizeof(DWORD));
                    dwActSize=100*sizeof(DWORD);
                    do{
                        if( !lpfEnumProcesses( procs, dwActSize, &dwSize ) ) {
                            AddToMessageLog(IDM_PROCS,GetLastError(),0);
                            if(procs) free(procs);
                            return 0;
                        }
                        if(dwActSize<=dwSize) {
                            dwActSize*=2;
                            procs=(DWORD*)realloc(procs,dwActSize);
                        } else finish=true;

                    } while(!finish);

                    dwSize /= sizeof( DWORD );
                    for( dwIndex = 0 ; dwIndex < dwSize ; dwIndex++ ){
                        szFileName[0] = 0 ;
                        // Open the process (if we can... security does not
                        // permit every process in the system).
                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ, 
                            FALSE, procs[ dwIndex ] ) ;           
                        if( hProcess != NULL ) {
                            // Here we call EnumProcessModules to get only the
                            // first module in the process this is important,
                            // because this will be the .EXE module for which we
                            // will retrieve the full path name in a second.
                            if( lpfEnumProcessModules( hProcess, &hMod,
                                sizeof( hMod ), &dwActSize ) )  {
                                    // Get Full pathname:
                                    if( !lpfGetModuleFileNameEx( hProcess, hMod,
                                        szFileName, sizeof( szFileName ) ) )  {
                                            szFileName[0] = 0 ;                    
                                        }               
                                } else {
                                    // We could not open the proc. So just try the next
                                    CloseHandle(hProcess);
                                    continue;
                                }
                                dwActSize = 255;
                                GetProcessOwner(hProcess,Owner,&dwActSize);
                                if(dwActSize>255) Owner[0]=0;
                                CloseHandle(hProcess);
                                pos = strrchr(szFileName,'\\');
                                if(!pos) pos = szFileName;
                                else pos++;     
                                R_ProcEnumCallback(Binding,procs[dwIndex],(unsigned char*)pos,(unsigned char*)Owner);
                        }
                    }
                    if(procs) free(procs);
                    //RpcRevertToSelf();    
                    return ERROR_SUCCESS;
                }
#if 0
                void GetIpInfo(unsigned char Name[MAX_SIZE],unsigned long IPS[100],unsigned long *count) {
                    SOCKET s;
                    void *buf=0;
                    DWORD returned=0,size=256;
                    int res;
                    INTERFACE_INFO *list;

                    //    sockaddr_in *in;

                    Name[0]=0;
                    gethostname((char*)Name,MAX_SIZE);

                    s = WSASocket(AF_INET,SOCK_STREAM,0,0,0,0);
                    if(s == INVALID_SOCKET) {
                        *count = 0;
                        return;
                    }


                    do {
                        buf = realloc(buf,size);
                        if(!buf) {
                            *count = 0;
                            return;
                        }
                        res = WSAIoctl(s,SIO_GET_INTERFACE_LIST,0,0,buf,size,&returned,0,0);  
                        size *= 2;
                    } while(res == SOCKET_ERROR && WSAGetLastError() == WSAENOBUFS);

                    if(res == SOCKET_ERROR) {
                        closesocket(s);
                        *count = 0;
                        if(buf) free(buf);
                        return;
                    }

                    closesocket(s);


                    *count = returned/sizeof(INTERFACE_INFO);

                    list = (INTERFACE_INFO*)buf;
                    for(int i=0;i<min(*count,100);++i) {    
                        IPS[i] = list->iiAddress.AddressIn.sin_addr.s_addr;
                        ++list;
                    }

                    if(buf) free(buf);

                }
#else // This requires SP4
                void GetIpInfo(unsigned char Name[MAX_SIZE],unsigned long IPS[100],unsigned long Speeds[100], unsigned long *count) {
                    MIB_IPADDRTABLE *IpAddrTable=0;
                    MIB_IFROW IfRow;
                    DWORD size = 0;
                    DWORD i;

                    Name[0]=0;
                    gethostname((char*)Name,MAX_SIZE);
                    *count = 0;

                    GetIpAddrTable(IpAddrTable,&size,TRUE);
                    if(size >0) {
                        IpAddrTable = (MIB_IPADDRTABLE*)alloca(size);
                        if(!IpAddrTable) {
                            return ;
                        }
                        if(GetIpAddrTable(IpAddrTable,&size,TRUE) != NO_ERROR) {
                            AddToMessageLog(IDM_NOTIFY,GetLastError(),"GetIpAddrTable() (2)");
                            return;
                        }
                    } else {
                        AddToMessageLog(IDM_NOTIFY,GetLastError(),"GetIpAddrTable() (1)");
                        return;
                    }
                    if(IpAddrTable->dwNumEntries>100)
                        IpAddrTable->dwNumEntries = 100;

                    //*count = IpAddrTable->dwNumEntries;
                    for(i=0;i<IpAddrTable->dwNumEntries;++i) {
                        if(!IpAddrTable->table[i].dwAddr) continue;
                        IPS[i] = IpAddrTable->table[i].dwAddr;
                        IfRow.dwIndex = IpAddrTable->table[i].dwIndex;
                        GetIfEntry(&IfRow);
                        Speeds[i] = IfRow.dwSpeed;
                    }
                    *count = i;

                }
#endif
                error_status_t R_GetSystemInfo( handle_t IDL_handle,R_MACHINE_INFO __RPC_FAR *lpMachineInfo) {
                    memcpy(lpMachineInfo,&MachineInfo,sizeof(MachineInfo));
                    return RPC_S_OK;
                }

                void GetMachineData() {
                    MEMORYSTATUS memStat={sizeof(MEMORYSTATUS)};
                    SYSTEM_INFO  SysInfo;
                    OSVERSIONINFO VersionInfo={sizeof(VersionInfo)};
                    LARGE_INTEGER Frequ;

                    //DBG("Entering GetMachineData()");
                    QueryPerformanceFrequency(&Frequ);
                    GlobalMemoryStatus(&memStat);
                    GetSystemInfo(&SysInfo);
                    GetVersionEx(&VersionInfo);

                    sprintf((char*)MachineInfo.ServerString,"%s Ver.: %d.%s",SERVER_STRING,VERSION,SUBVERSION);
                    memcpy(&MachineInfo.OS,&VersionInfo.dwMajorVersion,sizeof(VersionInfo)-sizeof(DWORD));
                    MachineInfo.HW.dwTotalPhysMem = memStat.dwTotalPhys;
                    MachineInfo.HW.dwNumberOfProcessors = SysInfo.dwNumberOfProcessors;
                    MachineInfo.HW.dwActiveProcessorMask = SysInfo.dwActiveProcessorMask;
                    MachineInfo.HW.wProcessorArchitecture = SysInfo.wProcessorArchitecture;
                    MachineInfo.HW.wProcessorLevel = SysInfo.wProcessorLevel; 
                    MachineInfo.HW.wProcessorRevision = SysInfo.wProcessorRevision; 
                    MachineInfo.HW.Mhz = (DWORD)(Frequ.QuadPart / 1000000);
#if 0
                    GetIpInfo(MachineInfo.IP.Hostname_ip,MachineInfo.IP.IPS,&MachineInfo.IP.NumEntries);
#else
                    GetIpInfo(MachineInfo.IP.Hostname_ip,MachineInfo.IP.IPS,MachineInfo.IP.Speeds,&MachineInfo.IP.NumEntries);
#endif
                    //DBG("Leaving GetMachineData()");
                }

                int InitLockService() {

                    HMODULE hKernel;
                    NET_API_STATUS res;
                    DBG("Entering InitLockService()");
                    hKernel = GetModuleHandle("kernel32.dll");
                    if(hKernel) {
                        ProcessId2SessionId=(BOOL(WINAPI *)(DWORD,DWORD*))GetProcAddress(hKernel,"ProcessIdToSessionId");
                    }

                    DBG("Loading psapi.dll");
                    hInstLib = LoadLibraryA( "PSAPI.DLL" ) ;         
                    if( hInstLib == NULL ) {
                        AddToMessageLog(IDM_LOADLIBRARY,GetLastError(),"PSAPI.DLL");
                        DBG("Could not load psapi.dll\nLeaving InitLockService()");
                        return -1;         
                    }
                    DBG("Querying EnumProcesses");
                    lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
                        GetProcAddress( hInstLib, "EnumProcesses" ) ;
                    if( lpfEnumProcesses == NULL) {
                        AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"EnumProcesses");
                        DBG("Could not query EnumProcessModules()\nLeaving InitLockService()");
                        FreeLibrary( hInstLib );
                        return -1;
                    }

                    DBG("Querying EnumProcessModules");
                    lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,DWORD, LPDWORD)) 
                        GetProcAddress( hInstLib,"EnumProcessModules" ) ;

                    if( lpfEnumProcessModules == NULL) {
                        AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"EnumProcessModules");   
                        FreeLibrary( hInstLib );
                        DBG("Could not query EnumProcessModules()\nLeaving InitLockService()");

                        return -1;
                    }
                    DBG("Querying GetModuleFileNameExA");
                    lpfGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE,LPTSTR, DWORD )) 
                        GetProcAddress( hInstLib,"GetModuleFileNameExA" ) ;
                    if( lpfGetModuleFileNameEx == NULL) {
                        AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"GetModuleFileNameExA");
                        FreeLibrary( hInstLib );
                        DBG("Could not query GetModuleFileNameExA()\nLeaving InitLockService()");
                        return -1;
                    }

                    //DBG("Calling NetGetDCName()");
                    res=NetGetDCName( NULL, NULL, (LPBYTE*)&wszDCName );
                    if(res != NERR_Success || !wszDCName) {
                        wszDCName = 0;
                        AddToMessageLog(IDM_GET_DCNAME,res,0);
                        DBG("NetGetDCName failed with "<<res);
                    }
                    /*
                    DBG("Calling CreateSidFromRid");
                    DomainAdmins = CreateSidFromRid(wszDCName,DOMAIN_GROUP_RID_ADMINS);
                    if(!DomainAdmins) {
                    AddToMessageLog(IDM_SIDLOOKUP,GetLastError(),"domain admin");
                    DBG("CreateSidFromRid failed");
                    } else
                    DBG("Calling LookupAliasFromRid");
                    LocalAdmins = LookupAliasFromRid(DOMAIN_ALIAS_RID_ADMINS);
                    if(!LocalAdmins) {
                    AddToMessageLog(IDM_SIDLOOKUP,GetLastError(),"local admin");
                    DBG("LookupAliasFromRid failed");
                    }
                    */
                    //DBG("Initializing critical section");
                    InitializeCriticalSection(&lockCS);
                    //DBG("Calling GetMachineData");
                    GetMachineData();
                    //DBG("Leaving InitLockService()");
                    return 0;
                }

                void StopLockService() {
                    DBG("Entering StopLockService()");
                    FreeLibrary(hInstLib);
                    if(LocalAdmins) 
                        FreeSid(LocalAdmins);
                    if(DomainAdmins) 
                        HeapFree(GetProcessHeap(), 0,DomainAdmins);
                    if(wszDCName) NetApiBufferFree(wszDCName);
                    if(LockSID) {
                        free(LockSID);
                        LockSID = 0;
                    }

                    DeleteCriticalSection(&lockCS);
                    DBG("Leaving StopLockService()");
                    //delete sock;
                }

}


//
//  FUNCTION: Ping
//
//  PURPOSE: Implements the Ping() operation.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    Ping() operation defined in rpcsvc.idl.
//    Used by clients to test the connection.
//

error_status_t
Ping(
    handle_t h
    )
{
	
    DBG("Ping called");
    return(0);
}

