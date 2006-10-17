/* statuselm.c */
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
#pragma weak MPI_STATUS_SET_ELEMENTS = PMPI_STATUS_SET_ELEMENTS
EXPORT_MPI_API void MPI_STATUS_SET_ELEMENTS (MPI_Status *, MPI_Datatype, int *, int * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_status_set_elements__ = pmpi_status_set_elements__
EXPORT_MPI_API void mpi_status_set_elements__ (MPI_Status *, MPI_Datatype, int *, int * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_status_set_elements = pmpi_status_set_elements
EXPORT_MPI_API void mpi_status_set_elements (MPI_Status *, MPI_Datatype, int *, int * );
#else
#pragma weak mpi_status_set_elements_ = pmpi_status_set_elements_
EXPORT_MPI_API void mpi_status_set_elements_ (MPI_Status *, MPI_Datatype, int *, int * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_STATUS_SET_ELEMENTS (MPI_Status *, MPI_Datatype,
	int *, int * ) __attribute__ ((weak, alias ("PMPI_STATUS_SET_ELEMENTS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_status_set_elements__ (MPI_Status *, MPI_Datatype,
	int *, int * ) __attribute__ ((weak, alias ("pmpi_status_set_elements__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_status_set_elements (MPI_Status *, MPI_Datatype,
	int *, int * ) __attribute__ ((weak, alias ("pmpi_status_set_elements")));
#else
EXPORT_MPI_API void mpi_status_set_elements_ (MPI_Status *, MPI_Datatype,
	int *, int * ) __attribute__ ((weak, alias ("pmpi_status_set_elements_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_STATUS_SET_ELEMENTS  MPI_STATUS_SET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_status_set_elements__  mpi_status_set_elements__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_status_set_elements  mpi_status_set_elements
#else
#pragma _HP_SECONDARY_DEF pmpi_status_set_elements_  mpi_status_set_elements_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_STATUS_SET_ELEMENTS as PMPI_STATUS_SET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_status_set_elements__ as pmpi_status_set_elements__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_status_set_elements as pmpi_status_set_elements
#else
#pragma _CRI duplicate mpi_status_set_elements_ as pmpi_status_set_elements_
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
#define mpi_status_set_elements_ PMPI_STATUS_SET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_status_set_elements_ pmpi_status_set_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_status_set_elements_ pmpi_status_set_elements
#else
#define mpi_status_set_elements_ pmpi_status_set_elements_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_status_set_elements_ MPI_STATUS_SET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_status_set_elements_ mpi_status_set_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_status_set_elements_ mpi_status_set_elements
#endif
#endif


EXPORT_MPI_API void FORTRAN_API mpi_status_set_elements_(MPI_Status *, MPI_Datatype, int *, int * );

/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
EXPORT_MPI_API void FORTRAN_API mpi_status_set_elements_(MPI_Status *status, MPI_Datatype datatype,
        int *count, int *__ierr )
{
*__ierr = MPI_Status_set_elements(status,
	(MPI_Datatype)*((int*)datatype),*count);
}
#if defined(__cplusplus)
}
#endif
