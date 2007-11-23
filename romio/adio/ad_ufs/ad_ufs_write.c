/* 
 *   $Id: ad_ufs_write.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_ufs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_UFS_WriteContig = PADIOI_UFS_WriteContig
#pragma weak ADIOI_UFS_WriteStrided = PADIOI_UFS_WriteStrided
#elif defined(HAVE_ATTRIBUTE_WEAK)
void ADIOI_UFS_WriteContig(ADIO_File fd, void *buf, int count, 
	MPI_Datatype datatype, int file_ptr_type,
	ADIO_Offset offset, ADIO_Status *status,
	int *error_code) __attribute__ ((weak, alias ("PADIOI_UFS_WriteContig")));
void ADIOI_UFS_WriteStrided(ADIO_File fd, void *buf, int count,
	MPI_Datatype datatype, int file_ptr_type,
	ADIO_Offset offset, ADIO_Status *status, int
	*error_code) __attribute__ ((weak, alias ("PADIOI_UFS_WriteStrided")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_UFS_WriteContig  ADIOI_UFS_WriteContig
#pragma _HP_SECONDARY_DEF PADIOI_UFS_WriteStrided  ADIOI_UFS_WriteStrided
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_UFS_WriteContig as PADIOI_UFS_WriteContig
#pragma _CRI duplicate ADIOI_UFS_WriteStrided as PADIOI_UFS_WriteStrided
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_UFS->PADIOI_UFS */
#define UFS_BUILD_PROFILING
#include "ufsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


void ADIOI_UFS_WriteContig(ADIO_File fd, void *buf, int count, 
                   MPI_Datatype datatype, int file_ptr_type,
	           ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    int err=-1, datatype_size, len;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_UFS_WRITECONTIG";
#endif

    MPI_Type_size(datatype, &datatype_size);
    len = datatype_size * count;

    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
	if (fd->fp_sys_posn != offset)
	    lseek(fd->fd_sys, offset, SEEK_SET);
	err = write(fd->fd_sys, buf, len);
	fd->fp_sys_posn = offset + err;
	/* individual file pointer not updated */        
    }
    else { /* write from curr. location of ind. file pointer */
	if (fd->fp_sys_posn != fd->fp_ind)
	    lseek(fd->fd_sys, fd->fp_ind, SEEK_SET);
	err = write(fd->fd_sys, buf, len);
	fd->fp_ind += err;
	fd->fp_sys_posn = fd->fp_ind;
    }

#ifdef HAVE_STATUS_SET_BYTES
    if (err != -1) MPIR_Status_set_bytes(status, datatype, err);
#endif

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
}



void ADIOI_UFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_WriteStrided(fd, buf, count, datatype, file_ptr_type,
                        offset, status, error_code);
}
