
#include <winsock2.h>
#include <wtypes.h>
#include <winbase.h>

#include <fstream>
#include <deque>


#include "cluma.h"
#include "service.h"
#include "Environment.h"
#include "helpers.h"
#include "insocket.h"
#include "update.h"
#include "messages.h"


#ifndef CREATE_WITH_USERPROFILE
#define CREATE_WITH_USERPROFILE     0x02000000
#endif



extern "C" {

extern BOOL NoUser;


std::ofstream *o=0;

struct ProcInfo {
	HANDLE hUser;
	HANDLE hProfile;
	HANDLE hProcess;
	SOCKET in,err;
};

typedef std::deque<struct ProcInfo*> ProcQueue;

static HANDLE *Processes;
static ProcInfo **ProcInfos;
static DWORD dwNumProcs=0;
static BOOL bStop;
static CRITICAL_SECTION CS;
static ProcQueue NewProcs;
static HANDLE ThreadEvent,ThreadHandle;

DWORD WINAPI ThreadFunc(LPVOID Param);

void InitLogging(char *Filename) {
	char datetime[255];
	GetDateTimeString(datetime);
    DBG(datetime<<":  Reopening logfile "<<Filename);
    if(!o) o = new std::ofstream;
    else o->close();
    o->open(Filename,std::ios::out);
    if(!*o) {
	delete o;
	o = 0;
    }
    DBG(datetime<<":Logfile "<<Filename<<" opened");
#ifdef _DEBUG
	DBG("Service DEBUG-Version "<<VERSION<<"."<<SUBVERSION)
#else
	DBG("Service Release-Version "<<VERSION<<"."<<SUBVERSION)
#endif
}

void CloseLogFile() {
	char datetime[255];
	GetDateTimeString(datetime);
    DBG(datetime<<":Closing Logfile");
    if(o) {
	o->close();
	delete o;
	o=0;
    }
}

BOOL InitProcessManagement(void) {
	DWORD id;
	
	DBG("Entering InitProcessManagement");
	if(!AllocConsole()) {
	    AddToMessageLog(IDM_NOTIFY,GetLastError(),"AllocConsole() failed");
	}

	DBG("Initializing...\n");
	InitializeCriticalSection(&CS);
	ThreadEvent=CreateEvent(0,FALSE,FALSE,0);

	if(!ThreadEvent) {
		AddToMessageLog(IDM_EVENT,GetLastError(),0);
		DBG("CreateEvent failed");
		return FALSE;
	}
	bStop = FALSE;
	ThreadHandle=CreateThread(0,0,ThreadFunc,0,0,&id);
	if(!ThreadHandle) {
		AddToMessageLog(IDM_THREAD,GetLastError(),0);	
		DBG("CreateThread failed");
		CloseHandle(ThreadEvent);
		ThreadEvent=0;
		return FALSE;
	}
	
	WORD wVersionRequested; 
	WSADATA wsaData; 
	int err;
	wVersionRequested=2;
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err) {
	    AddToMessageLog(IDM_WSAStartup,err,0);
	    CloseHandle(ThreadEvent);
	    CloseHandle(ThreadHandle);
	    DBG("WSAStartup failed with "<<err);
	    return FALSE;

	}

	AllowAccessToDesktop();
	DBG("Leaving InitProcessManagement");
	return InitEnvironment();
}

BOOL ShutdownProcessManagement() {
    DBG("Entering ShutdownProcessManagement");
    if(ThreadEvent) {
	DBG("Stopping thread");
	bStop = TRUE;
	SetEvent(ThreadEvent);
	WaitForSingleObject(ThreadHandle,INFINITE);
	CloseHandle(ThreadEvent);
	CloseHandle(ThreadHandle);
	DBG("Thread stopped");
    }
    DeleteCriticalSection(&CS);
    DBG("Leaving ShutdownProcessManagement");
    return FreeEnvironment();
    WSACleanup();
}

