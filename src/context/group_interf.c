/* group_inter.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_INTERSECTION = PMPI_GROUP_INTERSECTION
EXPORT_MPI_API void MPI_GROUP_INTERSECTION ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_intersection__ = pmpi_group_intersection__
EXPORT_MPI_API void mpi_group_intersection__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_intersection = pmpi_group_intersection
EXPORT_MPI_API void mpi_group_intersection ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_intersection_ = pmpi_group_intersection_
EXPORT_MPI_API void mpi_group_intersection_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_INTERSECTION ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_INTERSECTION")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_intersection__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_intersection__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_intersection ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_intersection")));
#else
EXPORT_MPI_API void mpi_group_intersection_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_intersection_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_INTERSECTION  MPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_intersection__  mpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_intersection  mpi_group_intersection
#else
#pragma _HP_SECONDARY_DEF pmpi_group_intersection_  mpi_group_intersection_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_INTERSECTION as PMPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_intersection__ as pmpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_intersection as pmpi_group_intersection
#else
#pragma _CRI duplicate mpi_group_intersection_ as pmpi_group_intersection_
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
#define mpi_group_intersection_ PMPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection
#else
#define mpi_group_intersection_ pmpi_group_intersection_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_intersection_ MPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_intersection_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                         MPI_Fint *,
					 MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_intersection_ ( MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *group_out, MPI_Fint *__ierr )
{
    MPI_Group l_group_out;
    *__ierr = MPI_Group_intersection(MPI_Group_f2c(*group1), 
                                     MPI_Group_f2c(*group2), 
                                     &l_group_out );
    *group_out = MPI_Group_c2f(l_group_out);
}
