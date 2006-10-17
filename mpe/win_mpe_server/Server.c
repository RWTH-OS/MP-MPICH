/*
    $Id$
*/


#include <windows.h>
#include <rpc.h>
#include "r_mpe.h"
#include "mpe_server.h"

extern HWND g_hNotifyWindow;
extern hMainThread;

// This service listens to all the protseqs listed in this array.
// This should be read from the service's configuration in the
// registery.

TCHAR *ProtocolArray[] = { TEXT("ncalrpc"),
                           TEXT("ncacn_ip_tcp"),
			               TEXT("ncacn_nb_tcp")
                         };

// Used in RpcServerUseProtseq, for some protseqs
// this is used as a hint for buffer size.
ULONG ProtocolBuffer = 3;

// Use in RpcServerListen().  More threads will increase performance,
// but use more memory.
ULONG MinimumThreads = 5;

DWORD dwErr;
HANDLE StartEvent;

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
VOID ServiceStart (HWND NotifyWindow)
{
    UINT i;
    RPC_BINDING_VECTOR *pbindingVector = 0;
    RPC_STATUS status;
    BOOL fListening = FALSE;

    ///////////////////////////////////////////////////
    //
    // Service initialization
    //

    //
    // Use protocol sequences (protseqs) specified in ProtocolArray.
    //

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_STARTING,0,0);

    for(i = 0; i < sizeof(ProtocolArray)/sizeof(TCHAR *); i++)
        {


        status = RpcServerUseProtseq(ProtocolArray[i],
                                     ProtocolBuffer,
                                     0);

        if (status == RPC_S_OK)
            {
	    if(NotifyWindow) PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);
            fListening = TRUE;
            }
        }

    if (!fListening)
        {
        // Unable to listen to any protocol!
        //
	    if(NotifyWindow) 
		PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	    dwErr = status;
        return;
        }

    //
    // Register the services interface(s).
    //

    status = RpcServerRegisterIf(R_MPE_v1_0_s_ifspec,0,0);


    if (status != RPC_S_OK) {
        if(NotifyWindow) 
	    PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);

        dwErr = status;
	hMainThread = 0;
        return;
    } 

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);


    // Register interface(s) and binding(s) (endpoints) with
    // the endpoint mapper.
    //

    status = RpcServerInqBindings(&pbindingVector);

    if (status != RPC_S_OK) {
	if(NotifyWindow) 
	    PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	dwErr = status;
	hMainThread = 0;
        return;
    }

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);


    status = RpcEpRegister(R_MPE_v1_0_s_ifspec,pbindingVector,0,0);

    if (status != RPC_S_OK) {
	dwErr = status;
	if(NotifyWindow) 
	    PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	hMainThread = 0;
        return;
    }


    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);


    // Enable NT LM Security Support Provider (NtLmSsp service)
    //
    status = RpcServerRegisterAuthInfo(0,
                                       RPC_C_AUTHN_WINNT,
                                       0,
                                       0
                                       );
    if (status != RPC_S_OK){
	dwErr = status;
	if(NotifyWindow) 
	    PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	hMainThread = 0;
        return;
    }

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);

    

    // Start accepting client calls.
    //
    status = RpcServerListen(MinimumThreads,
                             RPC_C_LISTEN_MAX_CALLS_DEFAULT,  // rpcdce.h
                             TRUE);                           // don't block.

    if (status != RPC_S_OK) {
	dwErr = status;
	PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	hMainThread = 0;
        return;
    }


    //
    // End of initialization
    //
    ////////////////////////////////////////////////////////////

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_STARTED,0,status);
    else 
	SetEvent(StartEvent);


    ////////////////////////////////////////////////////////////
    //
    // Cleanup
    //

    // RpcMgmtWaitServerListen() will block until the server has
    // stopped listening.  If this service had something better to
    // do with this thread, it would delay this call until
    // ServiceStop() had been called. (Set an event in ServiceStop()).
    //
    status = RpcMgmtWaitServerListen();

    if (status != RPC_S_OK) {
	dwErr = status;
	PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);
	hMainThread = 0;
        return;
    }


    NotifyWindow = g_hNotifyWindow;
    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);

    // ASSERT(status == RPC_S_OK)

    status = RpcServerUnregisterIf(R_MPE_v1_0_s_ifspec,0,TRUE);

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);

    // Remove entries from the endpoint mapper database.
    //
    RpcEpUnregister(R_MPE_v1_0_s_ifspec,   // from rpcsvc.h
                    pbindingVector,
                    0);

    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_PENDING,0,0);
    // Delete the binding vector
    //
    status = RpcBindingVectorFree(&pbindingVector);
    if(NotifyWindow) 
	PostMessage(NotifyWindow,MPM_SERVER_STOPPED,0,status);

    //
    ////////////////////////////////////////////////////////////
    
    return;
}



