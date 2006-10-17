/* 
 *   $Id: ad_svm_iwrite.c,v 1.2 2000/09/11 09:48:04 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_aio.h"

void ADIOI_SVM_IwriteContig(ADIO_File fd, void *buf, int count, MPI_Datatype datatype, 
			    int file_ptr_type, ADIO_Offset offset, ADIO_Request *request, 
			    int *error_code)  
{
    ADIO_Status status;
    int err = -1;
    int len, datatype_size;

    MPI_Type_size(datatype, &datatype_size);
    len = datatype_size * count;

    /* non-atomic mode => don't care about consisteny */
    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_WRITE;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    
#if 1
    /* bugs in ADIOI_SVM_aio */
    /* => use blocking version */
    ADIOI_SVM_WriteContig(fd, buf, count, datatype, file_ptr_type, offset, &status,
                    error_code);
		    
    (*request)->queued = 0;
 
#else
    /* MPI isn't thread-safe, but nonblocking versions use MPI-Routines... */
    if ((fd->iomode == M_ASYNC) || (fd->iomode == M_UNIX)) {
       if (file_ptr_type == ADIO_INDIVIDUAL) offset = fd->fp_ind;

       err = ADIOI_SVM_aio(fd, buf, len, offset, 1, 
                           &((*request)->handle));

       if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind += len;
    }
    
    (*request)->queued = 1;
    ADIOI_Add_req_to_list(request);

    /* added by RAY */
    if (err <= 0) {
       /* => error */   
       printf("ADIOI_SVM_IwriteContig(): Error in aio.\n");
    }
    else {
       /* => succesful */
       /* create new entry in thread-list */
       ADIOI_SVM_STRUCT_Create_thread((pthread_t *) &err,request);
       /*printf("Created new thread: %x\n",err);*/
    }
    /* end RAY */


    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
    
#endif

    fd->fp_sys_posn = -1;   /* set it to null. */

    fd->async_count++;
    
/* status info. must be linked to the request structure, so that it
   can be accessed later from a wait */
}




void ADIOI_SVM_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_WRITE;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    (*request)->queued = 0;
    (*request)->handle = 0;

/* call the blocking version. It is faster because it does data sieving. */
    ADIOI_SVM_WriteStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;

/* status info. must be linked to the request structure, so that it
   can be accessed later from a wait */

}

/* This function is for implementation convenience. It is not user-visible.
   It takes care of the differences in the interface for nonblocking I/O
   on various Unix machines! If wr==1 write, wr==0 read. */

int ADIOI_SVM_aio(ADIO_File fd, void *buf, int len, ADIO_Offset offset,
		  int wr, void *handle)
{
    int err=-1, error_code, fd_sys;

    aio_result_t *result;
    

    result = (aio_result_t *) ADIOI_Malloc(sizeof(aio_result_t));
    result->aio_return = AIO_INPROGRESS;
    /* changed by RAY */
    if (wr) err = ADIOI_SVM_Aiowrite(fd, buf, len, offset, result); 
    else err = ADIOI_SVM_Aioread(fd, buf, len, offset, result);

    if (err == -1) {
	if (errno == EAGAIN) { 
       /* the man pages say EPROCLIM, but in reality errno is set to EAGAIN! */

        /* exceeded the max. no. of outstanding requests.
           complete all previous async. requests and try again.*/

	    ADIOI_Complete_async(&error_code);
	    /* changed by RAY */
	    if (wr) err = ADIOI_SVM_Aiowrite(fd, buf, len, offset, result); 
	    else err = ADIOI_SVM_Aioread(fd, buf, len, offset, result);
	    /* end RAY */

	    while (err == -1) {
		if (errno == EAGAIN) {
                    /* sleep and try again */
                    sleep(1);
		    /* changed by RAY */
		    if (wr) err = ADIOI_SVM_Aioread(fd, buf, len, offset, result); 
		    else err = ADIOI_SVM_Aioread(fd, buf, len, offset, result);
		    /* end RAY */
		}
                else {
                    printf("Unknown errno %d in ADIOI_SVM_aio\n", errno);
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
	    }
	}
        else {
            printf("Unknown errno %d in ADIOI_SVM_aio\n", errno);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    *((aio_result_t **) handle) = result;

    return err;
}

