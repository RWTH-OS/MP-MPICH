/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#if !GM_OS_VXWORKS

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/mman.h>
#endif

#include "gmpi_packets.h"
#include "gmpi.h"
#include "gmpi_smpi.h"
#include "gmpid_debug.h"
#include "mpiddev.h"
#include "gm_stbar.h"
#include "reqalloc.h"
#include "../util/queue.h"

#ifndef GM_WRITEBAR
#define GM_WRITEBAR() GM_STBAR()
#endif
#ifndef GM_READBAR
#define GM_READBAR() GM_STBAR()
#endif

extern int expect_cancel_ack;

struct smpi_var smpi;
struct shared_mem * smpi_shmem;


void 
smpi_malloc_assert(void * ptr, char * fct, char * msg)
{
  if (ptr == NULL)
    {
      fprintf(stderr, "[%d]: alloc failed, not enough memory (Fatal Error)\n"
	      "Context: <%s: %s>\n", MPID_GM_rank, fct, msg);
      gmpi_abort (0);
    }
}


/* indicate if there is enough place in the shared memory receive queue
   for this message : flow control (unit = byte) */
gm_inline unsigned int 
smpi_able_to_send(int dest, int len)
{
  return ((((SMPI_TOTALIN(smpi.my_local_id, dest)) 
	    >= (SMPI_TOTALOUT(smpi.my_local_id, dest)))
	   && ((SMPI_TOTALIN(smpi.my_local_id, dest) 
		- SMPI_TOTALOUT(smpi.my_local_id, dest) 
		+ SMPI_ALIGN(len)) < (smpi.available_queue_length)))
	  || (((SMPI_TOTALIN(smpi.my_local_id, dest)) 
	       < (SMPI_TOTALOUT(smpi.my_local_id, dest)))
	      && ((SMPI_MAX_INT-SMPI_TOTALOUT(smpi.my_local_id, dest)
		   + SMPI_TOTALIN(smpi.my_local_id, dest)
		   + SMPI_ALIGN(len)) < (smpi.available_queue_length))));
}

gm_inline void 
smpi_complete_send(unsigned int my_id, 
		   unsigned int destination, 
		   unsigned int length)
{
  SMPI_NEXT(my_id, destination) += SMPI_ALIGN(length);
  if ((SMPI_NEXT(my_id, destination)) > (SMPI_LAST(my_id, destination))) {
    SMPI_NEXT(my_id, destination) = SMPI_FIRST(my_id, destination);
  }
  GM_WRITEBAR();
  SMPI_TOTALIN(my_id, destination) += SMPI_ALIGN(length);
}

gm_inline void 
smpi_complete_recv(unsigned int from_grank, 
		   unsigned int my_id, 
		   unsigned int length)
{
  SMPI_CURRENT(from_grank, my_id) += SMPI_ALIGN(length);
  if (SMPI_CURRENT(from_grank, my_id) > SMPI_LAST(from_grank, my_id))
    SMPI_CURRENT(from_grank, my_id) = SMPI_FIRST(from_grank, my_id);
  GM_WRITEBAR();
  SMPI_TOTALOUT(from_grank, my_id) += SMPI_ALIGN(length);
}


/* add a send request in the snd_request fifo to send it later */
gm_inline void 
smpi_queue_send(void *data, MPIR_SHANDLE *shandle, int len, int grank)
{
  struct smpi_send_fifo_req * smpi_send_fifo_ptr;
  
  smpi_send_fifo_ptr = (struct smpi_send_fifo_req *)
    gm_lookaside_alloc(smpi.send_fifo_lookaside);
  smpi_malloc_assert(smpi_send_fifo_ptr,
		     "smpi_queue_send",
		     "gm_lookaside_alloc: send fifo entry");
  
  smpi_send_fifo_ptr->data = data;
  smpi_send_fifo_ptr->shandle = shandle;
  smpi_send_fifo_ptr->len = len;
  smpi_send_fifo_ptr->grank = grank;
  smpi_send_fifo_ptr->next = NULL;

  if (smpi.send_fifo_head == NULL)
    {
      gmpi_debug_assert(smpi.send_fifo_queued == 0);
      gmpi_debug_assert(smpi.send_fifo_tail == NULL);
      smpi.send_fifo_head = smpi_send_fifo_ptr;
    }
  else
    {
      gmpi_debug_assert(smpi.send_fifo_tail != NULL);
      smpi.send_fifo_tail->next = smpi_send_fifo_ptr;
    }
  
  smpi.send_fifo_tail = smpi_send_fifo_ptr;
  smpi.send_fifo_queued++;
  smpi.malloc_send_buf_allocated += len;
  GMPI_DEBUG_FIFO_SEND_ADD(0, 0, smpi.send_fifo_queued);
}

/* this function send a eager message : build a packet into the 
   shared memory area and then copy the payload from the user buffer */
gm_inline void 
smpi_post_send_bufferinplace(void * buf, int len ,int src_lrank, 
			     int tag, int context_id, int destination, 
			     MPIR_SHANDLE * shandle)
{
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_SHORT_T * pkt;
  unsigned int my_id = smpi.my_local_id;

  gmpi_debug_assert(len < gmpi.eager_size);
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);

  /* build the packet */
  ptr_volatile = (void *)((&smpi_shmem->pool) 
			  + SMPI_NEXT(my_id, destination));
  ptr = (void *)ptr_volatile;
  pkt = (SMPI_PKT_SHORT_T *)ptr;
  pkt->mode = MPID_PKT_SHORT;
  pkt->context_id = context_id;
  pkt->lrank = src_lrank;
  pkt->tag = tag;
  pkt->len = len;
  pkt->send_id = shandle;

  DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);

  /*copy the data from user buffer */
  if (len > 0)
    {
      memcpy((void *)((gm_up_t)ptr + sizeof(SMPI_PKT_HEAD_T)), buf, len);
      DEBUG_PRINT_PKT_DATA("S Getting data from buf",pkt);
    }

  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr, 
			      (void *)((gm_up_t)ptr+sizeof(struct pkt_cksum)),
			      len + sizeof(SMPI_PKT_HEAD_T) 
			      - sizeof(struct pkt_cksum));
  
  /* update flow control */
  smpi_complete_send(my_id, destination, (len + sizeof(SMPI_PKT_HEAD_T)));

  DEBUG_PRINT_MSG("S Sent message in a single packet");
}

