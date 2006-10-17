/* waitany.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WAITANY = PMPI_WAITANY
EXPORT_MPI_API void MPI_WAITANY ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_waitany__ = pmpi_waitany__
EXPORT_MPI_API void mpi_waitany__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_waitany = pmpi_waitany
EXPORT_MPI_API void mpi_waitany ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_waitany_ = pmpi_waitany_
EXPORT_MPI_API void mpi_waitany_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WAITANY ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_WAITANY")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_waitany__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_waitany__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_waitany ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_waitany")));
#else
EXPORT_MPI_API void mpi_waitany_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_waitany_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WAITANY  MPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_waitany__  mpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_waitany  mpi_waitany
#else
#pragma _HP_SECONDARY_DEF pmpi_waitany_  mpi_waitany_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WAITANY as PMPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_waitany__ as pmpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_waitany as pmpi_waitany
#else
#pragma _CRI duplicate mpi_waitany_ as pmpi_waitany_
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
#define mpi_waitany_ PMPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitany_ pmpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitany_ pmpi_waitany
#else
#define mpi_waitany_ pmpi_waitany_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_waitany_ MPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitany_ mpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitany_ mpi_waitany
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_waitany_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint *, 
                              MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_waitany_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *__ierr )
{

    int lindex;
    MPI_Request *lrequest;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status c_status;
    int i;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request) * (int)*count),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITANY" );
	}
	else 
	    lrequest = local_lrequest;
	
	for (i=0; i<(int)*count; i++) 
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
    }
    else
	lrequest = 0;

    *__ierr = MPI_Waitany((int)*count,lrequest,&lindex,&c_status);

    if (lindex != -1) {
	if (!*__ierr) {
#ifdef OLD_POINTER
	    /* By checking for r[i] = 0, we handle persistant requests */
	    if (lrequest[lindex] == MPI_REQUEST_NULL) {
		MPIR_RmPointer( (int)(lrequest[lindex]) );
		array_of_requests[lindex] = 0;
	    }
	    else  
#endif
                array_of_requests[lindex] = MPI_Request_c2f(lrequest[lindex]);
	}
    }

   if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	FREE( lrequest );
    }

    /* See the description of waitany in the standard; the Fortran index ranges
       are from 1, not zero */
    *index = (MPI_Fint)lindex;
    if ((int)*index >= 0) *index = (MPI_Fint)*index + 1;
    MPI_Status_c2f(&c_status, status);
}

