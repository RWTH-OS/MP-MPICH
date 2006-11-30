/*
*   $Id$    
*/

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_close = PMPI_File_close
#elif defined(HAVE_ATTRIBUTE_WEAK)
int MPI_File_close(MPI_File *fh) __attribute__ ((weak, alias ("PMPI_File_close")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_close MPI_File_close
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_close as PMPI_File_close
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

int MPI_File_call_errorhandler(MPI_File fh, int error_code) {
    
    if ((fh)->cookie != ADIOI_FILE_COOKIE) { \
	error_code = MPIR_Err_setmsg(MPI_ERR_FILE, MPIR_ERR_FILE_CORRUPT, "MPI_File_call_errorhandler", 
	                            (char *) 0, (char *) 0);
	fh = MPI_FILE_NULL;
    }
    ADIOI_Error(fh,error_code,"MPI_File_call_errorhandler");
    
    return MPI_SUCCESS;
}
