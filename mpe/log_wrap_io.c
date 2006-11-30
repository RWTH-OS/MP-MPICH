/* $Id$ */

#include "log_wrap.h"

/* Service routines for managing requests .... */
void MPE_Add_write_req( count, datatype, dest, request, is_persistent )
int count, is_persistent;
MPI_Datatype datatype;
MPIO_Request request;
{
  request_list *newrq;
  int typesize;

  rq_alloc(requests_avail_0,newrq);
  if (newrq) {
      PMPI_Type_size( datatype, &typesize );
      newrq->request	   = request;
      newrq->status	   = RQ_WRITE;
      newrq->size	   = count * typesize;
      newrq->otherParty	   = dest;
      newrq->next	   = 0;
      newrq->is_persistent = is_persistent;
      rq_add( requests_head_0, requests_tail_0, newrq );
  }
}


void MPE_Add_read_req( count, datatype, request, is_persistent )
int count, is_persistent;
MPI_Datatype datatype;
MPIO_Request request;
{
  request_list *newrq;
  int typesize;

  /* We could pre-allocate request_list members, or allocate in
     blocks.  Do this is we see this is a bottleneck */
  rq_alloc( requests_avail_0, newrq );
  if (newrq) {
      PMPI_Type_size( datatype, &typesize );
      newrq->request	   = request;
      newrq->status	   = RQ_READ;
      newrq->size          = count*typesize;
      newrq->next	   = 0;
      newrq->is_persistent = is_persistent;
      rq_add( requests_head_0, requests_tail_0, newrq );
    }
}



/* routines that get Logging */


/* MPI_File__errorhandler functions not defined in mpio.h and no reason to log them */
#if 0
int MPI_File_call_errorhandler(MPI_File fh, int error_code) 
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_CALL_ERRHANDLER_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_call_errorhandler(fh, error_code);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_create_errhandler( 
	MPI_File_errhandler_fn *function,
	MPI_Errhandler       *errhandler)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_CREATE_ERRHANDLER_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_create_errhandler(function, errhandler);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_errhandler(MPI_File fh, MPI_Errhandler *errhandler)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_ERRHANDLER_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_errhandler(fh, errhandler);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}


int MPI_File_set_errhandler(MPI_File fh, MPI_Errhandler errhandler)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SET_ERRHANDLER_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_set_errhandler(fh, errhandler);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}
#endif

