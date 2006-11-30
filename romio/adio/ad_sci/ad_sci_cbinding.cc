//
// $Id$
//
// C bindings for the SCI file objects
//

#include "ad_sci.h"

extern C {

//
// administrative function (Open, Close; Fcntl, Flush, SetInfo, Resize, SeekIndividual)
//
void ADIOI_SCI_Open(ADIO_File fd, int *error_code)
{
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_OPEN";
#endif

    // create a new instance of AD_SCI_file 
    fd->fd_sys = (void *)(new AD_SCI_file(fd, error_code));

#ifdef PRINT_ERR_MSG
    *error_code = (fd->fd_sys == NULL) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (fd->fd_sys == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(ADIO_FILE_NULL, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}


void ADIOI_SCI_Close(ADIO_File fd, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_CLOSE";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    ~sci_file (&err);
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}


void ADIOI_SCI_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    int err; 
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_FCNTL";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->Fcntl (flag, fcntl_struct, &err);

#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}


void ADIOI_SCI_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    int err; 
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_SETINFO";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->SetInfo (users_info, &err);

#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}

void ADIOI_SCI_Flush(ADIO_File fd, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_FLUSH";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->Flush (&err);

#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}

ADIO_Offset ADIOI_SCI_SeekIndividual (ADIO_File fd, ADIO_Offset offset, 
				      int whence, int *error_code)
{
    int err;
    ADIO_Offset current_offset;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_SEEKINDIVIDUAL";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    current_offset = sci_file->SeekIndividual (offset, whence, &err);

#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return current_offset;
}

void ADIOI_SCI_Resize (ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_RESIZE";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->Resize (size, &err);

#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}

//
// synchronous individual read/write functions
//
void ADIOI_SCI_ReadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, int *error_code)  
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READCONTIG";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->ReadContig (buf, count, datatype, file_ptr_type, offset, &err);

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
    return;
}


void ADIOI_SCI_WriteContig(ADIO_File fd, void *buf, int count, 
			   MPI_Datatype datatype, int file_ptr_type,
			   ADIO_Offset offset, int *error_code)  
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_WRTIECONTIG";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->WriteContig (buf, count, datatype, file_ptr_type, offset, &err);

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
    return;
}


//
// asynchronous individual read/write functions
//
void ADIOI_SCI_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_IREADCONTIG";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->IreadContig (buf, count, datatype, file_ptr_type, offset, request, &err);

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
    return;
}


void ADIOI_SCI_IwriteContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_IWRTIECONTIG";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->IwriteContig (buf, count, datatype, file_ptr_type, offset, request, &err);

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
    return;
}


void ADIOI_SCI_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_IREADSTRIDED";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->IreadStrided (buf, count, datatype, file_ptr_type, offset, request, &err);

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
    return;
}


void ADIOI_SCI_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_IWRTIESTRIDED";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->IwriteStrided (buf, count, datatype, file_ptr_type, offset, request, &err);

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
    return;
}


//
// completion functions (read/write Complete/Done)
//
void ADIOI_SCI_ReadComplete (ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READCOMPLETE";
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return;
    }

    AD_SCI_file *sci_file = (AD_SCI_file *)(request->fd->fd_sys);
    sci_file->ReadComplete (request, status, &err);
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}
 
void ADIOI_SCI_WriteComplete (ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_WRITECOMPLETE";
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return;
    }

    AD_SCI_file *sci_file = (AD_SCI_file *)(request->fd->fd_sys);
    sci_file->WriteComplete (request, status, &err);
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return;
}

int ADIOI_SCI_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int err, done = 0; 
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READDONE";
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }

    AD_SCI_file *sci_file = (AD_SCI_file *)(request->fd->fd_sys);
    done = sci_file->ReadDone (request, status, &err);
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return done;
}


int ADIOI_SCI_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int err, done = 0; 
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READDONE";
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }

    AD_SCI_file *sci_file = (AD_SCI_file *)(request->fd->fd_sys);
    done = sci_file->ReadDone (request, status, &err);
    
#ifdef PRINT_ERR_MSG
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
				      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    } else 
      *error_code = MPI_SUCCESS;
#endif
    return done;
} 


//
// collective read/write functions
//
void ADIOI_SCI_ReadStridedColl(ADIO_File fd, void *buf, int count,
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Status *status, 
			       int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READSTRIDEDCOLL";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->ReadStridedColl (buf, count, datatype, file_ptr_type, offset, status, &err);

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
    return;
}

    
void ADIOI_SCI_WriteStridedColl(ADIO_File fd, void *buf, int count,
				MPI_Datatype datatype, int file_ptr_type,
				ADIO_Offset offset, ADIO_Status *status, 
				int *error_code)
{
    int err;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_WRITESTRIDEDCOLL";
#endif

    AD_SCI_file *sci_file = (AD_SCI_file *)(fd->fd_sys);
    sci_file->WriteStridedColl (buf, count, datatype, file_ptr_type, offset, status, &err);

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
    return;
}




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
