/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_aio.h"

int ADIOI_SVM_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int err, nbytes, done=0, ret;


    printf("Entering ADIOI_SVM_ReadDone().\n");
    
    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }

    if ((*request)->next != ADIO_REQUEST_NULL) {
	done = ADIOI_SVM_ReadDone(&((*request)->next), status, error_code);
    /* currently passing status and error_code here, but something else
       needs to be done to get the status and error info correctly */
	if (!done) {
	   *error_code = MPI_SUCCESS;
	   return done;
	}
    }

#ifndef RAY
    (*request)->fd->async_count--;
    ADIOI_Free_request((ADIOI_Req_node *) (*request));
    *request = ADIO_REQUEST_NULL;
    *error_code = MPI_SUCCESS;
    return 1;
#else
    if ((*request)->queued) {
    	
	/* added and changed by RAY */
	/* search thread in thread-list */
	ret = ADIOI_SVM_STRUCT_Get_thread_done(request);
	done = ret;
	/* end RAY */
	
	*error_code = MPI_SUCCESS;
	
    }
    else {
	/* ADIOI_Complete_Async completed this request, but request object
           was not freed. */
	done = 1;
	*error_code = MPI_SUCCESS;
    }

    if (done) {
	/* added by RAY */
	/* remove thread from list */
	ADIOI_SVM_STRUCT_Remove_thread(request);
	/* end RAY */
	/* if request is still queued in the system, it is also there
           on ADIOI_Async_list. Delete it from there. */
	if ((*request)->queued) ADIOI_Del_req_from_list(request);

	(*request)->fd->async_count--;
	if ((*request)->handle) ADIOI_Free((*request)->handle);
	ADIOI_Free_request((ADIOI_Req_node *) (*request));
	*request = ADIO_REQUEST_NULL;
	/* status to be filled */
    }
    return done;
#endif
}


int ADIOI_SVM_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    printf("Entering ADIOI_SVM_WriteDone().\n");
    
    return ADIOI_SVM_ReadDone(request, status, error_code);
} 
