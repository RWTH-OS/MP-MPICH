/* buffree.c */
/* Custom Fortran interface file */

/* Note that the calling args are different in Fortran and C */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_BUFFER_DETACH = PMPI_BUFFER_DETACH
EXPORT_MPI_API void MPI_BUFFER_DETACH ( void **, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_buffer_detach__ = pmpi_buffer_detach__
EXPORT_MPI_API void mpi_buffer_detach__ ( void **, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_buffer_detach = pmpi_buffer_detach
EXPORT_MPI_API void mpi_buffer_detach ( void **, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_buffer_detach_ = pmpi_buffer_detach_
EXPORT_MPI_API void mpi_buffer_detach_ ( void **, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_BUFFER_DETACH ( void **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_BUFFER_DETACH")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_buffer_detach__ ( void **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_buffer_detach__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_buffer_detach ( void **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_buffer_detach (")));
#else
EXPORT_MPI_API void mpi_buffer_detach_ ( void **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_buffer_detach_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_BUFFER_DETACH  MPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_buffer_detach__  mpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_buffer_detach  mpi_buffer_detach
#else
#pragma _HP_SECONDARY_DEF pmpi_buffer_detach_  mpi_buffer_detach_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_BUFFER_DETACH as PMPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_buffer_detach__ as pmpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_buffer_detach as pmpi_buffer_detach
#else
#pragma _CRI duplicate mpi_buffer_detach_ as pmpi_buffer_detach_
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
#define mpi_buffer_detach_ PMPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_detach_ pmpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_detach_ pmpi_buffer_detach
#else
#define mpi_buffer_detach_ pmpi_buffer_detach_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_buffer_detach_ MPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_detach_ mpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_detach_ mpi_buffer_detach
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_buffer_detach_ ANSI_ARGS(( void **, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_buffer_detach_( void **buffer, MPI_Fint *size, MPI_Fint *__ierr )
{
  void *tmp = (void *)buffer;
  int lsize;

  *__ierr = MPI_Buffer_detach(&tmp,&lsize);
  *size = (MPI_Fint)lsize;
}
