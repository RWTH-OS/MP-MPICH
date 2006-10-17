/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include "mpid.h"
#include "shmemdev.h"
#include "flow.h"
#include "../util/queue.h"
#include "chpackflow.h"
#include "shmemdef.h"
#include "shmemdebug.h"
#include "shmemcommon.h"
/* META */
#ifdef META
#include "metampi.h"
#endif
/* /META */

/***************************************************************************/
/* This is one of the main routines.  It checks for incoming messages and  */
/* dispatches them.  There is another such look in MPID_CH_blocking_recv   */
/* which is optimized for the important case of blocking receives for a    */
/* particular message.                                                     */
/*                                                                         */
/* This is a special version for shared memory.  It moves addresses of     */
/* packets, not packets, from one processor to another.                    */
/***************************************************************************/

/* Check for incoming messages.
    Input Parameter:
.   is_blocking - true if this routine should block until a message is
    available

    Returns -1 if nonblocking and no messages pending

    This routine makes use of a single dispatch routine to handle all
    incoming messages.  This makes the code a little lengthy, but each
    piece is relatively simple.
 */    
int MPID_SHMEM_Check_incoming( dev, is_blocking )
MPID_Device        *dev;
MPID_BLOCKING_TYPE is_blocking;
{
    MPID_PKT_T   *pkt;
    int          from_dev_lrank;
    MPIR_RHANDLE *rhandle;
    int          is_posted;
    int          err = MPI_SUCCESS;

    MPID_SHMEM_DEBUG_PRINT_MSG("Entering check_incoming");

    /* If nonblocking and no headers available, exit */
    if (is_blocking == MPID_NOTBLOCKING) {
	if (!(MPID_local || *MPID_incoming)) {
	    MPID_SHMEM_DEBUG_PRINT_MSG("Leaving check_incoming (no messages)")
		return -1;
	}
	MPID_SHMEM_DEBUG_PRINT_MSG("Message is available!");
    }
    MPID_SHMEM_DEBUG_PRINT_MSG("Waiting for message to arrive");
    MPID_SHMEM_ReadControl( &pkt, 0, &from_dev_lrank );
    MPID_SHMEM_DEBUG_PRINT_PKT("R received message",pkt);

    /* Separate the incoming messages from control messages */
    if (MPID_PKT_IS_MSG(pkt->head.mode)) {
	MPID_SHMEM_DEBUG_PRINT_RECV_PKT("R rcvd msg",pkt);

	/* Is the message expected or not? 
	   This routine RETURNS a rhandle, creating one if the message 
	   is unexpected (is_posted == 0) */
	MPID_Msg_arrived( pkt->head.lrank, pkt->head.tag, 
			  pkt->head.context_id, 
			  &rhandle, &is_posted );

	/* Need the send handle address in order to cancel a send */
	if (!is_posted) {  /* begin if !is_posted */
	    if (pkt->head.mode == MPID_PKT_SEND_ADDRESS) 
		rhandle->send_id = pkt->sendadd_pkt.send_id;
	    else if (pkt->head.mode == MPID_PKT_SHORT) 
		rhandle->send_id = pkt->short_pkt.send_id; 
	    else if (pkt->head.mode == MPID_PKT_OK_TO_SEND_GET)
		rhandle->send_id = pkt->get_pkt.send_id; 
	} 
	/* META */
#ifdef META
	/* MPID_PKT_T of ch_shmem does not contain a msgrep entry because
	   normally this isn't necessary for shared memory communication.
	   This is not true for MetaMPICH. For a heterogenous cluster,
	   set this field in rhandle appropiately to ensure correct decoding. */
/*	if (MPIR_meta_cfg.is_hetero &&
   ((from_dev_lrank < MPIR_meta_cfg.metahost_firstrank)
   ||(from_dev_lrank >= MPIR_meta_cfg.metahost_firstrank 
   + MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank]))) {
   rhandle->msgrep = MPID_MSGREP_XDR;
	}
*/
	/* XXX Do we really need this? XXX*/
   if (MPIR_meta_cfg.is_hetero && (from_dev_lrank >= MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank]))
      rhandle->msgrep = MPID_MSGREP_XDR;
#endif
	/* /META */


#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	if (MPID_DebugFlag) {
	    fprintf( MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n", 
		     MPID_SHMEM_rank, 
		     is_posted ? "posted" : "unexpected", __FILE__, __LINE__ );
	}
