/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_xfs_done.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_xfs.h"

int ADIOI_XFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int err, done=0;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_XFS_READDONE";
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }

    if ((*request)->queued) {
	errno = aio_error64((const aiocb64_t *) (*request)->handle);
	if (errno == EINPROGRESS) {
	    done = 0;
	    *error_code = MPI_SUCCESS;
	}
	else {
	    err = aio_return64((aiocb64_t *) (*request)->handle); 
	    (*request)->nbytes = err;
	    errno = aio_error64((const aiocb64_t *) (*request)->handle);

	    done = 1;
	    if (err == -1) {
#ifdef MPICH2
		*error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
		    "**io %s", strerror(errno));
#elif defined(PRINT_ERR_MSG)
		*error_code = MPI_ERR_UNKNOWN;
#else /* MPICH-1 */
		*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
		ADIOI_Error((*request)->fd, *error_code, myname);	    
#endif
	    }
	    else *error_code = MPI_SUCCESS;
	}
    } /* if ((*request)->queued) */
    else {
	done = 1;
	*error_code = MPI_SUCCESS;
    }
#ifdef HAVE_STATUS_SET_BYTES
    if (done && ((*request)->nbytes != -1))
	MPIR_Status_set_bytes(status, (*request)->datatype, (*request)->nbytes);
#endif

    if (done) {
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
}


int ADIOI_XFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    return ADIOI_XFS_ReadDone(request, status, error_code);
} 
