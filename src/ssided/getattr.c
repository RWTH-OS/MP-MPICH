/* $Id$
 *
 * MPI_Win_get_attr		returns requested window parameters
 *
 * input parameters:
 *	win			window on which parameters are requested
 *	win_keyval	kind of information to be requested 
 *				currently are the following parameters available:
 *					MPI_WIN_BASE		window base address
 *					MPI_WIN_SIZE		window size (in bytes)
 *					MPI_WIN_DISP_UNIT	displacement unit associated with the
 *										window
 * 
 * output parameters:
 *	attribute_val	requested parameter - 	this parameter is a pointer to
 *											various types, depending of the
 *											kind of information requested:
 *						MPI_WIN_BASE		-> void **
 *						MPI_WIN_SIZE		-> MPI_Aint *
 *						MPI_WIN_DISP_UNIT	-> int *
 *	flag			currently always 1 (true)
 *
 * remark:
 *	normally other attributes can be associated with a window, which is
 *	important for caching, keyval is then a dynamicaly generated value.
 *	Caching is not implemented yet, and therefore neither the apropriated
 * 	functions such as MPI_Win_create_keyval, etc.
 *	Therefore do exist only these three static attributes.
 *	If you use one of these flag always does return 1 (true), 
 *	otherwise 0 (false).
 */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "help_funcs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_get_attr = PMPI_Win_get_attr
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_get_attr (MPI_Win win, int win_keyval, void *attribute_val, int *flag) __attribute__ ((weak, alias ("PMPI_Win_get_attr")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_get_attr  MPI_Win_get_attr
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_get_attr as PMPI_Win_get_attr
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
MPI_Win_get_attr (win, win_keyval, attribute_val, flag) 
	MPI_Win win;
	int 	win_keyval; 
	void 	* attribute_val;
	int 	* flag;
{
	MPID_Win 	* dwin;
	static char	myname[] = "MPI_WIN_GET_ATTR";

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin)) {
		*flag=0;
		MPIR_WIN_NULL_RETURN (myname); 
	}

	if (!attribute_val || !flag)
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);

	switch (win_keyval) {
	case MPI_WIN_BASE:
		*(void **) attribute_val = dwin->start_address;
		*flag = 1;
		break;
	case MPI_WIN_SIZE:
		*(MPI_Aint *) attribute_val = dwin->length;
		*flag = 1;
		break;
	case MPI_WIN_DISP_UNIT:
		*(int *) attribute_val = dwin->disp_unit;
		*flag = 1;
		break;
	default:
		*flag = 0;
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);
		break;
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
