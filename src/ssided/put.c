/* $Id: put.c,v 1.8 2003/06/13 17:55:51 rainer Exp $
 *
 * MPI_Put		writes data to remote memory
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
#pragma weak MPI_Put = PMPI_Put
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Put (void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, 
							MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win) __attribute__ ((weak, alias ("PMPI_Put")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Put  MPI_Put
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Put as PMPI_Put
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


EXPORT_MPI_API int MPI_Put (origin_addr, origin_count, origin_datatype, target_rank, 
							target_disp, target_count, target_datatype, win)
	void 			* origin_addr;
	int 			origin_count;
	MPI_Datatype 	origin_datatype;
	int 			target_rank;
	MPI_Aint 		target_disp;
	int 			target_count;
	MPI_Datatype 	target_datatype;
	MPI_Win 		win;
{
	MPID_Win					* dwin;
	MPID_Datatype				* origin_dtype,
								* target_dtype;
	size_t						origin_size,
								target_size;
	MPID_Hid_Put_emulation_t	* hinfo;
	struct iovec				* vector;
	int							dtypes_equal;
	int							is_contig;
	static char					myname[] = "MPI_PUT";
	int							error_code;

	dwin = MPID_GET_WIN_PTR (win);
	if (MPID_TEST_WIN_NOTOK (win, dwin))
		MPIR_WIN_NULL_RETURN (myname);
	origin_dtype = MPID_GET_DTYPE_PTR (origin_datatype);
	if (MPID_TEST_DTYPE_NOTOK (origin_datatype, origin_dtype))
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);
	target_dtype = MPID_GET_DTYPE_PTR (target_datatype);
	if (MPID_TEST_DTYPE_NOTOK (target_datatype, target_dtype))
		MPIR_WIN_RETURN (dwin, MPI_ERR_ARG, myname);

	dtypes_equal = MPIR_TEST_DTYPES_EQUAL (target_dtype, origin_dtype);
	is_contig = target_dtype->is_contig && origin_dtype->is_contig;


	if (is_contig) {
		target_size = target_count * target_dtype->size;
		origin_size = origin_count * origin_dtype->size;
		error_code = MPID_Put_contig (origin_addr, 
									  MIN (target_size, origin_size),
									  target_disp, target_rank, dwin,
									  NULL, 0);
	} else if (dtypes_equal) {
		error_code = MPID_Put_sametype (origin_addr, 
										MIN (target_count, origin_count),
										target_dtype, target_disp,
										target_rank, dwin, NULL, 0);
	} else {
		hinfo = MALLOC (sizeof (MPID_Hid_Put_emulation_t));
		if (!hinfo)
			MPIR_WIN_RETURN (dwin, MPI_ERR_NOMEM, myname);
		hinfo->target_offset	= target_disp;
		hinfo->target_count 	= target_count;
		hinfo->origin_count		= origin_count;
		if (MPID_IS_KNOWN_DTYPE (target_dtype, 
								MPID_WIN_TO_GRANK(dwin, target_rank))) {
			hinfo->kind_dtype	= MPID_GET_KNOWN_DTYPE_ID (target_dtype);
		} else {
			hinfo->kind_dtype	= MPID_DTYPE_UNKNOWN;
		}
		hinfo->dtypes_equal		= 0;

		vector = MALLOC (sizeof (struct iovec) * 4);
		if (!vector) {
			FREE (hinfo);
			MPIR_WIN_RETURN (dwin, MPI_ERR_NOMEM, myname);
		}
		vector[0].iov_base	= (void *)hinfo;
		vector[0].iov_len	= sizeof (hinfo);
		vector[1].iov_base	= (void *)origin_addr;
		vector[1].iov_len	= origin_count;	/* this is not correct, but
											the len info is not used,
											hence we don't care. */
		vector[2].iov_base	= (void *)target_dtype;
		vector[2].iov_len	= sizeof (MPID_Datatype);
		vector[3].iov_base	= (void *)origin_dtype;
		vector[3].iov_len	= sizeof (MPID_Datatype);

		error_code = MPID_Rhcv (target_rank, dwin, MPID_Hid_Put_emulation,
								vector, 4, NULL);
		FREE (hinfo);
		FREE (vector);
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
