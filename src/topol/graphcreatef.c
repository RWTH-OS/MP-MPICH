/* graph_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GRAPH_CREATE = PMPI_GRAPH_CREATE
EXPORT_MPI_API void MPI_GRAPH_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_graph_create__ = pmpi_graph_create__
EXPORT_MPI_API void mpi_graph_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_graph_create = pmpi_graph_create
EXPORT_MPI_API void mpi_graph_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_graph_create_ = pmpi_graph_create_
EXPORT_MPI_API void mpi_graph_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GRAPH_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GRAPH_CREATE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_graph_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_create__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_graph_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_create")));
#else
EXPORT_MPI_API void mpi_graph_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graph_create_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GRAPH_CREATE  MPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_create__  mpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graph_create  mpi_graph_create
#else
#pragma _HP_SECONDARY_DEF pmpi_graph_create_  mpi_graph_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GRAPH_CREATE as PMPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_graph_create__ as pmpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_graph_create as pmpi_graph_create
#else
#pragma _CRI duplicate mpi_graph_create_ as pmpi_graph_create_
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
#define mpi_graph_create_ PMPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_create_ pmpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_create_ pmpi_graph_create
#else
#define mpi_graph_create_ pmpi_graph_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_graph_create_ MPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_create_ mpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_create_ mpi_graph_create
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_graph_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_graph_create_ ( MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint *index, MPI_Fint *edges, MPI_Fint *reorder, MPI_Fint *comm_graph,
			 MPI_Fint *__ierr )
{

    MPI_Comm lcomm_graph;

    if (sizeof(MPI_Fint) == sizeof(int))
#if defined(_TWO_WORD_FCD)
        int tmp = *reorder;
        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), *nnodes,
                                    index, edges,
                                    MPIR_FROM_FLOG(tmp), 
                                    &lcomm_graph);
#else
        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), *nnodes,
                                    index, edges,
                                    MPIR_FROM_FLOG(*reorder), 
                                    &lcomm_graph);
#endif
    else {
        int i;
        int nedges;
        int *lindex;
        int *ledges;


        MPI_Graphdims_get(MPI_Comm_f2c(*comm_old), nnodes, &nedges);	
	MPIR_FALLOC(lindex,(int*)MALLOC(sizeof(int)* (int)*nnodes),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_create");
	MPIR_FALLOC(ledges,(int*)MALLOC(sizeof(int)* (int)nedges),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_create");

        for (i=0; i<(int)*nnodes; i++)
	    lindex[i] = (int)index[i];

        for (i=0; i<nedges; i++)
	    ledges[i] = (int)edges[i];

#if defined(_TWO_WORD_FCD)
        int tmp = *reorder;
        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), (int)*nnodes,
                                    lindex, ledges,
                                    MPIR_FROM_FLOG(tmp), 
                                    &lcomm_graph);
#else
        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), (int)*nnodes,
                                    lindex, ledges,
                                    MPIR_FROM_FLOG(*reorder), 
                                    &lcomm_graph);
#endif
	FREE( lindex );
	FREE( ledges );
    }
    *comm_graph = MPI_Comm_c2f(lcomm_graph);
}
