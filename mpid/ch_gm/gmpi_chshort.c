/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "flow.h"
#include "chpackflow.h"
#include "gmpi.h"

/* Prototype definitions */
int MPID_CH_Eagerb_send_short ( void *, int, int, int, int, int, 
					  MPID_Msgrep_t );
int MPID_CH_Eagerb_isend_short ( void *, int, int, int, int, int, 
					   MPID_Msgrep_t, MPIR_SHANDLE * );
int MPID_CH_Eagerb_recv_short ( MPIR_RHANDLE *, int, void * );
int MPID_CH_Eagerb_save_short ( MPIR_RHANDLE *, int, void * );
int MPID_CH_Eagerb_unxrecv_start_short ( MPIR_RHANDLE *, void * );
void MPID_CH_Eagerb_short_delete ( MPID_Protocol * );
/*
 * Definitions of the actual functions
 */

int MPID_CH_Eagerb_send_short( 
	void *buf, 
	int len, 
	int src_lrank, 
	int tag, 
	int context_id, 
	int dest,
	MPID_Msgrep_t msgrep )
{
  unsigned int pkt_len;
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  MPID_PKT_SHORT_T * pkt_ptr;
  
  GMPI_PROGRESSION_LOCK();
  pkt_len = sizeof(MPID_PKT_SHORT_T);
  gmpi_debug_assert(len < gmpi.eager_size);
  pkt_ptr = gmpi_allocate_packet(len + pkt_len, &gmpi_send_buf_ptr);

  while (gmpi.malloc_send_buf_allocated > GMPI_MAX_DELAYED_SEND_QUEUE_SIZE)
    {
      MPID_DeviceCheck(MPID_BLOCKING);
    } 
 
  DEBUG_PRINT_MSG("S Starting Eagerb_send_short");
  
  /* These references are ordered to match the order they appear in the 
     structure */
  pkt_ptr->mode = MPID_PKT_SHORT;
  pkt_ptr->context_id = context_id;
  pkt_ptr->lrank = src_lrank;
  pkt_ptr->to = dest;
  pkt_ptr->src = MPID_GM_rank;
  pkt_ptr->tag = tag;
  pkt_ptr->len = len;
  MPID_DO_HETERO(pkt_ptr->msgrep = (int)msgrep);
  
  DEBUG_PRINT_SEND_PKT("S Sending", pkt_ptr);
  MPID_PKT_PACK(pkt_ptr, pkt_len, dest);

  if (len > 0) 
    {
      memcpy((void *)((gm_up_t)pkt_ptr + pkt_len), buf, len);
      DEBUG_PRINT_PKT_DATA("S Getting data from buf", pkt_ptr);
    }
  
  DEBUG_PRINT_SEND_PKT("S Sending message in a single packet", pkt_ptr);
  
  gmpi_send_packet(gmpi_send_buf_ptr, dest);
  DEBUG_PRINT_MSG("S Sent message in a single packet");
  MPID_DRAIN_INCOMING
  GMPI_PROGRESSION_UNLOCK();
    
  return MPI_SUCCESS;
}

int MPID_CH_Eagerb_isend_short( 
	void *buf, 
	int len, 
	int src_lrank, 
	int tag, 
	int context_id, 
	int dest,
	MPID_Msgrep_t msgrep, 
	MPIR_SHANDLE *shandle )
{
  unsigned int pkt_len;
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  MPID_PKT_SHORT_T * pkt_ptr;
  
  GMPI_PROGRESSION_LOCK();
  pkt_len = sizeof(MPID_PKT_SHORT_T);
  gmpi_debug_assert(len < gmpi.eager_size);
  pkt_ptr = gmpi_allocate_packet(len + pkt_len, &gmpi_send_buf_ptr);
  
  DEBUG_PRINT_MSG("S Starting Eagerb_isend_short");
  
  /* These references are ordered to match the order they appear in the 
     structure */
  pkt_ptr->mode = MPID_PKT_SHORT;
  pkt_ptr->context_id = context_id;
  pkt_ptr->lrank = src_lrank;
  pkt_ptr->to = dest;
  pkt_ptr->src = MPID_GM_rank;
  pkt_ptr->tag = tag;
  pkt_ptr->len = len;
  MPID_DO_HETERO(pkt_ptr->msgrep = (int)msgrep);
  
  /* We save the address of the send handle in the packet; the receiver
     will return this to us */
  MPID_AINT_SET(pkt_ptr->send_id, shandle);
  
  /* Store partners rank in request in case message is cancelled */
  shandle->partner = dest;
  shandle->gm.rendez_vous = 0;
  
  DEBUG_PRINT_SEND_PKT("S Sending", pkt_ptr);
  MPID_PKT_PACK(pkt_ptr, pkt_len+len, dest);
  
  if (len > 0)
    {
      memcpy((void *)((gm_up_t)pkt_ptr + pkt_len), buf, len);
      DEBUG_PRINT_PKT_DATA("S Getting data from buf", pkt_ptr);
    }
 
  DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt_ptr);

  gmpi_send_packet(gmpi_send_buf_ptr, dest);
  shandle->is_complete = 1;
  DEBUG_PRINT_MSG("S Sent message in a single packet");
  MPID_DRAIN_INCOMING
  GMPI_PROGRESSION_UNLOCK();
  
  return MPI_SUCCESS;
}

