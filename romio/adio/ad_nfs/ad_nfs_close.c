/* 
 *   $Id: ad_nfs_close.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_NFS_Close = PADIOI_NFS_Close
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_NFS_Close(ADIO_File fd,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_Close")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_NFS_Close  ADIOI_NFS_Close
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_NFS_Close as PADIOI_NFS_Close
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_NFS->PADIOI_NFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


void ADIOI_NFS_Close(ADIO_File fd, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NFS_CLOSE";
#endif
    
    err = close(fd->fd_sys);
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}
