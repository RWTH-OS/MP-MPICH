
#ifdef VISUAL_FORTRAN

#include "mpio.h"
#include "adio.h"

#ifndef FORTRAN_API
#define FORTRAN_API __stdcall
#endif

#if defined(MPIO_BUILD_PROFILING)
#define mpi_file_iread_at_ PMPI_FILE_IREAD_AT
#define mpi_file_iread_shared_ PMPI_FILE_IREAD_SHARED
#define mpi_file_iread_ PMPI_FILE_IREAD
#define mpi_file_iwrite_at_ PMPI_FILE_IWRITE_AT
#define mpi_file_iwrite_shared_ PMPI_FILE_IWRITE_SHARED
#define mpi_file_iwrite_ PMPI_FILE_IWRITE
#define mpi_file_read_at_all_begin_ PMPI_FILE_READ_AT_ALL_BEGIN
#define mpi_file_read_at_all_end_ PMPI_FILE_READ_AT_ALL_END
#define mpi_file_read_all_begin_ PMPI_FILE_READ_ALL_BEGIN
#define mpi_file_read_all_end_ PMPI_FILE_READ_ALL_END
#define mpi_file_read_all_ PMPI_FILE_READ_ALL
#define mpi_file_read_at_all_ PMPI_FILE_READ_AT_ALL
#define mpi_file_read_at_ PMPI_FILE_READ_AT
#define mpi_file_read_ordered_begin_ PMPI_FILE_READ_ORDERED_BEGIN
#define mpi_file_read_ordered_end_ PMPI_FILE_READ_ORDERED_END
#define mpi_file_read_ordered_ PMPI_FILE_READ_ORDERED
#define mpi_file_read_shared_ PMPI_FILE_READ_SHARED
#define mpi_file_read_ PMPI_FILE_READ
#define mpi_file_write_at_all_begin_ PMPI_FILE_WRITE_AT_ALL_BEGIN
#define mpi_file_write_at_all_end_ PMPI_FILE_WRITE_AT_ALL_END
#define mpi_file_write_all_begin_ PMPI_FILE_WRITE_ALL_BEGIN
#define mpi_file_write_all_end_ PMPI_FILE_WRITE_ALL_END
#define mpi_file_write_all_ PMPI_FILE_WRITE_ALL
#define mpi_file_write_at_all_ PMPI_FILE_WRITE_AT_ALL
#define mpi_file_write_at_ PMPI_FILE_WRITE_AT
#define mpi_file_write_ordered_begin_ PMPI_FILE_WRITE_ORDERED_BEGIN
#define mpi_file_write_ordered_end_ PMPI_FILE_WRITE_ORDERED_END
#define mpi_file_write_ordered_ PMPI_FILE_WRITE_ORDERED
#define mpi_file_write_shared_ PMPI_FILE_WRITE_SHARED
#define mpi_file_write_ PMPI_FILE_WRITE
#include "mpioprof.h"
#else
#define mpi_file_iread_at_ MPI_FILE_IREAD_AT
#define mpi_file_iread_shared_ MPI_FILE_IREAD_SHARED
#define mpi_file_iread_ MPI_FILE_IREAD
#define mpi_file_iwrite_at_ MPI_FILE_IWRITE_AT
#define mpi_file_iwrite_shared_ MPI_FILE_IWRITE_SHARED
#define mpi_file_iwrite_ MPI_FILE_IWRITE
#define mpi_file_read_at_all_begin_ MPI_FILE_READ_AT_ALL_BEGIN
#define mpi_file_read_at_all_end_ MPI_FILE_READ_AT_ALL_END
#define mpi_file_read_all_begin_ MPI_FILE_READ_ALL_BEGIN
#define mpi_file_read_all_end_ MPI_FILE_READ_ALL_END
#define mpi_file_read_all_ MPI_FILE_READ_ALL
#define mpi_file_read_at_all_ MPI_FILE_READ_AT_ALL
#define mpi_file_read_at_ MPI_FILE_READ_AT
#define mpi_file_read_ordered_begin_ MPI_FILE_READ_ORDERED_BEGIN
#define mpi_file_read_ordered_end_ MPI_FILE_READ_ORDERED_END
#define mpi_file_read_ordered_ MPI_FILE_READ_ORDERED
#define mpi_file_read_shared_ MPI_FILE_READ_SHARED
#define mpi_file_read_ MPI_FILE_READ
#define mpi_file_write_at_all_begin_ MPI_FILE_WRITE_AT_ALL_BEGIN
#define mpi_file_write_at_all_end_ MPI_FILE_WRITE_AT_ALL_END
#define mpi_file_write_all_begin_ MPI_FILE_WRITE_ALL_BEGIN
#define mpi_file_write_all_end_ MPI_FILE_WRITE_ALL_END
#define mpi_file_write_all_ MPI_FILE_WRITE_ALL
#define mpi_file_write_at_all_ MPI_FILE_WRITE_AT_ALL
#define mpi_file_write_at_ MPI_FILE_WRITE_AT
#define mpi_file_write_ordered_begin_ MPI_FILE_WRITE_ORDERED_BEGIN
#define mpi_file_write_ordered_end_ MPI_FILE_WRITE_ORDERED_END
#define mpi_file_write_ordered_ MPI_FILE_WRITE_ORDERED
#define mpi_file_write_shared_ MPI_FILE_WRITE_SHARED
#define mpi_file_write_ MPI_FILE_WRITE
#endif


