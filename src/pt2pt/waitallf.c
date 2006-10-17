/* waitall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WAITALL = PMPI_WAITALL
EXPORT_MPI_API void MPI_WAITALL ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_waitall__ = pmpi_waitall__
EXPORT_MPI_API void mpi_waitall__ ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_waitall = pmpi_waitall
EXPORT_MPI_API void mpi_waitall ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint *);
#else
#pragma weak mpi_waitall_ = pmpi_waitall_
EXPORT_MPI_API void mpi_waitall_ ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WAITALL ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_WAITALL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_waitall__ ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_waitall__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_waitall ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_waitall")));
#else
EXPORT_MPI_API void mpi_waitall_ ( MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_waitall_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WAITALL  MPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_waitall__  mpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_waitall  mpi_waitall
#else
#pragma _HP_SECONDARY_DEF pmpi_waitall_  mpi_waitall_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WAITALL as PMPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_waitall__ as pmpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_waitall as pmpi_waitall
#else
#pragma _CRI duplicate mpi_waitall_ as pmpi_waitall_
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
#define mpi_waitall_ PMPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ pmpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ pmpi_waitall
#else
#define mpi_waitall_ pmpi_waitall_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_waitall_ MPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ mpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ mpi_waitall
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_waitall_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [],
			      MPI_Fint [][MPI_STATUS_SIZE], 
			      MPI_Fint *));

EXPORT_MPI_API void FORTRAN_API mpi_waitall_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint array_of_statuses[][MPI_STATUS_SIZE], MPI_Fint *__ierr )
{
    int i;
    MPI_Request *lrequest = 0;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status *c_status = 0;
    MPI_Status local_c_status[MPIR_USE_LOCAL_ARRAY];

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request) * 
                        (int)*count), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITALL" );

	    MPIR_FALLOC(c_status,(MPI_Status*)MALLOC(sizeof(MPI_Status) * 
                        (int)*count), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITALL" );
	}
	else {
	    lrequest = local_lrequest;
	    c_status = local_c_status;
	}

	for (i=0; i<(int)*count; i++) {
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}

	*__ierr = MPI_Waitall((int)*count,lrequest,c_status);
	/* By checking for lrequest[i] = 0, we handle persistant requests */
	for (i=0; i<(int)*count; i++) {
#ifdef OLD_POINTER
	    if (lrequest[i] == MPI_REQUEST_NULL) {
		MPIR_RmPointer( (int)(lrequest[i]) );
		array_of_requests[i] = 0;
	    }
	    else
#endif
	        array_of_requests[i] = MPI_Request_c2f( lrequest[i] );
	}
    }
    else 
	*__ierr = MPI_Waitall((int)*count,(MPI_Request *)0, c_status );

    for (i=0; i<(int)*count; i++) 
	MPI_Status_c2f(&(c_status[i]), &(array_of_statuses[i][0]) );
    
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
        FREE( lrequest );
        FREE( c_status );
    }
}






