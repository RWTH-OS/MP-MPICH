/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */
#define _DEBUG_EXTERN_REC
#include "mydebug.h"

#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "usockpackets.h"
#include "usockdebug.h"
#include "queue.h"
#include "channel.h"

/*
 * This file contains the routines to handle canceling a message
 *
 */

/* 
 * CancelSend 
 * This is fairly hard.  We need to send a "please_cancel_send", 
 * which, if the message is found in the unexpected queue, removes it.
 * However, if the message is being received at the "same" moment, the
 * ok_to_send and cancel_send messages could cross.  To handle this, the
 * receiver must ack the cancel_send message (making the success of the
 * cancel non-local).  There are even more complex protocols, but we won't
 * bother.
 * 
 * Don't forget to update MPID_n_pending as needed.
 */

/* This routine is called from MPI_Cancel.  Its purpose is to send an    
   anti-send packet to the calling process's partner.  If successful,
   the error_code will return MPI_SUCCESS, otherwise the error_code will
   return MPI_ERR_OTHER */
int MPID_USOCK_SendCancelPacket( shandle )
MPIR_SHANDLE *shandle;

{
    DSECTION("MPID_USOCK_SendCancelPacket");
    MPID_PKT_ANTI_SEND_T cancel_pkt;
    MPID_PKT_ANTI_SEND_T *pkt = &cancel_pkt;

    MPID_USOCK_Data_global_type *global_data;
    
    DSECTENTRYPOINT;

    global_data = (MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data;
    
    pkt->mode           = MPID_PKT_ANTI_SEND; 
    pkt->src_comm_lrank = global_data->MyWorldRank;
/*	pkt->to      = shandle->partner;*/
    MPID_AINT_SET(pkt->send_id,shandle); 
    
    MPID_USOCK_DEBUG_PRINT_BASIC_SEND_PKT("S Sending anti-send message\n", pkt);
    MPID_PKT_PACK( pkt, sizeof(*pkt), pkt->to );
    MPID_SendControl( (MPID_PKT_T*)pkt, sizeof(MPID_PKT_ANTI_SEND_T), shandle->partner );

    DSECTLEAVE
	return 0;
}


/* This routine is called when a process receives an anti-send pkt.  Its 
   purpose is to search for the request found in the pkt in the unexpected
   queue.  If found, set the pkt.cancel to 1, otherwise, set pkt.cancel to 
   0.  Send this information back in an anti-send-ok pkt. */
void MPID_USOCK_SendCancelOkPacket( in_pkt, from )
MPID_PKT_T  *in_pkt;
int  from;

{ 
    DSECTION("MPID_USOCK_SendCancelOkPacket");
    MPID_PKT_ANTI_SEND_T *pkt = (MPID_PKT_ANTI_SEND_T *)in_pkt;
    MPID_PKT_ANTI_SEND_T new_pkt; 
    int error_code;
    int found = 0;
    MPIR_SHANDLE *shandle=0;
    MPIR_RHANDLE *rhandle;

    MPID_USOCK_Data_global_type *global_data;
    
    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "SendCancelOkPacket");

    global_data = (MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data;

    /* A cancel packet is a little larger than the basic packet size and 
       may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1,
		     sizeof(MPID_PKT_ANTI_SEND_T) - sizeof(MPID_PKT_HEAD_T),
		     from ); 
    
    MPID_AINT_GET(shandle, pkt->send_id); 
    
    /* Look for request, if found, delete it */
    error_code = MPID_Search_unexpected_for_request(shandle, &rhandle, &found);
    
    if ( (error_code != MPI_SUCCESS) || (found == 0) || !rhandle->cancel ) {
	MPID_USOCK_DEBUG_PRINT_MSG2("Could not find cancel function, shandle == %x",shandle);
	new_pkt.cancel = 0;
    } else {
	MPID_USOCK_DEBUG_PRINT_MSG("Calling rhandle->cancel");
	(rhandle->cancel)(rhandle);  
	new_pkt.cancel = 1;
    }
    
    new_pkt.mode = MPID_PKT_ANTI_SEND_OK;
    new_pkt.src_comm_lrank = global_data->MyWorldRank;
    /*new_pkt.to = from; */
    new_pkt.send_id = pkt->send_id;  
    
    MPID_USOCK_DEBUG_PRINT_BASIC_SEND_PKT("S Sending anti_send_ok message\n", &new_pkt);
    MPID_PKT_PACK( &new_pkt, sizeof(new_pkt), from ); 
    MPID_SendControl( (MPID_PKT_T*)&new_pkt, sizeof(MPID_PKT_ANTI_SEND_T), from ); 
   
    DSECTLEAVE;
}

/* This routine is called when a process receives an anti-send-ok packet.
   If pkt->cancel = 1, then set the request found in the pkt as
   cancelled and complete.  If pkt->cancel = 0, do nothing. */
void MPID_USOCK_RecvCancelOkPacket( in_pkt, from )
MPID_PKT_T  *in_pkt;
int  from;

{ 
    DSECTION("MPID_USOCK_RecvCancelOkPacket");
    MPID_PKT_ANTI_SEND_T *pkt = (MPID_PKT_ANTI_SEND_T *)in_pkt;
    MPIR_SHANDLE *shandle=0;
    
    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "RecvCancelOkPacket");

    /* A cancel packet is a little larger than the basic packet size and 
       may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1,
		     sizeof(MPID_PKT_ANTI_SEND_T) - sizeof(MPID_PKT_HEAD_T),
		     from );
    
    MPID_AINT_GET(shandle, pkt->send_id);
    
    MPID_USOCK_DEBUG_PRINT_BASIC_SEND_PKT("R Receive anti-send ok message\n", pkt);  
    
    if (pkt->cancel) {   /* begin if pkt->cancel */
	/* Mark the request as cancelled */
	shandle->s.MPI_TAG = MPIR_MSG_CANCELLED; 
	/* Mark the request as complete */
	shandle->is_complete = 1;
	shandle->is_cancelled = 1;
	MPID_n_pending--;  
	if(shandle->cancel)
	    (shandle->cancel)(shandle);
	MPID_USOCK_DEBUG_PRINT_MSG("Request has been successfully cancelled");
    }   /* end if pkt->cancel */
    else {
	shandle->is_cancelled = 0;
	MPID_USOCK_DEBUG_PRINT_MSG("Unable to cancel request");
    }
    
    shandle->s.MPI_ERROR = MPI_SUCCESS;
    shandle->cancel_complete = 1;
    
    DSECTLEAVE;
}
