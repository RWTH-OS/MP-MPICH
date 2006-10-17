/* $Id$
 *
 * MPID_SMI_Get_contig		- gets remote memory regions
 *
 * input parameters:
 *	origin_buf		location in local memory (destination)
 *	n				number of bytes to copy
 *	target_offset	location in remote memory
 *	target_rank		rank of remote process in the communicator
 *	comm			Communicator
 *	target_flag		this is an id of a flag at the remote process
 *
 * output parameters:
 *	local_flag		address of a flag to be set when completed
 *
 * return value:
 *	error code
 */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"

#include "maprank.h"
#include "uniqtag.h"
#include "smimem.h"
#include "packdtype.h"
#include "sside_memcpy.h"
#include "mmu.h"
#include "smistat.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int do_local_copy (void *, int, size_t, int, MPID_Win *, int);

int MPID_SMI_Get_contig (	origin_buf, n, target_offset, target_rank, win, 
							local_flag, target_flag)
	void 			* origin_buf;
	int 			n;
	MPI_Aint 		target_offset;
	int 			target_rank;
	MPID_Win 		* win;
	volatile int 	* local_flag;
	MPI_Aint 		target_flag;
{
	int							cpflag;
	MPID_Hid_Get_emulation_t	* hinfo;
	int							local_isshared;
	int							remote_isshared;
	int							remote_put, emulate_get, remput_ifshared, get_direct;
	struct iovec				* vector;
	int							error_code = MPI_SUCCESS;

	MPID_STAT_ENTRY (sside_get_contig);

	if (local_flag)
		*local_flag = 0;
#if 0
	if ((size_t) target_offset *
			win->devinfo.w_smi.frames[target_rank].disp_unit >
			win->devinfo.w_smi.frames[target_rank].length) {
		return MPI_ERR_ARG;
	}
#endif

#if 0
	local_isshared = MPID_SMI_ISADDR_SHARED (origin_buf);
#else
	/* We restrict remote puts to the case, that origin_buf is in the
	   current window, this makes things a lot easier. */
	/* XXX should be changed! */
	local_isshared = (size_t) origin_buf >= (size_t) win->start_address
		&& (size_t) origin_buf < (size_t) win->start_address + win->length;
#endif
	remote_isshared = win->devinfo.w_smi.frames[target_rank].is_shared;
	remput_ifshared = (n >= MPID_SMI_cfg.SSIDED_RMTPUT_SHARED);
	remote_put = local_isshared && (remput_ifshared || !remote_isshared); 
	get_direct = (remote_isshared && n < MPID_SMI_cfg.SSIDED_RMTPUT_PRIVATE);
	emulate_get = (target_rank != win->devinfo.w_smi.lrank) && !get_direct; 
	
	if (emulate_get) {
		/* Delay the operation (and gather it with others) ? */
		if (n < MPID_SMI_cfg.SSIDED_DELAY) {
			MPID_SMI_Delayed_ta_t *os_ta = (MPID_SMI_Delayed_ta_t *)
				MPID_SBalloc(MPID_SMI_os_ta_allocator);
			
			os_ta->target_lrank  = target_rank;
			os_ta->ta_type       = OS_TA_GET;
			os_ta->ta_accu_op    = -1;
			os_ta->origin_addr   = origin_buf;
			os_ta->target_offset = target_offset;
			os_ta->contig_size   = n;
			
			error_code = MPID_SMI_Os_delayed_store (win, os_ta);
		} else {
			ALLOCATE(hinfo, MPID_Hid_Get_emulation_t *, sizeof(MPID_Hid_Get_emulation_t));
			hinfo->target_offset	= target_offset;
			hinfo->target_count 	= n;
			hinfo->origin_count 	= n;
			hinfo->kind_dtype 		= MPID_DTYPE_CONTIG;
			hinfo->dtypes_equal 	= 1;
			hinfo->do_remote_put	= remote_put;
			
			ALLOCATE(vector, struct iovec *, sizeof (struct iovec) * 2);
			vector[0].iov_base	= (void *)hinfo;
			vector[0].iov_len	= sizeof (hinfo);
			vector[1].iov_base	= origin_buf;
			vector[1].iov_len	= n;
			error_code = MPID_SMI_Rhcv (target_rank, win, MPID_Hid_Get_emulation, vector, 2, local_flag);
			FREE (hinfo);
			FREE (vector);
		}
		MPID_STAT_EXIT (sside_get_contig);
	} else {
		/* we can copy directly, and we don't have to wait */
		do_local_copy (origin_buf, n, target_offset, target_rank, win, local_isshared);

		/* finished - set flag */
		if (local_flag)
			*local_flag = 1;
	}

	MPID_STAT_EXIT (sside_get_contig);

	return MPI_SUCCESS;
}


/* 
 * help functions
 */
static int do_local_copy (	origin_buf, numofdata, target_offset, target_rank, 
							win, local_isshared)
	void		* origin_buf;
	int			numofdata;
	size_t		target_offset;
	int			target_rank;
	MPID_Win	* win;
	int			local_isshared;
{
	int		cpflag, drank;
	void	* target_addr;
	int		verify;

	/* XXX Use region management: Assure access to remote memory 
	   via MPID_SMI_Rmt_mem_map() */	

	target_addr = (void *) ((char *)win->devinfo.w_smi.frames[target_rank].start_address 
							+ (size_t) target_offset * win->devinfo.w_smi.frames[target_rank].disp_unit);
	if (target_rank == win->devinfo.w_smi.lrank) {
		cpflag = MPID_SMI_MEMCPY_SRC_LOCAL | MPID_SMI_MEMCPY_DEST_LOCAL;
		drank = 0;
	} else {
		cpflag = MPID_SMI_MEMCPY_SRC_LOCAL | (local_isshared ? 
					MPID_SMI_MEMCPY_LOCAL_SHARED : 0);
		drank = MPID_WIN_TO_DRANK_FAST (win, target_rank);
	}

	verify = 0;
	do {
		MPID_SMI_Sside_memcpy (	origin_buf, target_addr, numofdata, 
								drank, cpflag, &verify);
	} while (verify && SMI_Check_transfer_proc(drank, CHECK_MODE) != SMI_SUCCESS);

	/* XXX Release remote memory via MPID_SMI_Rmt_mem_release() */

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
