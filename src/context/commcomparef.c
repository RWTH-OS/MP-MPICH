/* comm_compare.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_COMPARE = PMPI_COMM_COMPARE
EXPORT_MPI_API void MPI_COMM_COMPARE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_compare__ = pmpi_comm_compare__
EXPORT_MPI_API void mpi_comm_compare__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_compare = pmpi_comm_compare
EXPORT_MPI_API void mpi_comm_compare ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_comm_compare_ = pmpi_comm_compare_
EXPORT_MPI_API void mpi_comm_compare_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_COMPARE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_COMPARE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_compare__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_compare__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_compare ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_compare")));
#else
EXPORT_MPI_API void mpi_comm_compare_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_compare_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_COMPARE  MPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_compare__  mpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_compare  mpi_comm_compare
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_compare_  mpi_comm_compare_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_COMPARE as PMPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_compare__ as pmpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_compare as pmpi_comm_compare
#else
#pragma _CRI duplicate mpi_comm_compare_ as pmpi_comm_compare_
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
#define mpi_comm_compare_ PMPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_compare_ pmpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_compare_ pmpi_comm_compare
#else
#define mpi_comm_compare_ pmpi_comm_compare_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_compare_ MPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_compare_ mpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_compare_ mpi_comm_compare
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_compare_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_compare_ ( MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *__ierr )
{
    int l_result;

    *__ierr = MPI_Comm_compare( MPI_Comm_f2c(*comm1), 
                                MPI_Comm_f2c(*comm2), &l_result);
    *result = l_result;
}
