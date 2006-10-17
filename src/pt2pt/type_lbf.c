/* type_lb.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_LB = PMPI_TYPE_LB
EXPORT_MPI_API void MPI_TYPE_LB ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_lb__ = pmpi_type_lb__
EXPORT_MPI_API void mpi_type_lb__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_lb = pmpi_type_lb
EXPORT_MPI_API void mpi_type_lb ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_lb_ = pmpi_type_lb_
EXPORT_MPI_API void mpi_type_lb_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_LB ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_LB")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_lb__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_lb__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_lb ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_lb")));
#else
EXPORT_MPI_API void mpi_type_lb_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_lb_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_LB  MPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_lb__  mpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_lb  mpi_type_lb
#else
#pragma _HP_SECONDARY_DEF pmpi_type_lb_  mpi_type_lb_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_LB as PMPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_lb__ as pmpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_lb as pmpi_type_lb
#else
#pragma _CRI duplicate mpi_type_lb_ as pmpi_type_lb_
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
#define mpi_type_lb_ PMPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb
#else
#define mpi_type_lb_ pmpi_type_lb_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_lb_ MPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_lb_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_lb_ ( MPI_Fint *datatype, MPI_Fint *displacement, MPI_Fint *__ierr )
{
    MPI_Aint   c_displacement;
  
    *__ierr = MPI_Type_lb(MPI_Type_f2c(*datatype), &c_displacement);
    /* Should check for truncation */
    *displacement = (MPI_Fint)c_displacement;
}
