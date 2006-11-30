/*  $Id$
 *
 * MPI_Win_create 	creates a Window that can be used for single side 
 * 					communication
 *
 * input parameters:
 *	base		base address of communication window
 *	size		size of communication window (pos. integer)
 *	disp_unit	displacement unit - typically 1 or size (type) - allows
 *				addressing like an array (pos. integer)
 *	info		info argument (handle)
 *	comm		communicator (handle)
 *
 * output parameters:
 *	win			window object returned by the call (handle)
 */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_create = PMPI_Win_create
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_create (void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win) __attribute__ ((weak, alias ("PMPI_Win_create")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_create  MPI_Win_create
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_create as PMPI_Win_create
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
MPI_Win_create (base, size, disp_unit, info, comm, win)
	void 		* base;
	MPI_Aint 	size;
	int 		disp_unit;
	MPI_Info 	info;
	MPI_Comm 	comm;
	MPI_Win 	* win;
{
	MPID_Comm	* dcomm;
	MPID_Info	* dinfo;
	MPID_Win	* dwin;
	static char	myname[] = "MPI_WIN_CREATE";

	dcomm = MPID_GET_COMM_PTR (comm);
	if (MPID_TEST_COMM_NOTOK (comm, dcomm)) 
		MPIR_RETURN (NULL, MPI_ERR_COMM, myname);

	if (size > 0 && !base) 
		MPIR_RETURN (dcomm, MPI_ERR_BASE, myname);
	if (size < 0)
		MPIR_RETURN (dcomm, MPI_ERR_SIZE, myname);
	if (disp_unit < 0)
		MPIR_RETURN (dcomm, MPI_ERR_DISP, myname);
	if (win == NULL)
		MPIR_RETURN (dcomm, MPI_ERR_WIN, myname);

	dinfo = MPID_GET_INFO_PTR (info);
	if (MPID_TEST_INFO_NOTOK (info, dinfo)) {
		return MPI_ERR_WIN;
	}

	dwin = MPID_Win_create (base, size, disp_unit, dinfo, dcomm);
	if (dwin) {
		dwin->errhandler = MPI_ERRORS_ARE_FATAL;
		*win = dwin->id;
	}

	MPIR_RETURN (dcomm, dwin ? MPI_SUCCESS : MPI_ERR_INTERN, myname);
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
