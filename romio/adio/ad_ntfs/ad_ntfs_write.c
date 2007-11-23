/* 
 *   $Id: ad_ntfs_write.c 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_WriteContig(ADIO_File fd, void *buf, int count, 
			    MPI_Datatype datatype,int file_ptr_type,
			    ADIO_Offset offset, ADIO_Status *status, 
			    int *error_code)
{
    int err=-1, datatype_size, len;
    DWORD Written=0,error;
    struct ADIOI_RequestD requ;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_WRITECONTIG";
#endif
    
    MPI_Type_size(datatype, &datatype_size);
    Written=len = datatype_size * count;
    requ.nbytes = requ.totransfer = len;
    requ.buf = buf;
    
    
    
    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
	requ.offset = offset;
	err=ADIOI_NTFS_aio(fd, 1, &requ,1, &error);
	fd->fp_sys_posn = offset + len-requ.totransfer;
	/* individual file pointer not updated */        
    }
    else {  /* read from curr. location of ind. file pointer */
	requ.offset = fd->fp_ind;
	err=ADIOI_NTFS_aio(fd, 1, &requ,1, &error);
	fd->fp_ind += len-requ.totransfer; 
	fd->fp_sys_posn = fd->fp_ind;
    } 
    
#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, datatype, len-requ.totransfer);
#endif
    
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == -1) ? MPI_ERR_IO : MPI_SUCCESS;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
	    myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(error));
	ADIOI_Error(fd, *error_code, myname);
    }
    else *error_code = MPI_SUCCESS;
#endif
}



void ADIOI_NTFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_WriteStrided(fd, buf, count, datatype, file_ptr_type,
                        offset, status, error_code);
}
