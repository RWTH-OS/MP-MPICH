/* 
 *   $Id: ad_pvfs_wait.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_ReadComplete = PADIOI_PVFS_ReadComplete
#pragma weak ADIOI_PVFS_WriteComplete = PADIOI_PVFS_WriteComplete
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_ReadComplete  ADIOI_PVFS_ReadComplete
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_WriteComplete  ADIOI_PVFS_WriteComplete
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_ReadComplete as PADIOI_PVFS_ReadComplete
#pragma _CRI duplicate ADIOI_PVFS_WriteComplete as PADIOI_PVFS_WriteComplete
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



void ADIOI_PVFS_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    if (*request == ADIO_REQUEST_NULL) {
        *error_code = MPI_SUCCESS;
        return;
    }

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, (*request)->datatype, (*request)->nbytes);
#endif
    (*request)->fd->async_count--;
    ADIOI_Free_request((ADIOI_Req_node *) (*request));
    *request = ADIO_REQUEST_NULL;
    *error_code = MPI_SUCCESS;
}

void ADIOI_PVFS_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    ADIOI_PVFS_ReadComplete(request, status, error_code);
}