#endif                  /* #DEBUG_END# */
	if (is_posted) {
	    /* We should check the size here for internal errors .... */
	    switch (pkt->head.mode) {
	    case MPID_PKT_SHORT:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->short_msg->recv,"dev->short->recv");
		err = (*dev->short_msg->recv)( rhandle, from_dev_lrank, pkt );
		break;

	    case MPID_PKT_SEND_ADDRESS:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->eager->recv,"dev->short->recv");
		err = (*dev->eager->recv)( rhandle, from_dev_lrank, pkt );
		break;

	    case MPID_PKT_REQUEST_SEND_GET:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->rndv->irecv,"dev->rndv->irecv");
		err = (*dev->rndv->irecv)( rhandle, from_dev_lrank, pkt );
		break;

	    default:
		fprintf( stderr, 
			 "[%d] Internal error: msg packet discarded (%s:%d)\n",
			 MPID_SHMEM_rank, __FILE__, __LINE__ );
	    }
	}
	else {
	    switch (pkt->head.mode) {
	    case MPID_PKT_SHORT:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->short_msg->unex,"dev->short->unex");
		err = (*dev->short_msg->unex)( rhandle, from_dev_lrank, pkt );
		break;
	    case MPID_PKT_SEND_ADDRESS:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->eager->unex,"dev->short->unex");
		err = (*dev->eager->unex)( rhandle, from_dev_lrank, pkt );
		break;
	    case MPID_PKT_REQUEST_SEND_GET:
		MPID_SHMEM_DEBUG_TEST_FCN(dev->rndv->unex,"dev->rndv->unex");
		err = (*dev->rndv->unex)( rhandle, from_dev_lrank, pkt );
		break;

	    default:
		fprintf( stderr, 
			 "[%d] Internal error: msg packet discarded (%s:%d)\n",
			 MPID_SHMEM_rank, __FILE__, __LINE__ );
	    }
	}
    }
    else {
	switch (pkt->head.mode) {
	case MPID_PKT_CONT_GET:
	case MPID_PKT_OK_TO_SEND_GET:
	    MPID_SHMEM_DEBUG_TEST_FCN(dev->rndv->do_ack,"dev->rndv->do_ack");
	    err = (*dev->rndv->do_ack)( pkt, from_dev_lrank );
	    break;

	case MPID_PKT_ANTI_SEND:
	    MPID_SHMEM_SendCancelOkPacket( (MPID_PKT_T *)pkt, from_dev_lrank ); 
	    break;
	    
	case MPID_PKT_ANTI_SEND_OK:
	    MPID_SHMEM_RecvCancelOkPacket( (MPID_PKT_T *)pkt, from_dev_lrank ); 
	    break;

#ifdef MPID_FLOW_CONTROL
	case MPID_PKT_FLOW:
	    MPID_RecvFlowPacket( &pkt, from_dev_lrank );
	    break;
#endif
#ifdef MPID_PACK_CONTROL
	case MPID_PKT_PROTO_ACK:
	case MPID_PKT_ACK_PROTO:
	    MPID_RecvProtoAck( (MPID_PKT_T *)pkt, from_dev_lrank ); 
	    break;
#endif

	default:
	    fprintf( stdout, "[%d] Mode %d is unknown (internal error) %s:%d!\n", 
		     MPID_SHMEM_rank, pkt->head.mode, __FILE__, __LINE__ );
	}
	/* Really should remember error in case subsequent events are 
	   successful */
    }
    MPID_SHMEM_DEBUG_PRINT_MSG("Exiting check_incoming");
    return err;
}
