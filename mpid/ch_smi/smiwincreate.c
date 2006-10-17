/* $Id$
 *
 * MPID_SMI_Win_create		- the device counterpart of MPI_Win_create
 *
 * input parameters:
 *	base		start address of window
 *	size		length of window (in bytes)
 *	disp_unit	displacement unit
 *	info		pointer to info strutcure
 *	comm		communicator
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	a new window or a NULL pointer if an error occured
 */

#include <stdlib.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* don't use alloca() at all since it tends to make linking problesm with
   various compilers etc. */
#if 0
#ifndef WIN32
#include <alloca.h>
#define _mpid_smi_alloca alloca
#else
typedef  _int32 int32_t ;
#define _mpid_smi_alloca malloc
#endif
#else
#define _mpid_smi_alloca malloc
#endif

#include "mpi.h"
#include "smi.h"

#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#include "req.h"

#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "mmu.h"
#include "sendrecvstubs.h"
#include "smistat.h"


/* for delayed & gathered one-sided transactions */
static int ta_compare (void *ta_1, void *ta_2)
{
    MPID_SMI_Delayed_ta_t *os_ta_1 = (MPID_SMI_Delayed_ta_t *)ta_1;
    MPID_SMI_Delayed_ta_t *os_ta_2 = (MPID_SMI_Delayed_ta_t *)ta_2;

    if ((size_t)os_ta_1->origin_addr > (size_t)os_ta_2->origin_addr)
		return 1;

    if ((size_t)os_ta_1->origin_addr < (size_t)os_ta_2->origin_addr)
		return -1;

    return 0;
}


