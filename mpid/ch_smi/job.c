/* $Id: job.c,v 1.15.8.1 2004/12/15 13:45:20 boris Exp $ */

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
#include "smistat.h"

/* XXX change job-counter configuration for remote (target) jobs as
   follows:

   The counter for target jobs should be located twice at the origin
   process: once in private memory, once in shared memory. The private
   counter is incremented by the origin proc when a job was triggered at 
   the target proc. The target proc in turn increments the shared counter 
   each time a job from the origin proc was processed.

   This way, the origin proc, to verify completion of all target jobs, only
   needs to poll *local* counters: it waits until these two counter match,
   then resets them to zero.

   Of course, this in fact requires a third "shadow counter" at the target 
   proc for it to know the current state of the shared counter w/o reading it.
   This allows for a remote-write-only implementation. */

#if USE_JOBMNGMT
/* A new job was created (here at the origin) that the target needs to
   response to to check for completion. This response is either the
   requested data (get) or the increment of the local counter directly
   by the target. */
int MPID_SMI_Add_target_job (win, target_rank)
	MPID_Win	* win;
    int target_rank;
{
	MPID_STAT_COUNT (sside_add_tgt_job);
	MPID_SMI_LOCK (&win->devinfo.w_smi.jobcnt_lock);

	win->devinfo.w_smi.ogn_jobcnt++;
	/* XXX individual counters not yet implemented - would that make sense? */

	MPID_SMI_UNLOCK (&win->devinfo.w_smi.jobcnt_lock);
	return 1;
}

/* A 'get' request was completed. */
int MPID_SMI_Remove_target_job (win, target_rank)
	MPID_Win	* win;
    int target_rank;
{
	MPID_STAT_COUNT (sside_rm_tgt_job);
	MPID_SMI_LOCK (&win->devinfo.w_smi.jobcnt_lock);

	win->devinfo.w_smi.ogn_jobcnt--;
	/* XXX individual counters not yet implemented - would that make sense? */

	MPID_SMI_UNLOCK (&win->devinfo.w_smi.jobcnt_lock);
	return 1;
}


/* A new job from an origin proc has arrived to be performed here
   at the target proc. */
int MPID_SMI_Add_job_request (win, origin_rank)
	MPID_Win	* win;
    int origin_rank;
{
	MPID_STAT_COUNT (sside_add_jobreq);
	MPID_SMI_LOCK (&win->devinfo.w_smi.jobcnt_lock);

	win->devinfo.w_smi.tgt_jobcnt++;
	/* XXX individual counters not yet implemented - would that make sense? */
	
	MPID_SMI_UNLOCK (&win->devinfo.w_smi.jobcnt_lock);
	return 1;
}

/* If 'put' or 'accumulate' job request has been completed, inform the
   origin process on this by incrementing the related counter. Decrement
   local job counter in all cases. */
int MPID_SMI_Job_request_completed (win, origin_rank, is_get)
	MPID_Win	* win;
    int origin_rank, is_get;
{
	int lrank;

	MPID_STAT_COUNT (sside_rm_jobreq);
	MPID_SMI_LOCK (&win->devinfo.w_smi.jobcnt_lock);

	win->devinfo.w_smi.tgt_jobcnt--;
	/* XXX individual counters not yet implemented - would that make sense? */

	if (!is_get) {
		/* XXX group does not necesarily contain this rank! Always use
		   communicator-rank. This is no problem, sequence-checking will
		   work fine.*/
		lrank = (0 & win->devinfo.w_smi.local_expose.is_set) ? 
			MPID_COMM_TO_GRANK_FAST (win->devinfo.w_smi.local_expose.group, origin_rank) 
			: origin_rank;
		
		win->devinfo.w_smi.tgt_cmpltcnt_loc[origin_rank]++;
		WRITE_RMT_PTR(win->devinfo.w_smi.tgt_cmpltcnt_rmt[origin_rank],
					  win->devinfo.w_smi.tgt_cmpltcnt_loc[origin_rank],
					  win->comm->lrank_to_grank[lrank]);
	}

	MPID_SMI_UNLOCK (&win->devinfo.w_smi.jobcnt_lock);
	return 1;
}


