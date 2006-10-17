#include <winsock2.h>
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <iostream>

#pragma hdrstop

#include "cluma.h"
#include "insocket.h"
#include "RexecClient.h"
#include "plugin.h"
//#include "netstate.h"
#include "Environment.h"
#include "Encryption.h"
#include "update.h"
#include "mpirun.h"

// needed to convert c and c++ calling conventions
typedef void (__stdcall * fnptr)(char *,unsigned long,byte ** ,unsigned long *);
typedef void (__stdcall * fnptr2)(char *,byte *,unsigned long,unsigned long *);
#ifndef _MSC_VER
extern "C" {
    int _RTLENTRY _EXPFUNC sprintf(char _FAR *__buffer, const char _FAR *__format, ...);
}
#endif

static HWND EnumWindow=0;

struct Creds {
    char Name[50];
    char Domain[50];
    char Password[128];
};

static CRITICAL_SECTION ClientCS;

#ifdef __cplusplus
extern "C" {
#endif
    char name[256];
    struct Session {
	Session() {	thread=INVALID_HANDLE_VALUE; port =0;}
	~Session() {
	    //cerr<<"Closing Handles\n";
	    closesocket(in);
	    //closesocket(out);
	    closesocket(error);
	    if(thread != INVALID_HANDLE_VALUE) CloseHandle(thread);
	}
	BOOL State;
	int port;
	HostData *Server;
	SEC_WINNT_AUTH_IDENTITY Ident;
	R_PROCESS_INFORMATION ProcInfo;
	SOCKET in,out,error;
	HANDLE thread;
	HWND Window;
    };
    
    void InitializeRexecClient() {
	InitializeCriticalSection(&ClientCS);
    }
    


    void ShutdownRexecClient() {
	DeleteCriticalSection(&ClientCS);
    }

/*************************************************************************************/
    
    LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize )
    {
	DWORD dwRet;
	LPTSTR lpszTemp = NULL;
	
	dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
	    NULL,
	    error,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    (LPTSTR)&lpszTemp,
	    0,
	    NULL );
	
	// supplied buffer is not long enough
	if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) ) {
	    if(dwSize>8) sprintf(lpszBuf,"%d",error);
	    else lpszBuf[0] = TEXT('\0');
	} else {
	    lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
	    sprintf( lpszBuf, TEXT("%s (%d)"), lpszTemp, error );
	}
	
	if ( lpszTemp )
	    LocalFree((HLOCAL) lpszTemp );
	
	return lpszBuf;
    }

/*************************************************************************************/

    
    DWORD ThreadFunc(Session* session) {
	char buf[1024];
	int len;
	while((len= recv(session->out,buf,1023,0))>0) {
	    buf[len]=0;
	    MessageBox(0,buf,"Received",IDOK);
	}
	sprintf(buf,"%d: Read error (%d)",WSAGetLastError(),len);
	MessageBox(session->Window,buf,"Received",IDOK);
	/*
	inSocket master(0,any),inS,outS,errS;
	DWORD error ;
	try {
	master.listen();
	session->port=master.getPort();
	outS.accept(master);
	inS.accept(master);
	errS.accept(master);
	master.close();
	} catch (socketException ) {
	error=WSAGetLastError();;
	master.close();
        inS.close();
        outS.close();
        errS.close();
	return error;
	}
	session->in = (SOCKET)(outS);
	session->out= (SOCKET)(inS);
	session->error = (SOCKET)(errS);
	error=WSAAsyncSelect(session->out,session->Window,IN_DATA,FD_READ|FD_CLOSE);
	error=WSAAsyncSelect(session->error,session->Window,ERR_DATA,FD_READ|FD_CLOSE);
	*/
	return 0;
    }//ThreadFunc

/*************************************************************************************/
    
    int RexecSendInput(HANDLE hRemoteProc,char* buf,DWORD len) {
	return send(((Session*)hRemoteProc)->in,buf,len,0);
    }
    
/*************************************************************************************/

    int ReadData(HANDLE hRemoteProc,char *buf,DWORD len,BOOL err) {
	if(!err)
	    len = recv(((Session*)hRemoteProc)->out,buf,len,0);
	else
	    len = recv(((Session*)hRemoteProc)->error,buf,len,0);
	return len;
    }

/*************************************************************************************/
    
    HANDLE GetRemoteThread(HANDLE hRemoteProc) {
	return ((Session*)hRemoteProc)->thread;
    }
    