/* process a send_request previously queued */
gm_inline void 
smpi_post_send_queued(void *data, MPIR_SHANDLE * shandle, 
		      int len, int destination)
{
  volatile void * ptr_volatile;
  void * ptr;
  int my_id = smpi.my_local_id; 

  /* the packet is already built */
  ptr_volatile = (void *)((&smpi_shmem->pool)
			  + SMPI_NEXT(my_id, destination));
  ptr = (void *)ptr_volatile;
  if (len > 0)
    {
      memcpy(ptr, data, len);
    }
  
  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,
                              (void *)((gm_up_t)ptr+sizeof(struct pkt_cksum)),
                              len + sizeof(SMPI_PKT_HEAD_T)
                              - sizeof(struct pkt_cksum));
  DEBUG_PRINT_SMP_SEND_PKT("S Sending queued msg", (SMPI_PKT_HEAD_T *)ptr);
 
  smpi_complete_send(my_id, destination, len);

  if (shandle)
    shandle->is_complete = 1;
}

/* send the GET_request packet for rendez-vous */
gm_inline void 
smpi_post_send_get(void * buf, int len, int src_lrank,
		   int tag, int context_id,
		   int destination, MPIR_SHANDLE * shandle)
{
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_GET_T * pkt;
  int my_id = smpi.my_local_id;
  
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);

  ptr_volatile = (void *)((&smpi_shmem->pool)
			  + SMPI_NEXT(my_id,destination));
  ptr = (void *)ptr_volatile;
  pkt = (SMPI_PKT_GET_T *)ptr;
  pkt->mode = MPID_PKT_DO_GET;
  pkt->context_id = context_id;
  pkt->lrank = src_lrank;
  pkt->tag = tag;
  pkt->len = len;
  pkt->send_id = shandle;
  pkt->address = buf;

  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,  
			      (void *)((gm_up_t)ptr+sizeof(struct pkt_cksum)),
			      sizeof(SMPI_PKT_GET_T)
			      - sizeof(struct pkt_cksum));
  DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);
 
  smpi_complete_send(my_id, destination, sizeof(SMPI_PKT_GET_T));
 
  shandle->start = buf;
  shandle->bytes_as_contig = len;
}

/* send the SEND_request packet for rendez-vous */
gm_inline void 
smpi_post_send_rndv(void * buf, int len, int src_lrank, int tag, 
		    int context_id, int destination, 
		    MPIR_SHANDLE * shandle)
{
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_RNDV_T * pkt;
  int my_id = smpi.my_local_id;
  
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);

  ptr_volatile = (void *)((&smpi_shmem->pool)
			  + SMPI_NEXT(my_id, destination));
  ptr = (void *)ptr_volatile;
  pkt = (SMPI_PKT_RNDV_T *)ptr;
  pkt->mode = MPID_PKT_REQUEST_SEND;
  pkt->context_id = context_id;
  pkt->lrank = src_lrank;
  pkt->tag = tag;
  pkt->len = len;
  pkt->send_id = shandle;

  GMPI_DEBUG_CHECKSUM_COMPUTE(&(pkt->cksum_large), buf, len);
  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,
                              (void *)((gm_up_t)ptr+sizeof(struct pkt_cksum)),
                              sizeof(SMPI_PKT_RNDV_T)
                              - sizeof(struct pkt_cksum));
  DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);

  smpi_complete_send(my_id, destination, sizeof(SMPI_PKT_RNDV_T));
}


/* send the DONE_GET message to ack the rendez-vous copy */
gm_inline void 
smpi_post_send_done_get(int destination, void * send_id)
{
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_GET_T * pkt;
  int my_id = smpi.my_local_id;
  
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);

  /* build packet */
  ptr_volatile = (void *)((&smpi_shmem->pool)
			  + SMPI_NEXT(my_id, destination));
  ptr = (void *)ptr_volatile;
  pkt = (SMPI_PKT_GET_T *)ptr;
  pkt->mode = MPID_PKT_DONE_GET;
  pkt->send_id = send_id;

  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,  
                              (void *)((gm_up_t)ptr+sizeof(struct pkt_cksum)),
                              sizeof(SMPI_PKT_GET_T)
                              - sizeof(struct pkt_cksum));
  DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);

  /* flow control */
  smpi_complete_send(my_id, destination, sizeof(SMPI_PKT_GET_T));
}

