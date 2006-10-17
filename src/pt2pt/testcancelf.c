/* testcancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TEST_CANCELLED = PMPI_TEST_CANCELLED
EXPORT_MPI_API void MPI_TEST_CANCELLED ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_test_cancelled__ = pmpi_test_cancelled__
EXPORT_MPI_API void mpi_test_cancelled__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_test_cancelled = pmpi_test_cancelled
EXPORT_MPI_API void mpi_test_cancelled ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_test_cancelled_ = pmpi_test_cancelled_
EXPORT_MPI_API void mpi_test_cancelled_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TEST_CANCELLED ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TEST_CANCELLED")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_test_cancelled__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test_cancelled__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_test_cancelled ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test_cancelled")));
#else
EXPORT_MPI_API void mpi_test_cancelled_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_test_cancelled_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TEST_CANCELLED  MPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_test_cancelled__  mpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_test_cancelled  mpi_test_cancelled
#else
#pragma _HP_SECONDARY_DEF pmpi_test_cancelled_  mpi_test_cancelled_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TEST_CANCELLED as PMPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_test_cancelled__ as pmpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_test_cancelled as pmpi_test_cancelled
#else
#pragma _CRI duplicate mpi_test_cancelled_ as pmpi_test_cancelled_
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
#define mpi_test_cancelled_ PMPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_cancelled_ pmpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_cancelled_ pmpi_test_cancelled
#else
#define mpi_test_cancelled_ pmpi_test_cancelled_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_test_cancelled_ MPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_cancelled_ mpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_cancelled_ mpi_test_cancelled
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_test_cancelled_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_test_cancelled_(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *__ierr)
{
    int lflag;
    MPI_Status c_status;

    MPI_Status_f2c(status, &c_status); 
    *__ierr = MPI_Test_cancelled(&c_status, &lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