/*************************************************************************************/

    DWORD CreateBinding(char *Host,char* Protocol,RPC_BINDING_HANDLE *Binding,
	                SEC_WINNT_AUTH_IDENTITY *Ident,	unsigned long SecurityLevel) {
	
	
	unsigned char *stringBinding=0;
	RPC_STATUS status;
	char errortext[256];
	status = RpcStringBindingCompose(0,
	    (unsigned char*)Protocol,
	    (unsigned char*)Host,
	    0,
	    0,
	    (unsigned char**)&stringBinding);
	if (status != RPC_S_OK) {
	    fprintf(stderr,"%s: RpcStringBindingCompose failed - \n%s\n",Host,GetLastErrorText(status,errortext,256));
	    return(status);
	}
	
	status = RpcBindingFromStringBinding(stringBinding, Binding);
	RpcStringFree(&stringBinding);
	
	if (status != RPC_S_OK) {
	    DBM(Host<<": RpcBindingFromStringBinding failed (Protocol "<<Protocol<<") - "
			<<GetLastErrorText(status,errortext,256)<<std::endl);
	    return(status);
	}
	
	
	if(Ident) {
	    if(Ident->Domain)
		Ident->DomainLength = strlen((const char*)Ident->Domain);
	    else
		Ident->DomainLength = 0;
	    if(Ident->Password)
		Ident->PasswordLength = strlen((const char*)Ident->Password);
	    else
		Ident->PasswordLength =0;
	    if(Ident->User)
		Ident->UserLength=strlen((const char*)Ident->User);
	    else
		Ident->UserLength = 0;
	    Ident->Flags=1;
	    
	}
	//Ident?RPC_C_AUTHN_LEVEL_PKT_PRIVACY:RPC_C_AUTHN_LEVEL_PKT_INTEGRITY);
	status =
	    RpcBindingSetAuthInfo(*Binding,
	    0,
	    SecurityLevel,
	    RPC_C_AUTHN_WINNT,
	    Ident,
	    0
	    );
	
	if (status != RPC_S_OK) {
	    printf("%s: RpcBindingSetAuthInfo failed - \n%s\n",Host, GetLastErrorText(status,errortext,256));
	    RpcBindingFree(Binding);
	    *Binding = 0;
	    return(status);
	}
	return status;
    }//CreateBinding

/*************************************************************************************/
	
//SI//23-08-2004 Use ncacn_np as workaround against XP SP2 RPC restrictions    
#define NUM_PROTOCOLS 4


    DWORD OpenHost(HostData *Server) {
	TServerHandle *hServer;
	SEC_WINNT_AUTH_IDENTITY *Ident;
	TAccount *Creds;
	int i;
	char *Protocols[] = { "ncalrpc",
	    "ncacn_ip_tcp",
	    "ncacn_nb_tcp",
		"ncacn_np"
	};
	
	DBM("Opening host");
	EnterCriticalSection(&ClientCS);
	if(!Server) {
	    LeaveCriticalSection(&ClientCS);
	    DBM("Got NULL Server handle");
	    return ERROR_INVALID_PARAMETER;
	}
	if(Server->handle) {
	    LeaveCriticalSection(&ClientCS);
	    DBM("Server is already open");
	    return ERROR_SUCCESS;
	}
	if(!Server->Security)
	    Server->Security=RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
	
	hServer = (TServerHandle*)malloc(sizeof(TServerHandle));
	Creds = Server->Account;
	if(Creds) {
	    Ident = &hServer->Ident;
	    Ident->Domain = (unsigned char*)Creds->Domain;
	    Ident->User = (unsigned char*)Creds->User;
	    Ident->Password = (unsigned char*)Creds->Password;
	    DBM("Set credential for "<<Ident->Domain<<"/"<<Ident->User);
	}  else Ident = 0;
	
	if(Ident) i=1;
	else i=0;
	for(;i<NUM_PROTOCOLS;++i) {
	    DBM("Trying protocol "<<Protocols[i]);
	    Server->LastError=CreateBinding(Server->Name,Protocols[i],
		&hServer->Binding,Ident,
		Server->Security);
	    if(Server->LastError != RPC_S_OK) {
		DBM("Failed with "<<Server->LastError);
		continue;
	    }
	    DBM("Pinging host");
	    Server->LastError = Ping(hServer->Binding);
	    if(Server->LastError == RPC_S_OK) {
		DBM("Host is alive");
		break;
	    }
	    DBM("Host is unreachable ("<<Server->LastError<<")");
	    RpcBindingFree(&hServer->Binding);
	}
	if(Server->LastError != RPC_S_OK) {
	    DBM("Could not open the host");
	    free(hServer);
	    Server->handle=0;
	    LeaveCriticalSection(&ClientCS);
	    return Server->LastError;
	}
	
	Server->handle = hServer;
	Server->LastError = ERROR_SUCCESS;
	LeaveCriticalSection(&ClientCS);
	return ERROR_SUCCESS;
    }//OpenHost

