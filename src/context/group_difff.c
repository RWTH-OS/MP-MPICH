/* group_diff.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_DIFFERENCE = PMPI_GROUP_DIFFERENCE
EXPORT_MPI_API void MPI_GROUP_DIFFERENCE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_difference__ = pmpi_group_difference__
EXPORT_MPI_API void mpi_group_difference__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_difference = pmpi_group_difference
EXPORT_MPI_API void mpi_group_difference ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_difference_ = pmpi_group_difference_
EXPORT_MPI_API void mpi_group_difference_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_DIFFERENCE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_DIFFERENCE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_difference__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_difference__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_difference ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_difference")));
#else
EXPORT_MPI_API void mpi_group_difference_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_difference_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_DIFFERENCE  MPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_difference__  mpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_difference  mpi_group_difference
#else
#pragma _HP_SECONDARY_DEF pmpi_group_difference_  mpi_group_difference_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_DIFFERENCE as PMPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_difference__ as pmpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_difference as pmpi_group_difference
#else
#pragma _CRI duplicate mpi_group_difference_ as pmpi_group_difference_
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
#define mpi_group_difference_ PMPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_difference_ pmpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_difference_ pmpi_group_difference
#else
#define mpi_group_difference_ pmpi_group_difference_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_difference_ MPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_difference_ mpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_difference_ mpi_group_difference
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_difference_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_difference_ ( MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *group_out, MPI_Fint *__ierr )
{
    MPI_Group l_group_out;

    *__ierr = MPI_Group_difference(MPI_Group_f2c(*group1), 
                                   MPI_Group_f2c(*group2), 
                                   &l_group_out );
    *group_out = MPI_Group_c2f(l_group_out);
}
