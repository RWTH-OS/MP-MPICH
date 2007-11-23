/* 
 *   $Id: ad_delete.c 920 2001-05-31 14:07:46Z karsten $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

#ifdef WIN32
void ADIO_Delete(char *filename, int *error_code)
{
    BOOL err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIO_DELETE";
#endif

    err = DeleteFile(filename);
#ifdef PRINT_ERR_MSG
    *error_code = (err) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (!err) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(GetLastError()));
	ADIOI_Error(MPI_FILE_NULL, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}
#else
void ADIO_Delete(char *filename, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIO_DELETE";
#endif

    err = unlink(filename);
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(MPI_FILE_NULL, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}
#endif
