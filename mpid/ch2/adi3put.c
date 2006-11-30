/* $Id$

   Wrapper for device-specific execution of MPI_Put. */

#include <stdio.h>
#include <stdlib.h>

#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpidmpi.h"
#include "sside_protocol.h"
#include "adi3types.h"



int MPID_Put_contig (origin_buf, n, target_offset, target_grank, win, 
					 local_flag, target_flag)
	void 			* origin_buf;
	int 			n;
	MPI_Aint 		target_offset;
	int 			target_grank; 
	MPID_Win 		* win;
	volatile int 	* local_flag;
	MPI_Aint 		target_flag;
{
	MPID_Device *dev = MPID_devset->dev[target_grank];

	if (dev->sside != NULL)
		return dev->sside->Put_contig (origin_buf, n, target_offset, target_grank, win, 
									   local_flag, target_flag);
	else
		return MPI_ERR_UNSUPPORTED_OPERATION;
}


int MPID_Put_sametype (origin_buf, n, dtype, target_offset, target_grank, 
					   win, local_flag, target_flag)
	void 			* origin_buf;
	int 			n;
	MPID_Datatype	* dtype;
	MPI_Aint 		target_offset;
	int 			target_grank;
	MPID_Win 		* win;
	volatile int 	* local_flag;
	MPI_Aint 		target_flag;
{
	MPID_Device *dev = MPID_devset->dev[target_grank];

	if (dev->sside != NULL)
		return dev->sside->Put_sametype (origin_buf, n, dtype, target_offset, target_grank, 
										 win, local_flag, target_flag);
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
