/* 
 *   $Id: lock.c 1328 2001-11-22 13:55:10Z nicolas $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

#ifdef _WIN32
#include "..\ad_ntfs\ad_ntfs.h"

int ADIOI_Set_lock(FD_TYPE fd_sys, DWORD flags, ADIO_Offset offset, int whence, ADIO_Offset len)
{
	LARGE_INTEGER *LI;
	OVERLAPPED *over;
	BOOL res;
	DWORD err;

	if(whence != SEEK_SET)
		fprintf(stderr,"ADIOIO_SetLock: Only SeekSet supportet on NTFS\nLocking may fail\n");

	over = GetOverlappedStruct();
	LI = (LARGE_INTEGER*)&offset;
	over->Offset = LI->LowPart;
	over->OffsetHigh = LI->HighPart;
	LI = (LARGE_INTEGER*)&len;
	res = LockFileEx(fd_sys,flags,0,LI->LowPart,LI->HighPart,over);
	if(!res) {
		err = GetLastError();
		if(err==ERROR_IO_PENDING) {
			res =1;
			while(!HasOverlappedIoCompleted(over)) Sleep(1);
		} else {
			printf("File locking failed in ADIOI_Set_lock\n");
			MPI_Abort(MPI_COMM_WORLD, err);
		}
	}
	
	FreeOverlappedStruct(over);

	return (res?MPI_SUCCESS:MPI_ERR_UNKNOWN);
}

int ADIOI_Remove_lock(FD_TYPE fd_sys, ADIO_Offset offset, int whence, ADIO_Offset len) 
{
	LARGE_INTEGER *LI;
	OVERLAPPED *over;
	BOOL res;
	DWORD err;

	if(whence != SEEK_SET)
		fprintf(stderr,"ADIOIO_SetLock: Only SeekSet supportet on NTFS\nLocking may fail\n");

	over = GetOverlappedStruct();
	LI = (LARGE_INTEGER*)&offset;
	over->Offset = LI->LowPart;
	over->OffsetHigh = LI->HighPart;
	LI = (LARGE_INTEGER*)&len;
	res = UnlockFileEx(fd_sys,0,LI->LowPart,LI->HighPart,over);
	if(!res) {
		err = GetLastError();
		if(err==ERROR_IO_PENDING) {
			res =1;
			while(!HasOverlappedIoCompleted(over)) Sleep(1);
		} else {
			printf("File unlocking failed in ADIOI_Set_lock\n");
			MPI_Abort(MPI_COMM_WORLD, err);
		}
	}
	
	FreeOverlappedStruct(over);

	return (res?MPI_SUCCESS:MPI_ERR_UNKNOWN);
}

#else

int ADIOI_Set_lock(int fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
    int err, error_code;
    struct flock lock;

    /* locking is not required for ad_svm */
#ifndef SVM
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    do {
	err = fcntl(fd, cmd, &lock);
    } while (err && (errno == EINTR));

    if (err && (errno != EBADF)) {
	FPRINTF(stderr, "File locking failed in ADIOI_Set_lock. If the file system is NFS, you need to use NFS version 3 and mount the directory with the 'noac' option (no attribute caching).\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    err = 0;
#endif

    error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
    return error_code;
}


#if (defined(HFS) || defined(XFS))
int ADIOI_Set_lock64(int fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
    int err, error_code;
    struct flock64 lock;

    /* locking is not required for ad_svm */
#ifndef SVM
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    do {
	err = fcntl(fd, cmd, &lock);
    } while (err && (errno == EINTR));

    if (err && (errno != EBADF)) {
	FPRINTF(stderr, "File locking failed in ADIOI_Set_lock64\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    err = 0;
#endif

    error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
    return error_code;
}
#endif

#endif /* _WIN32 */
