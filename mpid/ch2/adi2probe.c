/*
 *  $Id$
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#ifdef MPID_DEBUG_ALL
#include "mpiddev.h"
#endif
#include "mpid_debug.h"
#include "../util/queue.h"

void MPID_Iprobe(struct MPIR_COMMUNICATOR *comm_ptr, int tag, int context_id,
	int src_lrank, int *found, int *error_code, MPI_Status *status)
{
	MPIR_RHANDLE *rhandle;

	DEBUG_PRINT_MSG("Entering Iprobe");
	DEBUG_PRINT_ARGS("Iprobe");
	/* At this time, we check to see if the message has already been received */
	MPID_Search_unexpected_queue( src_lrank, tag, context_id, 0, &rhandle );
	if (!rhandle) {
		/* If nothing there, check for incoming messages.... */
		MPID_DeviceCheck( MPID_NOTBLOCKING );
		MPID_Search_unexpected_queue( src_lrank, tag, context_id, 0, &rhandle );
	}
	if (rhandle) {
		*found  = 1;
		if (status)
			*status = rhandle->s;
		DEBUG_PRINT_MSG(" Iprobe found msg");
	}
	else {
		*found = 0;
		DEBUG_PRINT_MSG(" Iprobe did not find msg");
	}
	DEBUG_PRINT_MSG("Exiting Iprobe");
}

void MPID_Probe(struct MPIR_COMMUNICATOR *comm_ptr, int tag, int context_id,
	int src_lrank, int *error_code, MPI_Status *status)
{
	int found;

	*error_code = 0;
	DEBUG_PRINT_MSG("Entering Probe");
	while (1) {
		/* Wait for a message */
		MPID_Iprobe(comm_ptr, tag, context_id, src_lrank, &found, error_code,
				status);
		if (found || *error_code)
			break;
	}
	DEBUG_PRINT_MSG("Exiting Probe");
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
 * vim:tw=0:ts=4:wm=0:sw=4:
 */
