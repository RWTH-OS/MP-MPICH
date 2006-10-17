/* abort.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ABORT = PMPI_ABORT
EXPORT_MPI_API void MPI_ABORT ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_abort__ = pmpi_abort__
EXPORT_MPI_API void mpi_abort__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_abort = pmpi_abort
EXPORT_MPI_API void mpi_abort ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_abort_ = pmpi_abort_
EXPORT_MPI_API void mpi_abort_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ABORT ( MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ABORT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_abort__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_abort__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_abort ( MPI_Fint *, MPI_Fint *, MPI_Fint * )  __attribute__ ((weak, alias ("pmpi_abort")));
#else
EXPORT_MPI_API void mpi_abort_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_abort_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ABORT  MPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_abort__  mpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_abort  mpi_abort
#else
#pragma _HP_SECONDARY_DEF pmpi_abort_  mpi_abort_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ABORT as PMPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_abort__ as pmpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_abort as pmpi_abort
#else
#pragma _CRI duplicate mpi_abort_ as pmpi_abort_
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
#define mpi_abort_ PMPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_abort_ pmpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_abort_ pmpi_abort
#else
#define mpi_abort_ pmpi_abort_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_abort_ MPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_abort_ mpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_abort_ mpi_abort
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_abort_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_abort_( MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *__ierr )
{
    *__ierr = MPI_Abort( MPI_Comm_f2c(*comm), (int)*errorcode);
}



