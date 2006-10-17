/* $Id: smiwinlock.c,v 1.8.8.1 2004/12/15 13:45:21 boris Exp $ */

#include <stdlib.h>

#include "mpi.h"
#include "smi.h"

#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"


/*
 * MPID_SMI_Win_lock		locks a window
 *
 * input parameters:
 *	win		window to be locked
 *	rank	rank of window to be locked
 *
 * output parameters:
 *	<none>
 * 
 * return value:
 *   1      lock acquired
 *   0      error (window has no locks?!)
 */
int MPID_SMI_Win_lock (win, rank, type)
	MPID_Win 	* win;
	int 		rank;
	int			type;	
{
	if (!win->devinfo.w_smi.winlocks || 
						win->devinfo.w_smi.winlocks[rank] == -1)
		/* lock is not initialised - error! */
		return 0;

	switch (type) {
	case MPI_LOCK_SHARED:
		return MPID_SMI_Mutex_readlock (win->devinfo.w_smi.winlocks[rank]);
		break;
	case MPI_LOCK_EXCLUSIVE:
		return MPID_SMI_Mutex_lock (win->devinfo.w_smi.winlocks[rank]);
		break;
	}

	return 0;
}


/*
 * MPID_SMI_Win_unlock		unlocks a window after all remote and local jobs are complete.
 *
 * input parameters:
 *	win		window to be locked
 *	rank	rank of window to be locked
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *   1      lock released
 *   0      error (window has no locks?!)
 */
int MPID_SMI_Win_unlock (win, rank)
	MPID_Win 	* win;
	int 		rank;
{
	MPID_SMI_Complete_all_jobs (win);

	if (!win->devinfo.w_smi.winlocks || 
						win->devinfo.w_smi.winlocks[rank] == -1)
		/* lock is not initialised - error! */
		return 0;
	return MPID_SMI_Mutex_unlock (win->devinfo.w_smi.winlocks[rank]);
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
