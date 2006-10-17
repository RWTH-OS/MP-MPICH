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
#include "gm_stbar.h"
#include "gmpi_smpi.h"
#include "gmpid_debug.h"


/* Prototype definitions */
int MPID_SMP_Eagerb_send_short(void *, int, int, int, int, int,
			       MPID_Msgrep_t);
int MPID_SMP_Eagerb_isend_short(void *, int, int, int, int, int, 
				  MPID_Msgrep_t, MPIR_SHANDLE *);
int MPID_SMP_Eagerb_recv_short(MPIR_RHANDLE *, int, void *);
int MPID_SMP_Eagerb_save_short(MPIR_RHANDLE *, int, void *);
int MPID_SMP_Eagerb_unxrecv_start_short(MPIR_RHANDLE *, void *);
void MPID_SMP_Eagerb_short_delete(MPID_Protocol *);
int MPID_SMP_Cancel_send(MPIR_SHANDLE *);

/*
 * Definitions of the actual functions
 */
int 
MPID_SMP_Eagerb_send_short(void * buf, int len, int src_lrank,
			   int tag, int context_id, int dest,
			   MPID_Msgrep_t msgrep)
{
  int destination = smpi.local_nodes[dest];
  
  GMPI_PROGRESSION_LOCK();
  DEBUG_PRINT_MSG("S starting MPID_SMP_Eagerb_send_short");
  /* Flow control : is there enough place on the shared memory area */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, len+sizeof(SMPI_PKT_HEAD_T)))
    {
      /* let's go to send it */
      smpi_post_send_bufferinplace(buf, len, src_lrank, tag,
				   context_id, destination, NULL);
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_SHORT_T * pkt_ptr;
      
      while (smpi.malloc_send_buf_allocated > SMPI_MAX_DELAYED_SEND_QUEUE_SIZE)
	{
	  MPID_DeviceCheck(MPID_BLOCKING);
	} 

      pkt_ptr = (SMPI_PKT_SHORT_T *)malloc(len + sizeof(SMPI_PKT_HEAD_T));
      smpi_malloc_assert(pkt_ptr, "MPID_SMP_Eagerb_send_short",
			 "malloc: pkt allocation");
      
      /* These references are ordered to match the order they appear in the 
	 structure */
      pkt_ptr->mode = MPID_PKT_SHORT;
      pkt_ptr->context_id = context_id;
      pkt_ptr->lrank = src_lrank;
      pkt_ptr->tag = tag;
      pkt_ptr->len = len;

      gmpi_debug_assert(dest < MPID_GM_size);
      gmpi_debug_assert(len < gmpi.eager_size);
      gmpi_debug_assert(destination > -1);
      gmpi_debug_assert(destination < smpi.num_local_nodes);
      
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_ptr);
      
      if (len > 0)
	{
	  memcpy((void *)((gm_up_t)pkt_ptr + sizeof(SMPI_PKT_SHORT_T)), 
                 buf, len);
	  DEBUG_PRINT_PKT_DATA("S Getting data from buf", pkt_ptr);
	}
      
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_ptr, 0, (len + sizeof(SMPI_PKT_HEAD_T)), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
  MPID_DRAIN_INCOMING
  GMPI_PROGRESSION_UNLOCK();
  return MPI_SUCCESS;
}

