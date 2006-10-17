/* type_ub.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_UB = PMPI_TYPE_UB
EXPORT_MPI_API void MPI_TYPE_UB ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_ub__ = pmpi_type_ub__
EXPORT_MPI_API void mpi_type_ub__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_ub = pmpi_type_ub
EXPORT_MPI_API void mpi_type_ub ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_ub_ = pmpi_type_ub_
EXPORT_MPI_API void mpi_type_ub_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_UB ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_UB")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_ub__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_ub__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_ub ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_ub")));
#else
EXPORT_MPI_API void mpi_type_ub_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_ub_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_UB  MPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_ub__  mpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_ub  mpi_type_ub
#else
#pragma _HP_SECONDARY_DEF pmpi_type_ub_  mpi_type_ub_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_UB as PMPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_ub__ as pmpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_ub as pmpi_type_ub
#else
#pragma _CRI duplicate mpi_type_ub_ as pmpi_type_ub_
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
#define mpi_type_ub_ PMPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub
#else
#define mpi_type_ub_ pmpi_type_ub_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_ub_ MPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_ub_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_ub_ ( MPI_Fint *datatype, MPI_Fint *displacement, MPI_Fint *__ierr )
{
    MPI_Aint c_displacement;

    *__ierr = MPI_Type_ub(MPI_Type_f2c(*datatype), &c_displacement);
    /* Should check for truncation */
    *displacement = (MPI_Fint)c_displacement;
}
