/* errget.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ERRHANDLER_GET = PMPI_ERRHANDLER_GET
EXPORT_MPI_API void MPI_ERRHANDLER_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_errhandler_get__ = pmpi_errhandler_get__
EXPORT_MPI_API void mpi_errhandler_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_errhandler_get = pmpi_errhandler_get
EXPORT_MPI_API void mpi_errhandler_get ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_errhandler_get_ = pmpi_errhandler_get_
EXPORT_MPI_API void mpi_errhandler_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ERRHANDLER_GET ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ERRHANDLER_GET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_errhandler_get__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_get__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_errhandler_get ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_get")));
#else
EXPORT_MPI_API void mpi_errhandler_get_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_get_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ERRHANDLER_GET  MPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_get__  mpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_get  mpi_errhandler_get
#else
#pragma _HP_SECONDARY_DEF pmpi_errhandler_get_  mpi_errhandler_get_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ERRHANDLER_GET as PMPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_get__ as pmpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_get as pmpi_errhandler_get
#else
#pragma _CRI duplicate mpi_errhandler_get_ as pmpi_errhandler_get_
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
#define mpi_errhandler_get_ PMPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get
#else
#define mpi_errhandler_get_ pmpi_errhandler_get_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_errhandler_get_ MPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_errhandler_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                     MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_errhandler_get_( MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *__ierr )
{
    MPI_Errhandler l_errhandler;
    *__ierr = MPI_Errhandler_get( MPI_Comm_f2c(*comm), &l_errhandler );
    *errhandler = MPI_Errhandler_c2f(l_errhandler);
}
