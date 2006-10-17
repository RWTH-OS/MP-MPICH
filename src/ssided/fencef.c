/* $Id: fencef.c,v 1.2 2003/06/13 17:55:51 rainer Exp $

/* universal fortran binding */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WIN_FENCE = PMPI_WIN_FENCE
EXPORT_MPI_API void MPI_WIN_FENCE (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_win_fence__ = pmpi_win_fence__
EXPORT_MPI_API void mpi_win_fence__ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_win_fence = pmpi_win_fence
EXPORT_MPI_API void mpi_win_fence (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_win_fence_ = pmpi_win_fence_
EXPORT_MPI_API void mpi_win_fence_ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WIN_FENCE (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_WIN_FENCE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_win_fence__ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_fence__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_win_fence (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_fence")));
#else
EXPORT_MPI_API void mpi_win_fence_ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_fence_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WIN_FENCE  MPI_WIN_FENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_fence__  mpi_win_fence__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_fence  mpi_win_fence
#else
#pragma _HP_SECONDARY_DEF pmpi_win_fence_  mpi_win_fence_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WIN_FENCE as PMPI_WIN_FENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_win_fence__ as pmpi_win_fence__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_win_fence as pmpi_win_fence
#else
#pragma _CRI duplicate mpi_win_fence_ as pmpi_win_fence_
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
#define mpi_win_fence_ PMPI_WIN_FENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_fence_ pmpi_win_fence__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_fence_ pmpi_win_fence
#else
#define mpi_win_fence_ pmpi_win_fence_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_win_fence_ MPI_WIN_FENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_fence_ mpi_win_fence__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_fence_ mpi_win_fence
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_win_fence_( MPI_Fint *assert, MPI_Fint *win, MPI_Fint *__ierr);

EXPORT_MPI_API void FORTRAN_API mpi_win_fence_ (MPI_Fint *assert, MPI_Fint *win, MPI_Fint *__ierr)
{
	*__ierr = MPI_Win_fence ((int)*assert, MPI_Win_f2c(*win));
}


















/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
