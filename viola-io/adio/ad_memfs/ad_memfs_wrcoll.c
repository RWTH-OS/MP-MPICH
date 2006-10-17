/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_ufs_wrcoll.c,v 1.3 2002/10/24 17:01:08 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_memfs.h"

void ADIOI_MEMFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_WriteStridedColl: Calling ADIOI_GEN_WriteStridedColl\n");    
#endif
    
    ADIOI_GEN_WriteStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
