/* 
 *   $Id: ad_nfs_seek.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"
#ifdef PROFILE
#include "mpe.h"
#endif



#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_NFS_SeekIndividual = PADIOI_NFS_SeekIndividual
#elif defined(HAVE_ATTRIBUTE_WEAK)
ADIO_Offset ADIOI_NFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_SeekIndividual")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_NFS_SeekIndividual  ADIOI_NFS_SeekIndividual
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_NFS_SeekIndividual as PADIOI_NFS_SeekIndividual
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_NFS->PADIOI_NFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif




ADIO_Offset ADIOI_NFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
