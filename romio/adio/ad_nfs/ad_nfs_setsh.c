/* 
 *   $Id: ad_nfs_setsh.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"



#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_NFS_Set_shared_fp = PADIOI_NFS_Set_shared_fp
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_NFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_Set_shared_fp")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_NFS_Set_shared_fp  ADIOI_NFS_Set_shared_fp
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_NFS_Set_shared_fp as PADIOI_NFS_Set_shared_fp
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_NFS->PADIOI_NFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif




/* set the shared file pointer to "offset" etypes relative to the current 
   view */

void ADIOI_NFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset, int *error_code)
{
    int err;
    MPI_Comm dupcommself;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NFS_SET_SHARED_FP";
#endif

    if (fd->shared_fp_fd == ADIO_FILE_NULL) {
	MPI_Comm_dup(MPI_COMM_SELF, &dupcommself);
	fd->shared_fp_fd = ADIO_Open(dupcommself, fd->shared_fp_fname, 
             fd->file_system, ADIO_CREATE | ADIO_RDWR | ADIO_DELETE_ON_CLOSE, 
             0, MPI_BYTE, MPI_BYTE, M_ASYNC, MPI_INFO_NULL, 
             ADIO_PERM_NULL, error_code);
    }

    if (*error_code != MPI_SUCCESS) return;

    ADIOI_WRITE_LOCK(fd->shared_fp_fd, 0, SEEK_SET, sizeof(ADIO_Offset));
    lseek(fd->shared_fp_fd->fd_sys, 0, SEEK_SET);
    err = write(fd->shared_fp_fd->fd_sys, &offset, sizeof(ADIO_Offset));
    ADIOI_UNLOCK(fd->shared_fp_fd, 0, SEEK_SET, sizeof(ADIO_Offset));

#ifdef PRINT_ERR_MSG
    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}

