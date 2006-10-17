/* $Id: errhandler.c,v 1.3 2003/06/13 17:55:51 rainer Exp $ */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Win_create_errhandler = PMPI_Win_create_errhandler
#pragma weak MPI_Win_set_errhandler = PMPI_Win_set_errhandler
#pragma weak MPI_Win_get_errhandler = PMPI_Win_get_errhandler
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Win_create_errhandler (MPI_Win_errhandler_fn *function, MPI_Errhandler* errhandler) __attribute__ ((weak, alias ("PMPI_Win_create_errhandler")));
EXPORT_MPI_API int MPI_Win_set_errhandler (MPI_Win win, MPI_Errhandler errhandler) __attribute__ ((weak, alias ("PMPI_Win_set_errhandler")));
EXPORT_MPI_API int MPI_Win_get_errhandler (MPI_Win win, MPI_Errhandler *errhandler) __attribute__ ((weak, alias ("PMPI_Win_get_errhandler")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Win_create_errhandler  MPI_Win_create_errhandler
#pragma _HP_SECONDARY_DEF PMPI_Win_set_errhandler  MPI_Win_set_errhandler
#pragma _HP_SECONDARY_DEF PMPI_Win_get_errhandler  MPI_Win_get_errhandler
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Win_create_errhandler as PMPI_Win_create_errhandler
#pragma _CRI duplicate MPI_Win_set_errhandler as PMPI_Win_set_errhandler
#pragma _CRI duplicate MPI_Win_get_errhandler as PMPI_Win_get_errhandler
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
MPI_Win_create_errhandler (function, errhandler)
	MPI_Win_errhandler_fn	* function;
	MPI_Errhandler 			* errhandler;
{
	return MPI_Errhandler_create (	(MPI_Handler_function *) function, 
									errhandler);
}



EXPORT_MPI_API
int
MPI_Win_set_errhandler (win, errhandler)
	MPI_Win			win;
	MPI_Errhandler	errhandler;
{
	MPID_Win	* dwin;
	
	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		return MPI_ERR_WIN;

	dwin->errhandler = errhandler;

	return MPI_SUCCESS;
}



EXPORT_MPI_API
int
MPI_Win_get_errhandler (win, errhandler)
	MPI_Win			win;
	MPI_Errhandler	* errhandler;
{
	MPID_Win	* dwin;
	
	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		return MPI_ERR_WIN;

	*errhandler = dwin->errhandler;

	return MPI_SUCCESS;
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