DWORD WINAPI ThreadFunc(LPVOID Param) {
    DWORD numHandles,res,newSize;
    DWORD memsize,error;
    char msg[50];
    numHandles = 1;
    memsize = 10;
    ProcInfo *CurProc;
    Processes = (HANDLE*)malloc(memsize * sizeof(HANDLE));
    ProcInfos  = (ProcInfo**)malloc(memsize * sizeof(ProcInfo*));
    Processes[0] = ThreadEvent;
    while(!bStop) {
	res=WaitForMultipleObjects(numHandles,Processes,FALSE,INFINITE);
	if(bStop) return 0;
	if(res == WAIT_FAILED) {
	    /* How to handle this ? */
	    error = GetLastError();
	    sprintf(msg,"NumHandles: %d",numHandles);
	    AddToMessageLog(IDM_WAIT,error,msg);
	    ServiceStop();
	    return -1;
	    //continue;
	}
	res -= WAIT_OBJECT_0;
	if(res) {
	    CurProc = ProcInfos[res];
#ifdef _DEBUG
	    DWORD exitCode;
	    GetExitCodeProcess(CurProc->hProcess,&exitCode);
	    DBG("Process "<<res<<" finished with "<<(int)exitCode)
#endif
	    shutdown(CurProc->in,SD_BOTH);
	    shutdown(CurProc->err,SD_BOTH);
	    closesocket(CurProc->in);
	    closesocket(CurProc->err);
	    
	    if(CurProc->hProfile) {
		EnterCriticalSection(&CS);
		UnloadUserProfile(CurProc->hUser,CurProc->hProfile);
		LeaveCriticalSection(&CS);
	    }
	    
	    CloseHandle(CurProc->hProcess);
	    CloseHandle(CurProc->hUser);
	    free(CurProc);
	    if(res<numHandles-1) {
		memmove(Processes+res,Processes+res+1,(numHandles-res-1)*sizeof(HANDLE));
		memmove(ProcInfos+res,ProcInfos+res+1,(numHandles-res-1)*sizeof(ProcInfo*));
	    }
	    numHandles--;
	}
	
	EnterCriticalSection(&CS);
	if(NewProcs.size()) {
	    newSize=numHandles + NewProcs.size();
	    DBG(NewProcs.size()<<" new processes started. Num now is: "<<newSize-1)
	    if(memsize<newSize) {
		memsize = max(2*memsize,newSize);
		Processes = (HANDLE*)realloc(Processes,memsize*sizeof(HANDLE));
		ProcInfos  = (ProcInfo**)realloc(Processes,memsize * sizeof(ProcInfo*));
	    }
	    while(!NewProcs.empty()) {
		ProcInfos[numHandles] = NewProcs.front();
		Processes[numHandles] = ProcInfos[numHandles]->hProcess;
		NewProcs.pop_front();
		numHandles++;
	    }
	}
	LeaveCriticalSection(&CS);
    }
    return 0;
}


