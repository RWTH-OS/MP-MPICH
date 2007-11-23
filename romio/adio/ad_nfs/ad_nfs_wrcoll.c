/* 
 *   $Id: ad_nfs_wrcoll.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"



#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_NFS_WriteStridedColl = PADIOI_NFS_WriteStridedColl
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_NFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_WriteStridedColl")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_NFS_WriteStridedColl  ADIOI_NFS_WriteStridedColl
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_NFS_WriteStridedColl as PADIOI_NFS_WriteStridedColl
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_NFS->PADIOI_NFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



void ADIOI_NFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_WriteStridedColl(fd, buf, count, datatype, file_ptr_type,
                              offset, status, error_code);
}