/* Wait for completion of target jobs or local job requests, or both. */
int MPID_SMI_Complete_target_jobs (win, rank)
	MPID_Win	* win;
	int			rank;
{
	int i, completed_jobs = 0;
	MPI_Request req;
	MPI_Status status;

	MPID_STAT_ENTRY(sside_cmplt_tgt_jobs);

	/* XXX individual counters not yet implemented - would that make sense
	   performance-wise? Now, we always check for *all* jobs to complete. */

	/* First, check for completed non-blocking data transfers. Then, check
	   if everything has been effectively processed at the target process. */
	while ((req = (MPI_Request)MPID_FIFO_pop(win->devinfo.w_smi.putaccu_req_fifo)) != NULL) {
		MPI_Wait(&req, &status);
	}
	while ((req = (MPI_Request)MPID_FIFO_pop(win->devinfo.w_smi.get_req_fifo)) != NULL) {
		MPI_Wait(&req, &status);
		/* XXX we don't know the target rank from request! For now, this
		   doesn't matter.*/
		MPID_SMI_Remove_target_job (win, -1);
	}

	while (win->devinfo.w_smi.ogn_jobcnt > completed_jobs) {
		/* count all jobs that are completed by now */
		for (completed_jobs = 0, i = 0; i < win->devinfo.w_smi.lsize; i++)
			completed_jobs += win->devinfo.w_smi.ogn_cmpltcnt[i];
		
		MPID_DeviceCheck (MPID_NOTBLOCKING);
	}
	
	/* prepare for next round */
	memset ((void *)win->devinfo.w_smi.ogn_cmpltcnt, 0, 
			win->devinfo.w_smi.lsize*sizeof(int));
	win->devinfo.w_smi.ogn_jobcnt = 0;

	MPID_STAT_EXIT(sside_cmplt_tgt_jobs);
	return 1;
}


int MPID_SMI_Complete_job_requests (win)
	MPID_Win	* win;
{
	MPID_STAT_ENTRY(sside_cmplt_jobreqs);

	MPID_SMI_Os_delayed_flush (win, -1);
	while (win->devinfo.w_smi.tgt_jobcnt > 0) {
		/* The only case why we might need to spin here is
		   processing of 'get' requests via rndv */
		MPID_DeviceCheck (MPID_NOTBLOCKING);
	}

	MPID_STAT_EXIT(sside_cmplt_jobreqs);
	return 1;
}


int MPID_SMI_Complete_all_jobs (win)
	MPID_Win	* win;
{
	MPID_SMI_Complete_job_requests (win);
	MPID_SMI_Complete_target_jobs (win, 0);
	
	return 1;
}

#else /* USE_JOBMNGMT */
/* for testing: disable job management */

int MPID_SMI_Add_target_job (win, target_rank)
	MPID_Win	* win;
    int target_rank;
{
	MPID_STAT_COUNT (sside_add_tgt_job);
	return 1;
}

int MPID_SMI_Remove_target_job (win, target_rank)
	MPID_Win	* win;
    int target_rank;
{
	MPID_STAT_COUNT (sside_rm_tgt_job);
	return 1;
}

int MPID_SMI_Add_job_request (win, origin_rank)
	MPID_Win	* win;
    int origin_rank;
{
	MPID_STAT_COUNT (sside_add_jobreq);
	return 1;
}

int MPID_SMI_Job_request_completed (win, origin_rank, is_get)
	MPID_Win	* win;
    int origin_rank, is_get;
{
	MPID_STAT_COUNT (sside_rm_jobreq);
	return 1;
}

int MPID_SMI_Complete_target_jobs (win, rank)
	MPID_Win	* win;
	int			rank;
{
	MPID_STAT_ENTRY(sside_cmplt_tgt_jobs);
	MPID_STAT_EXIT(sside_cmplt_tgt_jobs);
	return 1;
}

int MPID_SMI_Complete_job_requests (win)
	MPID_Win	* win;
{
	MPID_STAT_ENTRY(sside_cmplt_jobreqs);
	MPID_STAT_EXIT(sside_cmplt_jobreqs);
	return 1;
}

int MPID_SMI_Complete_all_jobs (win)
	MPID_Win	* win;
{
	return 1;
}



#endif














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