error_status_t R_CreateProcessNoUser( 
									  /* [in] */ handle_t Binding,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpApplicationName,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCommandLine,
    /* [in] */ unsigned long dwCreationFlags,
    /* [unique][in] */ byte __RPC_FAR *lpEnvironment,
    /* [in] */ unsigned long EnvSize,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCurrentDirectory,
    /* [ref][in] */ R_STARTUPINFO __RPC_FAR *lpStartupInfo,
    /* [ref][out] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation) {


	DBG("R_CreateProcessNoUser called with " << lpCommandLine);
	/* check if function call without user authentication is switched on */
	if (!NoUser){
		DBG("access denied, no-user feature is switched off");
		return ERROR_LOGON_NOT_GRANTED;
	}


	ProcInfo *pInfo;
	
	inSocket in;
	inSocket err;
	unsigned char host[MAX_SIZE], *oldAppName,*oldCommandLine;
	char MyName[MAX_PATH+20];
	DWORD Namelen;
	error_status_t stat;
	DWORD error;
	BOOL local;
	TOKEN_USER *pTokenUser;
	HANDLE User=0;
	HANDLE hProfile;
	BOOL result;
	UserInfo UI;
	DWORD attribs;
	HANDLE dirH;
	char Dir[1024],*Environment;
	unsigned char *OldD;
	BOOL Update = FALSE;

	

	DWORD UserSize=256,DomainSize=256,DirSize=1024;
	char DomainName[256],UserName[256];
	SID_NAME_USE NameUse;
	stat = RpcImpersonateClient(Binding);

	if(stat != RPC_S_OK) {
	    DBG("Impersonate client failed "<<stat)
	    return stat;
	}
	DBG("Impersonated client")

	if(LockSID && !CompareUser(LockSID,&error)) {
		DBG("Lock check failed "<<error)
		RpcRevertToSelf();
		return error;
	}
	
	pTokenUser=GetActualUserToken(&error);
	if(!pTokenUser) {
		DBG("Could not get actual user token "<<error)
		RpcRevertToSelf();
		return error;
	}

	if(!LookupAccountSid(0,pTokenUser->User.Sid,UserName,&UserSize,DomainName,&DomainSize,&NameUse)) {
	    error = GetLastError();	
	    DBG("LookupAccountSid failed "<<error)
		free(pTokenUser);
		RpcRevertToSelf();
		return error;
	}
	free(pTokenUser);
	char DateTime[25];
	DBG(GetDateTimeString(DateTime)<<": Remote user is: "<<DomainName<<"/"<<UserName)

	pTokenUser = GetLoggedOnUserToken(&User);
	if(pTokenUser) {
	    local = CompareUser(pTokenUser->User.Sid,&error);
	    free(pTokenUser);
	} else {
	    local = FALSE;
	}
	
	if(!local && User) {
	    CloseHandle(User);
	    User = 0;
	}

	DBG("Local is: "<<local)
	Namelen=255;

	DBGOUT("R_GetClientName("<<Binding<<","<<host<<","<<Namelen<<")")
	stat=R_GetClientName(Binding,host,&Namelen);

	
	if(stat != RPC_S_OK) {
		RpcRevertToSelf();
		DBG("Could not query client name "<<stat)
		return stat;
	}
	DBG("Remote client is: "<<host)
	try {
		in.create();
		in.bind();
		in.connect((char*)host,lpStartupInfo->hStdInput);//Port of socket of mpiexec or RexecShell
		err.create();
		err.bind();
		err.connect((char*)host,lpStartupInfo->hStdError);//Port of socket of mpiexec or RexecShell
	} catch (socketException &e) {
		error = WSAGetLastError();
		DBG("Could not create sockets "<<(const char*)e);
		in.close();
		err.close();
		AddToMessageLog(IDM_SOCKET,ERROR_SUCCESS,(char*)e);
		RpcRevertToSelf();
		return error;
	}
	
	if(!SetHandleInformation((HANDLE)(int)err,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
	    AddToMessageLog(IDM_SET_HANDLE,GetLastError(),"err");
	if(!SetHandleInformation((HANDLE)(int)in,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
	    AddToMessageLog(IDM_SET_HANDLE,GetLastError(),"in");

	lpStartupInfo->hStdInput=(int)in;//Get handle to socket to front-end for input redirection
	lpStartupInfo->hStdOutput = (int)in;//Get handle to socket to front-end for stdout
	lpStartupInfo->hStdError = (int)err;//Get handle to socket to front-end for stderr


	NormalizeDomainname((char*)DomainName);
	error = ERROR_SUCCESS;
	RpcRevertToSelf();
	if(!local) {
		DBG("LogonUser ("<<UserName<<","<<DomainName<<", Password,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&User))")
	    if(!LogonUser(UserName,DomainName,(char*)lpStartupInfo->lpPassword,LOGON32_LOGON_INTERACTIVE,
		LOGON32_PROVIDER_DEFAULT,&User)) {
		error = GetLastError();
		DBG("LogonUser failed with "<<error)
		AddToMessageLog(IDM_LOGON,error,UserName);
		in.close();
		err.close();
		return error;
	    }
	    
	    DBG("User logged on successfully")
	} else {
	    DBG("Duplicating token...")
	    result = GetPrimaryToken(&User);
	    if(result != ERROR_SUCCESS) {
		DBG("Failed with "<<result);
		in.close();
		err.close();
		return result;
	    }
	}
	if(dwCreationFlags & CREATE_WITH_USERPROFILE) {
	    EnterCriticalSection(&CS);
	    hProfile=MyLoadUserProfile(User,(char*)UserName,DomainName,UI);
	    LeaveCriticalSection(&CS);
	    dwCreationFlags &= ~CREATE_WITH_USERPROFILE;
	} else {
	    UI.LoadSystemEnvironment();
	    hProfile = 0;
	}
	if(!ImpersonateLoggedOnUser(User)) {
		error = GetLastError();
		DBG("Could not imperonate logged on user "<<error)
		AddToMessageLog(IDM_IMPERSONATE,error,UserName);
		in.close();
		err.close();
		return error;
	}
	DBG("Impersonated user");



	// Check if WD is valid
	if(lpCurrentDirectory) {
		attribs=GetFileAttributes((const char*)lpCurrentDirectory);
		dirH=CreateFile((const char*)lpCurrentDirectory,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,
			   OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);

		if((attribs&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY||dirH==INVALID_HANDLE_VALUE) {
			UI.GetEnvString("SystemRoot",Dir,&DirSize);
			if(!Dir[0]) strcpy(Dir,"C:\\");
		} else {
			strncpy(Dir,(const char*)lpCurrentDirectory,1024);
			Dir[1023]=0;
		}
		CloseHandle(dirH);
	} else {
		UI.GetEnvString("SystemRoot",Dir,&DirSize);
		if(!Dir[0]) strcpy(Dir,"C:\\");
	}

	if(lpCommandLine && !strcmp((const char*)lpCommandLine,MAGIC_UPDATE_STRING)) {
	    DBG("Starting update of myself");
	    Update = TRUE;
	    oldAppName = lpApplicationName;
	    oldCommandLine = lpCommandLine;
	    if(!GetModuleFileName(0,MyName,MAX_PATH)) {
		error = GetLastError();
		DBG("GetModuleFileName failed "<<error)
		in.close();
		err.close();
		RevertToSelf();
		return error;
	    }
	    lpApplicationName = 0;//(unsigned char*)MyName;
	    strcat(MyName," -update .\\rcluma.new");
	    lpCommandLine = (unsigned char*)&MyName[0];
	    strcpy(Dir,".");
	}

	DBG("Using WD "<<Dir);

	OldD = lpStartupInfo->lpDesktop;
	lpStartupInfo->lpDesktop = (unsigned char*)(local?"winsta0\\default":0);
	lpStartupInfo->dwFlags |= STARTF_USESTDHANDLES;
	
	UI.MergeEnvironment((char*)(lpEnvironment));
	Environment = UI.GetEnv();
	DBG("Calling CreateProcess("<<(void*)lpApplicationName<<","
	    <<(char*)lpCommandLine<<","<<NULL<<","<<NULL<<","<<TRUE<<","
	    <<dwCreationFlags<<","<<(void*)Environment<<","<<Dir<<","
	    <<(void*)lpStartupInfo<<","<<(void*)lpProcessInformation<<")")
		
//SI//23-08-2004: check with if, failed on lpDesktop invalid
    if ((lpStartupInfo) && (((STARTUPINFO*)lpStartupInfo)->hStdOutput))
		DBG("Startupinfo hStdOutput: "<<(WORD)((STARTUPINFO*)lpStartupInfo)->hStdOutput)
	if ((lpStartupInfo) && (((STARTUPINFO*)lpStartupInfo)->lpDesktop))
	    DBG("Startupinfo lpDesktop: "<<(char*)((STARTUPINFO*)lpStartupInfo)->lpDesktop)

    result=CreateProcess((char*)lpApplicationName,(char*)lpCommandLine,NULL,NULL,TRUE,
		dwCreationFlags,Environment,Dir,(STARTUPINFO*)lpStartupInfo,
		(PROCESS_INFORMATION*)lpProcessInformation);
	           
/*	result=CreateProcessAsUser(User,(char*)lpApplicationName,(char*)lpCommandLine,NULL,NULL,TRUE,
		dwCreationFlags,Environment,Dir,(STARTUPINFO*)lpStartupInfo,
		(PROCESS_INFORMATION*)lpProcessInformation);*/


	DBG("CreateProcess returned "<<result)
	DBG("new Process ID: "<<(WORD)(PROCESS_INFORMATION*)lpProcessInformation->dwProcessId<<" handle: "
	      <<(WORD)(PROCESS_INFORMATION*)lpProcessInformation->hProcess )
	lpStartupInfo->lpDesktop = OldD;
	if(!result) {
		error = GetLastError();
		DBG("Error is "<<error)
		EnterCriticalSection(&CS);
		if(hProfile) UnloadUserProfile(User,hProfile);
		LeaveCriticalSection(&CS);
		CloseHandle(User);
		in.close();
		err.close();
		AddToMessageLog(IDM_START_PROC,error,(char* const)lpCommandLine);
		//AddToMessageLog(IDM_NOTIFY,error,"Could not create process");
		RevertToSelf();
		return error;
	}

	if(Update) {
	    lpApplicationName = oldAppName;
	    lpCommandLine = oldCommandLine;
	}

	CloseHandle((HANDLE)lpProcessInformation->hThread);
	pInfo = (ProcInfo*)malloc(sizeof(ProcInfo));
	pInfo->hProcess = (HANDLE)lpProcessInformation->hProcess;
	pInfo->hProfile = hProfile;
	pInfo->hUser = User;
	pInfo->in = (const int)in;
	pInfo->err = (const int)err;
	EnterCriticalSection(&CS);
	NewProcs.push_back(pInfo);
	LeaveCriticalSection(&CS);
	SetEvent(ThreadEvent);
	RevertToSelf();
	DBG("Leaving CreateProcess");
	return RPC_S_OK;
}

error_status_t R_CreateProcess( 
    /* [in] */ handle_t Binding,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpApplicationName,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCommandLine,
    /* [in] */ unsigned long dwCreationFlags,
    /* [unique][in] */ byte __RPC_FAR *lpEnvironment,
    /* [in] */ unsigned long EnvSize,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCurrentDirectory,
    /* [ref][in] */ R_STARTUPINFO __RPC_FAR *lpStartupInfo,
    /* [ref][out] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation) {


	ProcInfo *pInfo;
	
	inSocket in;
	inSocket err;
	unsigned char host[MAX_SIZE], *oldAppName,*oldCommandLine;
	char MyName[MAX_PATH+20];
	DWORD Namelen;
	error_status_t stat;
	DWORD error;
	BOOL local;
	TOKEN_USER *pTokenUser;
	HANDLE User=0;
	HANDLE hProfile;
	BOOL result;
	UserInfo UI;
	DWORD attribs;
	HANDLE dirH;
	char Dir[1024],*Environment;
	unsigned char *OldD;
	BOOL Update = FALSE;

	DBG("R_CreateProcess called with " << lpCommandLine);

	DWORD UserSize=256,DomainSize=256,DirSize=1024;
	char DomainName[256],UserName[256];
	SID_NAME_USE NameUse;
	stat = RpcImpersonateClient(Binding);

	if(stat != RPC_S_OK) {
	    DBG("Impersonate client failed "<<stat)
	    return stat;
	}
	DBG("Impersonated client")

	if(LockSID && !CompareUser(LockSID,&error)) {
		DBG("Lock check failed "<<error)
		RpcRevertToSelf();
		return error;
	}
	
	pTokenUser=GetActualUserToken(&error);
	if(!pTokenUser) {
		DBG("Could not get actual user token "<<error)
		RpcRevertToSelf();
		return error;
	}

	if(!LookupAccountSid(0,pTokenUser->User.Sid,UserName,&UserSize,DomainName,&DomainSize,&NameUse)) {
	    error = GetLastError();	
	    DBG("LookupAccountSid failed "<<error)
		free(pTokenUser);
		RpcRevertToSelf();
		return error;
	}
	free(pTokenUser);
	char DateTime[25];
	DBG(GetDateTimeString(DateTime)<<": Remote user is: "<<DomainName<<"/"<<UserName)

	pTokenUser = GetLoggedOnUserToken(&User);
	if(pTokenUser) {
	    local = CompareUser(pTokenUser->User.Sid,&error);
	    free(pTokenUser);
	} else {
	    local = FALSE;
	}
	
	if(!local && User) {
	    CloseHandle(User);
	    User = 0;
	}

	DBG("Local is: "<<local)
	Namelen=255;

	DBGOUT("R_GetClientName("<<Binding<<","<<host<<","<<Namelen<<")")
	stat=R_GetClientName(Binding,host,&Namelen);

	
	if(stat != RPC_S_OK) {
		RpcRevertToSelf();
		DBG("Could not query client name "<<stat)
		return stat;
	}
	DBG("Remote client is: "<<host)
	try {
		in.create();
		in.bind();
		in.connect((char*)host,lpStartupInfo->hStdInput);//Port of socket of mpiexec or RexecShell
		err.create();
		err.bind();
		err.connect((char*)host,lpStartupInfo->hStdError);//Port of socket of mpiexec or RexecShell
	} catch (socketException &e) {
		error = WSAGetLastError();
		DBG("Could not create sockets "<<(const char*)e);
		in.close();
		err.close();
		AddToMessageLog(IDM_SOCKET,ERROR_SUCCESS,(char*)e);
		RpcRevertToSelf();
		return error;
	}
	
	if(!SetHandleInformation((HANDLE)(int)err,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
	    AddToMessageLog(IDM_SET_HANDLE,GetLastError(),"err");
	if(!SetHandleInformation((HANDLE)(int)in,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
	    AddToMessageLog(IDM_SET_HANDLE,GetLastError(),"in");

	lpStartupInfo->hStdInput=(int)in;//Get handle to socket to front-end for input redirection
	lpStartupInfo->hStdOutput = (int)in;//Get handle to socket to front-end for stdout
	lpStartupInfo->hStdError = (int)err;//Get handle to socket to front-end for stderr


	NormalizeDomainname((char*)DomainName);
	error = ERROR_SUCCESS;
	RpcRevertToSelf();
	if(!local) {
		DBG("LogonUser ("<<UserName<<","<<DomainName<<", Password,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&User))")
	    if(!LogonUser(UserName,DomainName,(char*)lpStartupInfo->lpPassword,LOGON32_LOGON_INTERACTIVE,
		LOGON32_PROVIDER_DEFAULT,&User)) {
		error = GetLastError();
		DBG("LogonUser failed with "<<error)
		AddToMessageLog(IDM_LOGON,error,UserName);
		in.close();
		err.close();
		return error;
	    }
	    
	    DBG("User logged on successfully")
	} else {
	    DBG("Duplicating token...")
	    result = GetPrimaryToken(&User);
	    if(result != ERROR_SUCCESS) {
		DBG("Failed with "<<result);
		in.close();
		err.close();
		return result;
	    }
	}
	if(dwCreationFlags & CREATE_WITH_USERPROFILE) {
	    EnterCriticalSection(&CS);
	    hProfile=MyLoadUserProfile(User,(char*)UserName,DomainName,UI);
	    LeaveCriticalSection(&CS);
	    dwCreationFlags &= ~CREATE_WITH_USERPROFILE;
	} else {
	    UI.LoadSystemEnvironment();
	    hProfile = 0;
	}
	if(!ImpersonateLoggedOnUser(User)) {
		error = GetLastError();
		DBG("Could not imperonate logged on user "<<error)
		AddToMessageLog(IDM_IMPERSONATE,error,UserName);
		in.close();
		err.close();
		return error;
	}
	DBG("Impersonated user");



	// Check if WD is valid
	if(lpCurrentDirectory) {
		attribs=GetFileAttributes((const char*)lpCurrentDirectory);
		dirH=CreateFile((const char*)lpCurrentDirectory,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,
			   OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);

		if((attribs&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY||dirH==INVALID_HANDLE_VALUE) {
			UI.GetEnvString("SystemRoot",Dir,&DirSize);
			if(!Dir[0]) strcpy(Dir,"C:\\");
		} else {
			strncpy(Dir,(const char*)lpCurrentDirectory,1024);
			Dir[1023]=0;
		}
		CloseHandle(dirH);
	} else {
		UI.GetEnvString("SystemRoot",Dir,&DirSize);
		if(!Dir[0]) strcpy(Dir,"C:\\");
	}

	if(lpCommandLine && !strcmp((const char*)lpCommandLine,MAGIC_UPDATE_STRING)) {
	    DBG("Starting update of myself");
	    Update = TRUE;
	    oldAppName = lpApplicationName;
	    oldCommandLine = lpCommandLine;
	    if(!GetModuleFileName(0,MyName,MAX_PATH)) {
		error = GetLastError();
		DBG("GetModuleFileName failed "<<error)
		in.close();
		err.close();
		RevertToSelf();
		return error;
	    }
	    lpApplicationName = 0;//(unsigned char*)MyName;
	    strcat(MyName," -update .\\rcluma.new");
	    lpCommandLine = (unsigned char*)&MyName[0];
	    strcpy(Dir,".");
	}

	DBG("Using WD "<<Dir);

	OldD = lpStartupInfo->lpDesktop;
	lpStartupInfo->lpDesktop = (unsigned char*)(local?"winsta0\\default":0);
	lpStartupInfo->dwFlags |= STARTF_USESTDHANDLES;
	
	UI.MergeEnvironment((char*)(lpEnvironment));
	Environment = UI.GetEnv();
	DBG("Calling CreateProcessAsUser("<<User<<","<<(void*)lpApplicationName<<","
	    <<(char*)lpCommandLine<<","<<NULL<<","<<NULL<<","<<TRUE<<","
	    <<dwCreationFlags<<","<<(void*)Environment<<","<<Dir<<","
	    <<(void*)lpStartupInfo<<","<<(void*)lpProcessInformation<<")")
		
//SI//23-08-2004: check with if, failed on lpDesktop invalid
    if ((lpStartupInfo) && (((STARTUPINFO*)lpStartupInfo)->hStdOutput))
		DBG("Startupinfo hStdOutput: "<<(WORD)((STARTUPINFO*)lpStartupInfo)->hStdOutput)
	if ((lpStartupInfo) && (((STARTUPINFO*)lpStartupInfo)->lpDesktop))
	    DBG("Startupinfo lpDesktop: "<<(char*)((STARTUPINFO*)lpStartupInfo)->lpDesktop)
	           
	result=CreateProcessAsUser(User,(char*)lpApplicationName,(char*)lpCommandLine,NULL,NULL,TRUE,
		dwCreationFlags,Environment,Dir,(STARTUPINFO*)lpStartupInfo,
		(PROCESS_INFORMATION*)lpProcessInformation);


	DBG("CreateProcessAsUser returned "<<result)
	DBG("new Process ID: "<<(WORD)(PROCESS_INFORMATION*)lpProcessInformation->dwProcessId<<" handle: "
	      <<(WORD)(PROCESS_INFORMATION*)lpProcessInformation->hProcess )
	lpStartupInfo->lpDesktop = OldD;
	if(!result) {
		error = GetLastError();
		DBG("Error is "<<error)
		EnterCriticalSection(&CS);
		if(hProfile) UnloadUserProfile(User,hProfile);
		LeaveCriticalSection(&CS);
		CloseHandle(User);
		in.close();
		err.close();
		AddToMessageLog(IDM_START_PROC,error,(char* const)lpCommandLine);
		//AddToMessageLog(IDM_NOTIFY,error,"Could not create process");
		RevertToSelf();
		return error;
	}

	if(Update) {
	    lpApplicationName = oldAppName;
	    lpCommandLine = oldCommandLine;
	}

	CloseHandle((HANDLE)lpProcessInformation->hThread);
	pInfo = (ProcInfo*)malloc(sizeof(ProcInfo));
	pInfo->hProcess = (HANDLE)lpProcessInformation->hProcess;
	pInfo->hProfile = hProfile;
	pInfo->hUser = User;
	pInfo->in = (const int)in;
	pInfo->err = (const int)err;
	EnterCriticalSection(&CS);
	NewProcs.push_back(pInfo);
	LeaveCriticalSection(&CS);
	SetEvent(ThreadEvent);
	RevertToSelf();
	DBG("Leaving CreateProcess");
	return RPC_S_OK;
}

static DWORD KillProc(HANDLE hProcess,DWORD RetValue) {
    	DWORD ExitCode;

	if(!InjectExitProc(hProcess,RetValue))
	    return GetLastError();
	ExitCode=WaitForSingleObject(hProcess,5000);
	if(ExitCode==WAIT_TIMEOUT) {
	    if(!TerminateProcess(hProcess,RetValue))
		return GetLastError();
	}
	return ERROR_SUCCESS;
}

error_status_t R_TerminateProcess( 
    /* [in] */ handle_t Binding,
    /* [in, ref] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation,
    /* [in] */ unsigned long RetValue) {


	DWORD ExitCode;
	error_status_t state;
	
	if(!GetExitCodeProcess((HANDLE)lpProcessInformation->hProcess,&ExitCode)) {
		return RPC_S_INVALID_ARG;
	}
	
	state = RpcImpersonateClient(Binding);
	if(state != RPC_S_OK) {
		return RPC_S_ACCESS_DENIED;
	}
	PostThreadMessage(lpProcessInformation->dwThreadId,WM_QUIT,1,0);
	ExitCode=WaitForSingleObject((HANDLE)lpProcessInformation->hProcess,2000);
	
	if(ExitCode==WAIT_TIMEOUT) 
	    ExitCode = KillProc((HANDLE)lpProcessInformation->hProcess,RetValue);
	else ExitCode = RPC_S_OK;


	RpcRevertToSelf();
	return ExitCode;
}


error_status_t R_KillProcess( /* [in] */ handle_t Binding,
			      /* [in] */ unsigned long id) {
	error_status_t state;
	HANDLE hProcess;

	state = RpcImpersonateClient(Binding);
	if(state != RPC_S_OK) {
	    return RPC_S_ACCESS_DENIED;
	}
	hProcess = OpenProcess(PROCESS_TERMINATE |PROCESS_CREATE_THREAD|
	                       PROCESS_VM_OPERATION|PROCESS_VM_WRITE|
			       PROCESS_VM_READ, FALSE, id ) ;

	if(hProcess) {
	    state = KillProc(hProcess,0);
	    CloseHandle(hProcess);
	} else state = GetLastError();
	RpcRevertToSelf();
	return state;
}


error_status_t R_ShutDown( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ long restart) {
	error_status_t state = 0;
     
	state = RpcImpersonateClient(IDL_handle); 


	if (restart)
		AddToMessageLog(IDM_SHUTDOWN,0,(char* const)"try to reboot computer");
	else
		AddToMessageLog(IDM_SHUTDOWN,0,(char* const)"try to shut down computer");

   RpcRevertToSelf();

	state = ShutDown(restart); 
	return state;
}

}