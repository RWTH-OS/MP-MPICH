/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_memfs_fcntl.c,v 1.15 2004/07/27 20:44:04 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *   
 *   Modified for memfs filesystem by Jan Seidel
 */

#include "ad_memfs.h"
#include "adio_extern.h"

void ADIOI_MEMFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    thread_comm_io(MEMFS_FCNTL, fd, NULL, error_code, flag, (MPI_Datatype) NULL, 0, (ADIO_Offset) NULL, NULL, fcntl_struct, NULL);

}
