/* $Id: wintest.c,v 1.5 2003/06/13 17:55:51 rainer Exp $
 *
 * MPI_Win_test		a nonblocking version of MPI_Win_wait
 *
 * input parameters:
 *	win		window object (handle)
 *
 * output parameters:
 *	flag	true if MPI_Win_wait would return, otherwise false
 */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_test = PMPI_Win_test
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_test (MPI_Win win, int *flag) __attribute__ ((weak, alias ("PMPI_Win_test")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_test  MPI_Win_test
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_test as PMPI_Win_test
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
MPI_Win_test (win, flag)
	MPI_Win win;
	int 	* flag;
{
	MPID_Win	* dwin;
	static char	myname[] = "MPI_WIN_TEST";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);
	if (!flag) 
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);

	/* do sync */
	if (MPID_Win_sync (MPID_WIN_SYNC_TEST, dwin, NULL, 0)) {
		*flag = 1;
	} else {
		*flag = 0;
	}

	MPIR_WIN_RETURN (dwin, MPI_SUCCESS, myname);
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
