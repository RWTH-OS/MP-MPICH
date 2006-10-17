/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "flow.h"
#include "chpackflow.h"

/* NonBlocking Rendezvous */

/* Prototype definitions */
int MPID_CH_Rndvn_send ( void *, int, int, int, int, int, MPID_Msgrep_t );
int MPID_CH_Rndvn_isend ( void *, int, int, int, int, int,
			MPID_Msgrep_t, MPIR_SHANDLE * );
int MPID_CH_Rndvn_irecv ( MPIR_RHANDLE *, int, void * );
int MPID_CH_Rndvn_save ( MPIR_RHANDLE *, int, void *);
int MPID_CH_Rndvn_unxrecv_start ( MPIR_RHANDLE *, void * );
int MPID_CH_Rndvn_ok_to_send ( MPIR_RHANDLE * );
int MPID_CH_Rndvn_ack ( void *, int );
void MPID_CH_Rndvn_delete ( MPID_Protocol * );

int  MPID_DeviceCheck ( MPID_BLOCKING_TYPE blocking );

static int 
do_recv(MPIR_RHANDLE *, int, void *, int);

/* Globals for this protocol */
int gmpi_sync_send = 0;

/*
 * Definitions of the actual functions
 */

/*
 * This is really the same as the blocking version, since the 
 * nonblocking operations occur only in the data transmission.
 */
int MPID_CH_Rndvn_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE  *shandle;
{
  struct gmpi_send_buf * gmpi_send_buf_ptr; 
  MPID_PKT_REQUEST_SEND_T * pkt_ptr;
  
  if (gmpi_sync_send == 0)
    {
      GMPI_PROGRESSION_LOCK();
    }
  
  pkt_ptr = gmpi_allocate_packet(sizeof(MPID_PKT_REQUEST_SEND_T),
				 &gmpi_send_buf_ptr);
  
  DEBUG_PRINT_MSG("S Starting Rndvn_isend");
  
  pkt_ptr->mode = MPID_PKT_REQUEST_SEND;
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
  gmpi_debug_assert(pkt_ptr->send_id != NULL);
  GMPI_DEBUG_CHECKSUM_COMPUTE(&(pkt_ptr->cksum_large), buf, len);
  
  /* Store info in the request for completing the message */
  shandle->is_complete = 0;
  shandle->start = buf;
  shandle->bytes_as_contig = len;
  shandle->gm.ptr = NULL;
  shandle->gm.current_expected = 0;
  shandle->gm.current_offset = 0;
  shandle->gm.current_done = 0;
  shandle->gm.rendez_vous = 1;
  /* Set the test/wait functions */
  shandle->wait = 0;
  shandle->test = 0;
  /* Store partners rank in request in case message is cancelled */
  shandle->partner = dest;
  /* shandle->finish must NOT be set here; it must be cleared/set
     when the request is created */

  DEBUG_PRINT_BASIC_SEND_PKT("S Sending rndv message", pkt_ptr);
  MPID_n_pending++;
  gmpi_debug_assert(pkt_ptr->send_id != NULL);
  gmpi_send_packet(gmpi_send_buf_ptr, dest);
  MPID_DRAIN_INCOMING
  if (gmpi_sync_send == 0)
    {
      GMPI_PROGRESSION_UNLOCK();
    }
  
  return MPI_SUCCESS;
}

int MPID_CH_Rndvn_send( buf, len, src_lrank, tag, context_id, dest,
			 msgrep )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
{
  MPIR_SHANDLE shandle;
  
  GMPI_PROGRESSION_LOCK();
  gmpi_sync_send = 1;
  DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
  MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE);
  MPID_SendInit( &shandle );
  shandle.finish = 0;
  MPID_CH_Rndvn_isend( buf, len, src_lrank, tag, context_id, dest,
		       msgrep, &shandle );
  
  GMPI_PROGRESSION_UNLOCK();
  while (!shandle.is_complete)
    {
      MPID_DeviceCheck(MPID_BLOCKING);
    }
  GMPI_PROGRESSION_LOCK();
  
  if (shandle.finish)
    {
      shandle.finish(&shandle);
    }
  gmpi_sync_send = 0;
  GMPI_PROGRESSION_UNLOCK();

  return MPI_SUCCESS;
}

