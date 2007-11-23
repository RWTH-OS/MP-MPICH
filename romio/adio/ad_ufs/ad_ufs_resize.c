/* 
 *   $Id: ad_ufs_resize.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_ufs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_UFS_Resize = PADIOI_UFS_Resize
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_UFS_Resize(ADIO_File fd, ADIO_Offset size,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_UFS_Resize")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_UFS_Resize  ADIOI_UFS_Resize
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_UFS_Resize as PADIOI_UFS_Resize
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_UFS->PADIOI_UFS */
#define UFS_BUILD_PROFILING
#include "ufsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


void ADIOI_UFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_UFS_RESIZE";
#endif
    
    err = ftruncate(fd->fd_sys, size);
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
