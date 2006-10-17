/* testsome.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TESTSOME = PMPI_TESTSOME
EXPORT_MPI_API void MPI_TESTSOME ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_testsome__ = pmpi_testsome__
EXPORT_MPI_API void mpi_testsome__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_testsome = pmpi_testsome
EXPORT_MPI_API void mpi_testsome ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#else
#pragma weak mpi_testsome_ = pmpi_testsome_
EXPORT_MPI_API void mpi_testsome_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *, MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TESTSOME ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TESTSOME")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_testsome__ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testsome__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_testsome ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testsome")));
#else
EXPORT_MPI_API void mpi_testsome_ ( MPI_Fint *, MPI_Fint [], MPI_Fint *,
	MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_testsome_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TESTSOME  MPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_testsome__  mpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_testsome  mpi_testsome
#else
#pragma _HP_SECONDARY_DEF pmpi_testsome_  mpi_testsome_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TESTSOME as PMPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_testsome__ as pmpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_testsome as pmpi_testsome
#else
#pragma _CRI duplicate mpi_testsome_ as pmpi_testsome_
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
#define mpi_testsome_ PMPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testsome_ pmpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testsome_ pmpi_testsome
#else
#define mpi_testsome_ pmpi_testsome_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_testsome_ MPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testsome_ mpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testsome_ mpi_testsome
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_testsome_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint *, 
                               MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE], 
			       MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_testsome_( MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], 
    MPI_Fint array_of_statuses[][MPI_STATUS_SIZE], MPI_Fint *__ierr )
{
    int i,j,found;
    int loutcount;
    int *l_indices = 0;
    int local_l_indices[MPIR_USE_LOCAL_ARRAY];
    MPI_Request *lrequest = 0;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status *c_status = 0;
    MPI_Status local_c_status[MPIR_USE_LOCAL_ARRAY];

    if ((int)*incount > 0) {
        if ((int)*incount > MPIR_USE_LOCAL_ARRAY) {
            MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request)* (int)*incount),
                        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
                        "MPI_TESTSOME");

	    MPIR_FALLOC(l_indices,(int*)MALLOC(sizeof(int)* (int)*incount),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		        "MPI_TESTSOME" );
	    MPIR_FALLOC(c_status,(MPI_Status*)MALLOC(sizeof(MPI_Status)* (int)*incount),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		        "MPI_TESTSOME" );
	}
        else {
	    lrequest = local_lrequest;
	    l_indices = local_l_indices;
	    c_status = local_c_status;
	}

	for (i=0; i<(int)*incount; i++) {
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}
	*__ierr = MPI_Testsome((int)*incount,lrequest,&loutcount,l_indices,
			       c_status);

	/* By checking for lrequest[l_indices[i] =  0, 
           we handle persistant requests */
	for (i=0; i<(int)*incount; i++) {
	    if ( i < loutcount ) {
#ifdef OLD_POINTER
	        if (l_indices[i] >= 0) {
		    if (lrequest[l_indices[i]] == 0) {
		        MPIR_RmPointer( (int)(lrequest[l_indices[i]]) );
		        array_of_requests[l_indices[i]] = 0;
		    }
		else
#endif
		    array_of_requests[l_indices[i]] =
			MPI_Request_c2f(lrequest[l_indices[i]] );
#ifdef OLD_POINTER
		}
#endif
	    }
	    else {
		found = 0;
		j = 0;
		while ( (!found) && (j<loutcount) ) {
		    if (l_indices[j++] == i)
			found = 1;
		}
		if (!found)
		    array_of_requests[i] = MPI_Request_c2f( lrequest[i] );
	    }
	}
    }
    else
	*__ierr = MPI_Testsome( (int)*incount, (MPI_Request *)0, &loutcount, 
				l_indices, c_status );

    for (i=0; i<loutcount; i++) {
        MPI_Status_c2f(&c_status[i], &(array_of_statuses[i][0]) );
	if (l_indices[i] >= 0)
	    array_of_indices[i] = l_indices[i] + 1;
    }
    *outcount = (MPI_Fint)loutcount;
    if ((int)*incount > MPIR_USE_LOCAL_ARRAY) {
        FREE( l_indices );
        FREE( lrequest );
        FREE( c_status );
    }

}


