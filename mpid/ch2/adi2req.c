/*
 *  $Id$
 *
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * This file handles initalization and free'ing of requests
 *
 */

#include "mpid.h"
#include "mpiddev.h"
#include "reqalloc.h"
#include "sendq.h"	/* For MPIR_FORGET_SEND */


#ifdef STDC_HEADERS
/* Prototype for memset() */
#include <string.h>
#elif defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_MEMORY_H)
#include <memory.h>
#endif


void MPID_Request_init (MPI_Request rq, MPIR_OPTYPE rq_type)
{
    MPID_Device *dev = NULL;

    rq->chandle.handle_type = rq_type;
    rq->chandle.ref_count   = 1;
    MPIR_SET_COOKIE(&(rq->chandle), MPIR_REQUEST_COOKIE);
    

    /* For persistent communication, the device may need to perform
       additional actions. */
    switch (rq->handle_type) {
    case MPIR_SEND:
	break;
    case MPIR_RECV:
	break;
    case MPIR_PERSISTENT_SEND:
	dev = MPID_devset->dev[((rq->persistent_shandle.perm_comm)->lrank_to_grank)
			      [rq->persistent_shandle.perm_dest]];
	
	if (dev->persistent_init != NULL)
	  MPID_Device_call_persistent_init (rq, dev);
	break;
    case MPIR_PERSISTENT_RECV: 
	if (rq->persistent_rhandle.perm_source >= 0) {
	    dev = MPID_devset->dev[rq->persistent_rhandle.perm_source];
	} else {
	    /* For a single device, we can use the related function, 
	       but what to do for multiple available devices? Multiple
	       persistent initialization? */
	    if (MPID_devset->ndev == 1) {
		dev = MPID_devset->dev_list;
	    }
	}

	if (dev != NULL && dev->persistent_init != NULL)
	  MPID_Device_call_persistent_init (rq, dev);
	break;
    }
    
    return;
}

void MPID_Request_free (MPI_Request request )
{
    MPID_Device *dev = NULL;
    MPI_Request rq = request ; /* MPID_devset->req_pending; */
    int mpi_errno = MPI_SUCCESS;
    
    switch (rq->handle_type) {
    case MPIR_SEND:
	if (MPID_SendIcomplete( rq, &mpi_errno )) {
	    MPIR_FORGET_SEND( &rq->shandle );
	    MPID_Send_free( rq );
	    /* MPID_devset->req_pending = 0;*/
	    rq = 0;
	}
	break;
    case MPIR_RECV:
	if (MPID_RecvIcomplete( rq, (MPI_Status *)0, &mpi_errno )) {
	    MPID_Recv_free( rq );
	    /* MPID_devset->req_pending = 0; */
	    rq = 0;
	}
	break;
    case MPIR_PERSISTENT_SEND:
	if (rq->persistent_shandle.active) {
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
			"Unimplemented operation - active persistent send free" );
	} else {
  	    dev = MPID_devset->dev[((rq->persistent_shandle.perm_comm)->lrank_to_grank)
				  [rq->persistent_shandle.perm_dest]];
	    if (dev->persistent_free != NULL)
	      MPID_Device_call_persistent_free (rq, dev);
	    
	    MPID_PSend_free( rq );
	}
	break;
    case MPIR_PERSISTENT_RECV:
	if (rq->persistent_rhandle.active) {
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
			"Unimplemented operation - active persistent recv free" );
	} else {
	    if (rq->persistent_rhandle.perm_source >= 0) {
		dev = MPID_devset->dev[rq->persistent_rhandle.perm_source];
	    } else {
		/* For a single device, we can use the related function, 
		   but what to do for multiple available devices? Multiple
		   persistent initialization? */
		if (MPID_devset->ndev == 1) {
		    dev = MPID_devset->dev_list;
		}
	    }
	    
	    if (dev != NULL && dev->persistent_free != NULL)
	      MPID_Device_call_persistent_free (rq, dev);
	}
	break;
    }

    MPID_DeviceCheck( MPID_NOTBLOCKING );
    /* 
     * If we couldn't complete it, decrement it's reference count
     * and forget about it.  This requires that the device detect
     * orphaned requests when they do complete, and process them
     * independent of any wait/test.
     */
    /*if (MPID_devset->req_pending) {*/
    if (rq) {
	rq->chandle.ref_count--;
/*	PRINTF( "Setting ref count to %d for %x\n", 
		rq->chandle.ref_count, (long)rq ); */
	/* MPID_devset->req_pending = 0; */
    }
}

