/* finalize.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FINALIZE = PMPI_FINALIZE
EXPORT_MPI_API void MPI_FINALIZE ( MPI_Fint *__ierr );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_finalize__ = pmpi_finalize__
EXPORT_MPI_API void mpi_finalize__ ( MPI_Fint *__ierr );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_finalize = pmpi_finalize
EXPORT_MPI_API void mpi_finalize ( MPI_Fint *__ierr );
#else
#pragma weak mpi_finalize_ = pmpi_finalize_
EXPORT_MPI_API void mpi_finalize_ ( MPI_Fint *__ierr );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_FINALIZE ( MPI_Fint *__ierr ) __attribute__ ((weak, alias ("PMPI_FINALIZE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_finalize__ ( MPI_Fint *__ierr ) __attribute__ ((weak, alias ("pmpi_finalize__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_finalize ( MPI_Fint *__ierr ) __attribute__ ((weak, alias ("pmpi_finalize")));
#else
EXPORT_MPI_API void mpi_finalize_ ( MPI_Fint *__ierr ) __attribute__ ((weak, alias ("pmpi_finalize_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FINALIZE  MPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_finalize__  mpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_finalize  mpi_finalize
#else
#pragma _HP_SECONDARY_DEF pmpi_finalize_  mpi_finalize_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FINALIZE as PMPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_finalize__ as pmpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_finalize as pmpi_finalize
#else
#pragma _CRI duplicate mpi_finalize_ as pmpi_finalize_
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
#define mpi_finalize_ PMPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ pmpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ pmpi_finalize
#else
#define mpi_finalize_ pmpi_finalize_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_finalize_ MPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ mpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ mpi_finalize
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_finalize_( MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_finalize_( MPI_Fint *__ierr )
{
    *__ierr = MPI_Finalize();

}
