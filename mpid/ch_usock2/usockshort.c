/*
 *  $Id: usockshort.c 3098 2005-02-01 08:38:18Z boris $
 *
 */

#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "flow.h"
#include "usockdebug.h"

#define _DEBUG_EXTERN_REC
#include "mydebug.h"

/* Prototype definitions */
int MPID_USOCK_Eagerb_send_short ANSI_ARGS(( void *, int, int, int, int, int, 
					  MPID_Msgrep_t, struct MPIR_DATATYPE * ));
int MPID_USOCK_Eagerb_isend_short ANSI_ARGS(( void *, int, int, int, int, int, 
					   MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * ));
int MPID_USOCK_Eagerb_recv_short ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_USOCK_Eagerb_save_short ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
int MPID_USOCK_Eagerb_unxrecv_start_short ANSI_ARGS(( MPIR_RHANDLE *, void * ));
void MPID_USOCK_Eagerb_short_delete ANSI_ARGS(( MPID_Protocol * ));
int MPID_USOCK_short_cancel_recv(MPIR_RHANDLE *) ;
/*
 * Definitions of the actual functions
 */

int MPID_USOCK_Eagerb_send_short( 
			      void *buf, 
			      int len, 
			      int src_lrank, 
			      int tag, 
			      int context_id, 
			      int dest,
			      MPID_Msgrep_t msgrep,
			      struct MPIR_DATATYPE *dtype )
{
    DSECTION("MPID_USOCK_Eagerb_send_short");
    int pkt_len;
    MPID_PKT_SHORT_T pkt;
    
    DSECTENTRYPOINT;
    
    MPID_USOCK_DEBUG_PRINT_MSG("S Starting Eagerb_send_short");


    /* What are these SS_MPID_* variables good for? They are only seen in this file.
     Maybe we never compile with MPID_PACK_CONTROL, so they are just useless. */
#ifdef MPID_PACK_CONTROL
    while (!MPID_PACKET_CHECK_OK(dest)) {  /* begin while !ok loop */
	/* Wait for a protocol ACK packet */
#ifdef MPID_DEBUG_ALL
	if (MPID_DebugFlag || MPID_DebugFlow) {
	    FPRINTF(MPID_DEBUG_FILE,
		"[%d] S Waiting for a protocol ACK packet (in eagerb_send_short) from %d\n",
		SS_MPID_MyWorldRank2, dest);
	}
#endif
	MPID_DeviceCheck( MPID_BLOCKING );
    }  /* end while !ok loop */
    MPID_PACKET_ADD_SENT(SS_MPID_MyWorldRank3, dest)
#endif
	
    /* These references are ordered to match the order they appear in the 
    structure */
    pkt_len        = sizeof(MPID_PKT_HEAD_T) + sizeof(MPID_Aint);
    pkt.mode	   = MPID_PKT_SHORT;
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_DO_HETERO(pkt.msgrep = (int)msgrep);
    
    MPID_USOCK_DEBUG_PRINT_SEND_PKT("S Sending",&pkt);
    MPID_PKT_PACK( &pkt, pkt_len, dest );
    
    if (len > 0) {
	MEMCPY( pkt.buffer, buf, len );
	MPID_USOCK_DEBUG_PRINT_PKT_DATA("S Getting data from buf",&pkt);
    }
    /* Always use a blocking send for short messages.
    (May fail with systems that do not provide adequate
    buffering.  These systems should switch to non-blocking sends)
    */
    MPID_USOCK_DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",&pkt);
    
    /* In case the message is marked as non-blocking, indicate that we don't
    need to wait on it.  We may also want to use nonblocking operations
    to send the envelopes.... */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    MPID_SendControlBlock( (MPID_PKT_T*)&pkt, len + pkt_len, dest );
    MPID_USOCK_DEBUG_PRINT_MSG("S Sent message in a single packet");
    
    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Eagerb_isend_short( 
	void *buf, 
	int len, 
	int src_lrank, 
	int tag, 
	int context_id, 
	int dest,
	MPID_Msgrep_t msgrep, 
	MPIR_SHANDLE *shandle,
	struct MPIR_DATATYPE *dtype )
{
    DSECTION("MPID_USOCK_agerb_isend_short");
    int pkt_len;
    MPID_PKT_SHORT_T pkt;
    
    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG("S Starting Eagerb_isend_short");

#ifdef MPID_PACK_CONTROL
    while (!MPID_PACKET_CHECK_OK(dest)) {  /* begin while !ok loop */
	/* Wait for a protocol ACK packet */
#ifdef MPID_DEBUG_ALL
	if (MPID_DebugFlag || MPID_DebugFlow) {
		FPRINTF(MPID_DEBUG_FILE,
   "[%d] S Waiting for a protocol ACK packet (in eagerb_send_short) from %d\n",
			SS_MPID_MyWorldRank5, dest);
	}
#endif
	MPID_DeviceCheck( MPID_BLOCKING );
    }  /* end while !ok loop */

    MPID_PACKET_ADD_SENT(SS_MPID_MyWorldRank6, dest)
#endif

    /* These references are ordered to match the order they appear in the 
       structure */
    pkt_len        = sizeof(MPID_PKT_HEAD_T) + sizeof(MPID_Aint);
    pkt.mode	   = MPID_PKT_SHORT;
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_DO_HETERO(pkt.msgrep = (int)msgrep);

    /* We save the address of the send handle in the packet; the receiver
       will return this to us */
    MPID_AINT_SET(pkt.send_id,shandle);
    
