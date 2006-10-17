/* cartdim_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_CARTDIM_GET = PMPI_CARTDIM_GET
EXPORT_MPI_API void MPI_CARTDIM_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_cartdim_get__ = pmpi_cartdim_get__
EXPORT_MPI_API void mpi_cartdim_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_cartdim_get = pmpi_cartdim_get
EXPORT_MPI_API void mpi_cartdim_get ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_cartdim_get_ = pmpi_cartdim_get_
EXPORT_MPI_API void mpi_cartdim_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_CARTDIM_GET ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_CARTDIM_GET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_cartdim_get__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cartdim_get__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_cartdim_get ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cartdim_get")));
#else
EXPORT_MPI_API void mpi_cartdim_get_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cartdim_get_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_CARTDIM_GET  MPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cartdim_get__  mpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cartdim_get  mpi_cartdim_get
#else
#pragma _HP_SECONDARY_DEF pmpi_cartdim_get_  mpi_cartdim_get_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_CARTDIM_GET as PMPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_cartdim_get__ as pmpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_cartdim_get as pmpi_cartdim_get
#else
#pragma _CRI duplicate mpi_cartdim_get_ as pmpi_cartdim_get_
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
#define mpi_cartdim_get_ PMPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get
#else
#define mpi_cartdim_get_ pmpi_cartdim_get_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_cartdim_get_ MPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_cartdim_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_cartdim_get_ ( MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *__ierr )
{
    int lndims;

    *__ierr = MPI_Cartdim_get( MPI_Comm_f2c(*comm), &lndims );
    *ndims = (MPI_Fint)lndims;
}
