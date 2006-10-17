/* $Id$
 *
 * MPID_SCI_Get_sametype		- copies memory from remote host to local
 *
 * input parameters:
 *	origin_buf		pointer to local memory to hold fetched data
 *	n				number of datatype to copy
 *	dtype			Datatype of source and dest
 *	target_offset	location of destination
 *	target_rank		rank of remote process in the communicator
 *	win				Window object
 *	target_flag		this is an id of a flag at the remote process
 *
 * output parameters:
 *	local_flag		address of a flag to be set when completed
 *
 * return value:
 *	error code
 */


#include <stdlib.h>

#include "smi.h"

#include "mpi.h"
#include "mpiimpl.h"
#include "mpichconf.h"
#include "adi3types.h"
#include "smidev.h"
#include "maprank.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "packdtype.h"
#include "sside_memcpy.h"
#include "mmu.h"
#include "sside_macros.h"
#include "smistat.h"

#ifdef MPID_USE_DEVTHREADS
#include <pthread.h>
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int walk_thru_dtype_and_get (MPID_Datatype *, void *, void *, int, 
									int, int*);
static int do_local_copy (	void *, int, MPID_Datatype *, size_t, int,
							MPID_Win *, int);

/* it's a poor heuristic, but efficient to calculate and quite
    usable in most cases
 */



int MPID_SMI_Get_sametype (	origin_buf, n, dtype, target_offset, target_rank, 
							win, local_flag, target_flag)
	void 			* origin_buf;
	int 			n;
	MPID_Datatype	* dtype;
	MPI_Aint 		target_offset;
	int 			target_rank;
	MPID_Win 		* win;
	volatile int 	* local_flag;
	MPI_Aint 		target_flag;
{
	size_t						size;
	MPID_Hid_Get_emulation_t	* hinfo;
	struct iovec				* vector;
	int							emulate_get, local_isshared, remote_isshared;
	int							remote_put, remput_iflocpriv, remput_ifshared;
	int							error_code = MPI_SUCCESS;

	MPID_STAT_ENTRY (sside_get_sametype);

	size = dtype->size * n;
	if (dtype->is_contig) {
		return MPID_SMI_Get_contig (origin_buf, size, target_offset,
									target_rank, win, local_flag,
									target_flag);
	}

	if (local_flag)
		*local_flag = 0;

#if 0
	local_isshared = MPID_SMI_ISADDR_SHARED (origin_buf);
#else
	/* we restrict remote puts to the case, that origin_buf is in the
		current window, this makes things a lot easier
		should be changed!!!
	 */
	local_isshared = (size_t) origin_buf >= (size_t) win->start_address
		&& (size_t) origin_buf < (size_t) win->start_address + win->length;
#endif
	remote_isshared = win->devinfo.w_smi.frames[target_rank].is_shared;
	remput_iflocpriv = size >= MPID_SMI_cfg.SSIDED_RMTPUT_PRIVATE;
	remput_ifshared = size >= MPID_SMI_cfg.SSIDED_RMTPUT_SHARED;
	remote_put = local_isshared && (remput_ifshared || !remote_isshared);
	emulate_get = !(target_rank == win->devinfo.w_smi.lrank) &&
					(remote_put || !remote_isshared ||
					!local_isshared && remput_iflocpriv ||
						!WANT_DO_LOCAL (n, dtype));

	if (emulate_get) {
        hinfo = MALLOC (sizeof (MPID_Hid_Get_emulation_t));
		if (!hinfo) return MPI_ERR_NOMEM;
		hinfo->target_offset	= target_offset;
		hinfo->target_count		= n;
		hinfo->origin_count		= n;
		hinfo->dtypes_equal		= 1;
		if (MPID_IS_KNOWN_DTYPE (dtype, 
							MPID_WIN_TO_GRANK_FAST(win, target_rank))) {
			hinfo->kind_dtype	= MPID_GET_KNOWN_DTYPE_ID (dtype);
		} else {
			hinfo->kind_dtype	= MPID_DTYPE_UNKNOWN;
		}
		hinfo->do_remote_put	= remote_put;
		vector = malloc (sizeof (struct iovec) * 3);
		if (!vector) {
			FREE (hinfo);
			return MPI_ERR_NOMEM;
		}
		vector[0].iov_base	= (void *)hinfo;
		vector[0].iov_len	= sizeof (hinfo);
		vector[1].iov_base	= origin_buf;
		vector[1].iov_len	= n;
		vector[2].iov_base	= (void *)dtype;
		vector[2].iov_len	= sizeof (MPID_Datatype);
		error_code = MPID_SMI_Rhcv (target_rank, win, MPID_Hid_Get_emulation,
									vector, 3, local_flag);
		FREE (vector);
		FREE (hinfo);
		MPID_STAT_EXIT (sside_get_sametype);
		return error_code;
	} else {
		/* we can copy directly, and we don't have to wait */
		do_local_copy (	origin_buf, n, dtype, target_offset, target_rank,
						win, local_isshared);
		
		/* finished - set flag */
		if (local_flag)
			*local_flag = 1;
	}

	MPID_STAT_EXIT (sside_get_sametype);
	return MPI_SUCCESS;
}



