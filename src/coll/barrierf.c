/* barrier.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_BARRIER = PMPI_BARRIER
EXPORT_MPI_API void MPI_BARRIER ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_barrier__ = pmpi_barrier__
EXPORT_MPI_API void mpi_barrier__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_barrier = pmpi_barrier
EXPORT_MPI_API void mpi_barrier ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_barrier_ = pmpi_barrier_
EXPORT_MPI_API void mpi_barrier_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_BARRIER ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_BARRIER")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_barrier__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_barrier__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_barrier ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_barrier ")));
#else
EXPORT_MPI_API void mpi_barrier_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_barrier_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_BARRIER  MPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_barrier__  mpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_barrier  mpi_barrier
#else
#pragma _HP_SECONDARY_DEF pmpi_barrier_  mpi_barrier_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_BARRIER as PMPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_barrier__ as pmpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_barrier as pmpi_barrier
#else
#pragma _CRI duplicate mpi_barrier_ as pmpi_barrier_
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
#define mpi_barrier_ PMPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ pmpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ pmpi_barrier
#else
#define mpi_barrier_ pmpi_barrier_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_barrier_ MPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ mpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ mpi_barrier
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_barrier_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_barrier_ ( MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Barrier( MPI_Comm_f2c(*comm) );
}









