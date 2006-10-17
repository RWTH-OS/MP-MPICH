/* commreq_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_REQUEST_FREE = PMPI_REQUEST_FREE
EXPORT_MPI_API void MPI_REQUEST_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_request_free__ = pmpi_request_free__
EXPORT_MPI_API void mpi_request_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_request_free = pmpi_request_free
EXPORT_MPI_API void mpi_request_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_request_free_ = pmpi_request_free_
EXPORT_MPI_API void mpi_request_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_REQUEST_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_REQUEST_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_request_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_request_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_request_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_request_free")));
#else
EXPORT_MPI_API void mpi_request_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_request_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_REQUEST_FREE  MPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_request_free__  mpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_request_free  mpi_request_free
#else
#pragma _HP_SECONDARY_DEF pmpi_request_free_  mpi_request_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_REQUEST_FREE as PMPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_request_free__ as pmpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_request_free as pmpi_request_free
#else
#pragma _CRI duplicate mpi_request_free_ as pmpi_request_free_
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
#define mpi_request_free_ PMPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_request_free_ pmpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_request_free_ pmpi_request_free
#else
#define mpi_request_free_ pmpi_request_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_request_free_ MPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_request_free_ mpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_request_free_ mpi_request_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_request_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_request_free_( MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest = MPI_Request_f2c(*request);
    *__ierr = MPI_Request_free( &lrequest );

#ifdef OLD_POINTER
/* 
   We actually need to remove the pointer from the mapping if the ref
   count is zero.  We do that by checking to see if lrequest was set to
   NULL.
 */
    if (!lrequest) {
	MPIR_RmPointer( *(int*)request );
	*(int*)request = 0;
    }
#endif
    *request = MPI_Request_c2f(lrequest);

}

