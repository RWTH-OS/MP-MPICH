/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs_resize.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"

void ADIOI_PVFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
    int ret, rank;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_PVFS_RESIZE";
#endif

    /* because MPI_File_set_size is a collective operation, and PVFS1 clients
     * do not cache metadata locally, one client can resize and broadcast the
     * result to the others */
    MPI_Comm_rank(fd->comm, &rank);
    if (rank == fd->hints->ranklist[0]) {
	err = pvfs_ftruncate64(fd->fd_sys, size);
    }
    MPI_Bcast(&err, 1, MPI_INT, 0, fd->comm);

    if (err == -1) {
#ifdef MPICH2
	*error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
	    "**io %s", strerror(errno));
#elif defined(PRINT_ERR_MSG)
	*error_code = MPI_ERR_UNKNOWN;
#else /* MPICH-1 */
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
#endif
    }
    else *error_code = MPI_SUCCESS;
}
