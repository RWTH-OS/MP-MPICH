/* 
 *   $Id: ad_svm_iread.c 240 2000-09-11 09:48:08Z joachim $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_aio.h"

void ADIOI_SVM_IreadContig(ADIO_File fd, void *buf, int count, MPI_Datatype datatype, 
			   int file_ptr_type, ADIO_Offset offset, ADIO_Request *request, 
			   int *error_code)  
{
    ADIO_Status status;
    int err = -1;
    int len, datatype_size;

    MPI_Type_size(datatype, &datatype_size);
    len = datatype_size * count;

    (*request) = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    
#if 1
    /* bugs in ADIOI_SVM_aio */
    /* => use blocking version */
    
    ADIOI_SVM_ReadContig(fd, buf, count, datatype, file_ptr_type, offset, &status,
			 error_code);
    (*request)->queued = 0;

#else
    /* MPI isn't thread-safe, but nonblocking versions use MPI-Routines... */
    if ((fd->iomode == M_ASYNC) || (fd->iomode == M_UNIX)) {
        if (file_ptr_type == ADIO_INDIVIDUAL) offset = fd->fp_ind;
	
        err = ADIOI_SVM_aio(fd, buf, len, offset, 0, 
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

    /* added by RAY - 03.01.2000 */
#endif

    fd->fp_sys_posn = -1;   /* set it to null. */
    
    fd->async_count++;

    /* status info. must be linked to the request structure, so that it
       can be accessed later from a wait */
}



void ADIOI_SVM_IreadStrided(ADIO_File fd, void *buf, int count, 
			    MPI_Datatype datatype, int file_ptr_type,
			    ADIO_Offset offset, ADIO_Request *request, int
			    *error_code)
{
    ADIO_Status status;

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    (*request)->queued = 0;
    (*request)->handle = 0;

    /* call the blocking version. It is faster because it does data sieving. */
    ADIOI_SVM_ReadStrided(fd, buf, count, datatype, file_ptr_type, 
			  offset, &status, error_code);  

    fd->async_count++;
    
    /* status info. must be linked to the request structure, so that it
       can be accessed later from a wait */

}
