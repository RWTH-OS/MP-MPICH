/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"
#include "gmpi_smpi.h"
#include "gmpid_debug.h"

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"


/* Prototype definitions */
int MPID_SMP_Rndvn_send ( void *, int, int, int, int, int, MPID_Msgrep_t );
int MPID_SMP_Rndvn_isend ( void *, int, int, int, int, int,
			   MPID_Msgrep_t, MPIR_SHANDLE * );
int MPID_SMP_Rndvn_irecv ( MPIR_RHANDLE *, int, void * );
int MPID_SMP_Rndvn_save ( MPIR_RHANDLE *, int, void *);
int MPID_SMP_Rndvn_unxrecv_posted ( MPIR_RHANDLE *, void * );
void MPID_SMP_Rndvn_delete ( MPID_Protocol * );
int  MPID_DeviceCheck(MPID_BLOCKING_TYPE blocking);

int smpi_sync_send = 0;

/*
 * This is really the same as the blocking version, since the 
 * nonblocking operations occur only in the data transmission.
 */
int MPID_SMP_Rndvn_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE  *shandle;
{
  int destination = smpi.local_nodes[dest];
  
  DEBUG_PRINT_MSG("S Starting Rndvn_isend");

  if (smpi_sync_send == 0)
    GMPI_PROGRESSION_LOCK();
#if SMP_ENABLE_DIRECTCOPY 
  
  DEBUG_PRINT_MSG("Using directcopy protocol"); 
  if (len > 0)
    {
      gmpi_use_interval((unsigned long)buf, len);
    }
  /* enough place to send the GET_request ? */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, sizeof(SMPI_PKT_GET_T)))
    {
      smpi_post_send_get(buf,len,src_lrank,tag, context_id,
			 destination, shandle);
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_GET_T * pkt_p;
      pkt_p = (SMPI_PKT_GET_T *)malloc(sizeof(SMPI_PKT_GET_T));
      smpi_malloc_assert(pkt_p,
			 "MPID_SMP_Rndvn_isend",
			 "malloc: pkt allocation");
      
      gmpi_debug_assert(dest > -1);
      gmpi_debug_assert(dest < MPID_GM_size);
      gmpi_debug_assert(destination > -1);
      gmpi_debug_assert(destination < smpi.num_local_nodes);
      
      pkt_p->mode       = MPID_PKT_DO_GET;
      pkt_p->context_id = context_id;
      pkt_p->lrank      = src_lrank;
      pkt_p->tag	      = tag;
      pkt_p->len     = len;
      pkt_p->send_id    = shandle;
      pkt_p->address    = buf;
      
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_GET_T), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
#else
  
  DEBUG_PRINT_MSG("Using 2 copies protocol");
  /* enough place to send the REQUEST_SEND request ? */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, sizeof(SMPI_PKT_RNDV_T)))
    {
      smpi_post_send_rndv(buf,len,src_lrank,tag, context_id,
			  destination, shandle);
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_RNDV_T * pkt_p;
      pkt_p = (SMPI_PKT_RNDV_T *)malloc(sizeof(SMPI_PKT_RNDV_T));
      smpi_malloc_assert(pkt_p,
			 "MPID_SMP_Rndvn_isend",
			 "malloc: pkt allocation");

      gmpi_debug_assert(dest < MPID_GM_size);
      gmpi_debug_assert(dest > -1);
      gmpi_debug_assert(destination > -1);
      gmpi_debug_assert(destination < smpi.num_local_nodes);
      
      pkt_p->mode       = MPID_PKT_REQUEST_SEND;
      pkt_p->context_id = context_id;
      pkt_p->lrank      = src_lrank;
      pkt_p->tag	      = tag;
      pkt_p->len     = len;
      pkt_p->send_id = shandle;

      GMPI_DEBUG_CHECKSUM_COMPUTE(&(pkt_p->cksum_large), buf, len);
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_RNDV_T), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
  shandle->gm.ptr = buf;
  shandle->gm.current_offset = len;
  shandle->gm.current_expected = 1;
  shandle->gm.current_done = 0;
