/* type_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_FREE = PMPI_TYPE_FREE
EXPORT_MPI_API void MPI_TYPE_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_free__ = pmpi_type_free__
EXPORT_MPI_API void mpi_type_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_free = pmpi_type_free
EXPORT_MPI_API void mpi_type_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_free_ = pmpi_type_free_
EXPORT_MPI_API void mpi_type_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_free")));
#else
EXPORT_MPI_API void mpi_type_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_FREE  MPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_free__  mpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_free  mpi_type_free
#else
#pragma _HP_SECONDARY_DEF pmpi_type_free_  mpi_type_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_FREE as PMPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_free__ as pmpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_free as pmpi_type_free
#else
#pragma _CRI duplicate mpi_type_free_ as pmpi_type_free_
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
#define mpi_type_free_ PMPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ pmpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ pmpi_type_free
#else
#define mpi_type_free_ pmpi_type_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_free_ MPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ mpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ mpi_type_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_free_ ( MPI_Fint *datatype, MPI_Fint *__ierr )
{
    MPI_Datatype ldatatype = MPI_Type_f2c(*datatype);
    *__ierr = MPI_Type_free(&ldatatype);
    *datatype = MPI_Type_c2f(ldatatype);
}
