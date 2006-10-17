/* $Id: adi3win.c,v 1.2 2002/03/28 16:30:58 joachim Exp $
   
   Wrapper for device-specific Window-creation.  */

#include <stdio.h>
#include <stdlib.h>
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpidmpi.h"
#include "sside_protocol.h"
#include "adi3types.h"





MPID_Win *MPID_Win_create (base, size, disp_unit, info, comm)
	void * 		base;
	size_t 		size;
	int 		disp_unit;
	MPID_Info 	*info;
	MPID_Comm 	*comm;
{
	MPID_Device *dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->sside != NULL)
		return dev->sside->Win_create (base, size, disp_unit, info, comm);
	else 
		return NULL;
}


int MPID_Win_free (win)
	MPID_Win *win;
{
	MPID_Device *dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->sside != NULL)
		return dev->sside->Win_free (win);
	else
		return MPI_ERR_UNSUPPORTED_OPERATION;
}


int MPID_Win_incr (win, incr)
	MPID_Win	*win;
	int 		incr;
{
	MPID_Device *dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->sside != NULL)
		return dev->sside->Win_incr (win, incr);
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
