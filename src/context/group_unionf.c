/* group_union.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_UNION = PMPI_GROUP_UNION
EXPORT_MPI_API void MPI_GROUP_UNION ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_union__ = pmpi_group_union__
EXPORT_MPI_API void mpi_group_union__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_union = pmpi_group_union
EXPORT_MPI_API void mpi_group_union ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_union_ = pmpi_group_union_
EXPORT_MPI_API void mpi_group_union_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_UNION ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_UNION")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_union__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_union__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_union ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_union")));
#else
EXPORT_MPI_API void mpi_group_union_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_union_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_UNION  MPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_union__  mpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_union  mpi_group_union
#else
#pragma _HP_SECONDARY_DEF pmpi_group_union_  mpi_group_union_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_UNION as PMPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_union__ as pmpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_union as pmpi_group_union
#else
#pragma _CRI duplicate mpi_group_union_ as pmpi_group_union_
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
#define mpi_group_union_ PMPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ pmpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ pmpi_group_union
#else
#define mpi_group_union_ pmpi_group_union_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_union_ MPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ mpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ mpi_group_union
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_union_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				  MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_union_ ( MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *group_out, MPI_Fint *__ierr )
{
    MPI_Group l_group_out;
    *__ierr = MPI_Group_union( MPI_Group_f2c(*group1), 
                               MPI_Group_f2c(*group2), 
                               &l_group_out );
    *group_out = l_group_out;
}
