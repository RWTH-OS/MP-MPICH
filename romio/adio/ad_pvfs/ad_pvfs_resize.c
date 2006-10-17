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
#pragma weak ADIOI_PVFS_Resize = PADIOI_PVFS_Resize
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_Resize  ADIOI_PVFS_Resize
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_Resize as PADIOI_PVFS_Resize
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



void ADIOI_PVFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
    int rank;
    static char myname[] = "ADIOI_PVFS_RESIZE";

    /* because MPI_File_set_size is a collective operation, and PVFS1 clients
     * do not cache metadata locally, one client can resize and broadcast the
     * result to the others */
    MPI_Comm_rank(fd->comm, &rank);
    if (rank == fd->hints->ranklist[0]) {
	err = pvfs_ftruncate64(fd->fd_sys, size);
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