MPID_Win *MPID_SMI_Win_create (base, size, disp_unit, info, comm)
	void 		* base;
	size_t 		size;
	int 		disp_unit;
	MPID_Info 	* info;
	MPID_Comm 	* comm;
{
	MPID_Info					*info_ptr;
	MPID_Win					*new_win;
	struct _sendinfo {
		size_t		offset;		
		int32_t		shregid;		/* an id of -1 means not shared */
		size_t		length;
		int32_t		disp_unit;
		size_t		job_count;		/* address (shared mem) */
		int32_t		jobcountid;		/* id of shared reg */
		int32_t		winid;
		int			start_post_tag;
		int			complete_wait_tag;
		int			fence_tag;
	}							sendinfo, recvinfo;
	MPID_SMI_Win_frame			*dframe;
	int							myrank, num_proc, drank, grank, *drank_list;
	int							*job_count = NULL;
	int							i, j, sendtag, error_code;

	MPID_STAT_ENTRY (sside_win_create);
	
	/* create new structure and initialize */
	new_win = MALLOC (sizeof (MPID_Win));
	if (!new_win) {
		goto error;
	}

	myrank = comm->local_rank;
	num_proc = comm->np;

	new_win->cookie = MPID_WIN_COOKIE;
	new_win->id = (int) MPIR_FromPointer (new_win);
	new_win->ref_count = 1;
	new_win->start_address = base;
	new_win->length = size;
	new_win->attributes = NULL;
	new_win->comm = comm;
	new_win->name[0] = 0;
	new_win->disp_unit = disp_unit;
	new_win->devinfo.w_smi.lrank = myrank;
	new_win->devinfo.w_smi.lsize = num_proc;
	ZALLOCATE (new_win->devinfo.w_smi.frames, MPID_SMI_Win_frame *, 
			   sizeof(MPID_SMI_Win_frame)*num_proc);

	new_win->devinfo.w_smi.fenced = 0;
	new_win->devinfo.w_smi.local_expose.is_set = 0;
	new_win->devinfo.w_smi.local_expose.group = NULL;
	new_win->devinfo.w_smi.local_expose.assert = 0;
	new_win->devinfo.w_smi.remote_expose.is_set = 0;
	new_win->devinfo.w_smi.remote_expose.group = NULL;
	new_win->devinfo.w_smi.remote_expose.assert = 0;

	if (myrank == 0) {
		new_win->devinfo.w_smi.start_post_tag = MPID_SMI_Get_uniq_tag ();
		new_win->devinfo.w_smi.complete_wait_tag = MPID_SMI_Get_uniq_tag ();
		new_win->devinfo.w_smi.fence_tag = MPID_SMI_Get_uniq_tag ();
		new_win->devinfo.w_smi.delayed_tag = MPID_SMI_Get_uniq_tag ();
	}

	if (!(drank_list = (int *)_mpid_smi_alloca (sizeof (int) * num_proc)))
		goto error;
	for (i = 0; i < num_proc; i++)
		drank_list[i] = MPID_COMM_TO_DRANK_FAST (comm, i);

	/* job counters */
	new_win->devinfo.w_smi.ogn_cmpltcnt 
		= (int *)MPID_SMI_Alloc_mem_internal (sizeof(int), MUST_BE_SHARED, NO_ALIGNMENT);
	if (!new_win->devinfo.w_smi.ogn_cmpltcnt)
		goto error;
	for (i = 0; i < num_proc; i++)
		new_win->devinfo.w_smi.ogn_cmpltcnt[i] = 0;
	ZALLOCATE (new_win->devinfo.w_smi.tgt_cmpltcnt_rmt, volatile int **, num_proc*sizeof(int *));
	ZALLOCATE (new_win->devinfo.w_smi.tgt_cmpltcnt_loc, int *, num_proc*sizeof(int));
	new_win->devinfo.w_smi.tgt_cmpltcnt_rmt[myrank] = &new_win->devinfo.w_smi.ogn_cmpltcnt[myrank];
	new_win->devinfo.w_smi.ogn_jobcnt = 0;
	new_win->devinfo.w_smi.tgt_jobcnt = 0;
 
	/* Broadcast window info to other procs in comm and establish connections, 
	   if required. */
	if (!MPID_SMI_Addr_to_offset (new_win->start_address, &sendinfo.shregid, 
								  &sendinfo.offset)) {
		sendinfo.shregid = -1;	
		new_win->devinfo.w_smi.is_shared = 0;
	}
	sendinfo.length = new_win->length;
	sendinfo.disp_unit = new_win->disp_unit;
	sendinfo.winid = new_win->id;
	MPID_SMI_Addr_to_offset ((void *)new_win->devinfo.w_smi.ogn_cmpltcnt, 
							 &sendinfo.jobcountid, &sendinfo.job_count);
	if (myrank == 0) {
		sendinfo.start_post_tag = new_win->devinfo.w_smi.start_post_tag;
		sendinfo.complete_wait_tag = new_win->devinfo.w_smi.complete_wait_tag;
		sendinfo.fence_tag = new_win->devinfo.w_smi.fence_tag;
	}

	sendtag = MPID_SMI_WIN_CREATE_TAG;
	for (i = 0; i < num_proc; i++) {
		if (i == myrank) {
			for (j = 0; j < num_proc; j++) {
				if (j == myrank) 
					continue;

				grank = MPID_COMM_TO_GRANK_FAST(comm, j);
				MPID_SMIstub_SendContig (&sendinfo, sizeof (sendinfo), myrank, 
										 sendtag, comm->send_context, grank, 
										 MPID_MSGREP_SENDER, &error_code);
				if (error_code) 
					goto error;
			}
		} else {
			MPID_SMIstub_RecvContig (&recvinfo, sizeof (recvinfo), i, sendtag, 
									 comm->recv_context, NULL, &error_code);
			if (error_code) 
				goto error;

			dframe = new_win->devinfo.w_smi.frames + i;
			if (recvinfo.shregid == -1) {
				dframe->is_shared = 0;
				dframe->start_address = NULL;
			} else {
				dframe->is_shared = 1;
				MPID_SMI_Shreg_tryconnect (i, recvinfo.shregid);
				dframe->start_address = MPID_SMI_Offset_to_addr (i, recvinfo.shregid, 
																 recvinfo.offset);
				if (!dframe->start_address)
					dframe->is_shared = 0;
			}

			dframe->length = recvinfo.length;
			dframe->disp_unit = recvinfo.disp_unit;
			dframe->winid = recvinfo.winid;
			if (i == 0 && myrank != 0) {
				new_win->devinfo.w_smi.start_post_tag = recvinfo.start_post_tag;
				new_win->devinfo.w_smi.complete_wait_tag = recvinfo.complete_wait_tag;
				new_win->devinfo.w_smi.fence_tag = recvinfo.fence_tag;
			}

			MPID_SMI_Shreg_tryconnect (i, recvinfo.jobcountid);
			new_win->devinfo.w_smi.tgt_cmpltcnt_rmt[i] = 
				MPID_SMI_Offset_to_addr (i, recvinfo.jobcountid, recvinfo.job_count);
		}
	}

	/* now fill in my own frame (for consintency) */
	dframe = new_win->devinfo.w_smi.frames + myrank;
	dframe->is_shared 		= 1; /* for me its always reachable ;-) */
	dframe->start_address 	= new_win->start_address;
	dframe->length 			= new_win->length;
	dframe->disp_unit 		= new_win->disp_unit;
	dframe->winid 			= new_win->id;
				
	MPID_SMI_INIT_LOCK (&new_win->mutex);
	MPID_SMI_INIT_LOCK (&new_win->devinfo.w_smi.job_lock);
	MPID_SMI_INIT_LOCK (&new_win->devinfo.w_smi.jobcnt_lock);
	MPID_SMI_INIT_LOCK (&new_win->devinfo.w_smi.accu_lock);
	
	/* Create winlock-mutexes only if "no_locks" info argument is not given. */
	new_win->devinfo.w_smi.winlocks = NULL;
	for (info_ptr = info; info_ptr != NULL; info_ptr = info_ptr->next) {
		if (!strcmp (info_ptr->key, "no_locks"))
			break;
	}
	if (!info_ptr) {	
		/* we havn't found the flag no_locks */
		ALLOCATE(new_win->devinfo.w_smi.winlocks, int *, sizeof(int)*num_proc);
		for (i = 0; i < new_win->devinfo.w_smi.lsize; i++) 
			MPID_SMI_Mutex_init (&new_win->devinfo.w_smi.winlocks[i], num_proc, 
								 drank_list, drank_list[i]);
	}

	new_win->devinfo.w_smi.putaccu_req_fifo = MPID_FIFO_init(MPID_UTIL_THREADSAFE);
	new_win->devinfo.w_smi.get_req_fifo     = MPID_FIFO_init(MPID_UTIL_THREADSAFE);

	/* Init data structures for delayed & gathered transactions, if enabled. */
 	if (MPID_SMI_cfg.SSIDED_DELAY > 0) {
		for (i = 0; i < 3; i++) {
			ZALLOCATE (new_win->devinfo.w_smi.ta_count[i], int *, sizeof(int)*new_win->comm->np);
			ZALLOCATE (new_win->devinfo.w_smi.ta_total_len[i], ulong *, sizeof(ulong)*new_win->comm->np);

			ALLOCATE (new_win->devinfo.w_smi.delayed_ta[i], MPID_tree_t *, 
					  sizeof(MPID_tree_t)*new_win->comm->np);
			for (j = 0; j < new_win->comm->np; j++)
				/* XXX Make the tree threadsafe if necessary. */
				new_win->devinfo.w_smi.delayed_ta[i][j] = MPID_tree_init (ta_compare, 0);

			new_win->devinfo.w_smi.ta_proc_count[i] = 0;
		}

		new_win->devinfo.w_smi.ta_accu_op = -1;
	}

	goto finish;

error:
	if (new_win) {
		if (new_win->devinfo.w_smi.frames)
			FREE (new_win->devinfo.w_smi.frames);
		FREE (new_win);
		new_win = NULL;
	}
	if (job_count) 
		MPID_SMI_Free_mem (job_count);

finish:
	MPID_STAT_EXIT (sside_win_create);
	return new_win;
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
