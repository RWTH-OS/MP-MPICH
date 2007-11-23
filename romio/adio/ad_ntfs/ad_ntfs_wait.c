/*
 * $Id: ad_ntfs_wait.c 922 2001-05-31 14:10:48Z karsten $
 *
 */


#include "ad_ntfs.h"

void ADIOI_NTFS_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    OVERLAPPED *tmp;
    BOOL res;
    DWORD error;
    
    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return;
    }
    
    
    tmp = (OVERLAPPED *) (*request)->handle;
    
    if ((*request)->queued) {  /* dequeue it */
	    res=GetOverlappedResult((*request)->fd->fd_sys,tmp,(DWORD*)&status->count,TRUE);
	    if(res && (*request)->totransfer>0) {
		res = ADIOI_NTFS_aio((*request)->fd, (*request)->optype == ADIOI_WRITE, *request,1,&error);
	    } else error = GetLastError();
	    
	    *error_code = (res  ?  MPI_SUCCESS:
	    ((error != ERROR_HANDLE_EOF)?MPI_ERR_IO:MPI_SUCCESS));	    
    }  else {
	*error_code = MPI_SUCCESS;
	
    }
    
#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, MPI_BYTE, (*request)->nbytes-(*request)->totransfer);
#endif
    
    /*status->count = (*request)->nbytes-(*request)->totransfer;*/
    
    if ((*request)->queued != -1) {
	
    /* queued = -1 is an internal hack used when the request must
	   be completed, but the request object should not be
	   freed. This is used in ADIOI_Complete_async, because the user
	   will call MPI_Wait later, which would require status to
	   be filled. Ugly but works. queued = -1 should be used only
	   in ADIOI_Complete_async. 
	This should not affect the user in any way. */
	
	/* if request is still queued in the system, it is also there
	on ADIOI_Async_list. Delete it from there. */
	if ((*request)->queued) ADIOI_Del_req_from_list(request);
	
	(*request)->fd->async_count--;
	
	if (tmp) {
	    FreeOverlappedStruct(tmp);
	}
	ADIOI_Free_request((ADIOI_Req_node *) (*request));
	*request = ADIO_REQUEST_NULL;
    }
    
#ifndef PRINT_ERR_MSG
    if(*error_code != MPI_SUCCESS) {
    	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      "ADIOI_NTFS_ReadComplete", "I/O Error", "I/O Error: %s", ad_ntfs_error(error));
	ADIOI_Error((*request)->fd, *error_code, "ADIOI_NTFS_ReadComplete");	  
    }
#endif    
    status->MPI_ERROR=*error_code;
}


void ADIOI_NTFS_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    ADIOI_NTFS_ReadComplete(request, status, error_code);
}
