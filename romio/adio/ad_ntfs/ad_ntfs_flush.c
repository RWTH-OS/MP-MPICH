/* 
 *   $Id: ad_ntfs_flush.c 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_Flush(ADIO_File fd, int *error_code)
{
    BOOL err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_Flush";
#endif
    *error_code = (fd->access_mode & (ADIO_WRONLY | ADIO_RDWR)) ? 
		(FlushFileBuffers(fd->fd_sys) ? MPI_SUCCESS:MPI_ERR_UNKNOWN):MPI_SUCCESS;

    if (*error_code != MPI_SUCCESS) {

#ifndef PRINT_ERR_MSG
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(GetLastError()));
	ADIOI_Error(fd, *error_code, myname);	    
    }
#endif
	
}
