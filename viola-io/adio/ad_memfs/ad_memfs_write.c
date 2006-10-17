/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_ufs_write.c,v 1.12 2003/04/18 20:15:04 David Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_memfs.h"


void ADIOI_MEMFS_WriteContig(ADIO_File fd, void *buf, int count, 
                   MPI_Datatype datatype, int file_ptr_type,
                   ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    UNLOCK_MPI();

#ifdef DEBUG_THREADS
    int comm_rank;
    MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
    fprintf(stderr, "MEMFS_WriteContig [%d]: offset: %lld, count: %d\n", comm_rank, offset, count);
#endif

#ifdef MEMFS_TIME
    double t1, t2;
    t1 = gettime();
#endif

    assert(offset >= 0);

    /* communicate with MEMFS main thread */
    thread_comm_io(MEMFS_WRITECONT, fd, buf, error_code, count, datatype, file_ptr_type, offset, status, NULL, NULL);

#ifdef MEMFS_TIME
    t2 = gettime();
    settime(WRITE_TIME, t2-t1);
    settime(TOTAL_TIME, t2-t1);
#endif

    LOCK_MPI();
}



void ADIOI_MEMFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    UNLOCK_MPI();
    fprintf(stderr, "Using MEMFS_WRITESTRIDED\n");
    thread_comm_io(MEMFS_WRITESTRIDED, fd, buf, error_code, count, datatype, file_ptr_type, offset, status, NULL, NULL);

    LOCK_MPI();
}
