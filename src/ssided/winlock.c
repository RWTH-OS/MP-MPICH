/* $Id: winlock.c,v 1.6 2003/06/13 17:55:51 rainer Exp $
 *
 * MPI_Win_lock		starts an RMA access epoch
 *
 * input parameters:
 *	lock_type	either 	MPI_LOCK_EXCLUSIVE or
 *						MPI_LOCK_SHARED
 *	rank		rank of locked window (pos. integer)
 *	assert		program assertion (integer)
 *	win			window object (handle)
 *
 * output parameters:
 *	<none>
 */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_lock = PMPI_Win_lock
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_lock (int lock_type, int rank, int assert, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Win_lock")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_lock  MPI_Win_lock
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_lock as PMPI_Win_lock
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


EXPORT_MPI_API
int 
MPI_Win_lock (lock_type, rank, assert, win)
	int lock_type;
	int rank;
	int assert;
	MPI_Win win;
{
	MPID_Win	* dwin;
	int 		num_procs;
	int 		error_code = MPI_SUCCESS;
	static char	myname[] = "MPI_WIN_LOCK";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		 MPIR_WIN_NULL_RETURN (myname);

	num_procs = dwin->comm->np;

	if (rank < 0 || rank >= num_procs) {
		error_code = MPI_ERR_RANK;
	} else if (lock_type != MPI_LOCK_EXCLUSIVE 
							&& lock_type != MPI_LOCK_SHARED) {
		error_code = MPI_ERR_LOCKTYPE;
	} else if (!MPID_Win_lock (dwin, rank, lock_type))  {
		error_code = MPI_ERR_RMA_SYNC;
	}

	MPIR_WIN_RETURN (dwin, error_code, myname);
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
