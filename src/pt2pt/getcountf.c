/* getcount.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GET_COUNT = PMPI_GET_COUNT
EXPORT_MPI_API void MPI_GET_COUNT ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_get_count__ = pmpi_get_count__
EXPORT_MPI_API void mpi_get_count__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_get_count = pmpi_get_count
EXPORT_MPI_API void mpi_get_count ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_get_count_ = pmpi_get_count_
EXPORT_MPI_API void mpi_get_count_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GET_COUNT ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GET_COUNT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_get_count__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_count__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_get_count ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_count")));
#else
EXPORT_MPI_API void mpi_get_count_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_count_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GET_COUNT  MPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_count__  mpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_count  mpi_get_count
#else
#pragma _HP_SECONDARY_DEF pmpi_get_count_  mpi_get_count_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GET_COUNT as PMPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_get_count__ as pmpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_get_count as pmpi_get_count
#else
#pragma _CRI duplicate mpi_get_count_ as pmpi_get_count_
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
#define mpi_get_count_ PMPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ pmpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ pmpi_get_count
#else
#define mpi_get_count_ pmpi_get_count_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_get_count_ MPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ mpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ mpi_get_count
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_get_count_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_get_count_( MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *__ierr )
{
    int lcount;
    MPI_Status c_status;

    MPI_Status_f2c(status, &c_status); 
    *__ierr = MPI_Get_count(&c_status, MPI_Type_f2c(*datatype), 
                            &lcount);
    *count = (MPI_Fint)lcount;

}

