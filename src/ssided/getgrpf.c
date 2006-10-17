/* $Id: getgrpf.c,v 1.2 2003/06/13 17:24:24 rainer Exp $ */

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
#pragma weak MPI_WIN_GET_GROUP = PMPI_WIN_GET_GROUP
EXPORT_MPI_API void MPI_WIN_GET_GROUP ( MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_win_get_group__ = pmpi_win_get_group__
EXPORT_MPI_API void mpi_win_get_group__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_win_get_group = pmpi_win_get_group
EXPORT_MPI_API void mpi_win_get_group ( MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_win_get_group_ = pmpi_win_get_group_
EXPORT_MPI_API void mpi_win_get_group_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_WIN_GET_GROUP ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_WIN_GET_GROUP")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_win_get_group__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_group__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_win_get_group ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_group")));
#else
EXPORT_MPI_API void mpi_win_get_group_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_win_get_group_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_WIN_GET_GROUP  MPI_WIN_GET_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_get_group__  mpi_win_get_group__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_win_get_group  mpi_win_get_group
#else
#pragma _HP_SECONDARY_DEF pmpi_win_get_group_  mpi_win_get_group_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_WIN_GET_GROUP as PMPI_WIN_GET_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_win_get_group__ as pmpi_win_get_group__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_win_get_group as pmpi_win_get_group
#else
#pragma _CRI duplicate mpi_win_get_group_ as pmpi_win_get_group_
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
#define mpi_win_get_group_ PMPI_WIN_GET_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_get_group_ pmpi_win_get_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_get_group_ pmpi_win_get_group
#else
#define mpi_win_get_group_ pmpi_win_get_group_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_win_get_group_ MPI_WIN_GET_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_win_get_group_ mpi_win_get_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_win_get_group_ mpi_win_get_group
#endif
#endif


EXPORT_MPI_API
void mpi_win_get_group_ (MPI_Fint *win, MPI_Fint *group, MPI_Fint *__ierr)
{
	*__ierr = MPI_Win_get_group((MPI_Win)*win, (MPI_Group *)group);
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
