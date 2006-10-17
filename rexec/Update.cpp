#include <windows.h>
#include <stdio.h>
#include "helpers.h"
#include "cluma.h"
#include "SERVICE.H"
#include "Update.h"


DWORD GetServiceHandle(SC_HANDLE *schService) {
        
    SC_HANDLE   schSCManager;
    DWORD err = ERROR_SUCCESS;
    
    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );
    
    if ( schSCManager != NULL ) {
        *schService = OpenService(schSCManager,SZSERVICENAME,SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP);
	if(*schService == NULL) {
	    fprintf(stderr,"OpenService() failed\n");
	    fflush(stderr);
	    err = GetLastError();
	}
	CloseServiceHandle(schSCManager);
    } else {
	err = GetLastError();
	fprintf(stderr,"OpenSCManager() failed\n");
	fflush(stderr);
    }
    return err;
}


DWORD MoveFiles(char *NewName) {
    char szPath[512],backupPath[512];
    DWORD res=ERROR_SUCCESS;

    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        return GetLastError();
    }
    strcpy(backupPath,szPath);
    strcat(backupPath,".old");
    // Just to be sure...
    DeleteFile(backupPath);
    if(!MoveFile(szPath,backupPath)) {
	res =GetLastError();
	fprintf(stderr,"MoveFile(%s,%s) failed (%d)\n",szPath,backupPath,res);
	fflush(stderr);
	return res;
    }
    if(!MoveFile(NewName,szPath)) {
	res = GetLastError();
	fprintf(stderr,"MoveFile(%s,%s) failed (%d)\n",NewName,szPath,res);
	fflush(stderr);
	MoveFile(backupPath,szPath);
	return res;
    }

    return res;
}

DWORD UpdateService(char *NewName) {
    SC_HANDLE schService;
    DWORD res = ERROR_SUCCESS,
	  count = 0,
	  loop = 0,
	  lastCheck;
    SERVICE_STATUS status;
    res =  GetServiceHandle(&schService);
    if(res != ERROR_SUCCESS) return res;
  
    // Stop the actually running service
    if(!ControlService(schService,SERVICE_CONTROL_STOP,&status)) {
	res = GetLastError();
	CloseServiceHandle(schService);
	fprintf(stderr,"ControlService(SERVICE_CONTROL_STOP) failed (%d)\n",res);
	fflush(stderr);
	return res;
    }
    res = MoveFiles(NewName);
    QueryServiceStatus(schService,&status);
    lastCheck = status.dwCheckPoint;
    do {
	switch(status.dwCurrentState) {
	case SERVICE_STOPPED: break;
	case SERVICE_STOP_PENDING: Sleep(status.dwWaitHint); 
	                           // Fall through
	default: 
	    if(!QueryServiceStatus(schService,&status)) {
		res = GetLastError();
		status.dwCurrentState = SERVICE_STOPPED;
	    }
	    if(status.dwCurrentState != SERVICE_STOPPED && lastCheck == status.dwCheckPoint) {
	        ++loop;
	        Sleep(2000);
	    } else {
		loop = 0;
		lastCheck = status.dwCheckPoint;
	    }
	    break;
	}	
    } while(loop <=5 && status.dwCurrentState != SERVICE_STOPPED);
    
    
    if(!StartService(schService,0,NULL)) {
	res = GetLastError();
	fprintf(stderr,"StartService failed (%d)\n",res);
	fflush(stderr);
    }
    
    CloseServiceHandle(schService);

    return res;
}

static error_status_t Transfer( char *Filename,byte_pipe *data) {
    HANDLE hFile;
    byte pcBuffer[MAX_BUFFER_SIZE];  // Buffer to store the elements in
    DWORD transferred;

    unsigned long nActualTransferCount;   // Actual number of elements received 
    BOOL bDone = FALSE;         // Inidicates when the pipe is done
    //DWORD transferred,res ;

    DWORD res=RPC_S_OK;
    hFile = CreateFile((const char*)&(Filename[0]),GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
    if(hFile == INVALID_HANDLE_VALUE) {
	DBG("CreateFile(\"Filename\") failed\n");
	res = GetLastError();
	return res;
    }

	char DateTime[25];
    DBG(GetDateTimeString(DateTime)<<": Transferring file "<<Filename);

    while(FALSE == bDone)
    {
        data->pull(      // Grab a chunck of data from the client
            data->state, // Pointer to the state 
            pcBuffer,           // Buffer to put data in
            MAX_BUFFER_SIZE/sizeof(byte),    // Max number of elements to receive
            &nActualTransferCount); // Actual number of elements received

        // A data transfer count of 0 means the end of the pipe
        if(nActualTransferCount == 0) {
            bDone = TRUE;
        } else  {
            if(!WriteFile(hFile,pcBuffer,nActualTransferCount,&transferred,0)) {
		res = GetLastError();
		CloseHandle(hFile);
		DeleteFile((const char*)&(Filename[0]));
		DBG("WriteFile() failed ("<<res<<")")
		return res;
	    }
        }
    }
    DBG("Done");
    CloseHandle(hFile);
    return res;
}


error_status_t R_Transfer(handle_t binding, byte_pipe *data) {
    

    DWORD res;
    res = RpcImpersonateClient(binding);
    
    if (res != RPC_S_OK) {
        return res;
    }
    res = Transfer(".\\rcluma.new",data);
    RpcRevertToSelf();
    return res;
}

error_status_t R_PutFile( 
    /* [in] */ handle_t binding,
    /* [ref][in] */ unsigned char __RPC_FAR __RPC_FAR Filename[ MAX_SIZE ],
    /* [ref][in] */ byte_pipe __RPC_FAR *data) {
            DWORD res;
    res = RpcImpersonateClient(binding);
    
    if (res != RPC_S_OK) {
        return res;
    }
    res = Transfer((char*)&(Filename[0]),data);
    RpcRevertToSelf();
    return res;
}



