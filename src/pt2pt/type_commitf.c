/* type_commit.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_COMMIT = PMPI_TYPE_COMMIT
EXPORT_MPI_API void MPI_TYPE_COMMIT ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_commit__ = pmpi_type_commit__
EXPORT_MPI_API void mpi_type_commit__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_commit = pmpi_type_commit
EXPORT_MPI_API void mpi_type_commit ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_commit_ = pmpi_type_commit_
EXPORT_MPI_API void mpi_type_commit_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_COMMIT ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_COMMIT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_commit__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_commit__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_commit ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_commit")));
#else
EXPORT_MPI_API void mpi_type_commit_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_commit_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_COMMIT  MPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_commit__  mpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_commit  mpi_type_commit
#else
#pragma _HP_SECONDARY_DEF pmpi_type_commit_  mpi_type_commit_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_COMMIT as PMPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_commit__ as pmpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_commit as pmpi_type_commit
#else
#pragma _CRI duplicate mpi_type_commit_ as pmpi_type_commit_
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
#define mpi_type_commit_ PMPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit
#else
#define mpi_type_commit_ pmpi_type_commit_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_commit_ MPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_commit_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_commit_ ( MPI_Fint *datatype, MPI_Fint *__ierr )
{
    MPI_Datatype ldatatype = MPI_Type_f2c(*datatype);
    *__ierr = MPI_Type_commit( &ldatatype );
    *datatype = MPI_Type_c2f(ldatatype);    
}
