/* pcontrol.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_PCONTROL = PMPI_PCONTROL
EXPORT_MPI_API void MPI_PCONTROL ( MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_pcontrol__ = pmpi_pcontrol__
EXPORT_MPI_API void mpi_pcontrol__ ( MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_pcontrol = pmpi_pcontrol
EXPORT_MPI_API void mpi_pcontrol ( MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_pcontrol_ = pmpi_pcontrol_
EXPORT_MPI_API void mpi_pcontrol_ ( MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_PCONTROL ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_PCONTROL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_pcontrol__ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pcontrol__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_pcontrol ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pcontrol")));
#else
EXPORT_MPI_API void mpi_pcontrol_ ( MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pcontrol_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_PCONTROL  MPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_pcontrol__  mpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_pcontrol  mpi_pcontrol
#else
#pragma _HP_SECONDARY_DEF pmpi_pcontrol_  mpi_pcontrol_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_PCONTROL as PMPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_pcontrol__ as pmpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_pcontrol as pmpi_pcontrol
#else
#pragma _CRI duplicate mpi_pcontrol_ as pmpi_pcontrol_
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
#define mpi_pcontrol_ PMPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol
#else
#define mpi_pcontrol_ pmpi_pcontrol_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_pcontrol_ MPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_pcontrol_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_pcontrol_( MPI_Fint *level, MPI_Fint *__ierr )
{
    *__ierr = MPI_Pcontrol((int)*level);
}
