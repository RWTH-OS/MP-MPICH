/* 
 *   $Id: ad_ntfs_open.c 922 2001-05-31 14:10:48Z karsten $    
 *
*/

#include "ad_ntfs.h"

#if defined(_WIN32) && ! defined(FILE_SHARE_DELETE)
#define FILE_SHARE_DELETE               0x00000004  
#endif

void ADIOI_NTFS_Open(ADIO_File fd, int *error_code)
{
    /*int perm, old_mask, amode;*/
	DWORD Access,Mode,Flags,error;
	LARGE_INTEGER Pos;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_OPEN";
#endif

	/* TODO: Set access rights correctly */
#if 0
    if (fd->perm == ADIO_PERM_NULL) {
	old_mask = umask(022);
	umask(old_mask);
	perm = old_mask ^ 0666;
    }
    else perm = fd->perm;
#endif

    
	Access = 0;

	if (fd->access_mode & ADIO_RDONLY)
		Access |=  GENERIC_READ;
	if (fd->access_mode & ADIO_WRONLY)
		Access |=  GENERIC_WRITE;
	if (fd->access_mode & ADIO_RDWR)
		Access =  GENERIC_READ|GENERIC_WRITE;

	Mode = OPEN_EXISTING;

    if (fd->access_mode & ADIO_CREATE) {
		if (fd->access_mode & ADIO_EXCL) {
			Mode = CREATE_NEW;
		} else {
			Mode = OPEN_ALWAYS;
		}

	}
    if(fd->access_mode & ADIO_SEQUENTIAL)
		Flags = FILE_FLAG_SEQUENTIAL_SCAN;
	else
		Flags = FILE_FLAG_RANDOM_ACCESS;
	if(fd->iomode == M_ASYNC || fd->iomode == M_UNIX)
		Flags |= FILE_FLAG_OVERLAPPED;

	fd->fd_sys=CreateFile(fd->filename,Access,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		       Mode,Flags,NULL);

    if ((fd->fd_sys != INVALID_HANDLE_VALUE) && (fd->access_mode & ADIO_APPEND)) {
		Pos.QuadPart=0;
		Pos.LowPart=SetFilePointer(fd->fd_sys,Pos.LowPart,&Pos.HighPart,FILE_END);
		fd->fp_sys_posn= fd->fp_ind = Pos.QuadPart;
    } else error = GetLastError();
	MPI_Barrier(fd->comm);
	/* the barrier ensures that no process races ahead and modifies
           the file size before all processes have opened the file. */
#ifdef PRINT_ERR_MSG
    *error_code = (fd->fd_sys == INVALID_HANDLE_VALUE) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if(fd->fd_sys == INVALID_HANDLE_VALUE) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
	    myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(error));
	ADIOI_Error(fd, *error_code, myname);	    
    }
#endif
}
