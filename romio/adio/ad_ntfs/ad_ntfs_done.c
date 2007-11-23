/* 
 *   $Id: ad_ntfs_done.c 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#include "ad_ntfs.h"

int ADIOI_NTFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int done=0;
    OVERLAPPED *tmp;
    BOOL res;
    DWORD error = ERROR_SUCCESS,read;
    
    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }
    
    tmp = (OVERLAPPED *) (*request)->handle;
    if ((*request)->queued) {
	done=HasOverlappedIoCompleted(tmp);
	if (done) {
	    *error_code = MPI_SUCCESS;
	} else {
	    res=GetOverlappedResult((*request)->fd->fd_sys,tmp,&read,TRUE);
	    
	    if(res) {
		(*request)->totransfer -= read;
		if((*request)->totransfer>0) {
		    (*request)->buf += read;
		    (*request)->offset += read;
		}
		while(error == ERROR_SUCCESS && (*request)->totransfer>0) {
		    res = ADIOI_NTFS_aio((*request)->fd, (*request)->optype == ADIOI_WRITE, *request,0,&error);
		}
		if(error != ERROR_IO_PENDING) done = 1;
	    } else error = GetLastError();
	     
	    *error_code = (res  ?  MPI_SUCCESS:
	    ((error != ERROR_HANDLE_EOF)?MPI_ERR_IO:MPI_SUCCESS));	    
	}
    } else {
    /* ADIOI_Complete_Async completed this request, but request object
	was not freed. */
	done = 1;
	*error_code = MPI_SUCCESS;
    }
#ifndef PRINT_ERR_MSG
    if(*error_code != MPI_SUCCESS) {
    	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      "ADIOI_NTFS_ReadDone", "I/O Error", "I/O Error: %s", ad_ntfs_error(error));
	ADIOI_Error((*request)->fd, *error_code, "ADIOI_NTFS_ReadDone");	  
    }
#endif
    
    if (done) {
    /* if request is still queued in the system, it is also there
	on ADIOI_Async_list. Delete it from there. */
	if ((*request)->queued) ADIOI_Del_req_from_list(request);
	
	(*request)->fd->async_count--;
	
	if (tmp) FreeOverlappedStruct(tmp);
	ADIOI_Free_request((ADIOI_Req_node *) (*request));
	*request = ADIO_REQUEST_NULL;
	/* status to be filled */
	status->count = (*request)->nbytes-(*request)->totransfer;
	status->MPI_ERROR=*error_code;
    }
    return done;
    
}


int ADIOI_NTFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    return ADIOI_NTFS_ReadDone(request, status, error_code);
} 