int MPID_CH_Eagerb_recv_short( 
	MPIR_RHANDLE *rhandle,
	int          from_grank,
	void         *in_pkt)
{
    MPID_PKT_SHORT_T *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    msglen		  = pkt->len;
  
  DEBUG_PRINT_MSG("R Starting Eagerb_recv_short");

  rhandle->s.MPI_TAG = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  
  MPID_CHK_MSGLEN(rhandle,msglen,err);
  if (msglen > 0) {
      memcpy(rhandle->buf,
	     (void *)((gm_up_t)pkt + sizeof(MPID_PKT_SHORT_T)), 
	     msglen);
    }
  rhandle->s.count = msglen;
  rhandle->s.MPI_ERROR = err;
  if (rhandle->finish) {
      MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
	(rhandle->finish)( rhandle );
    }
  rhandle->is_complete = 1;

  return err;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_CH_Eagerb_unxrecv_start_short( 
	MPIR_RHANDLE *rhandle,
	void         *in_runex)
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
  int msglen, err = 0;

  msglen = runex->s.count;
  DEBUG_PRINT_MSG("R Starting Eagerb_unxrecv_start_short");
    MPID_CHK_MSGLEN(rhandle,msglen,err);
  /* Copy the data from the local area and free that area */

  if (msglen > 0) 
    {
      memcpy(rhandle->buf, runex->start, msglen);
    }
  
  if (runex->s.count > 0)
    {
	FREE(runex->start);
	gmpi_debug_assert(gmpi.unexpected_short > 0);
	gmpi.unexpected_short--;
    }
  
  MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
  rhandle->s = runex->s;
  rhandle->s.count = msglen;
  rhandle->s.MPI_ERROR = err;
  rhandle->wait = 0;
  rhandle->test	= 0;
  rhandle->push	= 0;
  rhandle->is_complete = 1;
  if (rhandle->finish)
	(rhandle->finish)( rhandle );
    MPID_RecvFree( runex );
    
  return err;
}

/* Save an unexpected message in rhandle */
int MPID_CH_Eagerb_save_short( 
	MPIR_RHANDLE *rhandle,
	int          from,
	void         *in_pkt)
{
  MPID_PKT_SHORT_T   *pkt = (MPID_PKT_SHORT_T *)in_pkt;
  
  DEBUG_PRINT_MSG("R Starting Eagerb_save_short");

  rhandle->s.MPI_TAG = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR = 0;
  rhandle->from = from;
  rhandle->partner = pkt->to;

  rhandle->s.count = pkt->len;
  MPID_AINT_GET(rhandle->send_id, pkt->send_id)
  
  /* Need to save msgrep for heterogeneous systems */
  MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
  if (pkt->len > 0) {
      rhandle->start = (void *)MALLOC(pkt->len);
      gmpi_malloc_assert(rhandle->start,
			 "MPID_CH_Eagerb_save_short",
			 "MALLOC:unexpected buffer");
      if (!rhandle->start) {
	rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	return 1;
      }
      memcpy(rhandle->start,
	     (void *)((gm_up_t)pkt + sizeof(MPID_PKT_SHORT_T)), 
	      pkt->len);
      gmpi.unexpected_short++;
    }
  rhandle->push = MPID_CH_Eagerb_unxrecv_start_short;
  return 0;
}

void MPID_CH_Eagerb_short_delete( 
	MPID_Protocol *p)
{
    FREE( p );
}

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

int 
MPID_CH_Cancel_send (MPIR_SHANDLE * shandle)
{  
  struct gmpi_send_buf *gmpi_send_buf_ptr;
  MPID_PKT_ANTI_SEND_T *pkt;
  
  GMPI_PROGRESSION_LOCK();
  pkt = gmpi_allocate_packet(sizeof(MPID_PKT_ANTI_SEND_T),
                             &gmpi_send_buf_ptr);
  
  pkt->mode = MPID_PKT_ANTI_SEND; 
  pkt->lrank = MPID_GM_rank;
  pkt->src = MPID_GM_rank;
  pkt->to = shandle->partner;
  MPID_AINT_SET(pkt->send_id,shandle); 
  
  DEBUG_PRINT_BASIC_SEND_PKT("S Sending anti-send message\n", pkt);
  MPID_PKT_PACK( pkt, sizeof(*pkt), pkt->to ); 
  
  GMPI_DRAIN_GM_INCOMING;
  gmpi_send_packet(gmpi_send_buf_ptr, pkt->to);
  GMPI_PROGRESSION_UNLOCK();
  return MPI_SUCCESS;
}


MPID_Protocol *MPID_CH_Short_setup()
{
    MPID_Protocol *p;
  
    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
  p->send = MPID_CH_Eagerb_send_short;
  p->recv = MPID_CH_Eagerb_recv_short;
  p->isend = MPID_CH_Eagerb_isend_short;
  p->wait_send = 0;
  p->push_send = 0;
  p->cancel_send = MPID_CH_Cancel_send;
  p->irecv = 0;
  p->wait_recv = 0;
  p->push_recv = 0;
  p->cancel_recv = 0;
  p->do_ack = 0;
  p->unex = MPID_CH_Eagerb_save_short;
  p->delete = MPID_CH_Eagerb_short_delete;
  
  return p;
}
