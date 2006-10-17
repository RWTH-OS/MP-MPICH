/* 
 *   $Id: ad_delete.c,v 1.1 2000/04/19 17:41:58 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 *
 */

#include "adio.h"

void ADIO_Delete(char *filename, int *error_code)
{
    int err;
   
#ifndef __SVM
    err = unlink(filename);
#else
    err = ADIOI_SVM_Delete(filename);
#endif

    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
