/* graph_nbr.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GRAPH_NEIGHBORS = PMPI_GRAPH_NEIGHBORS
EXPORT_MPI_API void MPI_GRAPH_NEIGHBORS ( MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_graph_neighbors__ = pmpi_graph_neighbors__
EXPORT_MPI_API void mpi_graph_neighbors__ ( MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_graph_neighbors = pmpi_graph_neighbors
EXPORT_MPI_API void mpi_graph_neighbors ( MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_graph_neighbors_ = pmpi_graph_neighbors_
EXPORT_MPI_API void mpi_graph_neighbors_ ( MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GRAPH_NEIGHBORS ( MPI_Fint*, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GRAPH_NEIGHBORS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_graph_neighbors__ ( MPI_Fint*, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_graph_neighbors ( MPI_Fint*, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors")));
#else
EXPORT_MPI_API void mpi_graph_neighbors_ ( MPI_Fint*, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_neighbors_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GRAPH_NEIGHBORS  MPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors__  mpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors  mpi_graph_neighbors
#else
#pragma _HP_SECONDARY_DEF pmpi_graph_neighbors_  mpi_graph_neighbors_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GRAPH_NEIGHBORS as PMPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_graph_neighbors__ as pmpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_graph_neighbors as pmpi_graph_neighbors
#else
#pragma _CRI duplicate mpi_graph_neighbors_ as pmpi_graph_neighbors_
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
#define mpi_graph_neighbors_ PMPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_ pmpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_ pmpi_graph_neighbors
#else
#define mpi_graph_neighbors_ pmpi_graph_neighbors_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_graph_neighbors_ MPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_ mpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_ mpi_graph_neighbors
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_graph_neighbors_ ANSI_ARGS(( MPI_Fint*, MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_graph_neighbors_ ( MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint *neighbors, MPI_Fint *__ierr )
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Graph_neighbors( MPI_Comm_f2c(*comm), *rank,
                                       *maxneighbors, neighbors);
    else {
        int *lneighbors;
        int i;

	MPIR_FALLOC(lneighbors,(int*)MALLOC(sizeof(int)* (int)*maxneighbors),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_neighbors");

        *__ierr = MPI_Graph_neighbors( MPI_Comm_f2c(*comm), (int)*rank,
                                       (int)*maxneighbors, lneighbors);
        for (i=0; i<*maxneighbors; i++)
	    neighbors[i] = (MPI_Fint)lneighbors[i];
	
	FREE( lneighbors );
    }
}