/*************************************************************************************/

    
    DWORD CloseHost(HostData *Server) {
	TServerHandle *hServer;
	EnterCriticalSection(&ClientCS);
	if(!Server || !Server->handle) {
	    LeaveCriticalSection(&ClientCS);
	    return ERROR_INVALID_DATA;
	}
	hServer = (TServerHandle*)Server->handle;
	RpcBindingFree(&hServer->Binding);
	free(hServer);
	Server->handle = 0;
	LeaveCriticalSection(&ClientCS);
	return ERROR_SUCCESS;
    }//CloseHost


/*************************************************************************************/
    
    void FreeHostStatus(HostData *Server) {
	EnterCriticalSection(&ClientCS);
	if(!Server || !Server->State) {
	    LeaveCriticalSection(&ClientCS);
	    return;
	}
	
	if(--Server->State->RefCount==0 ) {
	    if(Server->State->UserDataSize) {
		free(Server->State->UserData);
		Server->State->UserData=0;
		Server->State->UserDataSize = 0;
	    }
	    if(Server->State->Configuration) {
		free(Server->State->Configuration);
		Server->State->Configuration = 0;
	    }
	    free(Server->State);
	}
	Server->State = 0;
	LeaveCriticalSection(&ClientCS);
    }//FreeHostStatus


/*************************************************************************************/
    
    TStateData *CopyStateData(HostData *dst,HostData *src) {
	//TStateData *d,*s;
	
	if(!dst) return 0;
	
	FreeHostStatus(dst);
	
	if(!src || !src->State) {
	    return 0;
	}
	++src->State->RefCount;
	dst->State = src->State;
	return dst->State;
	/*
	s = src->State;
	
	  if(!dst->State) {
	  dst->State = (TStateData*)malloc(sizeof(TStateData));
	  memset(dst->State,0,sizeof(TStateData));
	  }
	  d=dst->State;
	  if(s->UserData) {
	  d->UserData = realloc(d->UserData,s->UserDataSize);
	  memcpy(d->UserData,s->UserData,s->UserDataSize);
	  } else if(d->UserData) {
	  free(d->UserData);
	  d->UserData=0;
	  }
	  d->UserDataSize = s->UserDataSize;
	  strcpy(d->ConsoleUser,s->ConsoleUser);
	  strcpy(d->LockedBy,s->LockedBy);
	  d->Locked=s->Locked;
	  d->Valid = s->Valid;
	  
	    return d;
	*/
    }//*CopyStateData


/*************************************************************************************/	
    
    HostData* CopyHost(HostData *dst,HostData *src) {
	if(!src) return 0;
	if(!dst) {
	    dst = (HostData*)malloc(sizeof(HostData));
	    memset(dst,0,sizeof(HostData));
	}
	strcpy(dst->Name,src->Name);
	dst->handle=0;
	dst->LastError = src->LastError;
	dst->Security = src->Security;
	dst->Alive = src->Alive;
	CopyProcData(dst,src);
	SetHostAccount(dst,src->Account);
	CopyStateData(dst,src);
	return dst;
    }//CopyHost


/*************************************************************************************/
    
    void FreeHost(HostData *Server) {
	EnterCriticalSection(&ClientCS);
	if(!Server) {
	    LeaveCriticalSection(&ClientCS);
	    return;
	}
	if(Server->handle) CloseHost(Server);
	SetHostAccount(Server,0);
	FreeHostStatus(Server);
	
	// This is implemented in Environment.cpp
	FreeProcData(Server);
	free(Server);
	LeaveCriticalSection(&ClientCS);
    }//FreeHost


