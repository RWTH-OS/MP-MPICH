/* $Id: adi3rhc.c,v 1.2 2002/03/28 16:30:58 joachim Exp $

   Remote-Handler-Call (vectorized) for emulated remote memory access and synchronization. */

#include <stdio.h>
#include <stdlib.h>
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpidmpi.h"
#include "sside_protocol.h"
#include "adi3types.h"


int MPID_Rhcv (grank, win, id, vector, count, local_flag)
	int 				grank; 
	MPID_Win 			* win;
	MPID_Handler_id 	id;
	const struct iovec	vector[];
	int 				count;
	int 				* local_flag;
{
	MPID_Device * dev = MPID_devset->dev[grank]; 

	if (dev->sside != NULL)
		return dev->sside->Rhcv (grank, win, id, vector, count, local_flag);
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
