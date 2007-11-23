/* 
 *   $Id: ad_nfs_iread.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"




#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_NFS_IreadContig = PADIOI_NFS_IreadContig
#pragma weak ADIOI_NFS_IreadStrided = PADIOI_NFS_IreadStrided
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_NFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request,
				int *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_IreadContig")));
void ADIOI_NFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Request *request, int
               *error_code) __attribute__ ((weak, alias ("PADIOI_NFS_IreadStrided")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_NFS_IreadContig  ADIOI_NFS_IreadContig
#pragma _HP_SECONDARY_DEF PADIOI_NFS_IreadStrided  ADIOI_NFS_IreadStrided
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_NFS_IreadContig as PADIOI_NFS_IreadContig
#pragma _CRI duplicate ADIOI_NFS_IreadStrided as PADIOI_NFS_IreadStrided
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_NFS->PADIOI_NFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif




void ADIOI_NFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    int len, typesize;
#ifdef NO_AIO
    ADIO_Status status;
#else
    int err=-1;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NFS_IREADCONTIG";
#endif
#endif

    (*request) = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->datatype = datatype;

    MPI_Type_size(datatype, &typesize);
    len = count * typesize;

#ifdef NO_AIO
    /* HP, FreeBSD, Linux */
    /* no support for nonblocking I/O. Use blocking I/O. */

    ADIOI_NFS_ReadContig(fd, buf, len, MPI_BYTE, file_ptr_type, offset, 
                         &status, error_code);  
    (*request)->queued = 0;
#ifdef HAVE_STATUS_SET_BYTES
    if (*error_code == MPI_SUCCESS) {
	MPI_Get_elements(&status, MPI_BYTE, &len);
	(*request)->nbytes = len;
    }
#endif

#else
    if (file_ptr_type == ADIO_INDIVIDUAL) offset = fd->fp_ind;
    err = ADIOI_NFS_aio(fd, buf, len, offset, 0, &((*request)->handle));
    if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind += len;

    (*request)->queued = 1;
    ADIOI_Add_req_to_list(request);

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
#endif

    fd->fp_sys_posn = -1;   /* set it to null. */
    fd->async_count++;
}



void ADIOI_NFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;
#ifdef HAVE_STATUS_SET_BYTES
    int typesize;
#endif

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->datatype = datatype;
    (*request)->queued = 0;
    (*request)->handle = 0;

/* call the blocking version. It is faster because it does data sieving. */
    ADIOI_NFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;

#ifdef HAVE_STATUS_SET_BYTES
    if (*error_code == MPI_SUCCESS) {
	MPI_Type_size(datatype, &typesize);
	(*request)->nbytes = count * typesize;
    }
#endif
}