/* send the OK_TO_SEND message to ack the SEND_REQUEST rendez-vous pkt */
gm_inline void 
smpi_post_send_ok_to_send(int destination, MPIR_RHANDLE * rhandle)
{
  volatile void * ptr_volatile;
  void * ptr = NULL;
  SMPI_PKT_RNDV_T * pkt;
  int my_id = smpi.my_local_id;

  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);
  
  rhandle->gm.netbuf = rhandle->buf;
  rhandle->gm.current_offset = rhandle->len;
  rhandle->s.MPI_ERROR = 0;
  rhandle->from = destination;

  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, sizeof(SMPI_PKT_RNDV_T)))
    {
      /* build packet */
      ptr_volatile = (void *)((&smpi_shmem->pool)
			      + SMPI_NEXT(my_id, destination));
      ptr = (void *)ptr_volatile;
      pkt = (SMPI_PKT_RNDV_T *)ptr;
      pkt->mode = MPID_PKT_OK_TO_SEND;
      pkt->len = rhandle->gm.current_offset;
      pkt->send_id  = rhandle->send_id;
      pkt->recv_id = rhandle;
     
      GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,
                                  (void *)((gm_up_t)ptr
                                           + sizeof(struct pkt_cksum)),
                                  sizeof(SMPI_PKT_RNDV_T)
                                  - sizeof(struct pkt_cksum)); 
      DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);
      
      /* flow control */ 
      smpi_complete_send(my_id, destination, sizeof(SMPI_PKT_RNDV_T));
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_RNDV_T * pkt_p;
      
      pkt_p = (SMPI_PKT_RNDV_T *)malloc(sizeof(SMPI_PKT_RNDV_T));
      smpi_malloc_assert(pkt_p,
			 "smpi_post_send_ok_to_send",
			 "malloc: packet allocation");
      
      pkt_p->mode = MPID_PKT_OK_TO_SEND;
      pkt_p->len = rhandle->gm.current_offset;
      pkt_p->send_id = rhandle->send_id;
      pkt_p->recv_id = rhandle;
     
      GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr, 
				  (void *)((gm_up_t)ptr  
					   + sizeof(struct pkt_cksum)),
				  sizeof(SMPI_PKT_RNDV_T)
				  - sizeof(struct pkt_cksum)); 
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_RNDV_T), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
}

/* send the OK_TO_SEND message to ack the SEND_REQUEST rendez-vous pkt */
gm_inline void 
smpi_post_send_ok_to_send_cont(int destination, MPIR_RHANDLE * rhandle)
{
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_RNDV_T * pkt;
  int my_id = smpi.my_local_id;

  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);

  if (!smpi.send_fifo_head 
      && smpi_able_to_send(destination, sizeof(SMPI_PKT_RNDV_T)))
    {
      /* build packet */
      ptr_volatile = (void *)((&smpi_shmem->pool)
			      + SMPI_NEXT(my_id, destination));
      ptr = (void *)ptr_volatile;
      pkt = (SMPI_PKT_RNDV_T *)ptr;
      pkt->mode = MPID_PKT_OK_TO_SEND;
      pkt->len = rhandle->gm.current_offset;
      pkt->send_id = rhandle->send_id;
      pkt->recv_id = rhandle;
      
      GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,  
                                  (void *)((gm_up_t)ptr  
                                           + sizeof(struct pkt_cksum)),
                                  sizeof(SMPI_PKT_RNDV_T)
                                  - sizeof(struct pkt_cksum));
      DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);
      
      /* flow control */
      smpi_complete_send(my_id, destination, sizeof(SMPI_PKT_RNDV_T));
    }
  else
    {
      /* not enough place, we will send it later */
      SMPI_PKT_RNDV_T * pkt_p;
      pkt_p = (SMPI_PKT_RNDV_T *)malloc(sizeof(SMPI_PKT_RNDV_T));
      smpi_malloc_assert(pkt_p,
			 "smpi_post_send_ok_to_send_cont",
			 "malloc: packet allocation");
      
      pkt_p->mode = MPID_PKT_OK_TO_SEND;
      pkt_p->len = rhandle->gm.current_offset;
      pkt_p->send_id = rhandle->send_id;
      pkt_p->recv_id = rhandle;
      
      GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)pkt_p,  
				  (void *)((gm_up_t)pkt_p  
					   + sizeof(struct pkt_cksum)),
				  sizeof(SMPI_PKT_RNDV_T)
				  - sizeof(struct pkt_cksum));
      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
      DEBUG_PRINT_MSG("S Sending message in the send_queue");
      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_RNDV_T), destination);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
}

/* on the sender, when it receives the DONE_GET message, can complete 
   the rendez-vous Send and free the user buffer */
gm_inline int 
smpi_recv_done_get(int from, int my_id, void * send_id)
{
#if SMP_ENABLE_DIRECTCOPY
  MPIR_SHANDLE * shandle;
  
  /* flow control */  
  smpi_complete_recv(from, my_id, sizeof(SMPI_PKT_GET_T));
  shandle = (MPIR_SHANDLE *)send_id; 
  
  if (shandle->bytes_as_contig > 0)
    {
      gmpi_unuse_interval((gm_up_t)shandle->start, shandle->bytes_as_contig);  
    }
  
  MPID_n_pending--;
  smpi.pending--;
  shandle->is_complete  = 1;
  if (shandle->finish)
    shandle->finish(shandle);
  return MPI_SUCCESS;
#endif
  return 0;
}