static void 
ok_to_send_for_zero_length_packet(MPIR_RHANDLE *rhandle)
{
  MPID_CH_Rndvn_ok_to_send(rhandle);
  rhandle->is_complete = 1;
  if (rhandle->finish)
    {
      (rhandle->finish)(rhandle);
    }
}

/*
 * This is the routine called when a packet of type MPID_PKT_REQUEST_SEND is
 * seen and the receive has been posted.  Note the use of a nonblocking
 * receiver BEFORE sending the ack.
 */
int MPID_CH_Rndvn_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
  DEBUG_PRINT_MSG("R Starting rndvn irecv");
  
  return do_recv(rhandle, from, in_pkt, 1);
}


/* Save an unexpected message in rhandle.  This is the same as
   MPID_CH_Rndvb_save except for the "push" function */
int MPID_CH_Rndvn_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
  DEBUG_PRINT_MSG("Saving info on unexpected message");
  
  rhandle->buf = 0;
  return do_recv(rhandle, from, in_pkt, 0);
}

/*
 * This is an internal routine to return an OK TO SEND packet.
 * It is the same as the Rndvb version
 */
int 
MPID_CH_Rndvn_ok_to_send(MPIR_RHANDLE * rhandle)
{
  gm_up_t reg_start;
  unsigned int registered, chunk_size;
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  MPID_PKT_OK_TO_SEND_T * pkt_ptr;
#if GM_DISABLE_REGISTRATION
  gm_up_t bounce_addr;
#endif

  if (rhandle->gm.current_expected > 0)
    {
      while ((rhandle->gm.in_pipe < GMPI_MAX_PUT_IN_PIPE) 
	     && (rhandle->gm.current_offset 
		 < rhandle->gm.current_expected))
	{
	  chunk_size = (rhandle->gm.current_expected 
			- rhandle->gm.current_offset);
	  if (chunk_size > GMPI_MAX_PUT_LENGTH)
	    {
	      chunk_size = GMPI_MAX_PUT_LENGTH;
	    }
	  
	  reg_start = ((gm_up_t)rhandle->buf + rhandle->gm.current_offset);

#if GM_DISABLE_REGISTRATION	      
	      bounce_addr = gmpi_allocate_bounce_buffer(GMPI_BOUNCE_RECV, 
							reg_start, chunk_size);
	      if (bounce_addr == 0)
		{
		  registered = 0;
		}
	      else
		{
		  reg_start = bounce_addr;
		  registered = chunk_size;
		}
#else
	      registered = gmpi_use_interval(reg_start, chunk_size);
#endif
	  pkt_ptr = gmpi_allocate_packet(sizeof(MPID_PKT_OK_TO_SEND_T),
					 &gmpi_send_buf_ptr);
	  pkt_ptr->mode = MPID_PKT_OK_TO_SEND;
	  pkt_ptr->lrank = MPID_GM_rank;
	  pkt_ptr->src = MPID_GM_rank;
          gmpi_debug_assert(rhandle->send_id != NULL);
	  pkt_ptr->send_id = rhandle->send_id;
          MPID_AINT_SET(pkt_ptr->recv_id, rhandle);
	  MPID_AINT_SET(pkt_ptr->target_ptr, (void *)reg_start);
	  gmpi_debug_assert((void *)reg_start != NULL);
	  DEBUG_PRINT_BASIC_SEND_PKT("S Ok send", pkt_ptr);
	  if (registered == chunk_size)
	    {
	      gmpi_send_packet(gmpi_send_buf_ptr, rhandle->from);
	    }
	  else
	    {
#if !GM_DISABLE_REGISTRATION
	      gmpi_unuse_interval(reg_start, registered);
#endif
	      gmpi_queue_packet_register(gmpi_send_buf_ptr,
					 (void *)reg_start,
					 chunk_size,
					 rhandle->from);
	    }
	  rhandle->gm.current_offset += chunk_size;
	  rhandle->gm.in_pipe++;
	}
    }
  else
    {
      pkt_ptr = gmpi_allocate_packet(sizeof(MPID_PKT_OK_TO_SEND_T),
				     &gmpi_send_buf_ptr);
      pkt_ptr->mode = MPID_PKT_OK_TO_SEND;
      pkt_ptr->lrank = MPID_GM_rank;
      pkt_ptr->src = MPID_GM_rank;
      pkt_ptr->send_id = rhandle->send_id;
      MPID_AINT_SET(pkt_ptr->recv_id, rhandle);
      MPID_AINT_SET(pkt_ptr->target_ptr, NULL);
      DEBUG_PRINT_BASIC_SEND_PKT("S Ok send", pkt_ptr);
      gmpi_send_packet(gmpi_send_buf_ptr, rhandle->from);
    }
  
  return MPI_SUCCESS;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message.  Note that we start a nonblocking receive FIRST.
 */
int MPID_CH_Rndvn_unxrecv_start( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
  MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;

  int msglen;
  int err = MPI_SUCCESS;

  rhandle->recv_handle = -1;
  rhandle->s = runex->s;
  /* we use rhandle->buf != 0 to mean recv posted */
  gmpi_debug_assert(rhandle->buf != NULL);
  
  /* Must NOT set finish, since it may have been set elsewhere */
  msglen = runex->gm.current_expected;
  rhandle->is_complete = 0;
  rhandle->push	= 0;
  rhandle->from = runex->from;
  rhandle->partner = runex->partner;
  rhandle->send_id = runex->send_id;
  gmpi_debug_assert(rhandle->send_id != NULL);
  rhandle->unex_buf = 0;
  rhandle->gm.netbuf = runex->gm.netbuf;
  rhandle->gm.current_offset = 0;
  rhandle->gm.current_done = 0;
  rhandle->gm.in_pipe = 0;
  GMPI_DEBUG_CHECKSUM_COPY(&(rhandle->gm.cksum), &(runex->gm.cksum));
  MPID_CHK_MSGLEN(rhandle, msglen, err);
  rhandle->gm.current_expected = msglen;
  rhandle->s.count = msglen;
  
  if (msglen)
    {
      MPID_CH_Rndvn_ok_to_send(rhandle);
    }
  else
    {
      gmpi_debug_assert(rhandle->gm.current_expected == 0);
      ok_to_send_for_zero_length_packet(rhandle);
    }
  MPID_RecvFree(runex);
  
  return err;
}



/* 
 * This is the routine that is called when an "ok to send" packet is
 * received.  
 */
int MPID_CH_Rndvn_ack( in_pkt, from_grank )
void  *in_pkt;
int   from_grank;
{
  MPID_PKT_OK_TO_SEND_T *pkt = (MPID_PKT_OK_TO_SEND_T *)in_pkt;
  MPIR_SHANDLE *shandle=0;
  unsigned int chunk_size;
  void * target_ptr;
  
  DEBUG_PRINT_MSG("R Starting Rndvb_ack");
  
    MPID_AINT_GET(shandle,pkt->send_id);
#ifdef MPIR_HAS_COOKIES
    if (shandle->cookie != MPIR_REQUEST_COOKIE) {
	FPRINTF( stderr, "shandle is %lx\n", (long)shandle );
	FPRINTF( stderr, "shandle cookie is %lx\n", shandle->cookie );
	MPID_Print_shandle( stderr, shandle );
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		    "Bad address in Rendezvous send" );
    }
#endif	
  
#ifdef MPID_DEBUG_ALL
    if (MPID_DebugFlag) {
	FPRINTF( MPID_DEBUG_FILE, "[%d]S for ", MPID_GM_rank );
	MPID_Print_shandle( MPID_DEBUG_FILE, shandle );
    }
#endif

  DEBUG_PRINT_MSG("Sending data on channel with nonblocking send");
  shandle->is_complete = 0;
  
  if (shandle->bytes_as_contig > 0)
    {
      struct gmpi_send_buf * gmpi_send_buf_ptr;
      MPID_PKT_DONE_T * pkt_ptr;
      
      MPID_AINT_GET(target_ptr, pkt->target_ptr);
      gmpi_debug_assert(target_ptr != NULL);
      
      if (shandle->gm.current_offset + GMPI_MAX_PUT_LENGTH 
	  >= shandle->bytes_as_contig)
	{
	  MPID_n_pending--;
	}
      
      chunk_size = (shandle->bytes_as_contig 
		    - shandle->gm.current_offset);
      gmpi_debug_assert(chunk_size > 0);
      if (chunk_size > GMPI_MAX_PUT_LENGTH)
	{
	  chunk_size = GMPI_MAX_PUT_LENGTH;
	}
      
      pkt_ptr = gmpi_allocate_packet(sizeof(MPID_PKT_DONE_T),
				     &gmpi_send_buf_ptr);
      pkt_ptr->mode = MPID_PKT_DONE_SEND;
      pkt_ptr->lrank = MPID_GM_rank;
      pkt_ptr->src = MPID_GM_rank;
      pkt_ptr->recv_id = pkt->recv_id;
      MPID_AINT_SET(pkt_ptr->done_target_ptr, target_ptr);
      DEBUG_PRINT_BASIC_SEND_PKT("S Ack Long send", pkt_ptr);
      
      gmpi_put_data(shandle, from_grank, chunk_size,
		    (void *)((gm_up_t)shandle->start 
			     + shandle->gm.current_offset), target_ptr);
      shandle->gm.current_offset += chunk_size;
      gmpi_debug_assert(shandle->gm.current_offset 
			<= shandle->bytes_as_contig);
      gmpi_send_packet(gmpi_send_buf_ptr, from_grank);
    }
  else
    {
      MPID_n_pending--;
      shandle->is_complete = 1;
      if (shandle->finish)
	(shandle->finish)( shandle );
    }
  return MPI_SUCCESS;
}

