/* $Id$

   Wrapper for device-specific execution of window synchronization. */

#include <stdio.h>
#include <stdlib.h>
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpidmpi.h"
#include "sside_protocol.h"


int MPID_Win_lock (win, rank, type)
	MPID_Win 	* win;
	int 		rank;
	int			type;
{
	MPID_Device *dev = MPID_devset->dev[rank];

	if (dev->sside != NULL)
		return dev->sside->Win_lock (win, rank, type);
	else
		return MPI_ERR_UNSUPPORTED_OPERATION;
}


int MPID_Win_unlock (win, rank)
	MPID_Win 	* win;
	int 		rank;
{
	MPID_Device *dev = MPID_devset->dev[rank];

	if (dev->sside != NULL)
		return dev->sside->Win_unlock (win, rank);
	else
		return MPI_ERR_UNSUPPORTED_OPERATION;
}


int MPID_Win_sync (type, win, group, assert)
	int 		type;
	MPID_Win	*win;
	MPID_Group 	*group;
	int			assert;
{
	MPID_Device *dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->sside != NULL)
		return dev->sside->Win_sync (type, win, group, assert);
	else
		return MPI_ERR_UNSUPPORTED_OPERATION;
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
