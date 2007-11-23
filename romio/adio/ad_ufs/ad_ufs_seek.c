/* 
 *   $Id: ad_ufs_seek.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_ufs.h"
#ifdef PROFILE
#include "mpe.h"
#endif


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_UFS_SeekIndividual = PADIOI_UFS_SeekIndividual
#elif defined(HAVE_ATTRIBUTE_WEAK)
ADIO_Offset ADIOI_UFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, int whence,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_UFS_SeekIndividual")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_UFS_SeekIndividual  ADIOI_UFS_SeekIndividual
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_UFS_SeekIndividual as PADIOI_UFS_SeekIndividual
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_UFS->PADIOI_UFS */
#define UFS_BUILD_PROFILING
#include "ufsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


ADIO_Offset ADIOI_UFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
