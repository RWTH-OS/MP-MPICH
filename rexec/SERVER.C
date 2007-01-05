// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1995-1996  Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   server.c
//
//  PURPOSE:  Implements the body of the RPC service sample.
//
//  FUNCTIONS:
//            Called by service.c:
//            ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
//            ServiceStop( );
//
//            Called by RPC:
//            error_status_t Ping(handle_t)
//
//  COMMENTS: The ServerStart and ServerStop functions implemented here are
//            prototyped in service.h.  The other functions are RPC manager
//            functions prototypes in rpcsvc.h.
//              
//
//  AUTHOR: Craig Link - Microsoft Developer Support
//          Mario Goertzel - RPC Development
//


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <rpc.h>
#include "service.h"
#include "cluma.h"
#include "messages.h"
#include "helpers.h"

#define KEY "System\\CurrentControlSet\\Services\\rcluma\\Parameters"

extern DWORD dwErr;
extern LONG GlobalExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);

//
// RPC configuration.
//

// This service listens to all the protseqs listed in this array.
// This should be read from the service's configuration in the
// registery.

TCHAR *ProtocolArray[] = {  TEXT("ncalrpc"),
                            TEXT("ncacn_ip_tcp"),
							TEXT("ncacn_nb_tcp"),
			   				TEXT("ncadg_ip_udp"),
                          	TEXT("ncadg_ipx" ),
                          	TEXT("ncadg_mq"),
                          	TEXT("ncacn_spx"),
                          	TEXT("ncacn_np"),
                          	TEXT("ncacn_nb_nb"),
                          	TEXT("ncacn_nb_ipx")
                         };

// Used in RpcServerUseProtseq, for some protseqs
// this is used as a hint for buffer size.
ULONG ProtocolBuffer = 10;

// Use in RpcServerListen().  More threads will increase performance,
// but use more memory.
ULONG MinimumThreads = 5;
BOOL Reconfig=FALSE;
BOOL debug_flag=0;
char *DebugFileName=0;
BOOL NoUser=FALSE;

void ReadValues(char** LogFile,ULONG *NumThreads, BOOL* NoUser) {
    
    HKEY hKey;
    LONG lError;
    DWORD ValType,ValSize;
    BOOL log;


    lError=RegOpenKeyEx(HKEY_LOCAL_MACHINE,KEY,0,KEY_READ,&hKey);
    
    if(lError!=ERROR_SUCCESS) {
	*NumThreads = 5;
	*LogFile=0;
	return;
    }

    ValSize = sizeof(BOOL);
    lError=RegQueryValueEx(hKey,"LOGGING",0,&ValType,(LPBYTE) &log,&ValSize);
    if(lError !=ERROR_SUCCESS) {
	log = FALSE;
	*LogFile = 0;
    }
    ValSize = sizeof(DWORD);
    lError=RegQueryValueEx(hKey,"NUM_THREADS",0,&ValType,(LPBYTE) NumThreads,&ValSize);
    if(lError !=ERROR_SUCCESS) {
	*NumThreads = 5;
    }

	ValSize = sizeof(BOOL);
    lError=RegQueryValueEx(hKey,"NOUSER",0,&ValType,(LPBYTE) NoUser,&ValSize);
    if(lError !=ERROR_SUCCESS) {
	*NoUser = FALSE;
    }

    if(log) {
	ValSize=0;
	lError=RegQueryValueEx(hKey,"LOG_FILE",0,&ValType,(LPBYTE) *LogFile,&ValSize);
	if(lError !=ERROR_SUCCESS && lError != ERROR_MORE_DATA) {
	    *LogFile = 0;	    
	} else {
	    ++ValSize;
	    *LogFile = (char*)malloc(sizeof(char)*(ValSize+1));
	    lError=RegQueryValueEx(hKey,"LOG_FILE",0,&ValType,(LPBYTE) *LogFile,&ValSize);
	    if(lError !=ERROR_SUCCESS) {
		free(*LogFile);
		
	    }
	}
    } else *LogFile = 0;
    RegCloseKey(hKey);
}

