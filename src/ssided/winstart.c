/* $Id: winstart.c,v 1.6 2003/07/03 21:42:28 joachim Exp $
 *
 * MPI_Win_start	Starts an RMA access epoch for win
 *
 * input parameters:
 *	group	group of target process (handle)
 *	assert	program assertion (integer)
 *	win		window object (handle)
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
#pragma weak MPI_Win_start = PMPI_Win_start
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_start (MPI_Group group, int assert, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Win_start")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_start  MPI_Win_start
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_start as PMPI_Win_start
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
MPI_Win_start (group, assert, win)
	MPI_Group 	group;
	int 		assert;
	MPI_Win 	win;
{
	MPID_Win	* dwin;
	MPID_Group	* dgroup;
	int			error_code = MPI_SUCCESS;
	static char	myname[] = "MPI_WIN_START";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);
	dgroup = MPID_GET_GROUP_PTR (group);
	if (MPID_TEST_GROUP_NOTOK (group, dgroup))
		MPIR_WIN_RETURN (dwin, MPI_ERR_GROUP, myname);

	/* do the sync */
	if (!MPID_Win_sync (MPID_WIN_SYNC_START, dwin, dgroup, assert)) {
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
