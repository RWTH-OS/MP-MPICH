/* 
 *   $Id: ad_ntfs_close.c 922 2001-05-31 14:10:48Z karsten $    
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_Close(ADIO_File fd, int *error_code)
{
    BOOL err;

    
	/* NT does not sync the file on close, but the standard 
	   requires this.
	*/
	ADIOI_NTFS_Flush(fd,error_code);

	err=CloseHandle(fd->fd_sys);
	if(!err) {
#ifdef PRINT_ERR_MSG
	    *error_code = MPI_ERR_UNKNOWN;
#else
	    *error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      "ADIOI_NTFS_Close", "I/O Error", "I/O Error: %s", ad_ntfs_error(GetLastError()));
	    ADIOI_Error(fd, *error_code, "ADIOI_NTFS_Close");	    
#endif
	}


}