gm_inline int 
smpi_recv_ok_to_send(int from, int my_id, void * send_id, void * recv_id)
{ 
  volatile void * ptr_volatile;
  void * ptr;
  SMPI_PKT_CONT_GET_T * pkt;
  int destination = from;
  MPIR_SHANDLE * shandle;
  void * recv_handle_id, * send_handle_id;
  int length_to_send, pkt_len, pkt_payload, flow_control_ok, last;

  shandle = (MPIR_SHANDLE *)send_id;
  send_handle_id = send_id;
  recv_handle_id = recv_id;

  /* flow control */
  smpi_complete_recv(from, my_id, sizeof(SMPI_PKT_RNDV_T));
  
  gmpi_debug_assert(destination > -1);
  gmpi_debug_assert(destination < smpi.num_local_nodes);
  
  shandle->gm.current_expected--;
  gmpi_debug_assert(shandle->gm.current_expected >= 0);
  length_to_send = shandle->gm.current_offset;
  flow_control_ok = 1;
  last = 0;
  
  while((flow_control_ok) && ((length_to_send > 0)
			      || (shandle->gm.current_done == 0)))
    {
      if (length_to_send >= gmpi.eager_size)
	{ 
	  pkt_len = (sizeof(SMPI_PKT_CONT_GET_T) + gmpi.eager_size);
	  pkt_payload = gmpi.eager_size;
	}
      else
	{
	  pkt_len = (sizeof(SMPI_PKT_CONT_GET_T) + length_to_send);
	  pkt_payload = length_to_send;
	}
      
      if (length_to_send == pkt_payload)
	last = 1;
      
      /* flow control */ 
      if (!smpi.send_fifo_head && smpi_able_to_send(destination, pkt_len))
	{
	  /* build the packet */
	  ptr_volatile = (void *)((&smpi_shmem->pool)
				  + SMPI_NEXT(my_id, destination));
	  ptr = (void *)ptr_volatile;
	  pkt = (SMPI_PKT_CONT_GET_T *)ptr;
	  if (last)
	    pkt->mode = MPID_PKT_DONE_GET;
	  else
	    pkt->mode = MPID_PKT_CONT_GET;
	  pkt->send_id = send_handle_id;
	  pkt->recv_id = recv_handle_id;
      
	  /*copy the data from user buffer */
	  if (pkt_payload > 0)
	    {
	      memcpy((void *)((gm_up_t)pkt + sizeof(SMPI_PKT_CONT_GET_T)), 
                     shandle->gm.ptr, pkt_payload);
	      pkt->len = pkt_payload;
	      shandle->gm.ptr = (void *)((gm_up_t)shandle->gm.ptr 
					  + pkt_payload);
	      length_to_send -= pkt_payload;
	    }
	  shandle->gm.current_done = 1;
	  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,
                                      (void *)((gm_up_t)ptr
                                               + sizeof(struct pkt_cksum)),
                                      pkt_payload + sizeof(SMPI_PKT_CONT_GET_T)
                                      - sizeof(struct pkt_cksum)); 
	  DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);
	  
	  /* update flow control */  
	  smpi_complete_send(my_id, destination, pkt_len);
	}
      else
	{
	  flow_control_ok = 0;
	  DEBUG_PRINT_MSG("S Pipeline stopped because of flow control");
	  if (!smpi.send_fifo_head 
	      && smpi_able_to_send(destination, sizeof(SMPI_PKT_CONT_GET_T)))
	    {
	      /* build the packet */
	      ptr_volatile = (void *)((&smpi_shmem->pool)
				      + SMPI_NEXT(my_id, destination));
	      ptr = (void *)ptr_volatile;
	      pkt = (SMPI_PKT_CONT_GET_T *)ptr;
	      pkt->mode = MPID_PKT_FLOW;
	      pkt->send_id = send_handle_id;
	      pkt->recv_id = recv_handle_id;
	      pkt->len = 0;
	      shandle->gm.current_expected++;
	      
	      GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)ptr,
                                          (void *)((gm_up_t)ptr
                                                   + sizeof(struct pkt_cksum)),
                                          sizeof(SMPI_PKT_CONT_GET_T)
                                          - sizeof(struct pkt_cksum));
              DEBUG_PRINT_SMP_SEND_PKT("S Sending", pkt);
	      
	      /* update flow control */
	      smpi_complete_send(my_id, destination, 
				 sizeof(SMPI_PKT_CONT_GET_T));
	    }
	  else
	    {
	      /* we need to keep the circuit full */
	      SMPI_PKT_CONT_GET_T * pkt_p;
	      pkt_p = (SMPI_PKT_CONT_GET_T *)
		malloc(sizeof(SMPI_PKT_CONT_GET_T));
	      smpi_malloc_assert(pkt_p,
				 "smpi_recv_ok_to_send",
				 "malloc: packet allocation");
	
	      pkt_p->mode = MPID_PKT_FLOW;
	      pkt_p->send_id = send_handle_id;
	      pkt_p->recv_id = recv_handle_id;
	      pkt_p->len = 0;
	      shandle->gm.current_expected++;
	      
	      DEBUG_PRINT_SMP_SEND_PKT("S Sending in the send_queue", pkt_p);
	      DEBUG_PRINT_MSG("S Sending message in the send_queue");
	      smpi_queue_send(pkt_p, 0, sizeof(SMPI_PKT_CONT_GET_T), 
			      destination);
	      DEBUG_PRINT_MSG("S Sent message in the send_queue");
	    }
	}
    }
  
  shandle->gm.current_offset = length_to_send;
  
  if ((length_to_send == 0) && (shandle->gm.current_done == 1) 
      && (shandle->gm.current_expected == 0))
    {
      MPID_n_pending--;
      smpi.pending--;
      shandle->is_complete  = 1;
      if (shandle->finish)
	shandle->finish(shandle);
    }
  return MPI_SUCCESS;
}

gm_inline int 
smpi_recv_get(int from, int my_id, void * in_pkt)
{ 
  int msglen, pkt_len, last_pkt=0, send_progress=0;
  MPIR_RHANDLE * rhandle;
  SMPI_PKT_CONT_GET_T *pkt = (SMPI_PKT_CONT_GET_T *)in_pkt;

  rhandle = (MPIR_RHANDLE *)pkt->recv_id;
  msglen = pkt->len;
  pkt_len = sizeof(SMPI_PKT_CONT_GET_T) + msglen;
  
  if (pkt->mode == MPID_PKT_DONE_GET)
    {
      last_pkt = 1;
    }
  else
    { 
      if (pkt->mode == MPID_PKT_FLOW)
	{
	  send_progress = 1;
	}
    }
  
  if (msglen > 0)
    {
      memcpy(rhandle->gm.netbuf, 
	     (void *)((gm_up_t)pkt + sizeof(SMPI_PKT_CONT_GET_T)), msglen );
      rhandle->gm.netbuf += msglen;
      rhandle->gm.current_offset -= msglen;
    }
  
  /* flow control */
  smpi_complete_recv(from, my_id, pkt_len);
  
  if (send_progress)
    smpi_post_send_ok_to_send_cont(from, rhandle);
  
  gmpi_debug_assert(rhandle->gm.current_offset >= 0);
  if (last_pkt)
    {
      GMPI_DEBUG_CHECKSUM_CHECK("large SMP message",
                                (void *)(rhandle->buf),
				from, &(rhandle->gm.cksum));
      rhandle->is_complete  = 1;
      if (rhandle->finish)
	(rhandle->finish)(rhandle);
    }
  return MPI_SUCCESS;
}