#endif
  
  shandle->gm.rendez_vous = 1;
  shandle->is_complete     = 0;
  /* Store partners rank in request in case message is cancelled */
  shandle->partner         = dest;
  
  MPID_n_pending++;
  smpi.pending++;
  if (smpi_sync_send == 0)
    {
      GMPI_PROGRESSION_UNLOCK();
    }

  return MPI_SUCCESS;
}

int MPID_SMP_Rndvn_send( buf, len, src_lrank, tag, context_id, dest,
			 msgrep )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
{
    MPIR_SHANDLE shandle;

    GMPI_PROGRESSION_LOCK();
    smpi_sync_send = 1;
    DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE);
    MPID_SendInit( &shandle );
    shandle.finish = 0;
    MPID_SMP_Rndvn_isend( buf, len, src_lrank, tag, context_id, dest,
			  msgrep, &shandle );
    GMPI_PROGRESSION_UNLOCK();
    while (!shandle.is_complete) 
      MPID_DeviceCheck(MPID_BLOCKING);
    
    GMPI_PROGRESSION_LOCK();
    if (shandle.finish)
      shandle.finish(&shandle);
    smpi_sync_send = 0;
    GMPI_PROGRESSION_UNLOCK();
    
    return MPI_SUCCESS;
}


int MPID_SMP_Rndvn_irecv( rhandle, from, in_pkt )
     MPIR_RHANDLE *rhandle;
     int          from;
     void         *in_pkt;
{
  int my_id = smpi.my_local_id; 
  int err = MPI_SUCCESS;
  int msglen;
  SMPI_PKT_GET_T   *pkt = (SMPI_PKT_GET_T *)in_pkt;
    
  DEBUG_PRINT_MSG("R Starting rndvn irecv");
  msglen = pkt->len;
  MPID_CHK_MSGLEN(rhandle,msglen,err);


  rhandle->s.MPI_TAG	= pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR  = err;
  rhandle->s.count      = msglen;
  rhandle->from         = from;
  rhandle->send_id      = pkt->send_id;

  /* the big job : do the get */  
  smpi_do_get(from, pkt->address, rhandle->buf, msglen);

  /* post the ack to free the send buffer */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(from, sizeof(SMPI_PKT_GET_T)))
    {
      smpi_post_send_done_get(from, rhandle->send_id);
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_GET_T * pkt_p;
      pkt_p = (SMPI_PKT_GET_T *)malloc(sizeof(SMPI_PKT_GET_T));
      smpi_malloc_assert(pkt_p,
			 "MPID_SMP_Rndvn_irecv",
			 "malloc: pkt allocation");

      gmpi_debug_assert(from > -1);
      gmpi_debug_assert(from < smpi.num_local_nodes);
      
      pkt_p->mode    = MPID_PKT_DONE_GET;
      pkt_p->send_id = rhandle->send_id;
     
      /* is this the right macro here? there's no destination declared, so it won't work */
      /* DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p); */
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_GET_T), from);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
  /* flow control */
  smpi_complete_recv(from, my_id, sizeof(SMPI_PKT_GET_T));
  
  rhandle->is_complete  = 1;
  if (rhandle->finish)
    (rhandle->finish)(rhandle);
  return MPI_SUCCESS;
}


