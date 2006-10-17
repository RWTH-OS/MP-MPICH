/* $Id$
 *
 * MPI_Accumulate		combines data with data in target memory
 *
 * input parameters:
 *	origin_addr			initial address of origin buffer
 *	origin_count		number of entries in origin buffer (pos. integer)
 *	origin_datatype		datatype of each entry in origin buffer (handle)
 *	target_rank			rank of target (pos. integer)
 *	target_disp			displacement from start of window to target buffer
 *						(pos. integer)
 *	target_count		number of ontries in target buffer (pos. integer)
 *	target_datatype		datatype of each entry in target buffer (handle)
 *	op					reduce operation (handle)
 *	win					window object used for communication (handle)
 *
 * output parameters:
 *	<none>
 */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#include "packdtype.h"
#include "maprank.h"
#include "mpimem.h"
#include "help_funcs.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Accumulate = PMPI_Accumulate
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Accumulate (void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, 
				MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Accumulate")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Accumulate  MPI_Accumulate
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Accumulate as PMPI_Accumulate
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
MPI_Accumulate (origin_addr, origin_count, origin_datatype, target_rank, 
				target_disp, target_count, target_datatype, op, win)
	void 			* origin_addr;
	int 			origin_count;
	MPI_Datatype 	origin_datatype;
	int 			target_rank;
	MPI_Aint 		target_disp;
	int 			target_count;
	MPI_Datatype 	target_datatype;
	MPI_Op			op;
	MPI_Win 		win;
{
	MPID_Hid_Accumulate_t	* hinfo;
	struct iovec			* vector;
	static char				myname[] = "MPI_ACCUMULATE";
	MPID_Win				* dwin;
	int						dtypes_equal;
	MPID_Datatype			* target_dtype,
							* origin_dtype;
	int						error_code;

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin)) 
		MPIR_WIN_NULL_RETURN (myname);
	target_dtype = MPID_GET_DTYPE_PTR (target_datatype);
	if (MPID_TEST_DTYPE_NOTOK (target_datatype, target_dtype))
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);
	origin_dtype = MPID_GET_DTYPE_PTR (origin_datatype);
	if (MPID_TEST_DTYPE_NOTOK (origin_datatype, origin_dtype))
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);

	hinfo = MALLOC (sizeof (MPID_Hid_Accumulate_t));
	if (!hinfo) 
		MPIR_WIN_RETURN (dwin, MPI_ERR_NOMEM, myname);

	dtypes_equal = MPIR_TEST_DTYPES_EQUAL (origin_dtype, target_dtype);
	hinfo->target_offset	= target_disp;
	hinfo->target_count 	= target_count;
	hinfo->origin_count 	= origin_count;
	if (MPID_IS_KNOWN_DTYPE (target_dtype, 
								MPID_WIN_TO_GRANK (dwin, target_rank))) {
		hinfo->kind_dtype	= MPID_GET_KNOWN_DTYPE_ID (target_dtype);
	} else {
		hinfo->kind_dtype	= MPID_DTYPE_UNKNOWN;
	}
	hinfo->dtypes_equal 	= dtypes_equal;
	hinfo->op 				= op;
	
	vector = MALLOC (sizeof (struct iovec) * 4);
	if (!vector) {
		FREE (hinfo);
		MPIR_WIN_RETURN (dwin, MPI_ERR_NOMEM, myname);
	}

	vector[0].iov_base	= (void *)hinfo;
	vector[0].iov_len	= sizeof (MPID_Hid_Accumulate_t);
	vector[1].iov_base	= (void *)origin_addr;
	vector[1].iov_len	= origin_count;		/* well, it isn't correct, but we
											don't need the len info, so
											we don't care */
	vector[2].iov_base	= (void *)target_dtype;
	vector[2].iov_len	= sizeof (MPID_Datatype);
	vector[3].iov_base	= (void *)origin_dtype;
	vector[3].iov_len	= sizeof (MPID_Datatype);

	error_code = MPID_Rhcv (target_rank, dwin, MPID_Hid_Accumulate, 
							vector, 4, NULL);

	FREE (hinfo);
	FREE (vector);

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