/* GM call to use the kernel to "directcopy" the send buffer into the
   receive buffer. Kernel overhead but only one copy */
gm_inline void 
smpi_do_get(int from, void * source, void * target, unsigned int length)
{ 
#if SMP_ENABLE_DIRECTCOPY
  gm_status_t status;

  gmpi_debug_assert((0 <= from) && (from <=smpi.num_local_nodes));
  status = gm_directcopy_get(gmpi.port, source, target, length,
			     smpi_shmem->board_id[from],
			     smpi_shmem->port_id[from]);
		   
  /* something did wrong ? */
  if (status != GM_SUCCESS)
    {
      fprintf(stderr, "ERROR : [%d], gm_directcopy failed (status=%d), "
	      "maybe the Directcopy support should be enabled in GM\n",
	      MPID_GM_rank, status);
      gm_perror ("gm_directcopy: ", status);
      gmpi_abort (0);
    }
#else
  fprintf(stderr, "ERROR : [%d], directcopy is disabled or GM cannot "
	  "register memory\n", MPID_GM_rank);
  gmpi_abort (0);
#endif
}

gm_inline void
smpi_send_cancelok (void *in_pkt, int from)
{
  volatile void * ptr_volatile;
  void * ptr;
  int queued;
  SMPI_PKT_ANTI_SEND_T *pkt = (SMPI_PKT_ANTI_SEND_T *)in_pkt;
  SMPI_PKT_ANTI_SEND_T *new_pkt;

  int error_code;
  int found = 0;
  
  MPIR_SHANDLE *shandle=0;
  MPIR_RHANDLE *rhandle;
  
  GMPI_PROGRESSION_LOCK();
  DEBUG_PRINT_MSG("S starting MPID_SMP_Cancel_send");
  /* Flow control : is there enough place on the shared memory area */
  if (!smpi.send_fifo_head 
      && smpi_able_to_send(from, sizeof(SMPI_PKT_ANTI_SEND_T)))
    {
      queued = 0;
      ptr_volatile = (void *)((&smpi_shmem->pool) 
			      + SMPI_NEXT(smpi.my_local_id, from));
      ptr = (void *)ptr_volatile;
      new_pkt = (SMPI_PKT_ANTI_SEND_T *)ptr;
    }
  else
    {
      /* not enough place, we will send it later */
      queued = 1;
      new_pkt = (SMPI_PKT_ANTI_SEND_T *)malloc (sizeof (SMPI_PKT_ANTI_SEND_T));
      smpi_malloc_assert (new_pkt, "smpi_send_cancelok",
			  "malloc: pkt allocation");
    }

  shandle = pkt->send_id;

  /* Look for request, if found, delete it */
  error_code = MPID_Search_unexpected_for_request(shandle, &rhandle, &found);
  
  if ((error_code != MPI_SUCCESS) || (found == 0))
    {
      new_pkt->cancel = 0;
    }
  else
    {
      if (rhandle->s.count < gmpi.eager_size)
	{
	  /* begin if short/eager message */
	  FREE ( rhandle->start ); 
	  rhandle->start = 0;
	  MPID_RecvFree( rhandle ); 
	} 
      else { /* begin if rndv message */
	  MPID_RecvFree( rhandle );
	}
      new_pkt->cancel = 1;
    }

  new_pkt->mode = MPID_PKT_ANTI_SEND_OK;
  new_pkt->send_id = shandle;  
  
  if (queued == 0)
    {
      DEBUG_PRINT_MSG("S Sending anti_send_ok message\n");
      
      /* update flow control */
      smpi_complete_send(smpi.my_local_id, from, sizeof(SMPI_PKT_ANTI_SEND_T));
    }
  else
    {
      DEBUG_PRINT_MSG("S Sending anti_send_ok in the send_queue\n");
      smpi_queue_send(new_pkt, 0, sizeof(SMPI_PKT_ANTI_SEND_T), from);
      DEBUG_PRINT_MSG("S Sent message in the send_queue");
    }
  
  GMPI_PROGRESSION_UNLOCK();
}

gm_inline void
smpi_recv_cancelok (void *in_pkt, int from)
{
  SMPI_PKT_ANTI_SEND_T *pkt = (SMPI_PKT_ANTI_SEND_T *)in_pkt;
  MPIR_SHANDLE *shandle = (MPIR_SHANDLE *)pkt->send_id;
  
  if (pkt->cancel) {   /* begin if pkt->cancel */
      /* Mark the request as cancelled */
      shandle->s.MPI_TAG = MPIR_MSG_CANCELLED; 
      /* Mark the request as complete */
      shandle->is_complete = 1;
      shandle->is_cancelled = 1;
      if (shandle->gm.rendez_vous == 1)
	{
	  MPID_n_pending--;
	  smpi.pending--;
	}
      DEBUG_PRINT_MSG("Request has been successfully cancelled");
    }   /* end if pkt->cancel */
  else {
      shandle->is_cancelled = 0;
      DEBUG_PRINT_MSG("Unable to cancel request");
    }
  shandle->cancel_complete = 1;
  
  expect_cancel_ack--;
}


/* to check new messages in the shared memory receive queues and process
   them : poll from all the other local nodes */
