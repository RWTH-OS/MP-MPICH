/* 
 *   $Id: ad_svm_wait.c,v 1.1 2000/04/19 17:42:13 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_aio.h"

void ADIOI_SVM_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int err, nbytes, ret;
    

    /*printf("Entering ADIOI_SVM_ReadComplete().\n");*/
    
    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return;
    }

    if (((*request)->next != ADIO_REQUEST_NULL) && ((*request)->queued != -1))
	/* the second condition is to take care of the ugly hack in
	   ADIOI_Complete_async */
	ADIOI_SVM_ReadComplete(&((*request)->next), status, error_code);
    /* currently passing status and error_code here, but something else
       needs to be done to get the status and error info correctly */

#ifdef RAY

    if ((*request)->queued) {  /* dequeue it */
    	
	/* added by RAY */
	/* wait until thread terminates */
	do {
	    ret = ADIOI_SVM_STRUCT_Get_thread_done(request);
	    /* suspend execution for 100 microseconds - is it a good value ? */
	    usleep(100);
	} while (!ret);
	/* Remove thread from list... */
	ADIOI_SVM_STRUCT_Remove_thread(request);
	/* end RAY */
	
	*error_code = MPI_SUCCESS;
    }
    else *error_code = MPI_SUCCESS;

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
	if ((*request)->handle) ADIOI_Free((*request)->handle);
	ADIOI_Free_request((ADIOI_Req_node *) (*request));
	*request = ADIO_REQUEST_NULL;
    }
#else
    (*request)->fd->async_count--;
    ADIOI_Free_request((ADIOI_Req_node *) (*request));
    *request = ADIO_REQUEST_NULL;
    *error_code = MPI_SUCCESS;

#endif


    /* status to be filled */
}


void ADIOI_SVM_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    /*printf("Entering ADIOI_SVM_WriteComplete().\n");*/
    
    ADIOI_SVM_ReadComplete(request, status, error_code);
}
