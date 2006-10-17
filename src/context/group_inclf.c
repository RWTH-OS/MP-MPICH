/* group_incl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#include "mpimem.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_INCL = PMPI_GROUP_INCL
EXPORT_MPI_API void MPI_GROUP_INCL ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_incl__ = pmpi_group_incl__
EXPORT_MPI_API void mpi_group_incl__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_incl = pmpi_group_incl
EXPORT_MPI_API void mpi_group_incl ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_incl_ = pmpi_group_incl_
EXPORT_MPI_API void mpi_group_incl_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_INCL ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_INCL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_incl__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_incl__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_incl ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_incl")));
#else
EXPORT_MPI_API void mpi_group_incl_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_incl_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_INCL  MPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_incl__  mpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_incl  mpi_group_incl
#else
#pragma _HP_SECONDARY_DEF pmpi_group_incl_  mpi_group_incl_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_INCL as PMPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_incl__ as pmpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_incl as pmpi_group_incl
#else
#pragma _CRI duplicate mpi_group_incl_ as pmpi_group_incl_
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
#define mpi_group_incl_ PMPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl
#else
#define mpi_group_incl_ pmpi_group_incl_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_incl_ MPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_incl_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_incl_ ( MPI_Fint *group, MPI_Fint *n, MPI_Fint *ranks, MPI_Fint *group_out, MPI_Fint *__ierr )
{
    MPI_Group l_group_out;

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_incl( MPI_Group_f2c(*group), *n,
                                  ranks, &l_group_out );
    else {
	int *l_ranks;
	int i;

	MPIR_FALLOC(l_ranks,(int*)MALLOC(sizeof(int)* (int)*n),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_incl");
	for (i=0; i<*n; i++)
	    l_ranks[i] = (int)ranks[i];

        *__ierr = MPI_Group_incl( MPI_Group_f2c(*group), (int)*n,
                                  l_ranks, &l_group_out );
	
	FREE( l_ranks );
    }

    *group_out = MPI_Group_c2f(l_group_out);
}