int 
MPID_SMP_Eagerb_isend_short(void * buf, int len, int src_lrank,
			    int tag, int context_id, int dest,
			    MPID_Msgrep_t msgrep, MPIR_SHANDLE * shandle)
{
  int destination = smpi.local_nodes[dest];
  
  GMPI_PROGRESSION_LOCK();
  
  DEBUG_PRINT_MSG("S starting MPID_SMP_Eagerb_isend_short");
  
  /* Flow control : is there enough place on the shared memory area */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, len+sizeof(SMPI_PKT_HEAD_T)))
    {
      /* let's go to send it ! */
      smpi_post_send_bufferinplace(buf, len, src_lrank, tag,
				   context_id, destination, shandle);
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_SHORT_T * pkt_ptr;
      pkt_ptr = (SMPI_PKT_SHORT_T *)malloc(len + sizeof(SMPI_PKT_HEAD_T));
      smpi_malloc_assert(pkt_ptr, "MPID_SMP_Eagerb_isend_short",
			 "malloc: pkt allocation");
      
      /* These references are ordered to match the order they appear in the 
	 structure */
      pkt_ptr->mode = MPID_PKT_SHORT;
      pkt_ptr->context_id = context_id;
      pkt_ptr->lrank = src_lrank;
      pkt_ptr->tag = tag;
      pkt_ptr->len = len;
      pkt_ptr->send_id = shandle;

      gmpi_debug_assert(dest < MPID_GM_size);
      gmpi_debug_assert(len < gmpi.eager_size);
      gmpi_debug_assert(destination > -1);
      gmpi_debug_assert(destination < smpi.num_local_nodes);
      
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_ptr);
      
      if (len > 0)
	{
	  memcpy((void *)((gm_up_t)pkt_ptr + sizeof(SMPI_PKT_SHORT_T)), 
                 buf, len);
	  DEBUG_PRINT_PKT_DATA("S Getting data from buf", pkt_ptr);
	}
      
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_ptr, shandle, (len + sizeof(SMPI_PKT_HEAD_T)),
		      destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
  /* Store partners rank in request in case message is cancelled */
  shandle->partner = dest;    
  shandle->gm.rendez_vous = 0;
  
  shandle->is_complete = 1;
  MPID_DRAIN_INCOMING
  GMPI_PROGRESSION_UNLOCK();
  return MPI_SUCCESS;
}


int 
MPID_SMP_Eagerb_recv_short(MPIR_RHANDLE * rhandle, int from_grank,
			   void * in_pkt)
{
  SMPI_PKT_SHORT_T * pkt = (SMPI_PKT_SHORT_T *)in_pkt;
  int msglen;
  int err = MPI_SUCCESS;
  int my_id = smpi.my_local_id;
  
  msglen		  = pkt->len;

  DEBUG_PRINT_MSG("R Starting Eagerb_recv_short");
  rhandle->s.MPI_TAG = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;

  MPID_CHK_MSGLEN(rhandle,msglen,err);
  if (msglen > 0) {
      memcpy(rhandle->buf,
	     (void *)((gm_up_t)pkt + sizeof(SMPI_PKT_SHORT_T)), 
	     msglen);
    }
  rhandle->s.count = msglen;
  rhandle->s.MPI_ERROR = err;
  if (rhandle->finish) {
        (rhandle->finish)( rhandle );
    }
  rhandle->is_complete = 1;
  
  /* Flow control */
  smpi_complete_recv(from_grank, my_id, (pkt->len + sizeof(SMPI_PKT_HEAD_T)));
  
  return err;
}


/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int 
MPID_SMP_Eagerb_unxrecv_start_short(MPIR_RHANDLE *rhandle,
				    void * in_runex)
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
    }
  rhandle->s = runex->s;
  rhandle->s.count = msglen;
  rhandle->s.MPI_ERROR = err;
  rhandle->wait	= 0;
  rhandle->test	= 0;
  rhandle->push	= 0;
  rhandle->is_complete = 1;
  if (rhandle->finish) 
        (rhandle->finish)( rhandle );
  MPID_RecvFree( runex );
  
  return err;
}


