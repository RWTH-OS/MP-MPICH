/* keyval_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_KEYVAL_FREE = PMPI_KEYVAL_FREE
EXPORT_MPI_API void MPI_KEYVAL_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_keyval_free__ = pmpi_keyval_free__
EXPORT_MPI_API void mpi_keyval_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_keyval_free = pmpi_keyval_free
EXPORT_MPI_API void mpi_keyval_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_keyval_free_ = pmpi_keyval_free_
EXPORT_MPI_API void mpi_keyval_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_KEYVAL_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_KEYVAL_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_keyval_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_keyval_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_keyval_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_keyval_free")));
#else
EXPORT_MPI_API void mpi_keyval_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_keyval_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_KEYVAL_FREE  MPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_keyval_free__  mpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_keyval_free  mpi_keyval_free
#else
#pragma _HP_SECONDARY_DEF pmpi_keyval_free_  mpi_keyval_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_KEYVAL_FREE as PMPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_keyval_free__ as pmpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_keyval_free as pmpi_keyval_free
#else
#pragma _CRI duplicate mpi_keyval_free_ as pmpi_keyval_free_
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
#define mpi_keyval_free_ PMPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free
#else
#define mpi_keyval_free_ pmpi_keyval_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_keyval_free_ MPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_keyval_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_keyval_free_ ( MPI_Fint *keyval, MPI_Fint *__ierr )
{
    int l_keyval = (int)*keyval;
    *__ierr = MPI_Keyval_free(&l_keyval);
    *keyval = (MPI_Fint)l_keyval;
}
