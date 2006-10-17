/*
 *  $Id$
 *
 */

#include "mpid.h"
#include "shmemdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "shmempackets.h"
#include "shmemdebug.h"
#include "shmemcommon.h"
#include "../util/queue.h"

/*
 * This file contains the routines to handle canceling a message
 * Special for the ch_smi device. Each device has its own version of this
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
int MPID_SHMEM_SendCancelPacket( shandle )
MPIR_SHANDLE *shandle;
{
    MPID_PKT_ANTI_SEND_T *pkt;
    int pkt_len;
    int* tmpzero = 0;

    pkt_len         = sizeof(MPID_PKT_ANTI_SEND);

    MPID_SHMEM_DEBUG_PRINT_MSG("S Getting a packet");
    pkt = (MPID_PKT_ANTI_SEND_T *)MPID_SHMEM_GetSendPkt(0);

    pkt->mode       = MPID_PKT_ANTI_SEND; 
    pkt->context_id = 0;
    pkt->lrank      = MPID_SHMEM_rank;
    pkt->to         = shandle->partner;
    pkt->seqnum     = pkt_len;
    pkt->owner      = MPID_SHMEM_rank;
    pkt->src        = MPID_SHMEM_rank;
    pkt->tag        = 0;
    pkt->len        = 0;
    pkt->cancel     = 0;
    MPID_AINT_SET(pkt->send_id, shandle);
    MPID_AINT_SET(pkt->recv_id, tmpzero);

    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending cancel message ",pkt);
    MPID_SHMEM_SendControl( (MPID_PKT_T*)pkt, pkt_len, shandle->partner);
    
    MPID_n_pending++; 
    return 0;
}  


/* This routine is called when a process receives an anti-send pkt.  Its 
   purpose is to search for the request found in the pkt in the unexpected
   queue.  If found, set the pkt.cancel to 1, otherwise, set pkt.cancel to 
   0.  Send this information back in an anti-send-ok pkt. */
void MPID_SHMEM_SendCancelOkPacket( in_pkt, from )
MPID_PKT_T  *in_pkt;
int  from;
{  
    MPID_PKT_ANTI_SEND_T *pkt;
    MPIR_SHANDLE *shandle = 0;
    MPIR_RHANDLE *rhandle;
    MPID_Aint remote_send_id;
    int pkt_len, is_canceled, error_code, found = 0;
    int* tmpzero=0;   
         
    MPID_AINT_GET(shandle, ((MPID_PKT_ANTI_SEND_T *)in_pkt)->send_id); 
    
    /* Look for request, if found, delete it */
    error_code = MPID_Search_unexpected_for_request(shandle, &rhandle, &found);
    if ( (error_code != MPI_SUCCESS) || (found == 0) || !rhandle->cancel ) {
	MPID_SHMEM_DEBUG_PRINT_MSG2("Could not find cancel function, shandle == %x",shandle);
	is_canceled = 0;
    } else {
	MPID_SHMEM_DEBUG_PRINT_MSG("Calling rhandle->cancel");
	(rhandle->cancel)(rhandle);  
	is_canceled = 1;
    }

    /* remember send_id so that we can free the recv packet */
    MPID_AINT_SET(remote_send_id, shandle);
    MPID_SHMEM_FreeRecvPkt ((MPID_PKT_T *)in_pkt);

    MPID_SHMEM_DEBUG_PRINT_MSG("S Getting a packet");
    pkt = (MPID_PKT_ANTI_SEND_T *)MPID_SHMEM_GetSendPkt(0);

    pkt_len         = sizeof(MPID_PKT_ANTI_SEND);
    pkt->mode       = MPID_PKT_ANTI_SEND_OK; 
    pkt->context_id = 0;
    pkt->lrank      = MPID_SHMEM_rank;
    pkt->to         = from;
    pkt->seqnum     = pkt_len;
    pkt->owner      = MPID_SHMEM_rank;
    pkt->tag        = 0;
    pkt->len        = 0;
    pkt->cancel     = is_canceled;
    pkt->send_id    = remote_send_id;
    MPID_AINT_SET(pkt->recv_id, tmpzero);

    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending cancel_send_ok message", pkt);
    MPID_SHMEM_SendControl((MPID_PKT_T*)pkt, pkt_len, from );

    return;
}

/* This routine is called when a process receives an anti-send-ok packet.
   If pkt->cancel = 1, then set the request found in the pkt as
   cancelled and complete.  If pkt->cancel = 0, do nothing. */
void MPID_SHMEM_RecvCancelOkPacket( in_pkt, from )
MPID_PKT_T *in_pkt;
int  from;
{
   MPID_PKT_ANTI_SEND_T *pkt = (MPID_PKT_ANTI_SEND_T *)in_pkt;
   MPIR_SHANDLE *shandle=0;
   
   MPID_SHMEM_DEBUG_PRINT_SEND_PKT("R Receive anti-send ok message\n", pkt);  
   MPID_AINT_GET(shandle, pkt->send_id);
   
   if (pkt->cancel) { 
       /* Mark the request as cancelled */
       shandle->s.MPI_TAG = MPIR_MSG_CANCELLED; 
       /* Mark the request as complete */
       shandle->is_complete = 1;
       shandle->is_cancelled = 1;
       if (shandle->cancel)
	   (shandle->cancel)(shandle);

       MPID_SHMEM_DEBUG_PRINT_MSG("Request has been successfully cancelled");
   } else {
       shandle->is_cancelled = 0;

       MPID_SHMEM_DEBUG_PRINT_MSG("Unable to cancel request");
   }
   
   shandle->s.MPI_ERROR = MPI_SUCCESS;
   shandle->cancel_complete = 1;
   
   /* this is for the cancel msg */
   MPID_n_pending--;  
   
   return;
} 
