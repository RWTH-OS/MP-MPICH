/* $Id$
 *
 * MPI_Win_fence	collective synchronization call
 *
 * input parameters:
 *	assert	program assertions (integer)
 *	win		window object to synch (handle)
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
#pragma weak MPI_Win_fence = PMPI_Win_fence
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_fence (int assert, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Win_fence")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_fence  MPI_Win_fence
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_fence as PMPI_Win_fence
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
int MPI_Win_fence (assert, win)
	int 	assert;
	MPI_Win win;
{
	MPID_Win	* dwin;
	int			error_code = MPI_SUCCESS;
	static char	myname[] = "MPI_WIN_FENCE";
	
	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin)) 
		MPIR_WIN_NULL_RETURN (myname);

	/* ok, do the sync */
	if (!MPID_Win_sync (MPID_WIN_SYNC_FENCE, dwin, NULL, assert)) {
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
