/* 
 *   $Id: ad_ufs_hints.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_ufs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_UFS_SetInfo = PADIOI_UFS_SetInfo
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_UFS_SetInfo(ADIO_File fd, MPI_Info users_info,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_UFS_SetInfo")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_UFS_SetInfo  ADIOI_UFS_SetInfo
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_UFS_SetInfo as PADIOI_UFS_SetInfo
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_UFS->PADIOI_UFS */
#define UFS_BUILD_PROFILING
#include "ufsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


void ADIOI_UFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    ADIOI_GEN_SetInfo(fd, users_info, error_code); 
}
