/* ic_merge.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INTERCOMM_MERGE = PMPI_INTERCOMM_MERGE
EXPORT_MPI_API void MPI_INTERCOMM_MERGE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_intercomm_merge__ = pmpi_intercomm_merge__
EXPORT_MPI_API void mpi_intercomm_merge__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_intercomm_merge = pmpi_intercomm_merge
EXPORT_MPI_API void mpi_intercomm_merge ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_intercomm_merge_ = pmpi_intercomm_merge_
EXPORT_MPI_API void mpi_intercomm_merge_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INTERCOMM_MERGE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_INTERCOMM_MERGE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_intercomm_merge__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_merge__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_intercomm_merge ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_merge")));
#else
EXPORT_MPI_API void mpi_intercomm_merge_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_merge_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INTERCOMM_MERGE  MPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_intercomm_merge__  mpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_intercomm_merge  mpi_intercomm_merge
#else
#pragma _HP_SECONDARY_DEF pmpi_intercomm_merge_  mpi_intercomm_merge_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INTERCOMM_MERGE as PMPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_intercomm_merge__ as pmpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_intercomm_merge as pmpi_intercomm_merge
#else
#pragma _CRI duplicate mpi_intercomm_merge_ as pmpi_intercomm_merge_
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
#define mpi_intercomm_merge_ PMPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge
#else
#define mpi_intercomm_merge_ pmpi_intercomm_merge_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ MPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_intercomm_merge_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_intercomm_merge_ ( MPI_Fint *comm, MPI_Fint *high, MPI_Fint *comm_out, MPI_Fint *__ierr )
{
    MPI_Comm l_comm_out;

    *__ierr = MPI_Intercomm_merge( MPI_Comm_f2c(*comm), (int)*high, 
                                   &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