/*************************************************************************************/

    
    DWORD SetHostAccount(HostData *Server,TAccount *Creds) {
	if(!Server) return ERROR_INVALID_DATA;
	if(!Creds) {
	    if(Server->Account) {
		if(Server->handle)  CloseHost(Server);
		memset(Server->Account,0,sizeof(TAccount));
		free(Server->Account);
		Server->Account = 0;
	    }
	} else {
	    if(!Server->Account ||
		stricmp(Server->Account->User,Creds->User) ||
		stricmp(Server->Account->Domain,Creds->Domain) ||
		stricmp(Server->Account->Password,Creds->Password) ) {
		
		if(Server->handle)  CloseHost(Server);
		if(!Server->Account)
		    Server->Account = (TAccount*)malloc(sizeof(TAccount));
		memcpy(Server->Account,Creds,sizeof(TAccount));
	    }
	}
	Server->LastError = ERROR_SUCCESS;
	return ERROR_SUCCESS;
    }//SetHostAccount


/*************************************************************************************/
    
    DWORD GetHostStatus(HostData *Server, char *ServerDllName) {
	TServerHandle *hServer;
	
	EnterCriticalSection(&ClientCS);
	if(!Server) {
	    LeaveCriticalSection(&ClientCS);
	    return ERROR_INVALID_PARAMETER;
	}
	if(!ServerDllName) ServerDllName = "";
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		FreeHostStatus(Server);
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	} else {
	    if(Ping(((TServerHandle*)(Server->handle))->Binding) != RPC_S_OK) {
		CloseHost(Server);
		if(OpenHost(Server) != ERROR_SUCCESS) {
		    FreeHostStatus(Server);
		    LeaveCriticalSection(&ClientCS);
		    return Server->LastError;
		}
	    }
	}
	hServer = (TServerHandle*)Server->handle;
	if(!Server->State) {
	    Server->State = (TStateData*)malloc(sizeof(TStateData));
	    memset(Server->State,0,sizeof(TStateData));
	    Server->State->RefCount = 1;
	}
	Server->LastError =
	    R_GetConsoleUser(hServer->Binding,
	    (unsigned char*)Server->State->ConsoleUser);
	if(Server->LastError != RPC_S_OK) {
	    Server->State->Valid = FALSE;
	    FreeHostStatus(Server);
	    CloseHost(Server);
	    LeaveCriticalSection(&ClientCS);
	    return Server->LastError;
	}
	Server->State->Valid = TRUE;
	Server->LastError = R_GetLockStatus(hServer->Binding,&Server->State->Locked,
	    (unsigned char*)Server->State->LockedBy);
	if(Server->LastError != RPC_S_OK) {
	    CloseHost(Server);
	    LeaveCriticalSection(&ClientCS);
	    return Server->LastError;
	}
	if(!Server->State->Configuration) {
	    Server->State->Configuration = (R_MACHINE_INFO*)malloc(sizeof(R_MACHINE_INFO));
	    memset(Server->State->Configuration,0,sizeof(R_MACHINE_INFO));
	}
	Server->LastError = R_GetSystemInfo(hServer->Binding,Server->State->Configuration);
	
	if(Server->LastError != RPC_S_OK) {
	    CloseHost(Server);
	    free(Server->State->Configuration);
	    Server->State->Configuration = 0;
	}                                        
	
	if(ServerDllName && ServerDllName[0]) {
	    do {
		Server->LastError =
		    R_GetUserData(hServer->Binding,
		    (unsigned char*)ServerDllName,
		    (unsigned char*)Server->State->UserData,
		    Server->State->UserDataSize,
		    (long*)&Server->State->UserDataSize);
		
		if(Server->LastError == RPC_S_BUFFER_TOO_SMALL&&
		    Server->State->UserDataSize) {
		    Server->State->UserData=
			realloc(Server->State->UserData,
			Server->State->UserDataSize);
		}
	    } while(Server->LastError == RPC_S_BUFFER_TOO_SMALL &&
		Server->State->UserDataSize);
	    if(Server->LastError != RPC_S_OK) {
		CloseHost(Server);
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	} else if(Server->State->UserDataSize) {
	    free(Server->State->UserData);
	    Server->State->UserData=0;
	    Server->State->UserDataSize = 0;
	}
	Server->LastError = ERROR_SUCCESS;
	LeaveCriticalSection(&ClientCS);
	return ERROR_SUCCESS;
    }//GetHostStatus


