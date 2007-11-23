/* 
 *   $Id: error.c 2599 2003-10-22 09:45:44Z silke $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */
#ifdef MPICH
#include "mpi.h"
#include "mpi_error.h"
#include "../mpid/ch2/cookie.h"
#include "errhandler.h"
#endif
#include "adio.h"
#include "adio_extern.h"

int ADIOI_ERROR(ADIO_File fd, int error_code, char *string,char *file,int line)
{
    char buf[MPI_MAX_ERROR_STRING];
    int myrank, result_len; 
    MPI_Errhandler err_handler;

    if (fd == ADIO_FILE_NULL) err_handler = ADIOI_DFLT_ERR_HANDLER;
    else err_handler = fd->err_handler;
    if(!err_handler) err_handler = MPI_ERRORS_RETURN;

#ifndef MPICH
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if (err_handler == MPI_ERRORS_ARE_FATAL) {
	MPI_Error_string(error_code, buf, &result_len);
	FPRINTF(stderr, "[%d] - %s : %s\n", myrank, string, buf);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    else 
    if (err_handler != MPI_ERRORS_RETURN) {
	/* MPI_File_call_errorhandler(fd, error_code); */

	FPRINTF(stderr, "Only MPI_ERRORS_RETURN and MPI_ERRORS_ARE_FATAL are currently supported as error handlers for files\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
	/* Call handler routine */
    {	MPI_Comm err_comm;
	struct MPIR_Errhandler *errhand = (struct MPIR_Errhandler *)MPIR_ToPointer( err_handler );
	if (!errhand || !errhand->routine) {
	    fprintf( stderr, "Fatal error (adio); unknown error handler\n\
May be MPI call before MPI_INIT.  Error message is %s and code is %d\n", 
		string, error_code );	
	    return error_code;
	}
	
	if(errhand->type == MPIR_PREDEFINED_HANDLER || errhand->type == MPIR_COMM_HANDLER) {
	    if(fd == ADIO_FILE_NULL) err_comm=MPI_COMM_WORLD;
	    else err_comm = fd->comm;
	    errhand->routine(&err_comm, &error_code, string, file, &line );
	} else errhand->routine((int*)&fd, &error_code, string, file, &line );
    }
#endif
    return error_code;
}

