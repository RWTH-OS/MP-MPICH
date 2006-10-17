/* statuscancel.c */
/* Fortran interface file */

#include "mpiimpl.h"
/*
* This file was generated automatically by bfort from the C source
* file.  
 */

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_STATUS_SET_CANCELLED = PMPI_STATUS_SET_CANCELLED
EXPORT_MPI_API void MPI_STATUS_SET_CANCELLED (MPI_Status *, int *, int * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_status_set_cancelled__ = pmpi_status_set_cancelled__
EXPORT_MPI_API void mpi_status_set_cancelled__ (MPI_Status *, int *, int * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_status_set_cancelled = pmpi_status_set_cancelled
EXPORT_MPI_API void mpi_status_set_cancelled (MPI_Status *, int *, int * );
#else
#pragma weak mpi_status_set_cancelled_ = pmpi_status_set_cancelled_
EXPORT_MPI_API void mpi_status_set_cancelled_ (MPI_Status *, int *, int * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_STATUS_SET_CANCELLED (MPI_Status *, int *,
	int * ) __attribute__ ((weak, alias ("PMPI_STATUS_SET_CANCELLED")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_status_set_cancelled__ (MPI_Status *, int *,
	int * ) __attribute__ ((weak, alias ("pmpi_status_set_cancelled__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_status_set_cancelled (MPI_Status *, int *,
	int * ) __attribute__ ((weak, alias ("pmpi_status_set_cancelled")));
#else
EXPORT_MPI_API void mpi_status_set_cancelled_ (MPI_Status *, int *,
	int * ) __attribute__ ((weak, alias ("pmpi_status_set_cancelled_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_STATUS_SET_CANCELLED  MPI_STATUS_SET_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_status_set_cancelled__  mpi_status_set_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_status_set_cancelled  mpi_status_set_cancelled
#else
#pragma _HP_SECONDARY_DEF pmpi_status_set_cancelled_  mpi_status_set_cancelled_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_STATUS_SET_CANCELLED as PMPI_STATUS_SET_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_status_set_cancelled__ as pmpi_status_set_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_status_set_cancelled as pmpi_status_set_cancelled
#else
#pragma _CRI duplicate mpi_status_set_cancelled_ as pmpi_status_set_cancelled_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#ifdef FORTRANCAPS
#define mpi_status_set_cancelled_ PMPI_STATUS_SET_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_status_set_cancelled_ pmpi_status_set_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_status_set_cancelled_ pmpi_status_set_cancelled
#else
#define mpi_status_set_cancelled_ pmpi_status_set_cancelled_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_status_set_cancelled_ MPI_STATUS_SET_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_status_set_cancelled_ mpi_status_set_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_status_set_cancelled_ mpi_status_set_cancelled
#endif
#endif



EXPORT_MPI_API void FORTRAN_API mpi_status_set_cancelled_(MPI_Status *, int *, int * );

/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
EXPORT_MPI_API void FORTRAN_API mpi_status_set_cancelled_(MPI_Status *status, int *flag, int *__ierr )
{
	*__ierr = MPI_Status_set_cancelled(status,*flag);
}
#if defined(__cplusplus)
}
#endif
