/* 
 *   $Id: create_errh.c 723 2001-02-28 13:11:09Z karsten $    
 *
 */
#ifdef MPICH
#include "mpi.h"
#include "mpi_error.h"
#include "../mpid/ch2/cookie.h"
#include "errhandler.h"
#include "mpioimpl.h"
#include "adio_extern.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_set_errhandler = PMPI_File_set_errhandler
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

int MPI_File_create_errhandler( 
	MPI_File_errhandler_fn *function,
	MPI_Errhandler       *errhandler)
{
    return MPIR_Create_user_errhandler(function,MPIR_FILE_HANDLER,errhandler);
}

#endif