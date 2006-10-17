/* testall.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TESTALL = PMPI_TESTALL
EXPORT_MPI_API void MPI_TESTALL ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_testall__ = pmpi_testall__
EXPORT_MPI_API void mpi_testall__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_testall = pmpi_testall
EXPORT_MPI_API void mpi_testall ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#else
#pragma weak mpi_testall_ = pmpi_testall_
EXPORT_MPI_API void mpi_testall_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TESTALL ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TESTALL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_testall__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testall__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_testall ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testall")));
#else
EXPORT_MPI_API void mpi_testall_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testall_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TESTALL  MPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_testall__  mpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_testall  mpi_testall
#else
#pragma _HP_SECONDARY_DEF pmpi_testall_  mpi_testall_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TESTALL as PMPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_testall__ as pmpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_testall as pmpi_testall
#else
#pragma _CRI duplicate mpi_testall_ as pmpi_testall_
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
#define mpi_testall_ PMPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testall_ pmpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testall_ pmpi_testall
#else
#define mpi_testall_ pmpi_testall_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_testall_ MPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testall_ mpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testall_ mpi_testall
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_testall_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint *, 
                              MPI_Fint [][MPI_STATUS_SIZE],
			      MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_testall_( MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[][MPI_STATUS_SIZE], MPI_Fint *__ierr )
{
    int lflag;
    int i;
    MPI_Request *lrequest = 0;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status *c_status = 0;
    MPI_Status local_c_status[MPIR_USE_LOCAL_ARRAY];

    if ((int)*count > 0) {
        if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
            MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request)* (int)*count),
                        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
                        "MPI_TESTALL");
            MPIR_FALLOC(c_status,(MPI_Status*)MALLOC(sizeof(MPI_Status)* (int)*count),
                        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
                        "MPI_TESTTALL");
        }
        else {
	    lrequest = local_lrequest;
            c_status = local_c_status;
	}
	for (i=0; i<(int)*count; i++) {
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}

	*__ierr = MPI_Testall((int)*count,lrequest,&lflag,c_status);
        /* By checking for lrequest[i] = 0, we handle persistant requests */
	for (i=0; i<(int)*count; i++) {
#ifdef OLD_POINTER
	    if (lrequest[i] == MPI_REQUEST_NULL) {
		MPIR_RmPointer( (int)(array_of_requests[i]) );
		array_of_requests[i] = 0;
	    }
	    else
#endif
	        array_of_requests[i] = MPI_Request_c2f( lrequest[i] );
	}
    }
    else
	*__ierr = MPI_Testall((int)*count,(MPI_Request *)0,&lflag,c_status);
    
    *flag = MPIR_TO_FLOG(lflag);
    /* We must only copy for those elements that corresponded to non-null
       requests, and only if there is a change */
    if (lflag) {
	for (i=0; i<(int)*count; i++) {
	    MPI_Status_c2f( &c_status[i], &(array_of_statuses[i][0]) );
	}
    }

    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	FREE( lrequest );
	FREE( c_status );
    }
}


