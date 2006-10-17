/* initialize.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INITIALIZED = PMPI_INITIALIZED
EXPORT_MPI_API void MPI_INITIALIZED ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_initialized__ = pmpi_initialized__
EXPORT_MPI_API void mpi_initialized__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_initialized = pmpi_initialized
EXPORT_MPI_API void mpi_initialized ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_initialized_ = pmpi_initialized_
EXPORT_MPI_API void mpi_initialized_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INITIALIZED ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_INITIALIZED")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_initialized__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_initialized__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_initialized ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_initialized")));
#else
EXPORT_MPI_API void mpi_initialized_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_initialized_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INITIALIZED  MPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_initialized__  mpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_initialized  mpi_initialized
#else
#pragma _HP_SECONDARY_DEF pmpi_initialized_  mpi_initialized_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INITIALIZED as PMPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_initialized__ as pmpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_initialized as pmpi_initialized
#else
#pragma _CRI duplicate mpi_initialized_ as pmpi_initialized_
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
#define mpi_initialized_ PMPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ pmpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ pmpi_initialized
#else
#define mpi_initialized_ pmpi_initialized_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_initialized_ MPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ mpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ mpi_initialized
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_initialized_ ( MPI_Fint *, MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_initialized_( MPI_Fint *flag, MPI_Fint *__ierr )
{
    int lflag;
    *__ierr = MPI_Initialized(&lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
