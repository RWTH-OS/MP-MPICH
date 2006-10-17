/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_ufs_seek.c,v 1.3 2002/10/24 17:01:08 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_memfs.h"

ADIO_Offset ADIOI_MEMFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    ADIO_Offset off;
    off = (ADIO_Offset) thread_comm_io(MEMFS_SEEKIND, fd, NULL, error_code, whence, (MPI_Datatype) NULL, 0, offset, NULL, NULL, NULL);
    return off;
}