    /* Store partners rank in request in case message is cancelled */
    shandle->partner     = dest;
    shandle->is_complete = 1;
	shandle->cancel	     = 0;
   
    MPID_USOCK_DEBUG_PRINT_SEND_PKT("S Sending",&pkt);
    MPID_PKT_PACK( &pkt, sizeof(pkt), dest );

    if (len > 0) {
	MEMCPY( pkt.buffer, buf, len );
	MPID_USOCK_DEBUG_PRINT_PKT_DATA("S Getting data from buf",&pkt);
    }
    /* Always use a blocking send for short messages.
       (May fail with systems that do not provide adequate
       buffering.  These systems should switch to non-blocking sends)
     */
    MPID_USOCK_DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",&pkt);

    /* In case the message is marked as non-blocking, indicate that we don't
       need to wait on it.  We may also want to use nonblocking operations
       to send the envelopes.... */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    MPID_SendControlBlock( (MPID_PKT_T*)&pkt, len + pkt_len, dest ); 

    MPID_USOCK_DEBUG_PRINT_MSG("S Sent message in a single packet");
    
    DSECTLEAVE
	return MPI_SUCCESS;

}

int MPID_USOCK_Eagerb_recv_short( 
	MPIR_RHANDLE *rhandle,
	int          from_grank,
	void         *in_pkt)
{
    DSECTION("MPID_USOCK_Eagerb_recv_short");
    MPID_PKT_SHORT_T *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    DSECTENTRYPOINT;
    
    MPID_USOCK_DEBUG_PRINT_MSG("R Starting Eagerb_recv_short");

    msglen = pkt->len;
    
#ifdef MPID_PACK_CONTROL
    if (MPID_PACKET_RCVD_GET(pkt->src)) {
	MPID_SendProtoAck(pkt->to, pkt->src);
    }
    MPID_PACKET_ADD_RCVD(pkt->to, pkt->src);
#endif

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;

    MPID_CHK_MSGLEN(rhandle,msglen,err);
    if (msglen > 0) {
	MEMCPY( rhandle->buf, pkt->buffer, msglen ); 
    }
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_ERROR  = err;

    if (rhandle->finish) {
	MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
	(rhandle->finish)( rhandle );
    }
    rhandle->is_complete = 1;

    if(msglen<pkt->len) {
	MPID_USOCK_ConsumeData(pkt->len-msglen,pkt->lrank);
    }

    DSECTLEAVE
	return err;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_USOCK_Eagerb_unxrecv_start_short( 
	MPIR_RHANDLE *rhandle,
	void         *in_runex)
{
    DSECTION("MPID_USOCK_Eagerb_unxrecv_start_short");
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;

    DSECTENTRYPOINT;

    msglen = runex->s.count;
    MPID_USOCK_DEBUG_PRINT_MSG("R Starting Eagerb_unxrecv_start_short");
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
	MEMCPY( rhandle->buf, runex->start, msglen );
	FREE( runex->start );
    }
    MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
    rhandle->s		 = runex->s;
    rhandle->s.count     = msglen;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->cancel  = 0;
	rhandle->is_complete = 1;
	
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
/*    MPID_RecvFree( runex );   */

    DSECTLEAVE
	return err;
}



int MPID_USOCK_short_cancel_recv(runex)
MPIR_RHANDLE *runex;
{
	if (runex->s.count > 0) {
		FREE( runex->start );
    }
/*	MPID_RecvFree( runex ); */
	return 0;

}
/* Save an unexpected message in rhandle */
int MPID_USOCK_Eagerb_save_short( 
	MPIR_RHANDLE *rhandle,
	int          from,
	void         *in_pkt)
{
    DSECTION("MPID_USOCK_Eagerb_save_short");
    MPID_PKT_SHORT_T   *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    
    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG("R Starting Eagerb_save_short");
#ifdef MPID_PACK_CONTROL
    if (MPID_PACKET_RCVD_GET(pkt->src)) {
	MPID_SendProtoAck(pkt->to, pkt->src);
    }
    MPID_PACKET_ADD_RCVD(pkt->to, pkt->src);
#endif

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->from         = from;
    rhandle->partner      = from;
    rhandle->s.count      = pkt->len;
    rhandle->cancel	      = MPID_USOCK_short_cancel_recv;
    /* rhandle->is_complete  = 1; */
    /* Need to save msgrep for heterogeneous systems */
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
    if (pkt->len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->len );
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_NOMEM;
	    MPID_USOCK_ConsumeData(pkt->len,from);
	    rhandle->s.count = 0;
	    rhandle->push = MPID_USOCK_Eagerb_unxrecv_start_short;
	    DSECTLEAVE
		return 1;
	}
	MEMCPY( rhandle->start, pkt->buffer, pkt->len );
    }
    rhandle->push = MPID_USOCK_Eagerb_unxrecv_start_short;
    DSECTLEAVE
	return 0;
}

void MPID_USOCK_Eagerb_short_delete( 
	MPID_Protocol *p)
{
    FREE( p );
}

MPID_Protocol *MPID_USOCK_Short_setup()
{
    DSECTION("MPID_USOCK_Short_setup");
    MPID_Protocol *p;

    DSECTENTRYPOINT;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) {DSECTLEAVE return 0;}
    p->send	   = MPID_USOCK_Eagerb_send_short;
    p->recv	   = MPID_USOCK_Eagerb_recv_short;
    p->isend	   = MPID_USOCK_Eagerb_isend_short;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_USOCK_Eagerb_save_short;
    p->delete      = MPID_USOCK_Eagerb_short_delete;

    DSECTLEAVE
	return p;
}