/*************************************************************************************/

    
    DWORD CreateRemoteProcess(HostData *Server,RemoteStartupInfo *SI,HANDLE *hProcess) {
	
	Session *S;
	TServerHandle *hServer;
	R_STARTUPINFO StartupInfo;
	//DWORD tId;
	inSocket master(0,any),inS,errS;
	
	*hProcess = 0;
	DBM("Entering CreateRemoteProcess");
	if(!Server) {
	    DBM("The server handle is invalid");
	    return ERROR_INVALID_PARAMETER;
	}
	
	if(!Server->handle) {
	    Server->Security = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
	    if(OpenHost(Server) != ERROR_SUCCESS) return Server->LastError;
	}
	hServer = (TServerHandle*)Server->handle;
	S=new Session;
	S->Window=SI->Window;
	S->Server = Server;
	DBM("Setting up sockets");
	try {
	    master.listen(2);
	    S->port=master.getPort();
	    DBM("Port is :"<<S->port);
	} catch (socketException e) {
	    Server->LastError=WSAGetLastError();
	    DBM("listen failed with "<<e);
	    master.close();
	}
	/*S->thread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadFunc,S,0,&tId);
	
	  if(!S->thread) {
	  delete S;
	  Server->LastError=GetLastError();
	  return Server->LastError;
	  }
	*/
	memset(&StartupInfo,0,sizeof(StartupInfo));
	//while(!S->port) Sleep(1);
	 //redirect all output of remote processes to S->port 
	StartupInfo.hStdInput = StartupInfo.hStdOutput = StartupInfo.hStdError = S->port;
	if(Server->Account)
	    StartupInfo.lpPassword = (unsigned char*)Server->Account->Password;
//	DBM("Calling R_CreateProcess");

	Server->LastError = R_CreateProcess(hServer->Binding,0,
	    (unsigned char*)SI->Commandline,SI->CreationFlags,
	    (unsigned char*)SI->Environment,SI->EnvSize,
	    (unsigned char*)SI->WorkingDir,&StartupInfo,&S->ProcInfo);
	
	if(Server->LastError != RPC_S_OK) {
	    DBM("Call failed with "<<Server->LastError);
	    CloseHost(Server);
	    delete S;
	    master.close();
	    return Server->LastError;
	}
	DBM("Waiting for client to connect");
	try {
	    inS.accept(master);
	    errS.accept(master);
	    master.close();
	} catch (socketException e) {
	    DBM("Error "<<e<<" while waiting for connections");
	    Server->LastError=WSAGetLastError();
	    master.close();
	    inS.close();
	    errS.close();
	    CloseHost(Server);
	    *hProcess=0;
	    return Server->LastError;
	}
	S->in = (SOCKET)(inS);
	S->out= (SOCKET)(inS);
	S->error = (SOCKET)(errS);
	
	//if remote process sends output on socket connected with port S->out, then generate
	//windows-notification of type IN_DATA for window with handle SI->Window
	if(WSAAsyncSelect(S->out,SI->Window,IN_DATA,FD_READ|FD_CLOSE)) {
	    Server->LastError=WSAGetLastError();
	    DBM("WsaAsyncSelect(out) failed");
	    inS.close();
	    errS.close();
	    CloseHost(Server);
	    return Server->LastError;
	}
	//as above for error messages
	if(WSAAsyncSelect(S->error,SI->Window,ERR_DATA,FD_READ|FD_CLOSE)) {
	    Server->LastError=WSAGetLastError();
	    DBM("WsaAsyncSelect(error) failed");
	    inS.close();
	    errS.close();
	    CloseHost(Server);
	    return Server->LastError;
	}
	DBM("Remote process created succesfully");
	//S->thread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadFunc,S,0,&tId);
	S->State=TRUE;
	*hProcess=(HANDLE)S;
	Server->LastError = ERROR_SUCCESS;
	return ERROR_SUCCESS;
    }//CreateRemoteProcess


/*************************************************************************************/
    
    
    DWORD KillRemoteProcess(HANDLE hProcess) {
	Session *S = (Session*)hProcess;
	TServerHandle *hServer;
	if(!S || !S->Server )
	    return ERROR_INVALID_PARAMETER;
	if(!S->Server->handle)
	    if(OpenHost(S->Server) != ERROR_SUCCESS) return S->Server->LastError;
	    hServer=(TServerHandle*)S->Server->handle;
	    S->Server->LastError =R_TerminateProcess(hServer->Binding,&S->ProcInfo,-1);
	    return S->Server->LastError;
    }//KillRemoteProcess


/*************************************************************************************/
    
    DWORD WaitForRemoteEnd(HANDLE hProcess,DWORD Timeout) {
	
	DWORD result;
	
	result=WaitForSingleObject(((Session*)hProcess)->thread,Timeout);
	if(result!=WAIT_TIMEOUT&&result!=WAIT_FAILED) {
	    ((Session*)hProcess)->State=FALSE;
	} 
	return result;
	
    }//WaitForRemoteEnd