int MPID_SMP_Rndvn_save( rhandle, from, in_pkt )
     MPIR_RHANDLE *rhandle;
     int          from;
     void         *in_pkt;
{
  int my_id = smpi.my_local_id; 

#if SMP_ENABLE_DIRECTCOPY 
   SMPI_PKT_GET_T   *pkt = (SMPI_PKT_GET_T *)in_pkt;
  
  rhandle->s.MPI_TAG	= pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR  = 0;
  rhandle->s.count      = pkt->len;
  rhandle->is_complete  = 0;
  rhandle->from         = from;
  rhandle->send_id      = pkt->send_id;
  rhandle->start        = pkt->address;
  rhandle->unex_buf     = 0;
  rhandle->push = MPID_SMP_Rndvn_unxrecv_posted;
  
  smpi_complete_recv(from, my_id, sizeof(SMPI_PKT_GET_T));

#else   

  SMPI_PKT_RNDV_T   *pkt = (SMPI_PKT_RNDV_T *)in_pkt;
  
  rhandle->s.MPI_TAG	= pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR  = 0;
  rhandle->s.count      = pkt->len;
  rhandle->is_complete  = 0;
  rhandle->from         = from;
  rhandle->unex_buf     = 0;
  rhandle->push = MPID_SMP_Rndvn_unxrecv_posted;
  GMPI_DEBUG_CHECKSUM_COPY(&(rhandle->gm.cksum), &(pkt->cksum_large));
  
  smpi_complete_recv(from, my_id, (sizeof(SMPI_PKT_RNDV_T)));
#endif
  
  return MPI_SUCCESS;
}



/* 
 * This routine is called when it is time to receive an unexpected
 * message.  
 */
int MPID_SMP_Rndvn_unxrecv_posted( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int destination = runex->from;
    int msglen, err = 0;
 
    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle, msglen, err);     
    rhandle->s.count = msglen;

#if SMP_ENABLE_DIRECTCOPY
    smpi_do_get(destination, runex->start, rhandle->buf, runex->s.count);  
    if (!smpi.send_fifo_head 
	&& smpi_able_to_send(destination, sizeof(SMPI_PKT_GET_T)))
      {
	smpi_post_send_done_get(destination, runex->send_id);
      }
    else
      {
	/* not enough place, we will send it later */
	SMPI_PKT_GET_T * pkt_p;
	pkt_p = (SMPI_PKT_GET_T *)malloc(sizeof(SMPI_PKT_GET_T));
	smpi_malloc_assert(pkt_p,
			   "MPID_SMP_Rndvn_unxrecv_posted",
			   "malloc: pkt allocation");
	
	gmpi_debug_assert(destination > -1);
	gmpi_debug_assert(destination < smpi.num_local_nodes);
    
	pkt_p->mode    = MPID_PKT_DONE_GET;
	pkt_p->send_id = runex->send_id;
	
	DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
	DEBUG_PRINT_MSG("S Sending message in the send_queue");
	smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_GET_T), destination);
	DEBUG_PRINT_MSG("S Sent message in the send_queue");
      }
    
    rhandle->s.MPI_TAG = runex->s.MPI_TAG;
    rhandle->s.MPI_SOURCE = runex->s.MPI_SOURCE;
    rhandle->s.MPI_ERROR = 0;
    rhandle->s.count = runex->s.count; 
    rhandle->from = destination;
    rhandle->is_complete = 1;
    if (rhandle->finish)
      (rhandle->finish)(rhandle);
#else
    rhandle->send_id = runex->send_id;
    rhandle->s.MPI_TAG = runex->s.MPI_TAG;
    rhandle->s.MPI_SOURCE = runex->s.MPI_SOURCE;
    rhandle->s.MPI_ERROR = 0;
    rhandle->s.count = runex->s.count; 
    rhandle->from = destination;
    rhandle->is_complete = 0;
    GMPI_DEBUG_CHECKSUM_COPY(&(rhandle->gm.cksum), &(runex->gm.cksum));

    smpi_post_send_ok_to_send(destination, rhandle);
#endif
    
    MPID_RecvFree(runex);
   
    return err;
}


void MPID_SMP_Rndvn_delete( p )
MPID_Protocol *p;
{
  FREE( p );
}

/*
 * The only routing really visable outside this file; it defines the
 * Blocking Rendezvous protocol.
 */
MPID_Protocol *MPID_SMP_Rndv_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SMP_Rndvn_send;
    p->recv	   = 0;
    p->isend	   = MPID_SMP_Rndvn_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = MPID_SMP_Rndvn_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_SMP_Rndvn_save;
    p->delete      = MPID_SMP_Rndvn_delete;

    return p;
}
