/* attr_delval.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ATTR_DELETE = PMPI_ATTR_DELETE
EXPORT_MPI_API void MPI_ATTR_DELETE ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_attr_delete__ = pmpi_attr_delete__
EXPORT_MPI_API void mpi_attr_delete__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_attr_delete = pmpi_attr_delete
EXPORT_MPI_API void mpi_attr_delete ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_attr_delete_ = pmpi_attr_delete_
EXPORT_MPI_API void mpi_attr_delete_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ATTR_DELETE ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ATTR_DELETE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_attr_delete__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_delete__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_attr_delete ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_delete")));
#else
EXPORT_MPI_API void mpi_attr_delete_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_attr_delete_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ATTR_DELETE  MPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_attr_delete__  mpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_attr_delete  mpi_attr_delete
#else
#pragma _HP_SECONDARY_DEF pmpi_attr_delete_  mpi_attr_delete_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ATTR_DELETE as PMPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_attr_delete__ as pmpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_attr_delete as pmpi_attr_delete
#else
#pragma _CRI duplicate mpi_attr_delete_ as pmpi_attr_delete_
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
#define mpi_attr_delete_ PMPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete
#else
#define mpi_attr_delete_ pmpi_attr_delete_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_attr_delete_ MPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_attr_delete_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_attr_delete_ ( MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *__ierr )
{
    *__ierr = MPI_Attr_delete( MPI_Comm_f2c(*comm), (int)*keyval);
}
