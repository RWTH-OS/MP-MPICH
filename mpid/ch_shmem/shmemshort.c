/* $Id$ */

#include "mpid.h"
#include "shmemdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "flow.h"
#include "chpackflow.h"
#include "shmemdebug.h"
#include "shmemcommon.h"

/*
 * This is almost exactly like chshort.c, except that packets are allocated
 * from the pool rather than on the call stack, and there is no heterogeneous
 * support.
 */

/* Prototype definitions */
int MPID_SHMEM_Eagerb_send_short ( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * );
int MPID_SHMEM_Eagerb_isend_short ( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE * , struct MPIR_DATATYPE *);
int MPID_SHMEM_Eagerb_recv_short ( MPIR_RHANDLE *, int, void * );
int MPID_SHMEM_Eagerb_save_short ( MPIR_RHANDLE *, int, void *);
int MPID_SHMEM_Eagerb_unxrecv_start_short ( MPIR_RHANDLE *, void * );
void MPID_SHMEM_Eagerb_short_delete ( MPID_Protocol * );
int MPID_SHMEM_Short_cancel_recv( MPIR_RHANDLE * );

/*
 * Definitions of the actual functions
 */

int MPID_SHMEM_Eagerb_send_short( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
				  msgrep, dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE * dty;
{
    int pkt_len;
    MPID_PKT_SHORT_T *pkt;

    MPID_SHMEM_DEBUG_PRINT_MSG("S Starting Eagerb_send_short");

    /* GetSendPkt() hangs until successful */
    MPID_SHMEM_DEBUG_PRINT_MSG("S Getting a packet");
    pkt = (MPID_PKT_SHORT_T *)MPID_SHMEM_GetSendPkt(0);

#ifdef MPID_PACK_CONTROL
    while (!MPID_PACKET_CHECK_OK(dest_dev_lrank)) {  /* begin while !ok loop */
	/* Wait for a protocol ACK packet */
#ifdef MPID_DEBUG_ALL
	if (MPID_DebugFlag || MPID_DebugFlow) {
		FPRINTF(MPID_DEBUG_FILE,
   "[%d] S Waiting for a protocol ACK packet (in eagerb_send_short) from %d\n",
			MPID_SHMEM_rank, dest_dev_lrank);
	}
#endif
	MPID_DeviceCheck( MPID_BLOCKING );
    }  /* end while !ok loop */

    MPID_PACKET_ADD_SENT(MPID_SHMEM_rank, dest_dev_lrank);
#endif

    pkt_len         = sizeof(MPID_PKT_HEAD_T) + sizeof(MPID_Aint);
    pkt->mode	    = MPID_PKT_SHORT;
    pkt->context_id = context_id;
    pkt->lrank	    = src_comm_lrank;
    pkt->to         = dest_dev_lrank;
    pkt->seqnum     = len + pkt_len;
    pkt->tag	    = tag;
    pkt->len	    = len;

    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending",pkt);

    if (len > 0) {
	MEMCPY( pkt->buffer, buf, len );
	MPID_SHMEM_DEBUG_PRINT_PKT_DATA("S Getting data from buf",pkt);
    }
    /* Always use a blocking send for short messages.
       (May fail with systems that do not provide adequate
       buffering.  These systems should switch to non-blocking sends)
     */
    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt);

    /* In case the message is marked as non-blocking, indicate that we don't
       need to wait on it.  We may also want to use nonblocking operations
       to send the envelopes.... */
    MPID_SHMEM_SendControl( (MPID_PKT_T*)pkt, len + pkt_len, dest_dev_lrank );
    MPID_SHMEM_DEBUG_PRINT_MSG("S Sent message in a single packet");

    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagerb_isend_short( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			 msgrep, shandle, dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE * dty;
{

    int pkt_len;
    MPID_PKT_SHORT_T *pkt;

    /* These references are ordered to match the order they appear in the 
       structure */
    MPID_SHMEM_DEBUG_PRINT_MSG("S Getting a packet");
    pkt = (MPID_PKT_SHORT_T *)MPID_SHMEM_GetSendPkt(0);
    /* GetSendPkt hangs untill successful */
    MPID_SHMEM_DEBUG_PRINT_MSG("S Starting Eagerb_isend_short");
#ifdef MPID_PACK_CONTROL
    while (!MPID_PACKET_CHECK_OK(dest_dev_lrank)) {  /* begin while !ok loop */
	/* Wait for a protocol ACK packet */
#ifdef MPID_DEBUG_ALL
	if (MPID_DebugFlag || MPID_DebugFlow) {
		FPRINTF(MPID_DEBUG_FILE,
   "[%d] S Waiting for a protocol ACK packet (in eagerb_send_short) from %d\n",
			MPID_SHMEM_rank, dest_dev_lrank);
	}
#endif
	MPID_DeviceCheck( MPID_BLOCKING );
    }  /* end while !ok loop */

    MPID_PACKET_ADD_SENT(MPID_SHMEM_rank, dest_dev_lrank);
#endif

    pkt_len         = sizeof(MPID_PKT_HEAD_T) + sizeof(MPID_Aint);
    pkt->mode	    = MPID_PKT_SHORT;
    pkt->context_id = context_id;
    pkt->lrank	    = src_comm_lrank;
    pkt->to         = dest_dev_lrank;
    pkt->seqnum     = len + pkt_len;
    pkt->tag	    = tag;
    pkt->len	    = len;

    /* We save the address of the send handle in the packet; the receiver
       will return this to us */
    MPID_AINT_SET(pkt->send_id,shandle);
    
    /* Store partners rank in request in case message is cancelled */
    shandle->partner     = dest_dev_lrank;
    shandle->is_complete = 1;

    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending",pkt);

    if (len > 0) {
	MEMCPY( pkt->buffer, buf, len );
	MPID_SHMEM_DEBUG_PRINT_PKT_DATA("S Getting data from buf",pkt);
    }
    /* Always use a blocking send for short messages.
       (May fail with systems that do not provide adequate
       buffering.  These systems should switch to non-blocking sends)
     */
    MPID_SHMEM_DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt);

    /* In case the message is marked as non-blocking, indicate that we don't
       need to wait on it.  We may also want to use nonblocking operations
       to send the envelopes.... */
    MPID_SHMEM_SendControl( (MPID_PKT_T*)pkt, len + pkt_len, dest_dev_lrank );
    MPID_SHMEM_DEBUG_PRINT_MSG("S Sent message in a single packet");

    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagerb_recv_short( rhandle, from_dev_lrank, in_pkt )
MPIR_RHANDLE *rhandle;
int          from_dev_lrank;
void         *in_pkt;
{
    MPID_PKT_SHORT_T *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    msglen = pkt->len;
    MPID_SHMEM_DEBUG_PRINT_MSG("R Starting Eagerb_recv_short");
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
    rhandle->s.count      = msglen;
    rhandle->s.MPI_ERROR = err;
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    rhandle->cancel      = 0;
    rhandle->is_complete = 1;

    MPID_SHMEM_FreeRecvPkt( (MPID_PKT_T *)pkt );

    return err;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SHMEM_Eagerb_unxrecv_start_short( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;

    msglen = runex->s.count;
    MPID_SHMEM_DEBUG_PRINT_MSG("R Starting Eagerb_unxrecv_start_short");
#ifdef MPID_PACK_CONTROL
    if (MPID_PACKET_RCVD_GET(runex->from)) {
	MPID_SendProtoAck(runex->partner, runex->from);
    }
    MPID_PACKET_ADD_RCVD(runex->partner, runex->from);
#endif
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
	MEMCPY( rhandle->buf, runex->start, msglen );
	FREE( runex->start );
    }
    rhandle->s		 = runex->s;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->cancel      = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    MPID_Recv_free( runex );

    return err;
}

/* Save an unexpected message in rhandle */
int MPID_SHMEM_Eagerb_save_short( rhandle, from_dev_lrank, in_pkt )
MPIR_RHANDLE *rhandle;
int          from_dev_lrank;
void         *in_pkt;
{
    MPID_PKT_SHORT_T   *pkt = (MPID_PKT_SHORT_T *)in_pkt;

    MPID_SHMEM_DEBUG_PRINT_MSG("R Starting Eagerb_save_short");
#ifdef MPID_PACK_CONTROL
    if (MPID_PACKET_RCVD_GET(pkt->src)) {
	MPID_SendProtoAck(pkt->to, pkt->src);
    }
    MPID_PACKET_ADD_RCVD(pkt->to, pkt->src);
#endif
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->from         = from_dev_lrank;
    rhandle->partner      = pkt->to;
    rhandle->s.count      = pkt->len;

    /* Need to save msgrep for heterogeneous systems */
    if (pkt->len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->len );
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    return 1;
	}
	MEMCPY( rhandle->start, pkt->buffer, pkt->len );
    }
    rhandle->push = MPID_SHMEM_Eagerb_unxrecv_start_short;
    rhandle->cancel = MPID_SHMEM_Short_cancel_recv;

    MPID_SHMEM_FreeRecvPkt( (MPID_PKT_T *)pkt );

    return 0;
}

/* when a short-recv is canceled, free the buffer in which the msg was saved */
int MPID_SHMEM_Short_cancel_recv( runex )
MPIR_RHANDLE    *runex;
{
    FREE( runex->start );
    MPID_Recv_free( runex );

    return MPI_SUCCESS;
}

void MPID_SHMEM_Eagerb_short_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_SHMEM_Short_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SHMEM_Eagerb_send_short;
    p->recv	   = MPID_SHMEM_Eagerb_recv_short;
    p->isend	   = MPID_SHMEM_Eagerb_isend_short;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = MPID_SHMEM_Short_cancel_recv;
    p->do_ack      = 0;
    p->unex        = MPID_SHMEM_Eagerb_save_short;
    p->delete      = MPID_SHMEM_Eagerb_short_delete;

    return p;
}