static int 
do_recv(MPIR_RHANDLE * rhandle, int from, void * in_pkt, int expected)
{
  int err = 0;
  int msglen;
  
  MPID_PKT_REQUEST_SEND_T * pkt = (MPID_PKT_REQUEST_SEND_T *)in_pkt;

  MPIR_SET_COOKIE((rhandle), MPIR_REQUEST_COOKIE); 
  msglen = pkt->len;
  if (expected)
    {
      MPID_CHK_MSGLEN(rhandle, msglen, err);
    }

  gmpi_debug_assert(pkt->send_id != NULL);
  rhandle->s.MPI_TAG = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR = err;
  rhandle->s.count = msglen;
  rhandle->is_complete = 0;
  rhandle->from = from;
  rhandle->partner = pkt->to; 
  rhandle->send_id = pkt->send_id;
  rhandle->unex_buf = 0;
  rhandle->gm.netbuf = NULL;
  rhandle->gm.current_expected = msglen;
  rhandle->gm.current_offset = 0;
  rhandle->gm.current_done = 0;
  rhandle->gm.in_pipe = 0;
  GMPI_DEBUG_CHECKSUM_COPY(&(rhandle->gm.cksum), &(pkt->cksum_large));
  MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);

  if (expected)
    {
      if (msglen)
	{
	  gmpi_debug_assert(rhandle->buf != NULL);
	  gmpi_debug_assert(rhandle->gm.current_expected > 0);
          gmpi_debug_assert(rhandle->send_id != NULL);
	  MPID_CH_Rndvn_ok_to_send(rhandle);
	}
      else
	{
          gmpi_debug_assert(rhandle->send_id != NULL);
	  gmpi_debug_assert(rhandle->gm.current_expected == 0);
	  ok_to_send_for_zero_length_packet(rhandle);
	}
    }
  else
    {
      gmpi_debug_assert(rhandle->send_id != NULL);
      rhandle->push = MPID_CH_Rndvn_unxrecv_start;
    }
  return MPI_SUCCESS;
}


void MPID_CH_Rndvn_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}


/*
 * The only routing really visable outside this file; it defines the
 * Blocking Rendezvous protocol.
 */
MPID_Protocol *MPID_CH_Rndvn_setup()
{
    MPID_Protocol *p;
  
    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
  p->send = MPID_CH_Rndvn_send;
  p->recv = 0;
  p->isend = MPID_CH_Rndvn_isend;
  p->wait_send = 0;
  p->push_send = 0;
  p->cancel_send = 0;
  p->irecv = MPID_CH_Rndvn_irecv;
  p->wait_recv = 0;
  p->push_recv = 0;
  p->cancel_recv = 0;
  p->do_ack = MPID_CH_Rndvn_ack;
  p->unex = MPID_CH_Rndvn_save;
  p->delete = MPID_CH_Rndvn_delete;

  return p;
}
