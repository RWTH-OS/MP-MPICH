/* type_extent.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_EXTENT = PMPI_TYPE_EXTENT
EXPORT_MPI_API void MPI_TYPE_EXTENT ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_extent__ = pmpi_type_extent__
EXPORT_MPI_API void mpi_type_extent__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_extent = pmpi_type_extent
EXPORT_MPI_API void mpi_type_extent ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_extent_ = pmpi_type_extent_
EXPORT_MPI_API void mpi_type_extent_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_EXTENT ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_EXTENT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_extent__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_extent__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_extent ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_extent")));
#else
EXPORT_MPI_API void mpi_type_extent_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_extent_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_EXTENT  MPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_extent__  mpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_extent  mpi_type_extent
#else
#pragma _HP_SECONDARY_DEF pmpi_type_extent_  mpi_type_extent_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_EXTENT as PMPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_extent__ as pmpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_extent as pmpi_type_extent
#else
#pragma _CRI duplicate mpi_type_extent_ as pmpi_type_extent_
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
#define mpi_type_extent_ PMPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent
#else
#define mpi_type_extent_ pmpi_type_extent_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_extent_ MPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_extent_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_extent_( MPI_Fint *datatype, MPI_Fint *extent, MPI_Fint *__ierr )
{
    MPI_Aint c_extent;
    *__ierr = MPI_Type_extent(MPI_Type_f2c(*datatype), &c_extent);
    /* Really should check for truncation, ala mpi_address_ */
    *extent = (MPI_Fint)c_extent;
}