gm_inline int 
smpi_net_lookup(MPID_Device *dev, int blocking)
{
  volatile void * ptr;
  SMPI_PKT_T * pkt;
  int from, j;
  MPIR_RHANDLE *rhandle;
  int is_posted;
  int err = -1;
  
  for (j=1; j<smpi.num_local_nodes; j++)
    {
      from = (smpi.my_local_id+j)%smpi.num_local_nodes;
      if (SMPI_TOTALIN(from, smpi.my_local_id) 
	  != SMPI_TOTALOUT(from, smpi.my_local_id))
	{
	  GM_READBAR();
	  ptr = (void *)((&smpi_shmem->pool)
			 + SMPI_CURRENT(from, smpi.my_local_id));
	  pkt = (SMPI_PKT_T *)ptr;
	  
	  DEBUG_PRINT_SMP_PKT("R receiving msg", pkt);
	  
	  GMPI_DEBUG_CHECKSUM_CHECK("small SMP message",
                                    (void *)((gm_up_t)ptr 
					     + sizeof(struct pkt_cksum)),
				    from, (struct pkt_cksum *)ptr);
	  
          /* Separate the incoming messages from control messages */
	  if (MPID_PKT_IS_MSG(pkt->head.mode))
	    {
	      /* Is the message expected or not? 
		 This routine RETURNS a rhandle, creating one if the message 
		 is unexpected (is_posted == 0) */
	      MPID_Msg_arrived(pkt->head.lrank, pkt->head.tag,
			       pkt->head.context_id, &rhandle, &is_posted);
	      
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	      if (MPID_DebugFlag) {
		FPRINTF(MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n",
			MPID_GM_rank, is_posted ? "posted" : "unexpected",
			__FILE__, __LINE__ );
	      }
#endif                  /* #DEBUG_END# */
	      if (is_posted)
		{
		  switch (pkt->head.mode)
		    {
		    case MPID_PKT_SHORT:
		      DEBUG_TEST_FCN(dev->short_msg->recv,"dev->short->recv");
		      (*dev->short_msg->recv)( rhandle, from, pkt );
		      err = MPI_SUCCESS;
		      break;
		      
#if SMP_ENABLE_DIRECTCOPY
		    case MPID_PKT_DO_GET:
		      DEBUG_TEST_FCN(dev->rndv->irecv,"dev->rndv->irecv");
		      (*dev->rndv->irecv)( rhandle, from, pkt );
		      err = MPI_SUCCESS;
		      break;
#else	      
		    case MPID_PKT_REQUEST_SEND:
		      rhandle->s.MPI_TAG = pkt->rndv_pkt.tag;
		      rhandle->s.MPI_SOURCE = pkt->rndv_pkt.lrank;
		      rhandle->s.count = pkt->rndv_pkt.len;
		      rhandle->send_id = pkt->rndv_pkt.send_id;
		      GMPI_DEBUG_CHECKSUM_COPY(&(rhandle->gm.cksum), 
					       &(pkt->rndv_pkt.cksum_large));

		      /* flow control to receive the REQUEST_SEND */
		      smpi_complete_recv(from, smpi.my_local_id,
					 sizeof(SMPI_PKT_RNDV_T));
		      
		      smpi_post_send_ok_to_send(from, rhandle);
		      err = MPI_SUCCESS;
		      break;
#endif	      
		      
		    default:
		      fprintf( stderr, 
			       "[%d] Internal error: msg packet "
			       "discarded (%s:%d)\n",
			       MPID_GM_rank, __FILE__, __LINE__ );
		    }
		}
	      else
		{
		  switch (pkt->head.mode)
		    {
		    case MPID_PKT_SHORT:
		      rhandle->send_id = 0;
		      DEBUG_TEST_FCN(dev->short_msg->unex,"dev->short->unex");
		      (*dev->short_msg->unex)( rhandle, from, pkt );
		      err = MPI_SUCCESS;
		      break;
#if SMP_ENABLE_DIRECTCOPY
		    case MPID_PKT_DO_GET:
		      /* Need the send handle address to cancel a send */
		      rhandle->send_id = pkt->get_pkt.send_id;
		      DEBUG_TEST_FCN(dev->rndv->unex,"dev->rndv->unex");
		      (*dev->rndv->unex)( rhandle, from, pkt );
		      err = MPI_SUCCESS;		      
		      break;
#else
		    case MPID_PKT_REQUEST_SEND:
		      /* Need the send handle address to cancel a send */
		      rhandle->send_id = pkt->rndv_pkt.send_id;
		      DEBUG_TEST_FCN(dev->rndv->unex,"dev->rndv->unex");
		      (*dev->rndv->unex)( rhandle, from, pkt );
		      err = MPI_SUCCESS;
		      break;
#endif
		    default:
		      fprintf( stderr, 
			       "[%d] Internal error: msg packet "
			       "discarded (%s:%d)\n",
			       MPID_GM_rank, __FILE__, __LINE__ );
		    }
		}
	    }
	  else
	    {
	      switch (pkt->head.mode)
		{
#if SMP_ENABLE_DIRECTCOPY
		case MPID_PKT_DONE_GET:
		  smpi_recv_done_get(from, smpi.my_local_id,
				     pkt->get_pkt.send_id);
		  err = MPI_SUCCESS;
		  break;
#else
		case MPID_PKT_OK_TO_SEND:
		  smpi_recv_ok_to_send(from, smpi.my_local_id,
				       pkt->rndv_pkt.send_id,
				       pkt->rndv_pkt.recv_id);
		  err = MPI_SUCCESS;
		  break;
		  
		case MPID_PKT_FLOW:
		case MPID_PKT_CONT_GET:
		case MPID_PKT_DONE_GET:
		  smpi_recv_get(from, smpi.my_local_id, pkt);
		  err = MPI_SUCCESS;
		  break;
#endif
		case MPID_PKT_ANTI_SEND:
		  smpi_send_cancelok (pkt, from);
		  /* flow control to receive the ANTI_SEND */
		  smpi_complete_recv (from, smpi.my_local_id,
				      sizeof (SMPI_PKT_ANTI_SEND_T));
		  
		  err = MPI_SUCCESS;
		  break;
		  
		case MPID_PKT_ANTI_SEND_OK:
		  smpi_recv_cancelok (pkt, from); 
		  /* flow control to receive the ANTI_SEND_OK */
		  smpi_complete_recv (from, smpi.my_local_id,
				      sizeof(SMPI_PKT_ANTI_SEND_T));
		  
		  err = MPI_SUCCESS; 
		  break;
		  
		default:
		  fprintf( stdout, "[%d] Mode %d (0x%x) is unknown "
			   "(internal error) %s:%d!\n", 
			   MPID_GM_rank, pkt->head.mode, pkt->head.mode,
			   __FILE__, __LINE__ );
		}
	    }
	}
    }
  /* DEBUG_PRINT_MSG("Exiting check_incoming"); */
  return err;
}

