/* start.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_START = PMPI_START
EXPORT_MPI_API void MPI_START ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_start__ = pmpi_start__
EXPORT_MPI_API void mpi_start__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_start = pmpi_start
EXPORT_MPI_API void mpi_start ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_start_ = pmpi_start_
EXPORT_MPI_API void mpi_start_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_START ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_START")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_start__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_start__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_start ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_start")));
#else
EXPORT_MPI_API void mpi_start_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_start_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_START  MPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_start__  mpi_start__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_start  mpi_start
#else
#pragma _HP_SECONDARY_DEF pmpi_start_  mpi_start_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_START as PMPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_start__ as pmpi_start__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_start as pmpi_start
#else
#pragma _CRI duplicate mpi_start_ as pmpi_start_
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
#define mpi_start_ PMPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ pmpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ pmpi_start
#else
#define mpi_start_ pmpi_start_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_start_ MPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ mpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ mpi_start
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_start_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_start_( MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest = MPI_Request_f2c(*request );
    *__ierr = MPI_Start( &lrequest );
    *request = MPI_Request_c2f(lrequest);
}