int MPI_File_close(MPI_File *fh)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_CLOSE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_close(fh);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_delete(char *filename, MPI_Info info)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_DELETE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_delete(filename, info);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_amode(MPI_File fh, int *amode)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_AMODE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_amode(fh, amode);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_atomicity(MPI_File fh, int *flag)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_ATOMICITY_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_atomicity(fh, flag);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, MPI_Offset *disp)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_BYTE_OFFSET_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_byte_offset(fh, offset, disp);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_group(MPI_File fh, MPI_Group *group)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_GROUP_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_group(fh, group);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_info(MPI_File fh, MPI_Info *info_used)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_INFO_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_info(fh, info_used);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_position(MPI_File fh, MPI_Offset *offset)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_POSITION_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_position(fh, offset);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_POSITION_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_position_shared(fh, offset);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_size(MPI_File fh, MPI_Offset *size)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_SIZE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_size(fh, size);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, 
                             MPI_Aint *extent)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_TYPE_EXTENT_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_type_extent(fh, datatype, extent);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_get_view(MPI_File fh, MPI_Offset *disp, MPI_Datatype *etype,
		 MPI_Datatype *filetype, char *datarep)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_GET_VIEW_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_get_view(fh, disp, etype, filetype, datarep);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iread(MPI_File fh, void *buf, int count, 
                   MPI_Datatype datatype, MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IREAD_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iread(fh, buf, count, datatype, request);
    if (returnVal == MPI_SUCCESS) {
      MPE_Add_read_req( count, datatype, request, 0 );
    }

    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf,
                      int count, MPI_Datatype datatype, 
                      MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IREAD_AT_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iread_at(fh, offset, buf, count, datatype, request);
    if (returnVal == MPI_SUCCESS) {
      MPE_Add_read_req( count, datatype, request, 0 );
    }

    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iread_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IREAD_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iread_shared(fh, buf, count, datatype, request);
    if (returnVal == MPI_SUCCESS) {
      MPE_Add_read_req( count, datatype, request, 0 );
    }

    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iwrite(MPI_File fh, void *buf, int count, 
                    MPI_Datatype datatype, MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IWRITE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iwrite(fh, buf, count, datatype, request);
    /*    if (dest != MPI_PROC_NULL) {
	MPE_Add_write_req( count, datatype, dest, tag, *request, 0 );
	}*/

    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, void *buf,
                       int count, MPI_Datatype datatype, 
                       MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IWRITE_AT_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iwrite_at(fh, offset, buf, count, datatype, request);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_iwrite_shared(MPI_File fh, void *buf, int count, 
                       MPI_Datatype datatype, MPIO_Request *request)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_IWRITE_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_iwrite_shared(fh, buf, count, datatype, request);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_open(MPI_Comm comm, char *filename, int amode, 
                  MPI_Info info, MPI_File *fh)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_OPEN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_open(comm, filename, amode, info, fh);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_preallocate(MPI_File fh, MPI_Offset size)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_PREALLOCATE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_preallocate(fh, size);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read(MPI_File fh, void *buf, int count, 
                  MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_all(MPI_File fh, void *buf, int count, 
                      MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ALL_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_all(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_all_begin(MPI_File fh, void *buf, int count, 
                            MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ALL_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_all_begin(fh, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ALL_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_all_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf,
		    int count, MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_AT_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_at(fh, offset, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                         int count, MPI_Datatype datatype, 
                         MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_AT_ALL_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_at_all(fh, offset, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                         int count, MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_AT_ALL_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_at_all_begin(fh, offset, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_AT_ALL_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_at_all_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_ordered(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ORDERED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_ordered(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, 
				MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ORDERED_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_ordered_begin(fh, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_ORDERED_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_ordered_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_read_shared(MPI_File fh, void *buf, int count, 
                  MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_READ_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_read_shared(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_seek(MPI_File fh, MPI_Offset offset, int whence)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SEEK_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_seek(fh, offset, whence);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SEEK_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_seek_shared(fh, offset, whence);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_set_atomicity(MPI_File fh, int flag)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SET_ATOMICITY_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_set_atomicity(fh, flag);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_set_info(MPI_File fh, MPI_Info info)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SET_INFO_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_set_info(fh, info);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_set_size(MPI_File fh, MPI_Offset size)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SET_SIZE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_set_size(fh, size);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype,
		 MPI_Datatype filetype, char *datarep, MPI_Info info)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SET_VIEW_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_set_view(fh, disp, etype, filetype, datarep, info);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_sync(MPI_File fh)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_SYNC_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_sync(fh);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write(MPI_File fh, void *buf, int count, 
                   MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_all(MPI_File fh, void *buf, int count, 
                       MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ALL_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_all(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_all_begin(MPI_File fh, void *buf, int count, 
                            MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ALL_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_all_begin(fh, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_all_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ALL_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_all_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_at(MPI_File fh, MPI_Offset offset, void *buf,
                      int count, MPI_Datatype datatype, 
                      MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_AT_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_at(fh, offset, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                          int count, MPI_Datatype datatype, 
                          MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_AT_ALL_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_at_all(fh, offset, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                         int count, MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_AT_ALL_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_at_all_begin(fh, offset, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_at_all_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_AT_ALL_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_at_all_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_ordered(MPI_File fh, void *buf, int count, 
			   MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ORDERED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_ordered(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_ordered_begin(MPI_File fh, void *buf, int count, 
				 MPI_Datatype datatype)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ORDERED_BEGIN_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_ordered_begin(fh, buf, count, datatype);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_ordered_end(MPI_File fh, void *buf, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_ORDERED_END_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_ordered_end(fh, buf, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

int MPI_File_write_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status)
{
    int  returnVal;
    register MPE_State *state;
    
    MPE_LOG_STATE_BEGIN(MPE_FILE_WRITE_SHARED_ID,MPI_COMM_NULL);
    returnVal = PMPI_File_write_shared(fh, buf, count, datatype, status);
    MPE_LOG_STATE_END(MPI_COMM_NULL);
    return returnVal;
}