void FORTRAN_API mpi_file_iread_at_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                      int *count,MPI_Datatype *datatype,
                      MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iread_at(fh_c,*offset,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}

void FORTRAN_API mpi_file_iread_shared_(MPI_Fint *fh,void *buf,int len,int *count,
                   MPI_Datatype *datatype,MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iread_shared(fh_c,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}


void FORTRAN_API mpi_file_iread_(MPI_Fint *fh,void *buf,int len,int *count,
                   MPI_Datatype *datatype,MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iread(fh_c,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}

void FORTRAN_API mpi_file_iwrite_at_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                       int *count,MPI_Datatype *datatype,
                       MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iwrite_at(fh_c,*offset,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}

void FORTRAN_API mpi_file_iwrite_shared_(MPI_Fint *fh,void *buf,int len,int *count,
                    MPI_Datatype *datatype,MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iwrite_shared(fh_c,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}

void FORTRAN_API mpi_file_iwrite_(MPI_Fint *fh,void *buf,int len,int *count,
                    MPI_Datatype *datatype,MPI_Fint *request, int *ierr )
{
    MPI_File fh_c;
    MPIO_Request req_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_iwrite(fh_c,buf,*count,*datatype,&req_c);
    *request = MPIO_Request_c2f(req_c);
}

void FORTRAN_API mpi_file_read_at_all_begin_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                         int *count,MPI_Datatype *datatype, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_at_all_begin(fh_c,*offset,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_read_at_all_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_at_all_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_read_all_begin_(MPI_Fint *fh,void *buf,int len,int *count,
                      MPI_Datatype *datatype, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_all_begin(fh_c,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_read_all_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);

    *ierr = MPI_File_read_all_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_read_all_(MPI_Fint *fh,void *buf,int len,int *count,
                      MPI_Datatype *datatype,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_all(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_read_at_all_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                         int *count,MPI_Datatype *datatype,
                         MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_at_all(fh_c,*offset,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_read_at_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
      int *count,MPI_Datatype *datatype,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_at(fh_c,*offset,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_read_ordered_begin_(MPI_Fint *fh,void *buf,int len,int *count,
                      MPI_Datatype *datatype,int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_ordered_begin(fh_c,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_read_ordered_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);

    *ierr = MPI_File_read_ordered_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_read_ordered_(MPI_Fint *fh,void *buf,int len,int *count,
                      MPI_Datatype *datatype,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_ordered(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_read_shared_(MPI_Fint *fh,void *buf,int len,int *count,
                  MPI_Datatype *datatype,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_shared(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_read_(MPI_Fint *fh,void *buf,int len,int *count,
                  MPI_Datatype *datatype,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_at_all_begin_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                          int *count,MPI_Datatype *datatype, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_at_all_begin(fh_c,*offset,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_write_at_all_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);

    *ierr = MPI_File_write_at_all_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_write_all_begin_(MPI_Fint *fh,void *buf,int len,int *count,
                       MPI_Datatype *datatype, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_all_begin(fh_c,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_write_all_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);

    *ierr = MPI_File_write_all_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_write_all_(MPI_Fint *fh,void *buf,int len,int *count,
                       MPI_Datatype *datatype,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_all(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_at_all_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                          int *count,MPI_Datatype *datatype,
                          MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_at_all(fh_c,*offset,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_at_(MPI_Fint *fh,MPI_Offset *offset,void *buf,int len,
                      int *count,MPI_Datatype *datatype,
                      MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_at(fh_c,*offset,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_ordered_begin_(MPI_Fint *fh,void *buf,int len,int *count,
                       MPI_Datatype *datatype, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_ordered_begin(fh_c,buf,*count,*datatype);
}

void FORTRAN_API mpi_file_write_ordered_end_(MPI_Fint *fh,void *buf,int len,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);

    *ierr = MPI_File_write_ordered_end(fh_c,buf,status);
}

void FORTRAN_API mpi_file_write_ordered_(MPI_Fint *fh,void *buf,int len,int *count,
                       MPI_Datatype *datatype,MPI_Status *status, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_ordered(fh_c,buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_shared_(MPI_Fint *fh,void *buf,int len,int *count,
                   MPI_Datatype *datatype,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write_shared(fh_c, buf,*count,*datatype,status);
}

void FORTRAN_API mpi_file_write_(MPI_Fint *fh,void *buf,int len,int *count,
                   MPI_Datatype *datatype,MPI_Status *status, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_write(fh_c, buf,*count,*datatype,status);
}

#endif

