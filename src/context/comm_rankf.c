/* comm_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_RANK = PMPI_COMM_RANK
EXPORT_MPI_API void MPI_COMM_RANK ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_rank__ = pmpi_comm_rank__
EXPORT_MPI_API void mpi_comm_rank__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_rank = pmpi_comm_rank
EXPORT_MPI_API void mpi_comm_rank ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_comm_rank_ = pmpi_comm_rank_
EXPORT_MPI_API void mpi_comm_rank_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_RANK ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_RANK")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_rank__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_rank__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_rank ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_rank")));
#else
EXPORT_MPI_API void mpi_comm_rank_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_rank_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_RANK  MPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_rank__  mpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_rank  mpi_comm_rank
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_rank_  mpi_comm_rank_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_RANK as PMPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_rank__ as pmpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_rank as pmpi_comm_rank
#else
#pragma _CRI duplicate mpi_comm_rank_ as pmpi_comm_rank_
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
#define mpi_comm_rank_ PMPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank
#else
#define mpi_comm_rank_ pmpi_comm_rank_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_rank_ MPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_rank_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_rank_ ( MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *__ierr )
{
    int l_rank;
    *__ierr = MPI_Comm_rank( MPI_Comm_f2c(*comm), &l_rank);
    *rank = (MPI_Fint)l_rank;
}
