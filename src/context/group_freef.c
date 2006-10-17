/* group_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_FREE = PMPI_GROUP_FREE
EXPORT_MPI_API void MPI_GROUP_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_free__ = pmpi_group_free__
EXPORT_MPI_API void mpi_group_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_free = pmpi_group_free
EXPORT_MPI_API void mpi_group_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_free_ = pmpi_group_free_
EXPORT_MPI_API void mpi_group_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_free")));
#else
EXPORT_MPI_API void mpi_group_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_free_")));
#endif


#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_FREE  MPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_free__  mpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_free  mpi_group_free
#else
#pragma _HP_SECONDARY_DEF pmpi_group_free_  mpi_group_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_FREE as PMPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_free__ as pmpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_free as pmpi_group_free
#else
#pragma _CRI duplicate mpi_group_free_ as pmpi_group_free_
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
#define mpi_group_free_ PMPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ pmpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ pmpi_group_free
#else
#define mpi_group_free_ pmpi_group_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_free_ MPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ mpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ mpi_group_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_free_ ( MPI_Fint *group, MPI_Fint *__ierr )
{
    MPI_Group l_group = MPI_Group_f2c(*group);
    *__ierr = MPI_Group_free(&l_group);
    *group = MPI_Group_c2f(l_group);
}


