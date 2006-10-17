/* group_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_SIZE = PMPI_GROUP_SIZE
EXPORT_MPI_API void MPI_GROUP_SIZE ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_size__ = pmpi_group_size__
EXPORT_MPI_API void mpi_group_size__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_size = pmpi_group_size
EXPORT_MPI_API void mpi_group_size ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_size_ = pmpi_group_size_
EXPORT_MPI_API void mpi_group_size_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_SIZE ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_SIZE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_size__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_size__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_size ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_size")));
#else
EXPORT_MPI_API void mpi_group_size_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_size_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_SIZE  MPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_size__  mpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_size  mpi_group_size
#else
#pragma _HP_SECONDARY_DEF pmpi_group_size_  mpi_group_size_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_SIZE as PMPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_size__ as pmpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_size as pmpi_group_size
#else
#pragma _CRI duplicate mpi_group_size_ as pmpi_group_size_
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
#define mpi_group_size_ PMPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ pmpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ pmpi_group_size
#else
#define mpi_group_size_ pmpi_group_size_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_size_ MPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ mpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ mpi_group_size
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_size_ ( MPI_Fint *group, MPI_Fint *size, MPI_Fint *__ierr )
{
    int l_size;
    *__ierr = MPI_Group_size( MPI_Group_f2c(*group), &l_size );
    *size = l_size;
}
