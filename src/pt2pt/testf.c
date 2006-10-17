/* test.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TEST = PMPI_TEST
EXPORT_MPI_API void MPI_TEST ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_test__ = pmpi_test__
EXPORT_MPI_API void mpi_test__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_test = pmpi_test
EXPORT_MPI_API void mpi_test ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_test_ = pmpi_test_
EXPORT_MPI_API void mpi_test_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TEST ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TEST")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_test__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_test ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test")));
#else
EXPORT_MPI_API void mpi_test_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TEST  MPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_test__  mpi_test__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_test  mpi_test
#else
#pragma _HP_SECONDARY_DEF pmpi_test_  mpi_test_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TEST as PMPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_test__ as pmpi_test__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_test as pmpi_test
#else
#pragma _CRI duplicate mpi_test_ as pmpi_test_
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
#define mpi_test_ PMPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_ pmpi_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_ pmpi_test
#else
#define mpi_test_ pmpi_test_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_test_ MPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_ mpi_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_ mpi_test
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_test_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_test_ ( MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *__ierr )
{
    int        l_flag;
    MPI_Status c_status;
    MPI_Request lrequest = MPI_Request_f2c(*request);

    *__ierr = MPI_Test( &lrequest, &l_flag, &c_status);

#ifdef OLD_POINTER
    if (lrequest == MPI_REQUEST_NULL) {
	MPIR_RmPointer((int)lrequest);
	*request = 0;
    }
    else
#endif
        *request = MPI_Request_c2f(lrequest);

    *flag = MPIR_TO_FLOG(l_flag);
    if (l_flag) 
	MPI_Status_c2f(&c_status, status);
}