/* the main loop : check if we can process some queued send requests and poll
   for new messages */
gm_inline int 
MPID_SMP_Check_incoming(MPID_Device *dev, MPID_BLOCKING_TYPE blocking)
{
  int found;
  struct smpi_send_fifo_req *sreq;
 
  GMPI_PROGRESSION_LOCK();
  while ((sreq = smpi.send_fifo_head)
	 && smpi_able_to_send(sreq->grank, sreq->len))
    {
      smpi_post_send_queued(sreq->data, sreq->shandle,
			    sreq->len, sreq->grank);
      if (sreq == smpi.send_fifo_tail)
	{
	  smpi.send_fifo_head = NULL;
	  smpi.send_fifo_tail = NULL;
	  gmpi_debug_assert(smpi.send_fifo_queued == 1);
	}
      else
	{
	  smpi.send_fifo_head = sreq->next;
	  gmpi_debug_assert(smpi.send_fifo_queued > 0);
	}
      smpi.send_fifo_queued--;
      gmpi_debug_assert(smpi.malloc_send_buf_allocated > 0);
      smpi.malloc_send_buf_allocated -= sreq->len;
      GMPI_DEBUG_FIFO_SEND_REMOVE(0, smpi.send_fifo_queued);
      free (sreq->data);
      gm_lookaside_free(sreq);
    }
  /* polling */
  found = smpi_net_lookup(dev, blocking);
  GMPI_PROGRESSION_UNLOCK();
  
  return found;
}