/*************************************************************************************/
    
    void CloseRemoteHandle(HANDLE hProcess) {
	if(!hProcess) return;
	Session *S = (Session*)hProcess;
	delete S;
    }//CloseRemoteHandle

/*************************************************************************************/
    
    
    DWORD LockServer(HostData *Server) {
	//  HostData *Server;
	TServerHandle *hServer;
	
	//    Server = Servers[ServerName];
	if(!Server) return ERROR_INVALID_PARAMETER;
	EnterCriticalSection(&ClientCS);
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	}
	hServer = (TServerHandle*)Server->handle;
	
	Server->LastError=R_LockServer(hServer->Binding);
	LeaveCriticalSection(&ClientCS);
	return Server->LastError;
    }//LockServer


/*************************************************************************************/
	
    
    DWORD UnlockServer(HostData *Server) {
	//    HostData *Server;
	TServerHandle *hServer;
	
	//    Server = Servers[ServerName];
	if(!Server) return ERROR_INVALID_PARAMETER;
	EnterCriticalSection(&ClientCS);
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	}
	hServer = (TServerHandle*)Server->handle;
	
	Server->LastError=R_UnlockServer(hServer->Binding);
	LeaveCriticalSection(&ClientCS);
	return Server->LastError;
    }//UnlockServer


	
    
    /*
    BOOL IsServerAlive(char *ServerName) {
    HostData *Server;
    TServerHandle *hServer;
    
      Server = (*Servers)[ServerName];
      if(!Server) return ERROR_INVALID_DATA;
      EnterCriticalSection(&ClientCS);
      if(!Server->handle) {
      if(OpenHost(Server) != ERROR_SUCCESS) {
      LeaveCriticalSection(&ClientCS);
      return Server->LastError;
      }
      }
      hServer = (TServerHandle*)Server->handle;
      Server->LastError = Ping(hServer->Binding);
      LeaveCriticalSection(&ClientCS);
      return Server->LastError;
      }
    */
/*************************************************************************************/
	
    char *GetHostAccountName(HostData *Server,char *buffer,DWORD size) {
	DWORD len;
	if(!buffer) return 0;
	*buffer=0;
	
	if(!Server) {
	    return buffer;
	}
	EnterCriticalSection(&ClientCS);
	if(Server->Account) {
	    len = strlen(Server->Account->User)+strlen(Server->Account->Domain)+2;
	    if(len<=size)
		sprintf(buffer,"%s/%s",Server->Account->Domain,Server->Account->User);
	} else {
	    if(strlen(name)<size)
		strcpy(buffer,name);
	}
	LeaveCriticalSection(&ClientCS);
	return buffer;
    }//GetHostAccountName

	
    
#define CREDKEY "Software\\lfbs\\rexec\\Credentials"

/*************************************************************************************/    
    BOOL EnumStoredAccounts(AccountEnumCallback Callback,void *param) {
	BOOL res=TRUE;
	LONG lError;
	HKEY hKey;
	DWORD MaxValSize,MaxNameLen;
	char *ValueName;
	PBYTE ValueData;
	DWORD ValueType;
	DWORD index=0;
	DWORD NameLen,DataLen;
	Creds *result;
	
	lError=RegOpenKeyEx(HKEY_CURRENT_USER,CREDKEY,0,KEY_READ,&hKey);
	if(lError !=ERROR_SUCCESS) {
	    SetLastError(lError);
	    return FALSE;
	}
	
	lError=RegQueryInfoKey(hKey,0,0,0,0,0,0,0,&MaxNameLen,&MaxValSize,0,0);
	if(lError !=ERROR_SUCCESS) {
	    RegCloseKey(hKey);
	    SetLastError(lError);
	    return FALSE;
	}
	MaxNameLen++;
	ValueName=(char*)alloca(MaxNameLen);
	ValueData=(PBYTE)alloca(MaxValSize);
	
	do {
	    NameLen=MaxNameLen;
	    DataLen=MaxValSize;
	    lError=RegEnumValue(hKey,index,ValueName,&NameLen,0,&ValueType,ValueData,&DataLen);
	    index++;
	    if(ValueType!=REG_BINARY) {
		continue;
	    }
	    if((lError != ERROR_SUCCESS)&&(lError!=ERROR_MORE_DATA)) break;
	    
	    if(!DecryptData((char*)ValueData,&DataLen)) {
		continue;
	    }
	    result=(Creds*)(ValueData+DataLen);
	    res=Callback(ValueName,result->Name,result->Domain,result->Password,param);
	} while(res&&(lError==ERROR_SUCCESS||lError==ERROR_MORE_DATA));
	
	RegCloseKey(hKey);
	SetLastError(lError);
	return res;
    }//EnumStoredAccounts


