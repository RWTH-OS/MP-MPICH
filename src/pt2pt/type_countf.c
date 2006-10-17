/* type_count.c */
/* Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_COUNT = PMPI_TYPE_COUNT
EXPORT_MPI_API void MPI_TYPE_COUNT ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_count__ = pmpi_type_count__
EXPORT_MPI_API void mpi_type_count__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_count = pmpi_type_count
EXPORT_MPI_API void mpi_type_count ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_count_ = pmpi_type_count_
EXPORT_MPI_API void mpi_type_count_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_COUNT ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_COUNT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_count__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_count__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_count ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_count")));
#else
EXPORT_MPI_API void mpi_type_count_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_count_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_COUNT  MPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_count__  mpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_count  mpi_type_count
#else
#pragma _HP_SECONDARY_DEF pmpi_type_count_  mpi_type_count_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_COUNT as PMPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_count__ as pmpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_count as pmpi_type_count
#else
#pragma _CRI duplicate mpi_type_count_ as pmpi_type_count_
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
#define mpi_type_count_ PMPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_count_ pmpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_count_ pmpi_type_count
#else
#define mpi_type_count_ pmpi_type_count_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_count_ MPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_count_ mpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_count_ mpi_type_count
#endif
#endif


 void mpi_type_count_ ( datatype, count, __ierr )
MPI_Datatype  datatype;
int          *count;
int *__ierr;
{
*__ierr = MPI_Type_count( *(int*)(datatype) ),count);
}





