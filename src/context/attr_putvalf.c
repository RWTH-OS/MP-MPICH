/* attr_putval.c */
/* THIS IS A CUSTOM WRAPPER */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ATTR_PUT = PMPI_ATTR_PUT
EXPORT_MPI_API void MPI_ATTR_PUT ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_attr_put__ = pmpi_attr_put__
EXPORT_MPI_API void mpi_attr_put__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_attr_put = pmpi_attr_put
EXPORT_MPI_API void mpi_attr_put ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_attr_put_ = pmpi_attr_put_
EXPORT_MPI_API void mpi_attr_put_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ATTR_PUT ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ATTR_PUT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_attr_put__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_put__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_attr_put ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_put")));
#else
EXPORT_MPI_API void mpi_attr_put_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_put_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ATTR_PUT  MPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_attr_put__  mpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_attr_put  mpi_attr_put
#else
#pragma _HP_SECONDARY_DEF pmpi_attr_put_  mpi_attr_put_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ATTR_PUT as PMPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_attr_put__ as pmpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_attr_put as pmpi_attr_put
#else
#pragma _CRI duplicate mpi_attr_put_ as pmpi_attr_put_
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
#define mpi_attr_put_ PMPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put
#else
#define mpi_attr_put_ pmpi_attr_put_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_attr_put_ MPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_attr_put_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_attr_put_ ( MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attr_value, MPI_Fint *__ierr )
{
    *__ierr = MPI_Attr_put( MPI_Comm_f2c(*comm), (int)*keyval,
                            (void *)(MPI_Aint)((int)*attr_value));
}
