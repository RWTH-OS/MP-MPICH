/* $Id$ */

#include "mpi.h"
#include "mpid.h"
#include "../ch2/packets.h"
#include "req.h"
#include "mpiddev.h"

extern int MPID_n_pending;

/*
 * this works the following way:
 * 1. complete sending of pending message that is to be cancelled
 * 2. send cancel message
 * 3. wait for confirm message
 */
int MPID_Gateway_SendCancelPacket( shandle )
MPIR_SHANDLE *shandle;
{
    int err;
    MPI_Request req;
    MPID_Msgrep_t msgrep;
    Meta_Header *gw_msg;
    int dest_comm_all_rank;          /* rank in MPI_COMM_ALL of process that is receiver of message to be cancelled */
    MPID_PKT_ANTI_SEND_T cancel_msg, /* cancel message that we send */
	confirm_msg;                 /* confirm message that we receive to know if message was cancelled */
    int router_comm_host_rank,       /* rank in MPI_COMM_HOST of exporting router */
	router_comm_all_rank;        /* rank in MPI_COMM_ALL of exporting router */
    MPI_Status status;
    int tmpzero = 0;

    msgrep = MPID_MSGREP_UNKNOWN;
    
    /* complete the send operation */
    MPID_SendComplete( (MPI_Request)shandle, &err );
    
    /* initialize the cancel message */
    cancel_msg.mode       = MPID_PKT_ANTI_SEND; 
    cancel_msg.context_id = 0;
    cancel_msg.lrank      = 0;
    cancel_msg.to         = shandle->partner;
    cancel_msg.src        = MPID_MyAllRank;
    cancel_msg.seqnum     = sizeof( MPID_PKT_ANTI_SEND );
    cancel_msg.tag        = 0;
    cancel_msg.len        = 0;
    cancel_msg.cancel     = 0;
    MPID_AINT_SET(cancel_msg.send_id, shandle);
    MPID_AINT_SET(cancel_msg.recv_id, tmpzero);


    dest_comm_all_rank = shandle->partner_grank; /* saved there in MPI_Isend(), MPI_Issend(), MPI_Send_init(), ... */
    
    /* this must be fixed to make cancel work! */
    /* router_comm_host_rank = MPID_Gateway_real_dev->grank_to_devlrank[MPIR_meta_cfg.granks_to_router[dest_comm_all_rank]]; */
    
    gw_msg = MPID_Gateway_Wrap_Msg( &cancel_msg, shandle->src_lrank, dest_comm_all_rank,
				    sizeof( MPID_PKT_ANTI_SEND_T ), 0, 0, CANCEL, msgrep, shandle->msgid );
    if (!gw_msg) {
	return MPI_ERR_INTERN;

    }

    /* send cancel message */
    MPID_SendContig( gw_msg, sizeof(MPID_PKT_ANTI_SEND_T) + sizeof(Meta_Header), MPID_MyHostRank,
		     MPIR_MPIMSG_TAG, MPIR_HOST_PT2PT_CONTEXT,  MPIR_meta_cfg.granks_to_router[shandle->partner], msgrep, &err );
    

    /* 
     * receive confirm message; the router that imports the confirm message fakes the source rank with its
     * own rank by changing the meta header, therefore that router is the source of the confirm message for us here
     * I hope the confirm message doesn't get confused with other messages that are sent to us from that router;
     * all other messages should have another process as source, this is the reason for the faking of the source rank
     */
    router_comm_all_rank = MPIR_meta_cfg.metahost_firstrank + router_comm_host_rank;
    MPI_Recv( &confirm_msg, sizeof( MPID_PKT_ANTI_SEND_T ), MPI_BYTE,
	      router_comm_all_rank, MPIR_CANCEL_CONFIRM_TAG, MPI_COMM_ALL, &status );

    if( confirm_msg.cancel ) {
	/* message was successfully cancelled */
	shandle->s.MPI_TAG = MPIR_MSG_CANCELLED; 
	/* Mark the request as complete */
	shandle->is_complete = 1;
	shandle->is_cancelled = 1;
	
	DEBUG_PRINT_MSG("Request has been successfully cancelled");
    }
    else {
	/* message could not be cancelled */
	shandle->is_cancelled = 0;
      
	DEBUG_PRINT_MSG("Unable to cancel request");
    }

    shandle->s.MPI_ERROR = MPI_SUCCESS;
    shandle->cancel_complete = 1;
    
    return MPI_SUCCESS;
}


/*
  Description of how the cancelling of a pending send operation between two application processes running
  on distinct metahosts is implemented in MetaMPICH:

  The following naming conventions are used:

  receiving metahost               M0
  receiving application process    A0
  router process on M0             R0

  sending metahost                 M1
  sending application process      A1
  router process on M1             R1

  message that is to be cancelled  m

  The following send operations can be cancelled: nonblocking operations ( MPI_Isend(), MPI_Ibsend(),
  MPI_Issend(), MPI_Irsend() ) and operations initiated via persistent communication requests ( MPI_Send_init(),
  MPI_Bsend_init(), MPI_Ssend_init(), MPI_Rsend_init() ). When a send handle for one of these operations is
  created, the field "partner_grank" is initialized with the MPI_COMM_ALL-rank of A0, because this information
  is needed in MPID_Gateway_SendCancelPacket().
  A call to MPI_Cancel() for a message that was sent to another Metahost ultimately results in a call to
  MPID_Gateway_SendCancelPacket(). Here, the first thing we do is to complete the sending of m, i. e.
  we complete the send operation to R1 so that R1 can send the message to M0.
  This is necessary because on M1 we can't decide if the sending of m can be cancelled for we don't know
  if A0 has already posted or completed a matching receive operation.
  Now we send a cancel message to M0 to cancel the send operation. From the point of view of R1 this is just
  a normal MPI message, it just sends it to M0. There it is received by R0 which has already received m.
  R0 sets up basic data for the cancel confirm message and then tries to cancel the sending of m to A0. Two cases can
  occurr here: In the first case, m has already been delivered to A0. R0 sets confirm_message.cancel to 0 to indicate that
  the cancel operation failed. In the second case, the operation to send m to A0 is still pending. Then R0 tries to cancel
  this operation via the cancel mechanism provided by the native device. If this cancel operation succeeds, R0 sets
  confirm_message.cancel to 1, otherwise to 0.
  At this point the cancel confirm message has been set up properly and has to be sent to A1 in a way. Unfortunately, R0 and A1
  can't communicate directly, so the following is a bit tricky. First, a call to MPI_Send() is done with the rank of A1 in
  MPI_COMM_ALL as destination. This message is sent via the gateway device and therefore reaches the exporting
  part of R0. The message can't be sent to A1 just as any other message, because this may lead to confusion with "normal" MPI
  messages from which it must be distinguished. For this reason, the exporting part of R0 now changes the meta header by
  setting the messgage mode to CANCEL_CONFIRM. This can't be done earlier, because the meta header is added in the gateway device
  and so the importing part of R0 doesn't have access to the meta header.
  After the meta header has been changed, the cancel confirm message is sent to M1, where it reaches the importing part of R1. Of
  course, A1 cannot distinguish a cancel confirm message from other messages by the mode in the meta header, because the messages
  are ridden of their meta headers by the importing parts of the routers. For this reason, the importing part of R1 now fakes the
  source rank of the cancel confirm message with its own rank in MPI_COMM_ALL. A1 now can indentify cancel confirm messages for there
  should be no user messages coming from the router.

*/
