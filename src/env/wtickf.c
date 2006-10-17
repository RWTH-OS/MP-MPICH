/* wtick.c */
/* Custom Fortran interface file*/
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WTICK = PMPI_WTICK
EXPORT_MPI_API double MPI_WTICK ( void );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_wtick__ = pmpi_wtick__
EXPORT_MPI_API double mpi_wtick__ ( void );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_wtick = pmpi_wtick
EXPORT_MPI_API double mpi_wtick ( void );
#else
#pragma weak mpi_wtick_ = pmpi_wtick_
EXPORT_MPI_API double mpi_wtick_ ( void );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API double MPI_WTICK ( void ) __attribute__ ((weak, alias ("PMPI_WTICK")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API double mpi_wtick__ ( void ) __attribute__ ((weak, alias ("pmpi_wtick__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API double mpi_wtick ( void ) __attribute__ ((weak, alias ("pmpi_wtick")));
#else
EXPORT_MPI_API double mpi_wtick_ ( void ) __attribute__ ((weak, alias ("pmpi_wtick_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WTICK  MPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_wtick__  mpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_wtick  mpi_wtick
#else
#pragma _HP_SECONDARY_DEF pmpi_wtick_  mpi_wtick_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WTICK as PMPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_wtick__ as pmpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_wtick as pmpi_wtick
#else
#pragma _CRI duplicate mpi_wtick_ as pmpi_wtick_
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
#define mpi_wtick_ PMPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtick_ pmpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtick_ pmpi_wtick
#else
#define mpi_wtick_ pmpi_wtick_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_wtick_ MPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtick_ mpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtick_ mpi_wtick
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API double FORTRAN_API mpi_wtick_ ANSI_ARGS(( void ));

EXPORT_MPI_API double  FORTRAN_API mpi_wtick_()
{
    return MPI_Wtick();
}



