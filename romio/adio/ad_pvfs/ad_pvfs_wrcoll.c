/* 
 *   $Id: ad_pvfs_wrcoll.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_WriteStridedColl = PADIOI_PVFS_WriteStridedColl
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_WriteStridedColl  ADIOI_PVFS_WriteStridedColl
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_WriteStridedColl as PADIOI_PVFS_WriteStridedColl
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif




void ADIOI_PVFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_WriteStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
