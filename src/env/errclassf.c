/* errclass.c */
/* Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ERROR_CLASS = PMPI_ERROR_CLASS
EXPORT_MPI_API void MPI_ERROR_CLASS ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_error_class__ = pmpi_error_class__
EXPORT_MPI_API void mpi_error_class__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_error_class = pmpi_error_class
EXPORT_MPI_API void mpi_error_class ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_error_class_ = pmpi_error_class_
EXPORT_MPI_API void mpi_error_class_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ERROR_CLASS ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ERROR_CLASS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_error_class__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_error_class__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_error_class ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_error_class")));
#else
EXPORT_MPI_API void mpi_error_class_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_error_class_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ERROR_CLASS  MPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_error_class__  mpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_error_class  mpi_error_class
#else
#pragma _HP_SECONDARY_DEF pmpi_error_class_  mpi_error_class_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ERROR_CLASS as PMPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_error_class__ as pmpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_error_class as pmpi_error_class
#else
#pragma _CRI duplicate mpi_error_class_ as pmpi_error_class_
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
#define mpi_error_class_ PMPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_class_ pmpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_class_ pmpi_error_class
#else
#define mpi_error_class_ pmpi_error_class_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_error_class_ MPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_class_ mpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_class_ mpi_error_class
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_error_class_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_error_class_( MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *__ierr )
{
    int l_errorclass;

    *__ierr = MPI_Error_class((int)*errorcode, &l_errorclass);
    *errorclass = l_errorclass;
}
