/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_ufs_hints.c,v 1.3 2002/10/24 17:01:07 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_memfs.h"

void ADIOI_MEMFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    UNLOCK_MPI();

#ifdef MEMFS_TIME
    double t1, t2;
    t1 = gettime();
#endif

    /* MEMFS_Init(2); */
    thread_comm_io(MEMFS_SETINFO, fd, NULL, error_code, 0, (MPI_Datatype) NULL, 0, (ADIO_Offset) NULL, NULL, NULL, (void *)users_info);

#ifdef MEMFS_TIME
    t2 = gettime();
    settime(TOTAL_TIME, t2-t1);
#endif


    LOCK_MPI();
}