/* the init of the SMP device */
void smpi_init(void)
{
  unsigned int i,j,size,pool, pid, wait;
  char * buf;
  struct stat file_status;
  char shmem_file[64];

  if (smpi.num_local_nodes > SMPI_MAX_NUMLOCALNODES)
    {
      fprintf(stderr,
	      "ERROR: mpi node %d, too many local processes "
	      "(%d processes, %d maximum). Change the "
	      "SMPI_MAX_NUMLOCALNODES value in gmpi_smpi.h\n",
	      MPID_GM_rank, smpi.num_local_nodes,
	      SMPI_MAX_NUMLOCALNODES);
      gmpi_abort (0);
    }
  smpi.pending = 0;
  smpi.send_fifo_queued = 0;
  smpi.malloc_send_buf_allocated = 0;
  smpi.send_fifo_lookaside 
    = gm_create_lookaside(sizeof(struct smpi_send_fifo_req),
			  SMPI_INITIAL_SEND_FIFO);
  smpi_malloc_assert(smpi.send_fifo_lookaside,
		     "smpi_init",
		     "gm_create_lookaside: send fifo creation");

  smpi.available_queue_length = (SMPI_LENGTH_QUEUE
				 - gmpi.eager_size
				 - sizeof(SMPI_PKT_CONT_GET_T));

#if SMP_ENABLE_DIRECTCOPY
  if (MPID_GM_size == smpi.num_local_nodes)
    {
      gmpi_regcache_init();
    }
#endif
  
  sprintf(shmem_file, "/tmp/gmpi_shmem-%d:%d.tmp", gmpi.magic, 
	  gmpi.numa_nodes[MPID_GM_rank]);
#if PRINT_CONFINFO
  printf("Process %d shmem_file = '%s'\n", MPID_GM_rank, shmem_file);
#endif  
  
  /* open the shared memory file */
  smpi.fd = open(shmem_file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
  if (smpi.fd < 0)
    { 
      fprintf(stderr, "[%d] smpi_init:error in opening "
	      "shared memory file <%s>: %d\n",
	      MPID_GM_rank, shmem_file, errno);
      gmpi_abort (0);
    }

  /* compute the size of this file */
  size = (SMPI_CACHE_LINE_SIZE + sizeof(struct shared_mem) + GM_PAGE_LEN 
	  + (smpi.num_local_nodes * (smpi.num_local_nodes-1) 
	     * (SMPI_ALIGN(SMPI_LENGTH_QUEUE + GM_PAGE_LEN))));
  
  /*
   * Initialization of the shared memory file: set size, but don't
   * actually allocate pages for the memory.  This allows a NUMA
   * kernel to allocate pages from the appropriate location.
   */

  if (smpi.my_local_id == 0)
    {
      /* Nuke any existing data (shouldn't be any) */
      if (ftruncate(smpi.fd, 0)) 
	{
	  fprintf(stderr, "[%d] smpi_init:error in ftruncate to zero "
		  "on shared memory file: %d\n",
		  MPID_GM_rank, errno);
	  gmpi_abort (0);
	}
      
      /* Zero-fill to desired size -- without touching data */
      if (ftruncate(smpi.fd, size))
       	{
	  fprintf(stderr, "[%d] smpi_init:error in ftruncate to size "
		  "on shared memory file: %d\n",
		  MPID_GM_rank, errno);
	  gmpi_abort (0);
	}

      if (lseek(smpi.fd, 0, SEEK_SET) != 0)
	{
	  fprintf(stderr, "[%d] smpi_init:error in lseek "
		  "on shared memory file: %d\n",
		  MPID_GM_rank, errno);
	  gmpi_abort (0);
	}
    }

  /* synchronization between local processes */
  do
    {
      if (fstat(smpi.fd, &file_status) != 0)
	{
	  fprintf(stderr, "[%d] smpi_init:error in fstat "
		  "on shared memory file: %d\n",
		  MPID_GM_rank, errno);
	  gmpi_abort (0);
	}
      usleep(1000);  /* Don't smash the kernel */
    }
  while (file_status.st_size != size);
  
  /* mmap of the shared memory file */
  smpi.mmap_ptr = mmap(0, size, (PROT_READ | PROT_WRITE),
		       (MAP_SHARED), smpi.fd, 0); 
  if (smpi.mmap_ptr == (void*)-1)
    { 
      fprintf(stderr, "[%d] smpi_init:error in mmapping "
	      "shared memory: %d\n",
	      MPID_GM_rank, errno);
      gmpi_abort (0);
    }
  
  smpi_shmem = (struct shared_mem *)smpi.mmap_ptr;
  if (((gm_up_t)smpi_shmem & (SMPI_CACHE_LINE_SIZE-1)) != 0)
    {
      fprintf(stderr, "[%d] smpi_init:error in shifting mmapped "
	      "shared memory\n", MPID_GM_rank);
      gmpi_abort (0);
    }
  
  /* init rqueues in shared memory */
  if (smpi.my_local_id == 0)
    {
      pool = GM_PAGE_LEN;
      for (i=0; i<smpi.num_local_nodes; i++)
	{
	  for (j=0; j<smpi.num_local_nodes; j++)
	    {
	      if (i != j)
		{ 
		  GM_READBAR();
		  smpi_shmem->rqueues_limits[i][j].first = SMPI_ALIGN(pool);
		  smpi_shmem->rqueues_limits[i][j].last =
		    SMPI_ALIGN(pool + smpi.available_queue_length);
	  
		  smpi_shmem->rqueues_params[i].params[j].current =
		    SMPI_ALIGN(pool);
		  smpi_shmem->rqueues_params[j].params[i].next =
		    SMPI_ALIGN(pool);
		  smpi_shmem->rqueues_params[j].params[i].msgs_total_in = 0;
		  
		  smpi_shmem->rqueues_flow_out[i][j].msgs_total_out = 0;
		  pool += SMPI_ALIGN(SMPI_LENGTH_QUEUE + GM_PAGE_LEN);
		  GM_READBAR();
		}
	    }
	}
    }

  /* another synchronization barrier */
  if (smpi.my_local_id == 0)
    {
      wait = 1;
      while (wait)
	{
	  wait = 0;
	  for (i=1; i<smpi.num_local_nodes; i++)
	    {
	      if (smpi_shmem->pid[i] == 0)
		wait = 1;
	    }
	}
      /* id = 0, unlink the shared memory file, so that it is cleaned
	 up when everyone exits */
      unlink(shmem_file);
      pid = getpid();
      if (pid == 0)
	{
	  fprintf(stderr, "[%d] smpi_init:error in geting pid\n",
		  MPID_GM_rank);
	  gmpi_abort (0);
	}
      smpi_shmem->pid[smpi.my_local_id] = pid;
      GM_WRITEBAR();
    }
  else
    {
      while (smpi_shmem->pid[0] != 0);
      while (smpi_shmem->pid[0] == 0)
	{
	  smpi_shmem->pid[smpi.my_local_id] = getpid();
	  GM_WRITEBAR();
	}
    
      for (i=0;i<smpi.num_local_nodes;i++)
	{
	  if (smpi_shmem->pid[i] <= 0)
	    {
	      fprintf(stderr, "[%d] smpi_init:error in getting pid\n",
		      MPID_GM_rank);
	      gmpi_abort (0);
	    }
	}
    }
  
  /*
   * Okay, here we touch every page in the shared memory region.
   * We do this to get the pages allocated so that they are local
   * to the receiver on a numa machine (instead of all being located
   * near the first process).
   */
  {
    int sender, receiver;
    
    for (sender = 0; sender < smpi.num_local_nodes; sender++)
      {
	volatile char *ptr = &smpi_shmem->pool;
	volatile char tmp;

	receiver = smpi.my_local_id;
	if (sender != receiver)
	  {
	    int k;

	    for (k = SMPI_FIRST (sender, receiver);
		 k < SMPI_LAST (sender, receiver);
		 k += GM_PAGE_LEN)
	      {
		tmp = ptr[k];
	      }
	  }
      }
  }

  smpi_shmem->board_id[smpi.my_local_id] = gmpi.board_ids[MPID_GM_rank];
  smpi_shmem->port_id[smpi.my_local_id] = gmpi.port_ids[MPID_GM_rank];
  GM_READBAR();
}
  

/* Ok, we close everything and come back home */
void smpi_finish(void)
{
  while (smpi.send_fifo_head || smpi.pending)
    {
      MPID_DeviceCheck(MPID_BLOCKING);
    }
  
  /* unmap the shared memory file */
  munmap(smpi.mmap_ptr, 
	 (SMPI_CACHE_LINE_SIZE + sizeof(struct shared_mem) + GM_PAGE_LEN
	  + (smpi.num_local_nodes * (smpi.num_local_nodes-1)
	     * (SMPI_ALIGN(SMPI_LENGTH_QUEUE + GM_PAGE_LEN)))));
  close(smpi.fd);
  gm_destroy_lookaside(smpi.send_fifo_lookaside);

#if SMP_ENABLE_DIRECTCOPY
  if (MPID_GM_size == smpi.num_local_nodes)
    {
      gmpi_clear_all_intervals();
      gm_close(gmpi.port);
    }
#endif
}

#endif
