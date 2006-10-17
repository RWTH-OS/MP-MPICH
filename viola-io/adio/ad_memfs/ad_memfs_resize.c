/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_ufs_resize.c,v 1.10 2003/04/18 20:15:04 David Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_memfs.h"

void ADIOI_MEMFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    thread_comm_io(MEMFS_RESIZE, fd, NULL, error_code, 0, (MPI_Datatype) NULL, 0, size, NULL, NULL, NULL);
}
