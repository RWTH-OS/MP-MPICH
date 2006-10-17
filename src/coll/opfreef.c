/* opfree.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_OP_FREE = PMPI_OP_FREE
EXPORT_MPI_API void MPI_OP_FREE ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_op_free__ = pmpi_op_free__
EXPORT_MPI_API void mpi_op_free__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_op_free = pmpi_op_free
EXPORT_MPI_API void mpi_op_free ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_op_free_ = pmpi_op_free_
EXPORT_MPI_API void mpi_op_free_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_OP_FREE ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_OP_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_op_free__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_op_free ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_free")));
#else
EXPORT_MPI_API void mpi_op_free_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_OP_FREE  MPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_op_free__  mpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_op_free  mpi_op_free
#else
#pragma _HP_SECONDARY_DEF pmpi_op_free_  mpi_op_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_OP_FREE as PMPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_op_free__ as pmpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_op_free as pmpi_op_free
#else
#pragma _CRI duplicate mpi_op_free_ as pmpi_op_free_
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
#define mpi_op_free_ PMPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_free_ pmpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_free_ pmpi_op_free
#else
#define mpi_op_free_ pmpi_op_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_op_free_ MPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_free_ mpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_free_ mpi_op_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_op_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_op_free_( MPI_Fint *op, MPI_Fint *__ierr )
{
    MPI_Op l_op = MPI_Op_f2c(*op);
    *__ierr = MPI_Op_free(&l_op);
}
