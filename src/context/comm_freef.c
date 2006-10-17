/* comm_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_FREE = PMPI_COMM_FREE
EXPORT_MPI_API void MPI_COMM_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_free__ = pmpi_comm_free__
EXPORT_MPI_API void mpi_comm_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_free = pmpi_comm_free
EXPORT_MPI_API void mpi_comm_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_comm_free_ = pmpi_comm_free_
EXPORT_MPI_API void mpi_comm_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_free")));
#else
EXPORT_MPI_API void mpi_comm_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_FREE  MPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_free__  mpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_free  mpi_comm_free
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_free_  mpi_comm_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_FREE as PMPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_free__ as pmpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_free as pmpi_comm_free
#else
#pragma _CRI duplicate mpi_comm_free_ as pmpi_comm_free_
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
#define mpi_comm_free_ PMPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free
#else
#define mpi_comm_free_ pmpi_comm_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_free_ MPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_free_ ( MPI_Fint *comm, MPI_Fint *__ierr )
{
    MPI_Comm l_comm = MPI_Comm_f2c(*comm);
    *__ierr = MPI_Comm_free(&l_comm);
    *comm = MPI_Comm_c2f(l_comm);
}
