/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Test_cancelled = PMPI_Test_cancelled
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Test_cancelled( 
	MPI_Status *status,
	int        *flag) __attribute__ ((weak, alias ("PMPI_Test_cancelled")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Test_cancelled  MPI_Test_cancelled
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Test_cancelled as PMPI_Test_cancelled
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

/*@
  MPI_Test_cancelled - Tests to see if a request was cancelled

Input Parameter:
. status - status object (Status) 

Output Parameter:
. flag - (logical) 

.N fortran
@*/
EXPORT_MPI_API int MPI_Test_cancelled( 
	MPI_Status *status,
	int        *flag)
{
    *flag = (status->MPI_TAG == MPIR_MSG_CANCELLED);

    return MPI_SUCCESS;
}
