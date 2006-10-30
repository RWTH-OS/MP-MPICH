/* $Id$ 
 *
 * MPID_SMI_Win_free		- frees a window structure (collective call)
 *
 * input parameters:
 *	win		window to be freed
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	error code
 */


#include <stdlib.h>
#include <mpi.h>
#include <smi.h>
#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#ifndef WIN32
#  include <pthread.h>
#endif
#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "smistat.h"
#include "sendrecvstubs.h"
#include "mmu.h"



int MPID_SMI_Win_free (win)
	MPID_Win * win;
{
	int			i;
	int			num_proc, myrank;

	MPID_STAT_ENTRY (sside_win_free);

	MPID_SMI_Complete_all_jobs (win);
	MPID_SMIstub_Barrier (win->comm);

	myrank = win->devinfo.w_smi.lrank;
	num_proc = win->devinfo.w_smi.lsize;

	/* destroy SMI mutexes */
	if (win->devinfo.w_smi.winlocks) {
		for (i = 0; i < num_proc; i++) {
			if (win->devinfo.w_smi.winlocks[i] != -1) {
				MPID_SMI_Mutex_destroy (win->devinfo.w_smi.winlocks[i]);
				win->devinfo.w_smi.winlocks[i] = -1;
			}
		}
		FREE (win->devinfo.w_smi.winlocks);
	}

	MPID_SMI_DESTROY_LOCK (&win->mutex);
	MPID_SMI_DESTROY_LOCK (&win->devinfo.w_smi.job_lock);
	MPID_SMI_DESTROY_LOCK (&win->devinfo.w_smi.accu_lock);
	MPID_SMI_DESTROY_LOCK (&win->devinfo.w_smi.jobcnt_lock);

	/* disconnect the remote shared regions */
	for (i = 0; i < num_proc; i++) {
		if (i == myrank) 
			continue;
		MPID_SMI_Shreg_disconnect (MPID_SMI_Get_shreg (win->devinfo.w_smi.tgt_cmpltcnt_rmt[i]));
		MPID_SMI_Shreg_disconnect (MPID_SMI_Get_shreg (win->devinfo.w_smi.frames[i].start_address));
	}
	/* free the job counter in local shared memory */
	MPID_SMI_Free_mem (win->devinfo.w_smi.ogn_cmpltcnt);

	MPID_FIFO_destroy(win->devinfo.w_smi.putaccu_req_fifo);
	MPID_FIFO_destroy(win->devinfo.w_smi.get_req_fifo);
	
	FREE (win->devinfo.w_smi.frames);
	FREE (win->devinfo.w_smi.tgt_cmpltcnt_loc);
	FREE (win->devinfo.w_smi.tgt_cmpltcnt_rmt);
	FREE (win);

	MPID_STAT_EXIT (sside_win_free);
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
