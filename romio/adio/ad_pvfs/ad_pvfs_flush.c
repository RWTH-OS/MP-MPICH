/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_Flush = PADIOI_PVFS_Flush
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_Flush  ADIOI_PVFS_Flush
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_Flush as PADIOI_PVFS_Flush
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


void ADIOI_PVFS_Flush(ADIO_File fd, int *error_code)
{
    int err, rank, dummy=0, dummy_in=0;
    static char myname[] = "ADIOI_PVFS_FLUSH";

    /* a collective routine: because we do not cache data in PVFS1, one process
     * can initiate the fsync operation and broadcast the result to the others.
     * One catch: MPI_File_sync has special meaning with respect to file system
     * consistency.  Ensure no clients have outstanding write operations.
     */

    MPI_Comm_rank(fd->comm, &rank);
    MPI_Reduce(&dummy_in, &dummy, 1, MPI_INT, MPI_SUM, 
		    fd->hints->ranklist[0], fd->comm);
    if (rank == fd->hints->ranklist[0]) {
	    err = pvfs_fsync(fd->fd_sys);
    }
    MPI_Bcast(&err, 1, MPI_INT, 0, fd->comm);

    if (err == -1) {
	*error_code = MPIO_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
					   myname, __LINE__, MPI_ERR_IO,
					   "**io",
					   "**io %s", strerror(errno));
    }
    else *error_code = MPI_SUCCESS;
}
