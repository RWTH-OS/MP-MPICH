/* $Id$ */

#ifndef MPID_SMI_SSIDE_JOB_MANAGEMENT_H
#define MPID_SMI_SSIDE_JOB_MANAGEMENT_H

#include <stdlib.h>
#include <stdio.h>

#include "mpichconf.h"
#include "mpiimpl.h"
#include "adi3types.h"
#include "smidef.h"


#if 0
	/* XXX old API */
int MPID_SMI_Add_remote_job (MPID_Win * win);
int MPID_SMI_Add_local_job (MPID_Win * win);

int MPID_SMI_Remove_remote_job (MPID_Win * win);
int MPID_SMI_Remove_local_job (MPID_Win * win);

int MPID_SMI_Wait_remote_jobs (MPID_Win * win, int rank);
int MPID_SMI_Wait_local_jobs (MPID_Win * win);
int MPID_SMI_Wait_all_jobs (MPID_Win * win);
#else
int MPID_SMI_Add_target_job (MPID_Win *win, int target_rank);
int MPID_SMI_Remove_target_job (MPID_Win *win, int target_rank);

int MPID_SMI_Add_job_request (MPID_Win *win, int origin_rank);
int MPID_SMI_Job_request_completed (MPID_Win *win, int origin_rank, int is_get);

int MPID_SMI_Complete_target_jobs (MPID_Win * win, int rank);
int MPID_SMI_Complete_job_requests (MPID_Win * win);
int MPID_SMI_Complete_all_jobs (MPID_Win * win);
#endif

#if 0
/* XXX are these macros below needed at all? If a lock is required,
   it is acquired inside the called function! */
#ifdef MPID_USE_DEVTHREADS
#define MPID_SMI_GET_JOB_LOCK(win) \
				! MPID_SMI_LOCK (&(win)->devinfo.w_smi.job_lock)
#define MPID_SMI_TRYGET_JOB_LOCK(win) \
				! MPID_SMI_TRYLOCK (&(win)->devinfo.w_smi.job_lock)
#define MPID_SMI_RELEASE_JOB_LOCK(win) \
				! MPID_SMI_UNLOCK (&(win)->devinfo.w_smi.job_lock)
#else
#define MPID_SMI_GET_JOB_LOCK(win) 1
#define MPID_SMI_TRYGET_JOB_LOCK(win) 1
#define MPID_SMI_RELEASE_JOB_LOCK(win) 1
#endif

#if USE_JOBMNGMT
#define MPID_SMI_ADD_LOCAL_JOB(win) { \
				MPID_SMI_GET_JOB_LOCK (win); \
				MPID_SMI_Add_local_job (win); \
				MPID_SMI_RELEASE_JOB_LOCK (win); }

#define MPID_SMI_ADD_REMOTE_JOB(win) { \
				MPID_SMI_GET_JOB_LOCK (win); \
				MPID_SMI_Add_remote_job (win); \
				MPID_SMI_RELEASE_JOB_LOCK (win); }

#ifdef MPID_USE_DEVTHREADS
#define MPID_SMI_TRYADD_LOCAL_JOB(win) ( \
				MPID_SMI_TRYGET_JOB_LOCK (win) ? (\
				MPID_SMI_Add_local_job (win), \
				MPID_SMI_RELEASE_JOB_LOCK (win), 1 ) : 0 )
#else
#define MPID_SMI_TRYADD_LOCAL_JOB(win) ( \
				MPID_SMI_Add_local_job (win), 1)
#endif

#define MPID_SMI_REMOVE_LOCAL_JOB(win) { \
				MPID_SMI_Remove_local_job (win); }

#define MPID_SMI_REMOVE_REMOTE_JOB(win) { \
				MPID_SMI_Remove_remote_job (win); }

#else
/* XXX testing: job management is to "heavy", need to do without! */
#define MPID_SMI_ADD_TARGET_JOB(win)
#define MPID_SMI_RM_TARGET_JOB(win)
#define MPID_SMI_TRYADD_LOCAL_JOB(win) 1
#define MPID_SMI_REMOVE_LOCAL_JOB(win)
#define MPID_SMI_REMOVE_REMOTE_JOB(win)
#endif

#endif /* 0 */










#endif	/* MPID_SMI_SSIDE_JOB_MANAGEMENT_H */


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
