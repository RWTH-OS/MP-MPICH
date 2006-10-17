/*
 *  $Id: finalized.c,v 1.2 2003/06/05 11:50:35 rainer Exp $
 *
 *  (C) 1997 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Finalized = PMPI_Finalized
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Finalized( int *flag ) __attribute__ ((weak, alias ("PMPI_Finalized")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Finalized  MPI_Finalized
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Finalized as PMPI_Finalized
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
   MPI_Finalized - Indicates whether 'MPI_Finalize' has been called.

Output Parameter:
. flag - Flag is true if 'MPI_Finalize' has been called and false otherwise. 

.N fortran
@*/
EXPORT_MPI_API int MPI_Finalized( int *flag )
{
/* 
   MPI_Init sets MPIR_Has_been_initialized to 1, MPI_Finalize sets to 2.
 */
    *flag = MPIR_Has_been_initialized >= 2;
    return MPI_SUCCESS;
}
