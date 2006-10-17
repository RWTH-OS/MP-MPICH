/* $Id: wincomplete.c,v 1.5 2003/06/13 17:55:51 rainer Exp $
 *
 * MPI_Win_complete		completes an RMA access epoch on win started
 *						by MPI_Win_start
 *
 * input parameters:
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
#pragma weak MPI_Win_complete = PMPI_Win_complete
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_complete (MPI_Win win) __attribute__ ((weak, alias ("PMPI_Win_complete")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_complete  MPI_Win_complete
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_complete as PMPI_Win_complete
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
MPI_Win_complete (win)
	MPI_Win win;
{
	MPID_Win 	* dwin;
	int			error_code = MPI_SUCCESS;
	static char	myname[] = "MPI_WIN_COMPLETE";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);

	/* do sync */
	if (!MPID_Win_sync (MPID_WIN_SYNC_COMPLETE, dwin, NULL, 0)) {
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
