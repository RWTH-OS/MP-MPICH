/* $Id$
 *
 * MPI_Win_get_group	returns the group of procs using a window
 *
 * input parameters:
 *	win		window (handle)
 *
 * output parameters:
 *	group	requested group (handle)
 */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_get_group = PMPI_Win_get_group
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_get_group (MPI_Win win, MPI_Group *group) __attribute__ ((weak, alias ("PMPI_Win_get_group")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_get_group  MPI_Win_get_group
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_get_group as PMPI_Win_get_group
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
MPI_Win_get_group (win, group)
	MPI_Win 	win;
	MPI_Group 	* group;
{
	MPID_Win 	* dwin;
	static char	myname[] = "MPI_WIN_GET_GROUP";
	int			error_code;

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);
	
	if (!group) 
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);

	error_code = MPI_Comm_group (dwin->comm->self, group);

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
