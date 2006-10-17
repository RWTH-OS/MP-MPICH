/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_read.c                                           * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 

#include "ad_memfs.h"

void ADIOI_MEMFS_ReadContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    UNLOCK_MPI();


#ifdef DEBUG_MEMFS
    int comm_rank;
    MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
    fprintf(stderr, "MEMFS_ReadContig: [%d] offset: %lld, count: %d\n", comm_rank, offset, count);
#endif

#ifdef MEMFS_TIME
    double t1, t2;
    t1 = gettime();
    t2 = gettime();
    settime(REFERENCE, t2-t1);
    t1 = gettime();
#endif
    
    /* communicate with MEMFS main thread */
    thread_comm_io(MEMFS_READCONT, fd, buf, error_code, count, datatype, file_ptr_type, offset, status, NULL, NULL);

#ifdef MEMFS_TIME
    t2 = gettime();
    settime(READ_TIME, t2 - t1);
    settime(TOTAL_TIME, t2 - t1);
#endif

    LOCK_MPI();
}



void ADIOI_MEMFS_ReadStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    UNLOCK_MPI();

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_ReadStrided: Calling ADIOI_GEN_ReadStrided\n");
#endif
    thread_comm_io(MEMFS_READSTRIDED, fd, buf, error_code, count, datatype, file_ptr_type, offset, status, NULL, NULL);

    LOCK_MPI();
}