/*
 * help functions
 */
static int do_local_copy ( origin_buf, numofdata, dtype, target_offset,
						   target_rank, win, local_isshared)
	 void			* origin_buf;
	 int				numofdata;
	 MPID_Datatype	* dtype;
	 size_t			target_offset;
	 int				target_rank;
	 MPID_Win		* win;
	 int				local_isshared;
{
	void	* target_addr;
	int		i;
	int		drank, cpflag;
	int		verify;

	if (target_rank == win->devinfo.w_smi.lrank) {
		cpflag = MPID_SMI_MEMCPY_SRC_LOCAL | MPID_SMI_MEMCPY_DEST_LOCAL;
		drank = 0;
    } else {
		cpflag = MPID_SMI_MEMCPY_DEST_LOCAL | (local_isshared ?
					MPID_SMI_MEMCPY_LOCAL_SHARED : 0);
		drank = MPID_WIN_TO_DRANK_FAST (win, target_rank);
	}

	target_addr = (void *) (target_offset *
							(size_t)win->devinfo.w_smi.frames[target_rank].disp_unit +
							(char *)win->devinfo.w_smi.frames[target_rank].start_address);

	verify = 0;
	do {
		for (i=0; i<numofdata; i++) {
			walk_thru_dtype_and_get (dtype, (char *)target_addr + i*dtype->extent,
									 (char *)origin_buf + i*dtype->extent, drank, cpflag, &verify);
		}
	} while (verify && SMI_Check_transfer_proc(drank, CHECK_MODE) != SMI_SUCCESS);
	
	return 1;
}


static int walk_thru_dtype_and_get (dtype, target_addr, buffer, drank, cpflag, verify)
	MPID_Datatype	* dtype;
	void			* target_addr;
	void			* buffer;
	int				drank, cpflag;
	int				* verify;
{
	int i,j;

	if (dtype->is_contig) {
		MPID_SMI_Sside_memcpy (	buffer, target_addr, dtype->size, 
								drank, cpflag, verify);
	} else {
		switch (dtype->dte_type) {
		case MPIR_VECTOR:
		case MPIR_HVECTOR:
			for (i=0; i<dtype->count; i++) {
				if (dtype->old_type->is_contig) {
					MPID_SMI_Sside_memcpy ((char *)buffer + i*dtype->stride,
										   (char *)target_addr + i*dtype->stride,
										   dtype->blocklen * dtype->old_type->size,
										   drank, cpflag, verify);
					continue;	
				}
				for (j=0; j<dtype->blocklen; j++) {
					walk_thru_dtype_and_get (dtype->old_type,
											 (char *)target_addr + i*dtype->stride +
											 j*dtype->old_type->extent,
											 (char *)buffer + i*dtype->stride +
											 j*dtype->old_type->extent,
											 drank, cpflag, verify);
				}
			}
		break;
		case MPIR_INDEXED:
        case MPIR_HINDEXED:
			for (i=0; i<dtype->count; i++) {
				if (dtype->old_type->is_contig) {
					MPID_SMI_Sside_memcpy ((char *)buffer + dtype->indices[i],
										   (char *)target_addr + dtype->indices[i],
										   dtype->blocklens[i] * dtype->old_type->size,
										   drank, cpflag, verify);
					continue;	
				}
				for (j=0; j<dtype->blocklens[i]; j++) {
					walk_thru_dtype_and_get (dtype->old_type,
											 (char *)target_addr + dtype->indices[i] +
											 j*dtype->old_type->extent,
											 (char *)buffer + dtype->indices[i] +
											 j*dtype->old_type->extent,
											 drank, cpflag, verify);
				}
			}
			break;
		case MPIR_STRUCT:
			for (i=0; i<dtype->count; i++) {
				if (dtype->old_types[i]->is_contig) {
					MPID_SMI_Sside_memcpy ((char *)buffer + dtype->indices[i],
										   (char *)target_addr + dtype->indices[i],
										   dtype->blocklens[i] * dtype->old_types[i]->size,
										   drank, cpflag, verify);
					continue;	
				}
				for (j=0; j<dtype->blocklens[i]; j++) {
					walk_thru_dtype_and_get (dtype->old_types[i],
											 (char *)target_addr + dtype->indices[i] +
											 j*dtype->old_types[i]->extent,
											 (char *)buffer + dtype->indices[i] +
											 j*dtype->old_types[i]->extent,
											 drank, cpflag, verify);
				}
			}
			break;
		}
	}
	return 1;
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
