/* 
 *   $Id: ad_pvfs_iread.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_IreadContig = PADIOI_PVFS_IreadContig
#pragma weak ADIOI_PVFS_IreadStrided = PADIOI_PVFS_IreadStrided
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_IreadContig  ADIOI_PVFS_IreadContig
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_IreadStrided  ADIOI_PVFS_IreadStrided
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_IreadContig as PADIOI_PVFS_IreadContig
#pragma _CRI duplicate ADIOI_PVFS_IreadStrided as PADIOI_PVFS_IreadStrided
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



void ADIOI_PVFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    ADIO_Status status;
    int len, typesize;

/* PVFS does not support nonblocking I/O. Therefore, use blocking I/O */

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->queued = 0;
    (*request)->datatype = datatype;

    MPI_Type_size(datatype, &typesize);
    len = count * typesize;
    ADIOI_PVFS_ReadContig(fd, buf, len, MPI_BYTE, file_ptr_type, offset, &status,
		    error_code);  

#ifdef HAVE_STATUS_SET_BYTES
    if (*error_code == MPI_SUCCESS) {
	MPI_Get_elements(&status, MPI_BYTE, &len);
	(*request)->nbytes = len;
    }
#endif
    fd->async_count++;
}



void ADIOI_PVFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;
#ifdef HAVE_STATUS_SET_BYTES
    int typesize;
#endif

/* PVFS does not support nonblocking I/O. Therefore, use blocking I/O */

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->queued = 0;
    (*request)->datatype = datatype;

    ADIOI_PVFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;

#ifdef HAVE_STATUS_SET_BYTES
    if (*error_code == MPI_SUCCESS) {
	MPI_Type_size(datatype, &typesize);
	(*request)->nbytes = count * typesize;
    }
#endif
}
