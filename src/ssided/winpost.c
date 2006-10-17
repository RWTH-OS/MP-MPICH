/* $Id: winpost.c,v 1.5 2003/06/13 17:55:51 rainer Exp $
 *
 * MPI_Win_post		starts an RMA exposure epoch for the local
 *					window associated with win
 *
 * input parameters:
 *	group	group of origin process (handle)
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
#pragma weak MPI_Win_post = PMPI_Win_post
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_post (MPI_Group group, int assert, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Win_post")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_post  MPI_Win_post
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_post as PMPI_Win_post
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
MPI_Win_post (group, assert, win)
	MPI_Group 	group;
	int 		assert;
	MPI_Win 	win;
{
	MPID_Group	* dgroup;
	MPID_Win	* dwin;
	int			error_code = MPI_SUCCESS;
	static char	myname[] = "MPI_WIN_POST";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);
	dgroup = MPID_GET_GROUP_PTR (group);
	if (MPID_TEST_GROUP_NOTOK (group, dgroup))
		MPIR_WIN_RETURN (dwin, MPI_ERR_GROUP, myname);

	/* do the sync */
	if (!MPID_Win_sync (MPID_WIN_SYNC_POST, dwin, dgroup, assert)) {
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
