/* 
 *   $Id: set_errh.c 2193 2003-06-05 11:50:38Z rainer $    
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
#include "mpioimpl.h"
#include "adio_extern.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_set_errhandler = PMPI_File_set_errhandler
#elif defined(HAVE_ATTRIBUTE_WEAK)
int MPI_File_set_errhandler(MPI_File fh,
	MPI_Errhandler errhandler) __attribute__ ((weak, alias ("PMPI_File_set_errhandler")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_set_errhandler MPI_File_set_errhandler
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_set_errhandler as PMPI_File_set_errhandler
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/*@
    MPI_File_set_errhandler - Sets the error handler for a file

Input Parameters:
. fh - file handle (handle)
. errhandler - error handler (handle)

.N fortran
@*/

int MPI_File_set_errhandler(MPI_File fh, MPI_Errhandler errhandler)
{
    int error_code = MPI_SUCCESS;
    MPI_Errhandler old_errhandler;
#ifdef MPICH
    struct MPIR_Errhandler *old;
    int mpi_errno = MPI_SUCCESS;
#endif
#if !defined(PRINT_ERR_MSG) || defined(MPICH)
    static char myname[] = "MPI_File_set_errhandler";
#endif
#ifndef MPICH
    if ((errhandler != MPI_ERRORS_RETURN) || (errhandler != MPI_ERRORS_ARE_FATAL)) {
	FPRINTF(stderr, "Only MPI_ERRORS_RETURN and MPI_ERRORS_ARE_FATAL are currently supported for MPI_File_set_errhandler\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    old = (struct MPIR_Errhandler *)MPIR_ToPointer( errhandler );
    MPIR_TEST_ERRHANDLER(old);
    if(mpi_errno) return ADIOI_Error(fh, mpi_errno, myname );
    if((old->type != MPIR_COMM_HANDLER) && (old->type != MPIR_PREDEFINED_HANDLER)) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_ARG, MPIR_ERR_ERRHANDLER_TYPE, myname, (char *)0,(char*)0,"MPI_Comm_errhandler_fn" );
	return ADIOI_Error(fh, mpi_errno, myname );
    }
    
    
#endif
    if (fh == MPI_FILE_NULL) {
	old_errhandler = ADIOI_DFLT_ERR_HANDLER;
	ADIOI_DFLT_ERR_HANDLER = errhandler;
    }
    else if (fh->cookie != ADIOI_FILE_COOKIE) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_set_errhandler: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_FILE, MPIR_ERR_FILE_CORRUPT, 
	    myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    } else  {
	old_errhandler = fh->err_handler;
	fh->err_handler = errhandler;
    }
#ifdef MPICH
    (old)->ref_count++;
    if(old_errhandler)
	MPI_Errhandler_free( &old_errhandler );
#endif
    return error_code;
}

