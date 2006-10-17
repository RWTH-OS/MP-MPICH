/* cancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_CANCEL = PMPI_CANCEL
EXPORT_MPI_API void MPI_CANCEL ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_cancel__ = pmpi_cancel__
EXPORT_MPI_API void mpi_cancel__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_cancel = pmpi_cancel
EXPORT_MPI_API void mpi_cancel ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_cancel_ = pmpi_cancel_
EXPORT_MPI_API void mpi_cancel_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_CANCEL ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_CANCEL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_cancel__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cancel__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_cancel ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cancel")));
#else
EXPORT_MPI_API void mpi_cancel_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cancel_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_CANCEL  MPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cancel__  mpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cancel  mpi_cancel
#else
#pragma _HP_SECONDARY_DEF pmpi_cancel_  mpi_cancel_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_CANCEL as PMPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_cancel__ as pmpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_cancel as pmpi_cancel
#else
#pragma _CRI duplicate mpi_cancel_ as pmpi_cancel_
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
#define mpi_cancel_ PMPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ pmpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ pmpi_cancel
#else
#define mpi_cancel_ pmpi_cancel_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_cancel_ MPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ mpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ mpi_cancel
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_cancel_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_cancel_( MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;

    lrequest = MPI_Request_f2c(*request);  
    *__ierr = MPI_Cancel(&lrequest); 
}
