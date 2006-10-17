/* startall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_STARTALL = PMPI_STARTALL
EXPORT_MPI_API void MPI_STARTALL ( MPI_Fint *, MPI_Fint [], MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_startall__ = pmpi_startall__
EXPORT_MPI_API void mpi_startall__ ( MPI_Fint *, MPI_Fint [], MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_startall = pmpi_startall
EXPORT_MPI_API void mpi_startall ( MPI_Fint *, MPI_Fint [], MPI_Fint * );
#else
#pragma weak mpi_startall_ = pmpi_startall_
EXPORT_MPI_API void mpi_startall_ ( MPI_Fint *, MPI_Fint [], MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_STARTALL ( MPI_Fint *, MPI_Fint [],
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_STARTALL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_startall__ ( MPI_Fint *, MPI_Fint [],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_startall__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_startall ( MPI_Fint *, MPI_Fint [],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_startall")));
#else
EXPORT_MPI_API void mpi_startall_ ( MPI_Fint *, MPI_Fint [],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_startall_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_STARTALL  MPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_startall__  mpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_startall  mpi_startall
#else
#pragma _HP_SECONDARY_DEF pmpi_startall_  mpi_startall_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_STARTALL as PMPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_startall__ as pmpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_startall as pmpi_startall
#else
#pragma _CRI duplicate mpi_startall_ as pmpi_startall_
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
#define mpi_startall_ PMPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ pmpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ pmpi_startall
#else
#define mpi_startall_ pmpi_startall_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_startall_ MPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ mpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ mpi_startall
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_startall_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_startall_( MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *__ierr )
{ 
   MPI_Request *lrequest = 0;
   MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
   int i;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request) * (int)*count),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_STARTALL" );
	}
	else {
	    lrequest = local_lrequest;
	}
	for (i=0; i<(int)*count; i++) {
            lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}
	*__ierr = MPI_Startall((int)*count,lrequest);
    }
    else 
	*__ierr = MPI_Startall((int)*count,(MPI_Request *)0);

    for (i=0; i<(int)*count; i++) {
        array_of_requests[i] = MPI_Request_c2f( lrequest[i]);
    }
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	FREE( lrequest );
    }
}


