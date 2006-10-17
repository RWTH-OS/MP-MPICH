/* graphdims_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GRAPHDIMS_GET = PMPI_GRAPHDIMS_GET
EXPORT_MPI_API void MPI_GRAPHDIMS_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_graphdims_get__ = pmpi_graphdims_get__
EXPORT_MPI_API void mpi_graphdims_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_graphdims_get = pmpi_graphdims_get
EXPORT_MPI_API void mpi_graphdims_get ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_graphdims_get_ = pmpi_graphdims_get_
EXPORT_MPI_API void mpi_graphdims_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GRAPHDIMS_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GRAPHDIMS_GET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_graphdims_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graphdims_get__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_graphdims_get ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graphdims_get")));
#else
EXPORT_MPI_API void mpi_graphdims_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_graphdims_get_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GRAPHDIMS_GET  MPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graphdims_get__  mpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_graphdims_get  mpi_graphdims_get
#else
#pragma _HP_SECONDARY_DEF pmpi_graphdims_get_  mpi_graphdims_get_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GRAPHDIMS_GET as PMPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_graphdims_get__ as pmpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_graphdims_get as pmpi_graphdims_get
#else
#pragma _CRI duplicate mpi_graphdims_get_ as pmpi_graphdims_get_
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
#define mpi_graphdims_get_ PMPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get
#else
#define mpi_graphdims_get_ pmpi_graphdims_get_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_graphdims_get_ MPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_graphdims_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                    MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_graphdims_get_ ( MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *__ierr )
{
    int lnnodes;
    int lnedges;

    *__ierr = MPI_Graphdims_get( MPI_Comm_f2c(*comm), &lnnodes,
                                 &lnedges);
    *nnodes = (MPI_Fint)lnnodes;
    *nedges = (MPI_Fint)lnedges;
    
}
