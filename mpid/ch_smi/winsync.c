/* $Id: winsync.c,v 1.21.8.1 2004/12/15 13:45:21 boris Exp $
 *
 * MPID_Win_sync		- used to create or free exposure epochs
 *
 * input parameters:
 *	type		type of sync call - currently:
 *					MPID_WIN_SYNC_FENCE,
 *					MPID_WIN_SYNC_POST,
 *					MPID_WIN_SYNC_START,
 *					MPID_WIN_SYNC_COMPLETE,
 *					MPID_WIN_SYNC_WAIT or
 *					MPID_WIN_SYNC_TEST
 *	win			window to operate on
 *	group		group that participates
 *	assert		assert - options to optimize
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	0: if error occures or type is MPID_WIN_SYNC_TEST and the completes are 
 *		not sent yet
 *	1: otherwise
 */


#include <stdlib.h>

#include "smi.h"
#include "mpi.h"

#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"

#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "sendrecvstubs.h"

int MPID_SMI_Win_sync (type, win, group, assert)
	int 		type;
	MPID_Win 	* win;
	MPID_Group	* group;
	int			assert;
{
	MPID_Group				*ngroup;
	int						nassert;
	int						i, myrank, rank2, size;
	int						flag, error_code = MPI_SUCCESS;

	MPID_STAT_ENTRY (sside_win_sync);

	switch (type) {
	case MPID_WIN_SYNC_FENCE:
		/* For fence synchronistaion, we should differentiate between opening 
		   and closing an access epoch: when opening, we don't need to flush the
		   delayed transactions or wait for completion of local jobs (there are none!), 
		   thus we only need a single barrier, too. */
		if (win->devinfo.w_smi.fenced)
			MPID_SMI_Os_delayed_flush (win, -1);

#if WIN_FENCE_SHMEM_BARRIER
		/* We need two use a two-barrier-synchronization to avoid race conditions
		   and enforce message processing even in absence of asynchronous notification
		   because the barrrier *does not* access the message queues necessariy on SCI
		   as it works with shared memory.  Fortunately, this makes a barrier cheap on SCI. */
		win->comm->collops->Barrier (win->comm);
		if (win->devinfo.w_smi.fenced) {
			/* Now complete local send & recv operations relative to this window */
			MPID_SMI_Complete_all_jobs (win);
			win->comm->collops->Barrier (win->comm);
		}
#else
		MPICH_msg_barrier(win->comm);
		if (win->devinfo.w_smi.fenced) 
			MPID_SMI_Complete_all_jobs (win);
#endif
		win->devinfo.w_smi.fenced = !win->devinfo.w_smi.fenced;
		break;

	case MPID_WIN_SYNC_POST:
		/* set group info */
		MPID_SMI_LOCK (&win->mutex);
		if (win->devinfo.w_smi.local_expose.is_set == 1) {
			/* wrong order */
			MPID_SMI_UNLOCK (&win->mutex);
			return 0;
		}
		win->devinfo.w_smi.local_expose.is_set = 1;
		win->devinfo.w_smi.local_expose.group = group;
		win->devinfo.w_smi.local_expose.assert = assert;
		MPID_SMI_UNLOCK (&win->mutex);
		
		size = group->np;

		/* XXX get myrank from SMI function. Might cause problems? */
		myrank = MPID_SMI_myid;
		/* now send 'is_posted' message to all origin processes specified in the group */
		for (i = 0; i < size; i++) {
			rank2 = MPID_COMM_TO_GRANK_FAST (group, i);
			MPID_SMIstub_SendContig (NULL, 0, myrank, win->devinfo.w_smi.start_post_tag, 
						win->comm->send_context, rank2, MPID_MSGREP_SENDER, &error_code);
			if (error_code)
				return 0;
		}
		break;

	case MPID_WIN_SYNC_START:
		/* XXX Currently, only strong group synchronization is supported (see 
		   MPI-2 standard, page 122). Weak group sync can only be used if *all*
		   transactions are delayed (at least, until the 'is_posted' message 
		   arrives. This could be implemented quite easily. */

		MPID_SMI_LOCK (&win->mutex);
		if (win->devinfo.w_smi.remote_expose.is_set == 1) {
			/* wrong order */
			MPID_SMI_UNLOCK (&win->mutex);
			return 0;
		}
		win->devinfo.w_smi.remote_expose.is_set = 1;
		win->devinfo.w_smi.remote_expose.group = group;
		win->devinfo.w_smi.remote_expose.assert = assert;
		MPID_SMI_UNLOCK (&win->mutex);

		/* removed the thread code, because it only causes overhead, but does
		   not provide anything different from strong synchronization */
		
		/* now wait for 'is_posted' msgs from all target processes */
		for (i = 0; i < group->np; i++) {
			MPID_GRPRANK_TO_GRPRANK_FAST (group, i, win->comm, rank2);
			MPID_SMIstub_RecvContig (NULL, 0, rank2, win->devinfo.w_smi.start_post_tag,
								 win->comm->recv_context, NULL, &error_code);
			if (error_code) 
				return 0;
		}
		break;

	case MPID_WIN_SYNC_COMPLETE:
		/* Flush all delayed transactions, then wait for completion of local jobs. 
		   Then send 'is_complete' message to all target processes. */
		MPID_SMI_Os_delayed_flush (win, -1);

		MPID_SMI_LOCK (&win->mutex);
		if (win->devinfo.w_smi.remote_expose.is_set == 0) {
			/* wrong order */
			MPID_SMI_UNLOCK (&win->mutex);
			return 0;
		}
		win->devinfo.w_smi.remote_expose.is_set = 0;
		ngroup = win->devinfo.w_smi.remote_expose.group;
		nassert = win->devinfo.w_smi.remote_expose.assert;
		MPID_SMI_UNLOCK (&win->mutex);

		MPID_SMI_Complete_target_jobs(win, -1);

		/* XXX get myrank from SMI function. Might cause problems?? */
		myrank = MPID_SMI_myid;

		for (i = 0; i < ngroup->np; i++) {
			rank2 = MPID_COMM_TO_GRANK_FAST (ngroup, i);
			MPID_SMIstub_SendContig (NULL, 0, myrank, win->devinfo.w_smi.complete_wait_tag, 
									 win->comm->send_context, rank2, MPID_MSGREP_SENDER, &error_code);
			if (error_code) 
				return 0;
		}

		break;

	case MPID_WIN_SYNC_WAIT:
		MPID_SMI_LOCK (&win->mutex);
		if (win->devinfo.w_smi.local_expose.is_set == 0) {
			/* wrong order */
			MPID_SMI_UNLOCK (&win->mutex);
			MPID_STAT_EXIT (sside_win_sync);
			return 0;
		}
		win->devinfo.w_smi.local_expose.is_set = 0;
		ngroup = win->devinfo.w_smi.local_expose.group;
		nassert = win->devinfo.w_smi.local_expose.assert;
		MPID_SMI_UNLOCK (&win->mutex);

		/* now wait for finish of all jobs */
		MPID_SMI_Complete_job_requests (win);

		/* now wait for 'is_complete' messages from all origin processes */
		for (i = 0; i < ngroup->np; i++) {
			MPID_GRPRANK_TO_GRPRANK_FAST (ngroup, i, win->comm, rank2);
			MPID_SMIstub_RecvContig (NULL, 0, rank2, win->devinfo.w_smi.complete_wait_tag,
									 win->comm->recv_context, NULL, &error_code);
			if (error_code) 
				return 0;
		}

		break;

	case MPID_WIN_SYNC_TEST:
		/* get group info */
		MPID_SMI_LOCK (&win->mutex);
		if (win->devinfo.w_smi.local_expose.is_set == 0) {
			/* wrong order */
			MPID_SMI_UNLOCK (&win->mutex);
			MPID_STAT_EXIT (sside_win_sync);
			return 0;
		}
		ngroup = win->devinfo.w_smi.local_expose.group;
		nassert = win->devinfo.w_smi.local_expose.assert;
		MPID_SMI_UNLOCK (&win->mutex);

		size = ngroup->np;

		/* now probe for all completes */
		for (i = 0; i < size; i++) {
			MPID_GRPRANK_TO_GRPRANK_FAST (ngroup, i, win->comm, rank2);
			MPID_SMIstub_Iprobe (win->comm, win->devinfo.w_smi.complete_wait_tag,
								 win->comm->recv_context, rank2, &flag,&error_code, NULL);
			if (error_code) 
				return 0;
			if (!flag) 
				break;	/* we don't need to check the rest */
		}
		if (flag) {		
			/* all completes are sent - we can now recv all completes */
			for (i = 0; i < size; i++) {
				MPID_GRPRANK_TO_GRPRANK_FAST (ngroup, i, win->comm, rank2);
				MPID_SMIstub_RecvContig (NULL, 0, rank2, win->devinfo.w_smi.complete_wait_tag,
										 win->comm->recv_context, NULL, &error_code);
				if (error_code) 
					return 0;
			}
			/* unset local_exposed flag */
			MPID_SMI_LOCK (&win->mutex);
			win->devinfo.w_smi.local_expose.is_set = 0;
			MPID_SMI_UNLOCK (&win->mutex);
			/* now wait for finish of all jobs */
			MPID_SMI_Complete_job_requests (win);
		}
		
		MPID_STAT_EXIT (sside_win_sync);
		return flag;
		break;
	}

	MPID_STAT_EXIT (sside_win_sync);
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
 * vim:ts=4:sw=4:
 */
