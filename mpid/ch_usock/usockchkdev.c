/*
 *  $Id$
 *
 */

#include "mpid.h"
#include "usockdev.h"
#include "flow.h"
#include "usockdebug.h"
#include "queue.h"
/*#include "chpackflow.h"*/

#define _DEBUG_EXTERN_REC
#include "mydebug.h"

/***************************************************************************/
/* This is one of the main routines.  It checks for incoming messages and  */
/* dispatches them.  There is another such look in MPID_CH_blocking_recv   */
/* which is optimized for the important case of blocking receives for a    */
/* particular message.                                                     */
/***************************************************************************/

/* Check for incoming messages.
    Input Parameter:
.   is_blocking - true if this routine should block until a message is
    available

    Returns -1 if nonblocking and no messages pending.  Otherwise 
    returns error code (MPI_SUCCESS == 0 for success)

    This routine makes use of a single dispatch routine to handle all
    incoming messages.  This makes the code a little lengthy, but each
    piece is relatively simple.

    This is the message-passing version.  The shared-memory version is
    in chchkshdev.c .
 */    
int MPID_USOCK_Check_incoming( 
	MPID_Device *dev,
	MPID_BLOCKING_TYPE is_blocking)
{
    DSECTION("MPID_USOCK_Check_incoming");
    MPID_PKT_T   pkt;
    int          from_grank;
    MPIR_RHANDLE *rhandle;
    int          is_posted;
    int          err = MPI_SUCCESS;

    MPID_USOCK_Data_global_type *global_data;
    
    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Check_incoming");

    /* get the pointer to the global data struct of this device entity: */
    global_data = MPID_USOCK_Get_global_data(MPID_devset->active_dev);

    MPID_USOCK_DEBUG_PRINT_MSG("Entering check_incoming");

    /* If nonblocking and no headers available, exit */
    if (is_blocking == MPID_NOTBLOCKING) {
	if (!MPID_PKT_CHECK()) {
	    MPID_USOCK_DEBUG_PRINT_MSG("Leaving check_incoming (no messages)");
	    DSECTLEAVE
		return -1;
	}
	MPID_USOCK_DEBUG_PRINT_MSG("Message is available!");
    }
    MPID_USOCK_DEBUG_PRINT_MSG("Waiting for message to arrive");
    MPID_PKT_WAIT();
    /* 
       This unpacks ONLY the head of the message.
       Note that the payload is handled separately (MPIR_Unpack etc) and
       most of the other data can be considered just bits to return 
       uninterpreted. 

       There are exceptions (see rendevous code); 
       */
    MPID_PKT_UNPACK( &pkt, sizeof(MPID_PKT_HEAD_T), from_grank );

    MPID_USOCK_DEBUG_PRINT_PKT("R received message",&pkt);

    /* Separate the incoming messages from control messages */
    if (MPID_PKT_IS_MSG(pkt.head.mode)) {
	MPID_USOCK_DEBUG_PRINT_RECV_PKT("R rcvd msg",&pkt);

	/* Is the message expected or not? 
	   This routine RETURNS a rhandle, creating one if the message 
	   is unexpected (is_posted == 0) */
	MPID_Msg_arrived( pkt.head.src_comm_lrank, pkt.head.tag, pkt.head.context_id, 
			  &rhandle, &is_posted );

	/* Need the send handle address in order to cancel a send */
	if (!is_posted) {  /* begin if !is_posted */
	    if (pkt.head.mode == MPID_PKT_REQUEST_SEND) 
		rhandle->send_id = pkt.request_pkt.send_id;
	    else if (pkt.head.mode == MPID_PKT_SHORT)
		rhandle->send_id = pkt.short_pkt.send_id; 
	    else if (pkt.head.mode == MPID_PKT_LONG)
		rhandle->send_id = pkt.long_pkt.send_id;
	} 

	MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt.head.msgrep );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	if (MPID_DebugFlag) {
	    FPRINTF( MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n", 
		     global_data->MyWorldRank, 
		     is_posted ? "posted" : "unexpected", __FILE__, __LINE__ );
	}
#endif                  /* #DEBUG_END# */
	if (is_posted) {
	    /* We should check the size here for internal errors .... */
	    switch (pkt.head.mode) {
	    case MPID_PKT_SHORT:
		MPID_USOCK_DEBUG_TEST_FCN(dev->short_msg->recv,"dev->short->recv");
		err = MPID_Protocol_call_recv( dev->short_msg->recv, rhandle, from_grank, &pkt, dev);
		/*
		err = (*dev->short_msg->recv)( rhandle, from_grank, &pkt );
		*/
		break;

	    case MPID_PKT_REQUEST_SEND:
		MPID_USOCK_DEBUG_TEST_FCN(dev->rndv->irecv,"dev->rndv->irecv");
		err = MPID_Protocol_call_irecv( dev->rndv->irecv, rhandle, from_grank, &pkt, dev);
		/*
		err = (*dev->rndv->irecv)( rhandle, from_grank, &pkt );
		*/
		break;

	    case MPID_PKT_LONG:
		MPID_USOCK_DEBUG_TEST_FCN(dev->eager->irecv,"dev->eager->irecv");
		err = MPID_Protocol_call_irecv( dev->eager->irecv,
						rhandle, from_grank, &pkt,
						dev );
		/*
		err = (*dev->eager->irecv)( rhandle, from_grank, &pkt );
		*/
		break;

	    default:
		fprintf( stderr, 
			 "[%d] Internal error: msg packet discarded (%s:%d)\n",
			 global_data->MyWorldRank, __FILE__, __LINE__ );
		fflush( stderr );
	    }
	}
	else {
	    switch (pkt.head.mode) {
	    case MPID_PKT_SHORT:
		MPID_USOCK_DEBUG_TEST_FCN(dev->short_msg->unex,"dev->short->unex");
		err = MPID_Protocol_call_unex( dev->short_msg->unex,
					       rhandle, from_grank, &pkt,
					       dev );
		/*
		err = (*dev->short_msg->unex)( rhandle, from_grank, &pkt );
		*/
		break;
	    case MPID_PKT_REQUEST_SEND:
		MPID_USOCK_DEBUG_TEST_FCN(dev->short_msg->unex,"dev->rndv->unex");
		err = MPID_Protocol_call_unex( dev->short_msg->unex,
					       rhandle, from_grank, &pkt,
					       dev );
		/*
		err = (*dev->rndv->unex)( rhandle, from_grank, &pkt );
		*/
		break;

	    case MPID_PKT_LONG:
		MPID_USOCK_DEBUG_TEST_FCN(dev->eager->unex,"dev->eager->unex");
		err = MPID_Protocol_call_unex( dev->eager->unex,
					       rhandle, from_grank, &pkt,
					       dev );
		/*
		err = (*dev->eager->unex)( rhandle, from_grank, &pkt );
		*/
		break;

	    default:
		fprintf( stderr, 
			 "[%d] Internal error: msg packet discarded (%s:%d)\n",
			 global_data->MyWorldRank, __FILE__, __LINE__ );
	    }
	}
    }
    else {
	switch (pkt.head.mode) {
	case MPID_PKT_OK_TO_SEND:
	    MPID_USOCK_DEBUG_TEST_FCN(dev->rndv->do_ack,"dev->rndv->do_ack");
	    err = MPID_Protocol_call_do_ack( dev->rndv->do_ack,
					     &pkt, from_grank,
					     dev );
					     
	    /*
	    err = (*dev->rndv->do_ack)( &pkt, from_grank );
	    */
	    break;

	case MPID_PKT_ANTI_SEND:
	    MPID_USOCK_SendCancelOkPacket( &pkt, from_grank ); 
	    break;
	    
	case MPID_PKT_ANTI_SEND_OK:
	    MPID_USOCK_RecvCancelOkPacket( &pkt, from_grank ); 
	    break;
	    
#ifdef MPID_FLOW_CONTROL
	case MPID_PKT_FLOW:
	    MPID_RecvFlowPacket( &pkt, from_grank );
	    break;
#endif
#ifdef MPID_PACK_CONTROL
	case MPID_PKT_PROTO_ACK:
	case MPID_PKT_ACK_PROTO:
	    MPID_RecvProtoAck( &pkt, from_grank );
	    break;
#endif
	default:
	    fprintf( stdout, "[%d] Mode %d is unknown (internal error) %s:%d!\n", 
		     global_data->MyWorldRank, pkt.head.mode, 
		     __FILE__, __LINE__ );
	}
	/* Really should remember error in case subsequent events are 
	   successful */
    }
    MPID_USOCK_DEBUG_PRINT_MSG("Exiting check_incoming");
    DSECTLEAVE
	return err;
}
