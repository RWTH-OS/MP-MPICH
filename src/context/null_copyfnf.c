/* null_copy_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#undef MPI_NULL_COPY_FN

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_NULL_COPY_FN = PMPI_NULL_COPY_FN
EXPORT_MPI_API void MPI_NULL_COPY_FN ( MPI_Fint, MPI_Fint *, void *, void *, void *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_null_copy_fn__ = pmpi_null_copy_fn__
EXPORT_MPI_API void mpi_null_copy_fn__ ( MPI_Fint, MPI_Fint *, void *, void *, void *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_null_copy_fn = pmpi_null_copy_fn
EXPORT_MPI_API void mpi_null_copy_fn ( MPI_Fint, MPI_Fint *, void *, void *, void *, MPI_Fint * );
#else
#pragma weak mpi_null_copy_fn_ = pmpi_null_copy_fn_
EXPORT_MPI_API void mpi_null_copy_fn_ ( MPI_Fint, MPI_Fint *, void *, void *, void *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_NULL_COPY_FN ( MPI_Fint, MPI_Fint *, void *, void *,
	void *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_NULL_COPY_FN")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_null_copy_fn__ ( MPI_Fint, MPI_Fint *, void *, void *,
	void *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_null_copy_fn__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_null_copy_fn ( MPI_Fint, MPI_Fint *, void *, void *,
	void *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_null_copy_fn")));
#else
EXPORT_MPI_API void mpi_null_copy_fn_ ( MPI_Fint, MPI_Fint *, void *, void *,
	void *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_null_copy_fn_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_NULL_COPY_FN  MPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_null_copy_fn__  mpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_null_copy_fn  mpi_null_copy_fn
#else
#pragma _HP_SECONDARY_DEF pmpi_null_copy_fn_  mpi_null_copy_fn_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_NULL_COPY_FN as PMPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_null_copy_fn__ as pmpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_null_copy_fn as pmpi_null_copy_fn
#else
#pragma _CRI duplicate mpi_null_copy_fn_ as pmpi_null_copy_fn_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
#endif

#ifdef FORTRANCAPS
#define mpi_null_copy_fn_ PMPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_copy_fn_ pmpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_copy_fn_ pmpi_null_copy_fn
#else
#define mpi_null_copy_fn_ pmpi_null_copy_fn_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_null_copy_fn_ MPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_copy_fn_ mpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_copy_fn_ mpi_null_copy_fn
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_null_copy_fn_ ( MPI_Fint, MPI_Fint *, void *, void *, 
				   void *, MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_null_copy_fn_ ( MPI_Fint comm, MPI_Fint *keyval, void *extra_state, void *attr_in, void *attr_out, MPI_Fint *flag )
{
    /* Note the we actually need to fix the comm argument, except that the
       null function doesn't use it */
    *flag = MPIR_TO_FLOG(0);
}