/*************************************************************************************/
    
    BOOL StoreAccount(char *AccountName,char *UserName,char *Domain,char *Password) {
	DWORD disp;
	HKEY hKey;
	LONG lError;
	BOOL res;
	Creds data;
	DWORD BuffSize=sizeof(data);
	char *resultbuf;
	
	lError=RegCreateKeyEx(HKEY_CURRENT_USER,CREDKEY,0,"",
	    REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,0,&hKey,&disp);
	
	if(lError!=ERROR_SUCCESS) {
	    SetLastError(lError);
	    return FALSE;
	}
	strcpy(data.Name,UserName);
	strcpy(data.Domain,Domain);
	strcpy(data.Password,Password);
	res=EncryptData((char*)&data,&resultbuf,&BuffSize);
	if(!res) {
		//si generate debug message
		DWORD errcode = GetLastError();
		DBM("Encrypt Data failed with errorcode: "<<errcode);
	    RegCloseKey(hKey);
	    SetLastError(lError);
	    return FALSE;
	} 
	else
	{
		DBM("Encrypt Data succeeded");
	}
	lError=RegSetValueEx(hKey,AccountName,0,REG_BINARY,(PBYTE) resultbuf,BuffSize);
	RegCloseKey(hKey);
	if(resultbuf) free(resultbuf);
	SetLastError(lError);
	return (lError==ERROR_SUCCESS?TRUE:FALSE);
	
    }//StoreAccount



/*************************************************************************************/
    
    BOOL LoadAccount(char *AccountName,char *UserName,char *Domain,char *Password) {
	LONG lError;
	HKEY hKey;
	DWORD MaxValSize;
	PBYTE ValueData;
	DWORD ValueType;
	Creds *result;	
	
	lError=RegOpenKeyEx(HKEY_CURRENT_USER,CREDKEY,0,KEY_READ,&hKey);
	
	if(lError!=ERROR_SUCCESS) {
	    SetLastError(lError);
	    return FALSE;
	}
	
	lError=RegQueryInfoKey(hKey,0,0,0,0,0,0,0,0,&MaxValSize,0,0);
	if(lError !=ERROR_SUCCESS) {
	    RegCloseKey(hKey);
	    SetLastError(lError);
	    return FALSE;
	}
	ValueData=(PBYTE)alloca(MaxValSize);
	
	lError=RegQueryValueEx(hKey,AccountName,0,&ValueType,(PBYTE) ValueData,&MaxValSize);
	if(lError !=ERROR_SUCCESS) {
	    RegCloseKey(hKey);
	    SetLastError(lError);
	    return FALSE;
	}
	RegCloseKey(hKey);
	
	if(!DecryptData((char*)ValueData,&MaxValSize)) 
		return FALSE;
	
	result=(Creds*)(ValueData+MaxValSize);
	
	strcpy(UserName,result->Name);
	strcpy(Domain,result->Domain);
	strcpy(Password,result->Password);
	
	return TRUE;
    }//LoadAccount


/*************************************************************************************/
    
    BOOL DeleteAccount(char *Account) {
	LONG lError;
	HKEY hKey;
	lError=RegOpenKeyEx(HKEY_CURRENT_USER,CREDKEY,0,KEY_WRITE,&hKey);
	if(lError!=ERROR_SUCCESS) {
	    SetLastError(lError);
	    return FALSE;
	}
	lError=RegDeleteValue(hKey,Account);
	SetLastError(lError);
	return ((lError==ERROR_SUCCESS)?TRUE:FALSE);
    }//DeleteAccount



/*************************************************************************************/
    
    /* [callback] */ error_status_t R_GetClientName( handle_t IDL_handle,/* [string][out] */ unsigned char ClientName[ 255 ],
    unsigned long *size) {
	
	if(gethostname((char*)ClientName,*size)) {
	    *size=0;
	    return WSAGetLastError();
	}else {
	    (*size)=strlen((char*)ClientName)+1;
	}
	
	return RPC_S_OK;
    }//R_GetClientName


