/* graph_nbr_cnt.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GRAPH_NEIGHBORS_COUNT = PMPI_GRAPH_NEIGHBORS_COUNT
EXPORT_MPI_API void MPI_GRAPH_NEIGHBORS_COUNT ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_graph_neighbors_count__ = pmpi_graph_neighbors_count__
EXPORT_MPI_API void mpi_graph_neighbors_count__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_graph_neighbors_count = pmpi_graph_neighbors_count
EXPORT_MPI_API void mpi_graph_neighbors_count ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_graph_neighbors_count_ = pmpi_graph_neighbors_count_
EXPORT_MPI_API void mpi_graph_neighbors_count_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GRAPH_NEIGHBORS_COUNT ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GRAPH_NEIGHBORS_COUNT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_graph_neighbors_count__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors_count__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_graph_neighbors_count ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors_count")));
#else
EXPORT_MPI_API void mpi_graph_neighbors_count_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors_count_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GRAPH_NEIGHBORS_COUNT  MPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors_count__  mpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors_count  mpi_graph_neighbors_count
#else
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors_count_  mpi_graph_neighbors_count_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GRAPH_NEIGHBORS_COUNT as PMPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_graph_neighbors_count__ as pmpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_graph_neighbors_count as pmpi_graph_neighbors_count
#else
#pragma _CRI duplicate mpi_graph_neighbors_count_ as pmpi_graph_neighbors_count_
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
#define mpi_graph_neighbors_count_ PMPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count
#else
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_graph_neighbors_count_ MPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_count_ mpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_count_ mpi_graph_neighbors_count
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_graph_neighbors_count_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_graph_neighbors_count_ ( MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *__ierr )
{
    int lnneighbors;

    *__ierr = MPI_Graph_neighbors_count(MPI_Comm_f2c(*comm), (int)*rank,
                                        &lnneighbors);
    *nneighbors = (MPI_Fint)lnneighbors;
}