VOID ServiceReconfigure() {
    char *NewFile=0;
    
    Reconfig=FALSE;
    ReadValues(&NewFile,&MinimumThreads,&NoUser);
/*    DBG("Reconfiguring Service.\nNumThreads="<<MinimumThreads);
    if(NewFile) DBG("Logfile name = "<<NewFile);
*/
    if(!NewFile && debug_flag) {
	// ON->OFF
	CloseLogFile();
	if(DebugFileName) {
	    free(DebugFileName);
	    DebugFileName = 0;
	}
	debug_flag = 0;	
	return;
    }

    if(NewFile && !debug_flag) {
	// OFF->ON	
	DebugFileName = NewFile;
	debug_flag = 1;
	InitLogging(NewFile);
	return;
    }

    if(NewFile) {
	// ON->ON
	if(!strcmp(NewFile,DebugFileName)) {
	    // All the same
	    free(NewFile); NewFile = NULL; //no effect
	    return;
	}
	// The filename changed
	InitLogging(NewFile);
	free(DebugFileName);
	DebugFileName = NewFile;

	if(NoUser)
		{DBG("no user authentication needed - not recommended");}
	else
		{DBG("user authentication required");}

	return;
    } 
    // OFF->OFF
}

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    Starts the service listening for RPC requests.
//
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)
{
    UINT i;
    RPC_BINDING_VECTOR *pbindingVector = 0;
    RPC_STATUS status;
    BOOL fListening = FALSE;
	//unsigned int RPCFlags = RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH;
	unsigned int RPCFlags = 0;

    ///////////////////////////////////////////////////
    //
    // Service initialization
    //

    //
    // Use protocol sequences (protseqs) specified in ProtocolArray.
    //

#ifndef _DEBUG    
    //enable DebugBreak() for debug version
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)GlobalExceptionFilter);
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
    ServiceReconfigure();

    for(i = 0; i < sizeof(ProtocolArray)/sizeof(TCHAR *); i++)
        {

        // Report the status to the service control manager.
        if (!ReportStatusToSCMgr(
            SERVICE_START_PENDING, // service state
            NO_ERROR,              // exit code
            3000))                 // wait hint
            return;


        status = RpcServerUseProtseq(ProtocolArray[i],
                                     ProtocolBuffer,
                                     0);

        if (status == RPC_S_OK)
            {
            fListening = TRUE;
            }
        }

    if (!fListening)
        {
        // Unable to listen to any protocol!
        //
        AddToMessageLog(IDM_PROTO,status,0);
	dwErr = status;
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

    // Register the services interface(s).
    //

    /*status = RpcServerRegisterIf(cluma_v1_0_s_ifspec,   // from rpcsvc.h
                                 0,
                                 0);*/
	status = RpcServerRegisterIfEx(cluma_v1_0_s_ifspec,   // from rpcsvc.h
                                 0,
                                 0,
								 RPCFlags,
								 RPC_C_PROTSEQ_MAX_REQS_DEFAULT ,
								 NULL);

    if (status != RPC_S_OK) {
		dwErr = status;
		AddToMessageLog(IDM_REGISTERIF,status,0);
        return;
	}

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;


    // Register interface(s) and binding(s) (endpoints) with
    // the endpoint mapper.
    //

    status = RpcServerInqBindings(&pbindingVector);

    if (status != RPC_S_OK)
        {
		dwErr = status;
		AddToMessageLog(IDM_BINDINGS,status,0);
        return;
        }

    status = RpcEpRegister(cluma_v1_0_s_ifspec,   // from rpcsvc.h
                           pbindingVector,
                           0,
                           0);

    if (status != RPC_S_OK)
        {
		dwErr = status;
		AddToMessageLog(IDM_EPREGISTER,status,0);
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

    // Enable NT LM Security Support Provider (NtLmSsp service)
    //
    status = RpcServerRegisterAuthInfo(0,
                                       RPC_C_AUTHN_WINNT,
                                       0,
                                       0
                                       );
    if (status != RPC_S_OK)
        {
		dwErr = status;
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

	if(InitLockService() <0) return;

	// Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

	if(!InitProcessManagement()) {
		StopLockService();
		return;
	}

	// Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;


    // Start accepting client calls.
    //
    status = RpcServerListen(MinimumThreads,
                             RPC_C_LISTEN_MAX_CALLS_DEFAULT,  // rpcdce.h
                             TRUE);                           // don't block.

    if (status != RPC_S_OK)
        {
		dwErr = status;
		AddToMessageLog(IDM_LISTEN,status,0);
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,       // service state
        NO_ERROR,              // exit code
        0))                    // wait hint
        return;

    //
    // End of initialization
    //
    ////////////////////////////////////////////////////////////

	AddToMessageLog(IDM_STARTED,ERROR_SUCCESS,0);


	
	while(!DeleteFile(".\\rclumad.exe.old") && GetLastError() == ERROR_ACCESS_DENIED) {
	    if(++i > 10) break;
	    Sleep(1000);
	}
    ////////////////////////////////////////////////////////////
    //
    // Cleanup
    //

    // RpcMgmtWaitServerListen() will block until the server has
    // stopped listening.  If this service had something better to
    // do with this thread, it would delay this call until
    // ServiceStop() had been called. (Set an event in ServiceStop()).
    //
    do {
	status = RpcMgmtWaitServerListen();
	if(status != RPC_S_OK)
	    AddToMessageLog(IDM_WAIT_LISTEN,status,0);
	if(!Reconfig)  break;
	ServiceReconfigure();	
	status = RpcServerListen(MinimumThreads, 
	                         RPC_C_LISTEN_MAX_CALLS_DEFAULT,  // rpcdce.h
                                 TRUE);                           // don't block.
	if(status == RPC_S_OK)
	    ReportStatusToSCMgr(SERVICE_RUNNING,NO_ERROR,0);
	else {
	    AddToMessageLog(IDM_LISTEN,status,0);
	    ReportStatusToSCMgr(SERVICE_STOP_PENDING,NO_ERROR,3000);
	}

    } while(status == RPC_S_OK);

    dwErr = status;
    // ASSERT(status == RPC_S_OK)

    // Remove entries from the endpoint mapper database.
    //
    RpcEpUnregister(cluma_v1_0_s_ifspec,   // from rpcsvc.h
                    pbindingVector,
                    0);

    // Delete the binding vector
    //
    RpcBindingVectorFree(&pbindingVector);
	
	ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 3000);

	StopLockService();

	ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 3000);

	ShutdownProcessManagement();
	AddToMessageLog(IDM_STOPPED,ERROR_SUCCESS,0);
    //
    ////////////////////////////////////////////////////////////
    return;
}


//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop()
{
    // Stop's the server, wakes the main thread.
    RpcMgmtStopServerListening(0);
}




//
//  FUNCTIONS: MIDL_user_allocate and MIDL_user_free
//
//  PURPOSE: Used by stubs to allocate and free memory
//           in standard RPC calls. Not used when
//           [enable_allocate] is specified in the .acf.
//
//
//  PARAMETERS:
//    See documentations.
//
//  RETURN VALUE:
//    Exceptions on error.  This is not required,
//    you can use -error allocation on the midl.exe
//    command line instead.
//
//
void * __RPC_USER MIDL_user_allocate(size_t size)
{
    return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
}

void __RPC_USER MIDL_user_free( void *pointer)
{
    HeapFree(GetProcessHeap(), 0, pointer);
}

