/* $Id$ */

#include "log_wrap.h"
#include "adio.h"


/* routines that get logging */
/* XXX we need to determine automatically which filesystems are used! */

#ifdef PVFS
void ADIOI_PVFS_Close(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_PVFS_Close(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


int ADIOI_PVFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_READDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_PVFS_ReadDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


int ADIOI_PVFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_WRITEDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_PVFS_WriteDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_PVFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_FCNTL_ID,MPI_COMM_NULL);
    PADIOI_PVFS_Fcntl(fd, flag, fcntl_struct, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_Flush(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_FLUSH_ID,MPI_COMM_NULL);
    PADIOI_PVFS_Flush(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_SETINFO_ID,MPI_COMM_NULL);
    PADIOI_PVFS_SetInfo(fd, users_info, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_IREADCONTIG_ID,MPI_COMM_NULL);
    PADIOI_PVFS_IreadContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_IREADSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_PVFS_IreadStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_IwriteContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_IWRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_PVFS_IwriteContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_IWRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_PVFS_IwriteStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_Open(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_OPEN_ID,MPI_COMM_NULL);
    PADIOI_PVFS_Open(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_READSTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_PVFS_ReadStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_ReadContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_READCONTIG_ID,MPI_COMM_NULL);
    PADIOI_PVFS_ReadContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_ReadStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_READSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_PVFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_RESIZE_ID,MPI_COMM_NULL);
    PADIOI_PVFS_Resize(fd, size, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


ADIO_Offset ADIOI_PVFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset,
		      int whence, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_SEEKINDIVIDUAL_ID,MPI_COMM_NULL);
    returnVal = PADIOI_PVFS_SeekIndividual(fd, offset, whence, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_PVFS_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_READCOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_PVFS_ReadComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_WRITECOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_PVFS_WriteComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_WRITESTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_PVFS_WriteStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_WriteContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_WRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_PVFS_WriteContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_PVFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_PVFS_WRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_PVFS_WriteStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}
#endif


/* XXX we assume that NFS and UFS are always available - see above! */

/* NFS */
void ADIOI_NFS_Close(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_NFS_Close(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


int ADIOI_NFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_READDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_NFS_ReadDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


int ADIOI_NFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_WRITEDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_NFS_WriteDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_NFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_FCNTL_ID,MPI_COMM_NULL);
    PADIOI_NFS_Fcntl(fd, flag, fcntl_struct, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_Flush(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_FLUSH_ID,MPI_COMM_NULL);
    PADIOI_NFS_Flush(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_SETINFO_ID,MPI_COMM_NULL);
    PADIOI_NFS_SetInfo(fd, users_info, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_IREADCONTIG_ID,MPI_COMM_NULL);
    PADIOI_NFS_IreadContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_IREADSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_NFS_IreadStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_IwriteContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_IWRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_NFS_IwriteContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_IWRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_NFS_IwriteStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_Open(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_OPEN_ID,MPI_COMM_NULL);
    PADIOI_NFS_Open(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_READSTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_NFS_ReadStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_ReadContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_READCONTIG_ID,MPI_COMM_NULL);
    PADIOI_NFS_ReadContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_ReadStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_READSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_NFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_RESIZE_ID,MPI_COMM_NULL);
    PADIOI_NFS_Resize(fd, size, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


ADIO_Offset ADIOI_NFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset,
		      int whence, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_SEEKINDIVIDUAL_ID,MPI_COMM_NULL);
    returnVal = PADIOI_NFS_SeekIndividual(fd, offset, whence, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_NFS_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_READCOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_NFS_ReadComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_WRITECOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_NFS_WriteComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_WRITESTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_NFS_WriteStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_WriteContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_WRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_NFS_WriteContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_WRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_NFS_WriteStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_Get_shared_fp(ADIO_File fd, int incr, ADIO_Offset *shared_fp, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_NFS_Get_shared_fp(fd, incr, shared_fp, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_NFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_NFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_NFS_Set_shared_fp(fd, offset, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}

/* UFS */
void ADIOI_UFS_Close(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_UFS_Close(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


int ADIOI_UFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_READDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_UFS_ReadDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


int ADIOI_UFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_WRITEDONE_ID,MPI_COMM_NULL);
    returnVal = PADIOI_UFS_WriteDone(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_UFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_FCNTL_ID,MPI_COMM_NULL);
    PADIOI_UFS_Fcntl(fd, flag, fcntl_struct, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_Flush(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_FLUSH_ID,MPI_COMM_NULL);
    PADIOI_UFS_Flush(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_SETINFO_ID,MPI_COMM_NULL);
    PADIOI_UFS_SetInfo(fd, users_info, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_IreadContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_IREADCONTIG_ID,MPI_COMM_NULL);
    PADIOI_UFS_IreadContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_IREADSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_UFS_IreadStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_IwriteContig(ADIO_File fd, void *buf, int count, 
                MPI_Datatype datatype, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_IWRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_UFS_IwriteContig(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_IwriteStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_IWRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_UFS_IwriteStrided(fd, buf, count, datatype, file_ptr_type, offset, request, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_Open(ADIO_File fd, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_OPEN_ID,MPI_COMM_NULL);
    PADIOI_UFS_Open(fd, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_READSTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_UFS_ReadStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_ReadContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_READCONTIG_ID,MPI_COMM_NULL);
    PADIOI_UFS_ReadContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_ReadStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_READSTRIDED_ID,MPI_COMM_NULL);
    PADIOI_UFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_RESIZE_ID,MPI_COMM_NULL);
    PADIOI_UFS_Resize(fd, size, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


ADIO_Offset ADIOI_UFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset,
		      int whence, int *error_code)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_SEEKINDIVIDUAL_ID,MPI_COMM_NULL);
    returnVal = PADIOI_UFS_SeekIndividual(fd, offset, whence, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


void ADIOI_UFS_ReadComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_READCOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_UFS_ReadComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_WriteComplete(ADIO_Request *request, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_WRITECOMPLETE_ID,MPI_COMM_NULL);
    PADIOI_UFS_WriteComplete(request, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_WRITESTRIDEDCOLL_ID,MPI_COMM_NULL);
    PADIOI_UFS_WriteStridedColl(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_WriteContig(ADIO_File fd, void *buf, int count, 
                     MPI_Datatype datatype, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_WRITECONTIG_ID,MPI_COMM_NULL);
    PADIOI_UFS_WriteContig(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_WriteStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_WRITESTRIDED_ID,MPI_COMM_NULL);
    PADIOI_UFS_WriteStrided(fd, buf, count, datatype, file_ptr_type, offset, status, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}

#if 0
/* not available for UFS */
void ADIOI_UFS_Get_shared_fp(ADIO_File fd, int incr, ADIO_Offset *shared_fp, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_UFS_Get_shared_fp(fd, incr, shared_fp, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}


void ADIOI_UFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset, int *error_code)
{
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_UFS_CLOSE_ID,MPI_COMM_NULL);
    PADIOI_UFS_Set_shared_fp(fd, offset, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
}
#endif
