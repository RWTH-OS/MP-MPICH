/* errfree.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ERRHANDLER_FREE = PMPI_ERRHANDLER_FREE
EXPORT_MPI_API void MPI_ERRHANDLER_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_errhandler_free__ = pmpi_errhandler_free__
EXPORT_MPI_API void mpi_errhandler_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_errhandler_free = pmpi_errhandler_free
EXPORT_MPI_API void mpi_errhandler_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_errhandler_free_ = pmpi_errhandler_free_
EXPORT_MPI_API void mpi_errhandler_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ERRHANDLER_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ERRHANDLER_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_errhandler_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_errhandler_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_free")));
#else
EXPORT_MPI_API void mpi_errhandler_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ERRHANDLER_FREE  MPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_free__  mpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_free  mpi_errhandler_free
#else
#pragma _HP_SECONDARY_DEF pmpi_errhandler_free_  mpi_errhandler_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ERRHANDLER_FREE as PMPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_free__ as pmpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_free as pmpi_errhandler_free
#else
#pragma _CRI duplicate mpi_errhandler_free_ as pmpi_errhandler_free_
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
#define mpi_errhandler_free_ PMPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_free_ pmpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_free_ pmpi_errhandler_free
#else
#define mpi_errhandler_free_ pmpi_errhandler_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_errhandler_free_ MPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_free_ mpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_free_ mpi_errhandler_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_errhandler_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_errhandler_free_( MPI_Fint *errhandler, MPI_Fint *__ierr )
{
    MPI_Errhandler l_errhandler = MPI_Errhandler_c2f(*errhandler);
    *__ierr = MPI_Errhandler_free( &l_errhandler );
}
