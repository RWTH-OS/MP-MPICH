/* wait.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WAIT = PMPI_WAIT
EXPORT_MPI_API void MPI_WAIT ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_wait__ = pmpi_wait__
EXPORT_MPI_API void mpi_wait__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_wait = pmpi_wait
EXPORT_MPI_API void mpi_wait ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_wait_ = pmpi_wait_
EXPORT_MPI_API void mpi_wait_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WAIT ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_WAIT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_wait__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_wait__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_wait ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_wait")));
#else
EXPORT_MPI_API void mpi_wait_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_wait_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WAIT  MPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_wait__  mpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_wait  mpi_wait
#else
#pragma _HP_SECONDARY_DEF pmpi_wait_  mpi_wait_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WAIT as PMPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_wait__ as pmpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_wait as pmpi_wait
#else
#pragma _CRI duplicate mpi_wait_ as pmpi_wait_
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
#define mpi_wait_ PMPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wait_ pmpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wait_ pmpi_wait
#else
#define mpi_wait_ pmpi_wait_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_wait_ MPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wait_ mpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wait_ mpi_wait
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_wait_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_wait_ ( MPI_Fint *request, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    MPI_Status c_status;

    lrequest = MPI_Request_f2c(*request);
    *__ierr = MPI_Wait(&lrequest, &c_status);
#ifdef OLD_POINTER
    /* By checking for null, we handle persistant requests */
    if (lrequest == MPI_REQUEST_NULL) {
        MPIR_RmPointer( ((int)(lrequest)) );
        *request = 0;
    }
    else
#endif
        *request = MPI_Request_c2f(lrequest);

    MPI_Status_c2f(&c_status, status);
}
