/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpid_debug.h"
#include "mpimem.h"
#include "queue.h"

/*
 * This file contains the routines to handle canceling a message
 *
 * Note: for now, cancel will probably only work on unmatched receives.
 * However, this code provides the hooks for supporting more complete
 * cancel implementations.
 */

void MPID_SendCancel( request, error_code )
MPI_Request request;
int         *error_code;
{ 
    MPIR_SHANDLE *shandle = &request->shandle;
    MPID_Device *dev;
    int  (*fcn) (MPIR_SHANDLE *);

    DEBUG_PRINT_MSG("S Starting SendCancel");

    shandle->is_cancelled = 0;
    shandle->cancel_complete = 0;

    /* the rank of the communication partner in MPI_COMM_ALL has been saved in shandle->partner_grank; we
       can use it here to decide which cancel function to call */

    dev = MPID_devset->dev[shandle->partner_grank];

    fcn = dev->cancel;
    if (fcn != NULL) {
	/* send the request_to_cancel message */
	*error_code = MPID_Device_call_cancel(shandle, dev);
	
	if (*error_code == MPI_SUCCESS) {  
	    /* wait for the answer from the partner */
	    while (!shandle->cancel_complete) {  
		MPID_DeviceCheck( MPID_NOTBLOCKING );
	    }
	    
	    if (shandle->is_cancelled) { 
		/* message was canceled, clean up */
		if (shandle->finish)
		    (shandle->finish)(shandle);  
		if (shandle->handle_type == MPIR_PERSISTENT_SEND) {
		    MPIR_PSHANDLE *pshandle = (MPIR_PSHANDLE *)request;
		    pshandle->active = 0; 
		}
	    }
	}  
    }
    
    DEBUG_PRINT_MSG("E Exiting SendCancel");
}  

 
void MPID_RecvCancel( request, error_code )
MPI_Request request;
int         *error_code;
{

    MPIR_RHANDLE *rhandle = &request->rhandle;

    DEBUG_PRINT_MSG("S Starting RecvCancel"); 

    /* First, try to find in pending receives */
    if (MPID_Dequeue_posted( rhandle ) == MPI_SUCCESS) {
	/* Mark the request as cancelled */
	rhandle->s.MPI_TAG = MPIR_MSG_CANCELLED;
	/* Mark it as complete */
	rhandle->is_complete = 1;
	/* Should we call finish to free any space?  cancel? */
	if (rhandle->finish)
	    (rhandle->finish)( rhandle ); 
	/* Note that the request is still active until we complete it with
	   a wait/test operation */
    }
    if (rhandle->handle_type == MPIR_PERSISTENT_RECV) {
	MPIR_PRHANDLE *prhandle = (MPIR_PRHANDLE *)request;
	prhandle->active = 0; 
    }
    /* Mark the request as not cancelled */
    /* tag is already set >= 0 as part of receive */
    /* rhandle->s.tag = 0; */
    /* What to do about an inactive persistent receive? */

    /* In the case of a partly completed rendezvous receive, we might
       want to do something */
    *error_code = MPI_SUCCESS;
    DEBUG_PRINT_MSG("E Exiting RecvCancel");
}
