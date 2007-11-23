/* 
 *   $Id: ad_svm_flush.c 51 2000-04-19 17:42:21Z joachim $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm.h"

void ADIOI_SVM_Flush(ADIO_File fd, int *error_code)
{
    int err;

    err = ADIOI_SVM_Sync(fd);
    
    /* the barrier ensures that no process races ahead 
       before all processes have updated the file. */
    MPI_Barrier(fd->comm);

    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
