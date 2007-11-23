/* 
 * $Id: ad_ntfs_iwrite.c 922 2001-05-31 14:10:48Z karsten $
 *
 */
#include "ad_ntfs.h"

void ADIOI_NTFS_IwriteContig(ADIO_File fd, void *buf, int count, 
			     MPI_Datatype datatype,int file_ptr_type,
			     ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    ADIO_Status status;
    int err=-1;
    int len,typesize;
    DWORD error;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_IWRITECONTIG";
#endif
    
    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_WRITE;
    (*request)->fd = fd;
    (*request)->datatype = datatype;
    
    MPI_Type_size(datatype, &typesize);
    len = count * typesize;
    (*request)->nbytes = (*request)->totransfer = len;
    (*request)->buf = buf;
    
    if (file_ptr_type == ADIO_INDIVIDUAL) offset = fd->fp_ind;
    (*request)->offset = offset;
    
    
    err = ADIOI_NTFS_aio(fd, 1, *request,0,&error);
    
    if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind += len;
    
    
    if(error == ERROR_IO_PENDING) {
	(*request)->queued = 1;
	ADIOI_Add_req_to_list(request);
    } else {
	(*request)->queued = 0;
    }
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
	    myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(error));
	ADIOI_Error(fd, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
    fd->fp_sys_posn = -1;   /* set it to null. */
    fd->async_count++;
}




void ADIOI_NTFS_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_WRITE;
    (*request)->fd = fd;
	(*request)->datatype = datatype;
    (*request)->queued = 0;
    (*request)->handle = 0;

/* call the blocking version. It is faster because it does data sieving. */
    ADIOI_NTFS_WriteStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;

/* status info. must be linked to the request structure, so that it
   can be accessed later from a wait */

}


/* This function is for implementation convenience. It is not user-visible.
   If wr==1 write, wr==0 read. */

int ADIOI_NTFS_aio(ADIO_File fd, int wr, ADIO_Request request,int blocking, DWORD *error_code)
{
    BOOL err,restart,check=TRUE; 
    int CompleteError,Counter =0;
    FD_TYPE fd_sys;
    
    OVERLAPPED *result;
    LARGE_INTEGER *LI = (LARGE_INTEGER*)&request->offset;
    DWORD written;
    
    *error_code = NO_ERROR;
    fd_sys = fd->fd_sys;
    if(!request->totransfer) {
	request->handle = NULL;
	SetFilePointer(fd_sys,LI->LowPart,&LI->HighPart,FILE_BEGIN);
	return 0;
    }
    
    result = GetOverlappedStruct();
    
    
    do {
	check = TRUE;
	result->Offset = LI->LowPart;
	result->OffsetHigh = LI->HighPart;
	restart = FALSE; 
	if (wr) err = WriteFile(fd_sys,request->buf,request->totransfer,&written,result);
	else err = ReadFile(fd_sys,request->buf,request->totransfer,&written,result);
	while(check) {
	    check = FALSE;
	    if(!err) {
		*error_code = GetLastError();		
		switch (*error_code) {
		case ERROR_LOCK_VIOLATION:
		    restart = TRUE;
		    Sleep(1);
		    break;
		case ERROR_HANDLE_EOF:
		    FreeOverlappedStruct(result);
		    result = NULL;
		    request->totransfer -= written;
		    err = 1;
		    break;
		case ERROR_IO_PENDING:
		    if(blocking) {
			err = GetOverlappedResult(fd_sys,result,&written,TRUE);
			if(!err) {
			    check = TRUE;
			} else {

			    request->totransfer -= written;
			    if(request->totransfer>0) {

				request->buf += written;
				request->offset += written;
				restart = TRUE;
			    } else {
				*error_code = ERROR_SUCCESS;
				FreeOverlappedStruct(result);
				result = NULL;
			    }
			    err = 1;
			}
		    }
		    break;
		case ERROR_INVALID_USER_BUFFER :
		case ERROR_NOT_ENOUGH_MEMORY :
		    ADIOI_Complete_async(&CompleteError);
		    restart = TRUE;
		    if(++Counter<10) {
			Sleep(1);
			break;
		    }
		    /* else fall through */
		default:
		    fprintf(stderr,"Error %d in ADIOI_NTFS_aio\n", *error_code);
		    fflush(stderr);
		    /*MPI_Abort(MPI_COMM_WORLD, *error_code);*/
		    break;
		}
	    } else {
		request->totransfer -= written;
		if(request->totransfer>0) {
		    request->buf += written;
		    request->offset += written;
		    restart = TRUE;
		} else {
		    FreeOverlappedStruct(result);
		    result = NULL;
		    *error_code = ERROR_SUCCESS;
		}
	    }
	}
    } while (restart && Counter <10);
    
    request->handle = result;
    return (err?0:-1);
}
