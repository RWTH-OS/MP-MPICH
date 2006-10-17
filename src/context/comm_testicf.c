/* comm_test_ic.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_TEST_INTER = PMPI_COMM_TEST_INTER
EXPORT_MPI_API void MPI_COMM_TEST_INTER ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_test_inter__ = pmpi_comm_test_inter__
EXPORT_MPI_API void mpi_comm_test_inter__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_test_inter = pmpi_comm_test_inter
EXPORT_MPI_API void mpi_comm_test_inter ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_comm_test_inter_ = pmpi_comm_test_inter_
EXPORT_MPI_API void mpi_comm_test_inter_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_TEST_INTER ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_TEST_INTER")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_test_inter__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_test_inter__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_test_inter ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_test_inter")));
#else
EXPORT_MPI_API void mpi_comm_test_inter_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_test_inter_")));
#endif


#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_TEST_INTER  MPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_test_inter__  mpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_test_inter  mpi_comm_test_inter
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_test_inter_  mpi_comm_test_inter_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_TEST_INTER as PMPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_test_inter__ as pmpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_test_inter as pmpi_comm_test_inter
#else
#pragma _CRI duplicate mpi_comm_test_inter_ as pmpi_comm_test_inter_
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
#define mpi_comm_test_inter_ PMPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_test_inter_ pmpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_test_inter_ pmpi_comm_test_inter
#else
#define mpi_comm_test_inter_ pmpi_comm_test_inter_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_test_inter_ MPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_test_inter_ mpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_test_inter_ mpi_comm_test_inter
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_test_inter_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_test_inter_ ( MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *__ierr )
{
    int l_flag;
    *__ierr = MPI_Comm_test_inter( MPI_Comm_f2c(*comm), &l_flag);
    *flag = MPIR_TO_FLOG(l_flag);
}
