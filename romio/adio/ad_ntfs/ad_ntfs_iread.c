/* 
 * $Id: ad_ntfs_iread.c 922 2001-05-31 14:10:48Z karsten $
 * 
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_IreadContig(ADIO_File fd, void *buf, int count, 
			    MPI_Datatype datatype,int file_ptr_type,
			    ADIO_Offset offset, 
			    ADIO_Request *request, int *error_code)  
{
    ADIO_Status status;
    int err=-1;
    int len,typesize;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_IREADCONTIG";
#endif
    
    DWORD error;
    (*request) = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->datatype = datatype;
    (*request)->fd = fd;
    
    MPI_Type_size(datatype, &typesize);
    len = count * typesize;
    
    (*request)->nbytes = (*request)->totransfer = len;
    (*request)->buf = buf;
    
    if (file_ptr_type == ADIO_INDIVIDUAL) offset = fd->fp_ind;
    (*request)->offset = offset;
    err = ADIOI_NTFS_aio(fd, 0, *request,0,&error);
    if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind += len;
    
    if(error == ERROR_IO_PENDING) {
	(*request)->queued = 1;
	ADIOI_Add_req_to_list(request);
    } else
	(*request)->queued = 0;
    
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
	    myname, "I/O Error", "%s", ad_ntfs_error(error));
	ADIOI_Error(fd, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
    
    fd->fp_sys_posn = -1;   /* set it to null. */
    fd->async_count++;
    
}



void ADIOI_NTFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
	(*request)->datatype = datatype;
    (*request)->queued = 0;
    (*request)->handle = 0;

/* call the blocking version. It is faster because it does data sieving. */
    ADIOI_NTFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;


}
