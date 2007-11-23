/* 
 *   $Id: ad_ntfs_resize.c 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#include "ad_ntfs.h"

#ifdef NTFS
int ftruncate(FD_TYPE hFile,ADIO_Offset size) {

	LARGE_INTEGER OldPos,*NewPos;
	DWORD err;

	OldPos.QuadPart=0;
	NewPos=(LARGE_INTEGER*)&size;


	OldPos.LowPart=SetFilePointer(hFile,OldPos.LowPart,&OldPos.HighPart,FILE_CURRENT);
	if(OldPos.LowPart == 0xFFFFFF && GetLastError() != ERROR_SUCCESS) return -1;

	err = SetFilePointer(hFile,NewPos->LowPart,&NewPos->HighPart,FILE_BEGIN);
	if((err == 0xFFFFFF) && (GetLastError() != ERROR_SUCCESS)) return -1;
	if(!SetEndOfFile(hFile)) return -1;

	if(OldPos.QuadPart>size)
		err=SetFilePointer(hFile,NewPos->LowPart,&NewPos->HighPart,FILE_BEGIN);
	else
		err=SetFilePointer(hFile,OldPos.LowPart,&OldPos.HighPart,FILE_BEGIN);
	
	return (((err == 0xFFFFFF)&&(GetLastError() != ERROR_SUCCESS))?-1:0);
}
#endif

void ADIOI_NTFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
    
    err = ftruncate(fd->fd_sys, size);
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
	    "ADIOI_NTFS_Resize", "I/O Error", "I/O Error: %s", ad_ntfs_error(GetLastError()));
	ADIOI_Error(fd, *error_code, "ADIOI_NTFS_Resize");	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}
