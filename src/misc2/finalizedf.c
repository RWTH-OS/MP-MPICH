/* finalized.c */
/* Custom Fortran interface file */

#include "mpiimpl.h"
#include "mpifort.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FINALIZED = PMPI_FINALIZED
EXPORT_MPI_API void MPI_FINALIZED ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_finalized__ = pmpi_finalized__
EXPORT_MPI_API void mpi_finalized__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_finalized = pmpi_finalized
EXPORT_MPI_API void mpi_finalized ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_finalized_ = pmpi_finalized_
EXPORT_MPI_API void mpi_finalized_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_FINALIZED ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_FINALIZED")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_finalized__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_finalized__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_finalized ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_finalized")));
#else
EXPORT_MPI_API void mpi_finalized_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_finalized_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FINALIZED  MPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_finalized__  mpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_finalized  mpi_finalized
#else
#pragma _HP_SECONDARY_DEF pmpi_finalized_  mpi_finalized_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FINALIZED as PMPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_finalized__ as pmpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_finalized as pmpi_finalized
#else
#pragma _CRI duplicate mpi_finalized_ as pmpi_finalized_
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
#define mpi_finalized_ PMPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalized_ pmpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalized_ pmpi_finalized
#else
#define mpi_finalized_ pmpi_finalized_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_finalized_ MPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalized_ mpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalized_ mpi_finalized
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_finalized_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_finalized_( MPI_Fint *flag, MPI_Fint *__ierr )
{
    int lflag;
    *__ierr = MPI_Finalized(&lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