/* Save an unexpected message in rhandle */
int 
MPID_SMP_Eagerb_save_short(MPIR_RHANDLE * rhandle, int from,
			   void * in_pkt)
{
  int my_id = smpi.my_local_id;
  SMPI_PKT_SHORT_T * pkt = (SMPI_PKT_SHORT_T *)in_pkt;
  
  DEBUG_PRINT_MSG("R Starting Eagerb_save_short");
  rhandle->s.MPI_TAG = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR = 0;
  rhandle->from = from;
  rhandle->s.count = pkt->len;
  rhandle->send_id = pkt->send_id;
  
  if (pkt->len > 0) {
      rhandle->start = (void *)MALLOC(pkt->len);
      smpi_malloc_assert(rhandle->start,
			 "MPID_SMP_Eagerb_save_short",
			 "malloc: unexpected save");
      if (!rhandle->start) {
	  rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	  return 1;
	}
      memcpy(rhandle->start,
	     (void *)((gm_up_t)pkt + sizeof(SMPI_PKT_SHORT_T)), 
             pkt->len);
    }
  rhandle->push = MPID_SMP_Eagerb_unxrecv_start_short;
  
  /* flow control */
  smpi_complete_recv(from, my_id, (pkt->len + sizeof(SMPI_PKT_HEAD_T)));
  
  return 0;
}


void 
MPID_SMP_Eagerb_short_delete(MPID_Protocol * p)
{
    FREE( p );
}


int 
MPID_SMP_Cancel_send (MPIR_SHANDLE * shandle)
{
  volatile void *ptr_volatile;
  void *ptr;
  SMPI_PKT_ANTI_SEND_T *pkt;
  int destination = smpi.local_nodes[shandle->partner];
  
  gmpi_debug_assert(shandle->partner > -1);
  gmpi_debug_assert(shandle->partner < MPID_GM_size);
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);
  
  GMPI_PROGRESSION_LOCK();
  DEBUG_PRINT_MSG("S starting MPID_SMP_Cancel_send");
  /* Flow control : is there enough place on the shared memory area */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, sizeof(SMPI_PKT_ANTI_SEND_T)))
    {
      /* let's go to send it */
      ptr_volatile = (void *)((&smpi_shmem->pool)
			      + SMPI_NEXT(smpi.my_local_id, destination));
      ptr = (void *)ptr_volatile;
      pkt = (SMPI_PKT_ANTI_SEND_T *)ptr;
      pkt->mode = MPID_PKT_ANTI_SEND;
      pkt->send_id = shandle;
      DEBUG_PRINT_MSG("S Sending anti_send message\n"); 
      
      /* update flow control */
      smpi_complete_send(smpi.my_local_id, destination, 
			 sizeof(SMPI_PKT_ANTI_SEND_T));
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_ANTI_SEND_T * pkt_ptr;
      pkt_ptr = (SMPI_PKT_ANTI_SEND_T *)malloc (sizeof (SMPI_PKT_ANTI_SEND_T));
      smpi_malloc_assert (pkt_ptr, "MPID_SMP_Cancel_send",
			  "malloc: pkt allocation");
      
      pkt_ptr->mode = MPID_PKT_ANTI_SEND;
      pkt_ptr->send_id = shandle;
      
      DEBUG_PRINT_MSG("S Sending anti_send message in the send_queue\n");
      smpi_queue_send(pkt_ptr, 0, sizeof (SMPI_PKT_ANTI_SEND_T), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }

  GMPI_PROGRESSION_UNLOCK();
  return MPI_SUCCESS;
}


MPID_Protocol *
MPID_SMP_Short_setup(void)
{
  MPID_Protocol * p;
  
  p = (MPID_Protocol *)MALLOC(sizeof(MPID_Protocol));
  if (!p) 
    {
      return 0;
    }
  
  p->send = MPID_SMP_Eagerb_send_short;
  p->recv = MPID_SMP_Eagerb_recv_short;
  p->isend = MPID_SMP_Eagerb_isend_short;
  p->wait_send = 0;
  p->push_send = 0;
  p->cancel_send = MPID_SMP_Cancel_send;
  p->irecv = 0;
  p->wait_recv = 0;
  p->push_recv = 0;
  p->cancel_recv = 0;
  p->do_ack = 0;
  p->unex = MPID_SMP_Eagerb_save_short;
  p->delete = MPID_SMP_Eagerb_short_delete;
  
  return p;
}
