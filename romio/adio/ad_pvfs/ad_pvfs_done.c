/* 
 *   $Id: ad_pvfs_done.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_ReadDone = PADIOI_PVFS_ReadDone
#pragma weak ADIOI_PVFS_WriteDone = PADIOI_PVFS_WriteDone
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_ReadDone  ADIOI_PVFS_ReadDone
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_WriteDone  ADIOI_PVFS_WriteDone
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_ReadDone as PADIOI_PVFS_ReadDone
#pragma _CRI duplicate ADIOI_PVFS_WriteDone as PADIOI_PVFS_WriteDone
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



int ADIOI_PVFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    if (*request != ADIO_REQUEST_NULL) {
#ifdef HAVE_STATUS_SET_BYTES
	MPIR_Status_set_bytes(status, (*request)->datatype, (*request)->nbytes);
#endif
	(*request)->fd->async_count--;
	ADIOI_Free_request((ADIOI_Req_node *) (*request));
	*request = ADIO_REQUEST_NULL;
    }

    *error_code = MPI_SUCCESS;
    return 1;
}


int ADIOI_PVFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    return ADIOI_PVFS_ReadDone(request, status, error_code);
} 