/*************************************************************************************/
    
    DWORD GetHostProcs(HWND client,HostData *Server) {
	TServerHandle *hServer;
	if(!Server) return ERROR_INVALID_PARAMETER;
	EnterCriticalSection(&ClientCS);
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	}
	hServer = (TServerHandle*)Server->handle;
	EnumWindow = client;
	SendMessage(EnumWindow,RC_PROC_START,0,(LPARAM)Server);
	Server->LastError=R_GetProcs(hServer->Binding);
	SendMessage(EnumWindow,RC_PROC_END,0,(LPARAM)Server);
	EnumWindow = 0;
	LeaveCriticalSection(&ClientCS);
	return Server->LastError;
    }//GetHostProcs


/*************************************************************************************/
    
    DWORD KillProcess(HostData *Server,int id) {
	TServerHandle *hServer;
	if(!Server) return ERROR_INVALID_PARAMETER;
	EnterCriticalSection(&ClientCS);
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	}
	hServer = (TServerHandle*)Server->handle;
	Server->LastError=R_KillProcess(hServer->Binding,(DWORD)id);
	LeaveCriticalSection(&ClientCS);
	return Server->LastError;
    }//KillProcess

/*************************************************************************************/
 
DWORD ShutDown(HostData *Server,BOOL restart) { //si //10.6.
    TServerHandle *hServer;
	if(!Server)
    	return ERROR_INVALID_PARAMETER;
    EnterCriticalSection(&ClientCS);
    if(!Server->handle) {
        if(OpenHost(Server) != ERROR_SUCCESS) {
            LeaveCriticalSection(&ClientCS);
        	return Server->LastError;
        }
    }
    hServer = (TServerHandle*)Server->handle;
    Server->LastError=R_ShutDown(hServer->Binding,restart);
    LeaveCriticalSection(&ClientCS);
    return Server->LastError;
}
/*************************************************************************************/
    
    error_status_t R_ProcEnumCallback(handle_t IDL_handle,unsigned long id,
	unsigned char *name, unsigned char *Owner) {
	Enum_t param = {id,(char*)name,(char*)Owner};
	if(EnumWindow)
	    SendMessage(EnumWindow,RC_PROC,0,(LPARAM)&param);
	return RPC_S_OK;
    }//R_ProcEnumCallback
    

/*************************************************************************************/
	
    void  (__RPC_FAR PipeAlloc)(char  * state,unsigned long bsize, byte ** buf, unsigned long * bcount ) {   
		
	pipe_state *s = (pipe_state*)state;
	if(bsize>s->bufSize) {
	    s->buffer = (byte*)realloc(s->buffer,bsize);
	    if(!s->buffer) bsize = 0;		   
	    s->bufSize = bsize;
	}
	*bcount = bsize;	
	*buf = s->buffer;
    }//PipeAlloc


/*************************************************************************************/    
    void   (__RPC_FAR PipePull)(char * state,byte * buf,unsigned long esize,unsigned long * ecount ) {

	pipe_state	*psState = (pipe_state *) state;
	
	ReadFile(psState->hFile,buf,esize,ecount,0);
    }//PipePull


/*************************************************************************************/

    DWORD SendServerBinary(HostData *Server,char *filename) {
	TServerHandle *hServer;
	byte_pipe  pipe;    // Pipe structure used for the pipe
	pipe_state pState;

	if(!Server) return ERROR_INVALID_PARAMETER;
	EnterCriticalSection(&ClientCS);
	if(!Server->handle) {
	    if(OpenHost(Server) != ERROR_SUCCESS) {
		LeaveCriticalSection(&ClientCS);
		return Server->LastError;
	    }
	}
	pState.hFile = CreateFile(filename,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(pState.hFile == INVALID_HANDLE_VALUE) {
	    Server->LastError = GetLastError();
	    LeaveCriticalSection(&ClientCS);
	    return Server->LastError;
	}

	pState.buffer = 0;
	pState.bufSize = 0;
	hServer = (TServerHandle*)Server->handle;
        pipe.state	= (char *) &pState;
	pipe.alloc	=  PipeAlloc;
	pipe.pull	=  PipePull;
	pipe.push	= NULL;
	Server->LastError = R_Transfer(hServer->Binding,&pipe);
	CloseHandle(pState.hFile);
	if(pState.bufSize) {
	    free(pState.buffer);
	}
	LeaveCriticalSection(&ClientCS);
	return Server->LastError;
    }//SendServerBinary


/*************************************************************************************/

    void * __RPC_USER MIDL_user_allocate(size_t size)
    {
	return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
    }
    
    void __RPC_USER MIDL_user_free( void *pointer)
    {
	HeapFree(GetProcessHeap(), 0, pointer);
    }
    
    
#ifdef __cplusplus 
}
#endif