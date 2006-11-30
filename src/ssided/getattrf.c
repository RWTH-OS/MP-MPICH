/* $Id$ */

/* universal fortran binding */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_WIN_GET_ATTR = PMPI_WIN_GET_ATTR
EXPORT_MPI_API void MPI_WIN_GET_ATTR (MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_win_get_attr__ = pmpi_win_get_attr__
EXPORT_MPI_API void mpi_win_get_attr__ (MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_win_get_attr = pmpi_win_get_attr
EXPORT_MPI_API void mpi_win_get_attr (MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_win_get_attr_ = pmpi_win_get_attr_
EXPORT_MPI_API void mpi_win_get_attr_ (MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WIN_GET_ATTR (MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("PMPI_WIN_GET_ATTR")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_win_get_attr__ (MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_attr__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_win_get_attr (MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_attr")));
#else
EXPORT_MPI_API void mpi_win_get_attr_ (MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_attr_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WIN_GET_ATTR  MPI_WIN_GET_ATTR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_get_attr__  mpi_win_get_attr__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_get_attr  mpi_win_get_attr
#else
#pragma _HP_SECONDARY_DEF pmpi_win_get_attr_  mpi_win_get_attr_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WIN_GET_ATTR as PMPI_WIN_GET_ATTR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_win_get_attr__ as pmpi_win_get_attr__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_win_get_attr as pmpi_win_get_attr
#else
#pragma _CRI duplicate mpi_win_get_attr_ as pmpi_win_get_attr_
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
#define mpi_win_get_attr_ PMPI_WIN_GET_ATTR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_get_attr_ pmpi_win_get_attr__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_get_attr_ pmpi_win_get_attr
#else
#define mpi_win_get_attr_ pmpi_win_get_attr_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_win_get_attr_ MPI_WIN_GET_ATTR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_get_attr_ mpi_win_get_attr__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_get_attr_ mpi_win_get_attr
#endif
#endif


EXPORT_MPI_API
void mpi_win_get_attr_ (win, win_keyval, attribute_val, flag, __ierr) 
	MPI_Fint *win;
	MPI_Fint *win_keyval; 
	void 	 *attribute_val;
	MPI_Fint *flag;
	MPI_Fint *__ierr;
{
	*__ierr = MPI_Win_get_attr ((MPI_Win)*win, (int)*win_keyval, MPIR_F_PTR(attribute_val);
								(int *)flag);
	return;
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
