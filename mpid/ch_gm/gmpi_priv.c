/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/


#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "gmpi.h"

#include "mpid.h"
#include "mpiddev.h"
#include "mpid_bind.h"
#include "gmpi_packets.h"
#include "queue.h"
#include "reqalloc.h"

#include "gm.h"


struct gmpi_var  gmpi;
struct gm_port * gmpi_gm_port = NULL;
static unsigned int callback_flag;


static void gmpi_flush_fifo_send(void);
static void gmpi_send_packet_callback(struct gm_port *, void *, gm_status_t);
static void gmpi_put_data_callback(struct gm_port *, void *, gm_status_t);
int MPID_CH_Rndvn_ok_to_send(MPIR_RHANDLE *);
void MPD_Abort (int);


gm_inline void 
gmpi_malloc_assert(void * ptr, char * fct, char * msg)
{
  if (ptr == NULL)
    {
      fprintf(stderr, "[%d]: alloc failed, not enough memory (Fatal Error)\n"
	      "Context: <(%s) %s>\n", MPID_GM_rank, fct, msg);
      gmpi_abort (0);
    }
}


void * 
gmpi_dma_alloc(unsigned int length)
{
  void * ptr;
  
  ptr = gm_dma_malloc(gmpi_gm_port, length);
  GMPI_DEBUG_DMA_MEMORY_ALLOC(ptr, length);
  return(ptr);
}


void 
gmpi_dma_free(void *ptr)
{
  GMPI_DEBUG_DMA_MEMORY_FREE(ptr);
  gm_dma_free(gmpi_gm_port, ptr);
}


static void 
gmpi_drop_send_packet_callback (struct gm_port *port,
				void *context,
				gm_status_t status)
{
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  
  gmpi_send_buf_ptr = (struct gmpi_send_buf *)context;
  switch (status)
    {
    case GM_SUCCESS:  
      gmpi.dropped_sends[gmpi_send_buf_ptr->dest]--;
      gm_send_with_callback(port, (void *)(&(gmpi_send_buf_ptr->buffer)),
			    GMPI_PKT_GM_SIZE, 
			    gmpi_send_buf_ptr->length, 
			    GM_LOW_PRIORITY,
			    gmpi.local_node_ids[gmpi_send_buf_ptr->dest], 
			    gmpi.port_ids[gmpi_send_buf_ptr->dest],
			    gmpi_send_packet_callback, context);
      break;
      
    default:
      fprintf(stderr,
	      "FATAL ERROR on MPI node %d (%s): Drop callback status "
	      "is unexpected (%d)\n", MPID_GM_rank, 
              gmpi.my_hostname, status);
      gmpi_abort (0);
    }
}


static void 
gmpi_drop_put_data_callback (struct gm_port *port,
			     void *context,
			     gm_status_t status)
{
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  
  gmpi_send_buf_ptr = (struct gmpi_send_buf *)context;
  switch (status)
    {
    case GM_SUCCESS:  
      gmpi.dropped_sends[gmpi_send_buf_ptr->dest]--;
      gm_directed_send_with_callback 
	(port, (void *)(gmpi_send_buf_ptr->source_ptr),
	 (gm_remote_ptr_t)(gm_up_t)(gmpi_send_buf_ptr->target_ptr),
	 gmpi_send_buf_ptr->length,
	 GM_LOW_PRIORITY,
	 gmpi.local_node_ids[gmpi_send_buf_ptr->dest], 
	 gmpi.port_ids[gmpi_send_buf_ptr->dest],
	 gmpi_put_data_callback, context); 
      break;
      
    default:
      fprintf(stderr,
	      "FATAL ERROR on MPI node %d (%s): Drop put callback "
	      "status is unexpected (%d)\n", MPID_GM_rank, 
              gmpi.my_hostname, status);
      gmpi_abort (0);
    }
}


gm_inline int 
gmpi_check_send_status(gm_status_t status, int dest, int type, 
		       void * context)
{
  switch (status)
    {
    case GM_SUCCESS:
      return 1;
      
    case GM_SEND_TIMED_OUT:
      gmpi.dropped_sends[dest] = gmpi.pending_sends[dest];
      gm_drop_sends (gmpi_gm_port, GM_LOW_PRIORITY, 
		     gmpi.local_node_ids[dest], gmpi.port_ids[dest],
		     ((type == 1) ? gmpi_drop_send_packet_callback 
		      : gmpi_drop_put_data_callback), context);
      return 2;

    case GM_SEND_DROPPED:
      {
	struct gmpi_send_buf *gmpi_send_buf_ptr;
	
	gmpi_send_buf_ptr = (struct gmpi_send_buf *)context;
	gmpi.dropped_sends[dest]--;
	gmpi_debug_assert (dest == gmpi_send_buf_ptr->dest);
	if (type == 1)
	  {
	    gm_send_with_callback(gmpi_gm_port, 
				  (void *)(&(gmpi_send_buf_ptr->buffer)),
				  GMPI_PKT_GM_SIZE, 
				  gmpi_send_buf_ptr->length, 
				  GM_LOW_PRIORITY,
				  gmpi.local_node_ids[dest], 
				  gmpi.port_ids[dest],
				  gmpi_send_packet_callback, context);
	  }
	else
	  {
	    gm_directed_send_with_callback 
	      (gmpi_gm_port, (void *)(gmpi_send_buf_ptr->source_ptr),
	       (gm_remote_ptr_t)(gm_up_t)(gmpi_send_buf_ptr->target_ptr),
	       gmpi_send_buf_ptr->length,
	       GM_LOW_PRIORITY,
	       gmpi.local_node_ids[dest], 
	       gmpi.port_ids[dest],
	       gmpi_put_data_callback, context); 
	  }
      }
      return 2;

    case GM_SEND_TARGET_PORT_CLOSED:
      fprintf(stderr,
	      "FATAL ERROR %d on MPI node %d (%s): the GM port "
	      "on MPI node %d (%s) is closed, i.e. the process "
              "has not started, has exited or is dead\n", status, 
	      MPID_GM_rank, gmpi.my_hostname, dest,
	      gm_node_id_to_host_name(gmpi_gm_port, 
				      gmpi.global_node_ids[dest]));
      return 0;
      
    case GM_SEND_TARGET_NODE_UNREACHABLE:
      fprintf(stderr,
   	      "FATAL ERROR %d on MPI node %d (%s): MPI node %d "
              "(%s) is unreachable via Myrinet: "
              "check the host, cables or mapping\n",
              status, MPID_GM_rank, gmpi.my_hostname, dest,
	      gm_node_id_to_host_name(gmpi_gm_port, 
				      gmpi.global_node_ids[dest]));
      return 0;
      
    case GM_SEND_REJECTED:
      fprintf(stderr,
	      "FATAL ERROR %d on MPI node %d (%s): GM send to "
              "MPI node %d (%s) was dropped or rejected\n",
              status, MPID_GM_rank, gmpi.my_hostname, dest,
	      gm_node_id_to_host_name(gmpi_gm_port, 
				      gmpi.global_node_ids[dest]));
      return 0;
      
    default:
      fprintf(stderr,
	      "FATAL ERROR on MPI node %d (%s): GM send to "
              "MPI node %d (%s) failed for unknown reason (%d)\n",
              MPID_GM_rank, gmpi.my_hostname, dest,
	      gm_node_id_to_host_name(gmpi_gm_port, 
				      gmpi.global_node_ids[dest]));
      return 0;
    }
}


static void 
gmpi_send_packet_callback(struct gm_port * port,
			  void * context,
			  gm_status_t status)
{
  unsigned int err_code;
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  
  gmpi_send_buf_ptr = (struct gmpi_send_buf *)context;
  err_code = gmpi_check_send_status(status, gmpi_send_buf_ptr->dest, 1, 
				    context);
  
  switch (err_code)
    {
    case 1:
      GMPI_DEBUG_GM_SEND_FINISH (0, (void *)(&(gmpi_send_buf_ptr->buffer)),
				 gmpi_send_buf_ptr->length,
				 gmpi_send_buf_ptr->dest, context);
      
      gmpi_debug_assert(gmpi_send_buf_ptr->type == GMPI_DMA_SEND_BUF);
      gmpi_send_buf_ptr->next = gmpi.send_dma_buf_free;
      gmpi.send_dma_buf_free = gmpi_send_buf_ptr;
      gmpi.send_dma_buf_pool_remaining++;
      
      callback_flag = 1;
      gmpi.pending_sends[gmpi_send_buf_ptr->dest]--;
      gmpi.pending_send_callback--;
      gmpi_debug_assert(gmpi.send_tokens < gmpi.max_send_tokens);
      gmpi.send_tokens++;
      
      if (gmpi.send_buf_fifo_head != NULL)
	{
	  gmpi_flush_fifo_send();
	}
      break;
  
    case 2:
      break;

    case 0:
      fprintf(stderr, "Small/Ctrl message completion error!\n");
      gmpi_abort (0);
      break;
      
    default:
      fprintf(stderr, "Bad status code in send callback!\n");
      gmpi_abort (0);
    }
}


static void 
gmpi_put_data_callback(struct gm_port * port,
		       void * context,
		       gm_status_t status)
{
  unsigned int err_code;
  unsigned int chunk_size;
  struct gmpi_send_buf *gmpi_send_buf_ptr;
  MPIR_SHANDLE *shandle;

  gmpi_send_buf_ptr = (struct gmpi_send_buf *)context;
  shandle = gmpi_send_buf_ptr->shandle;
  gmpi_debug_assert (gmpi_send_buf_ptr->dest == shandle->partner);
  err_code = gmpi_check_send_status(status, gmpi_send_buf_ptr->dest, 2, 
				    context);
  
  switch (err_code)
    {
    case 1:
      chunk_size = (shandle->bytes_as_contig - shandle->gm.current_done);
      if (chunk_size > GMPI_MAX_PUT_LENGTH)
	{
	  chunk_size = GMPI_MAX_PUT_LENGTH;
	}
      gmpi_debug_assert (chunk_size == gmpi_send_buf_ptr->length);
      /* gmpi_debug_assert (((gm_up_t)(shandle->start) 
			  + shandle->gm.current_done)
	 == (gm_up_t) (gmpi_send_buf_ptr->source_ptr)); */
  
      GMPI_DEBUG_GM_SEND_FINISH(1, (void *)(gmpi_send_buf_ptr->source_ptr),
				gmpi_send_buf_ptr->length,
				gmpi_send_buf_ptr->dest, context);
  
#if GM_DISABLE_REGISTRATION
      gmpi_free_bounce_buffer(((gm_up_t)(shandle->start) 
			       + shandle->gm.current_done),
			      gmpi_send_buf_ptr->length);
#else
      gmpi_unuse_interval((gm_up_t) (gmpi_send_buf_ptr->source_ptr),
			  gmpi_send_buf_ptr->length);
#endif
      
      shandle->gm.current_done += chunk_size;
      
      gmpi_debug_assert(shandle->gm.current_done <= shandle->bytes_as_contig);
      if (shandle->gm.current_done == shandle->bytes_as_contig)
	{
	  shandle->is_complete = 1;
	  if (shandle->finish)
	    {
	      (shandle->finish)(shandle);
	    }
	}
      gmpi.pending_sends[gmpi_send_buf_ptr->dest]--;
      free(gmpi_send_buf_ptr);
      
      callback_flag = 1;
      gmpi.pending_put_callback--;
      gmpi_debug_assert(gmpi.send_tokens < gmpi.max_send_tokens);
      gmpi.send_tokens++;
      
      if (gmpi.send_buf_fifo_head != NULL)
	{
	  gmpi_flush_fifo_send();
	}
      break;
  
    case 2:
      break;
      
    case 0:
      fprintf(stderr, "Large message completion error !\n");
      gmpi_abort (0);
      break;
        
    default:
      fprintf(stderr, "Bad status code in send callback!\n");
      gmpi_abort (0);
    }
}


static void 
gmpi_flush_fifo_send (void)
{
  struct gmpi_send_buf *gmpi_send_buf_ptr;
  struct gmpi_send_buf *gmpi_new_send_buf_ptr;
  
  gmpi_debug_assert (gmpi.send_buf_fifo_head != NULL);
  
  while (gmpi.send_tokens > 0)
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_queued > 0);
      gmpi_send_buf_ptr = gmpi.send_buf_fifo_head;

      gmpi_debug_assert(gmpi_send_buf_ptr->dest < MPID_GM_size);
      if (gmpi.dropped_sends[gmpi_send_buf_ptr->dest] > 0)
	{
	  return;
	}

      if ((gmpi.send_dma_buf_free == NULL) 
	  && (gmpi_send_buf_ptr->type == GMPI_MALLOC_SEND_BUF))
	{
	  gmpi_debug_assert(gmpi.send_dma_buf_pool_remaining == 0);
	  return;
	}
      
      if (gmpi_send_buf_ptr->register_length > 0)
	{
#if GM_DISABLE_REGISTRATION
	  gm_up_t bounce_addr, bounce_data;
	  unsigned int bounce_length;
	  
	  bounce_length = gmpi_send_buf_ptr->register_length;
	  bounce_data = (gm_up_t)gmpi_send_buf_ptr->source_ptr;
	  if (gmpi_send_buf_ptr->type == GMPI_PUT_DATA) {
	    bounce_addr = gmpi_allocate_bounce_buffer(GMPI_BOUNCE_SEND, 
						      bounce_data,
						      bounce_length);
	  } else {
	    bounce_addr = gmpi_allocate_bounce_buffer(GMPI_BOUNCE_RECV, 
						      bounce_data,
						      bounce_length);
	  }
	  
	  if (bounce_addr != 0)
	    {
	      if (gmpi_send_buf_ptr->type == GMPI_PUT_DATA)
		{
		  memcpy((void *)bounce_addr,
			 (void *)bounce_data,
			 bounce_length);
		  gmpi_send_buf_ptr->source_ptr = (void *)bounce_addr;
		}
	      else
		{
		  MPID_PKT_OK_TO_SEND_T * pkt_ptr;

		  pkt_ptr = ((MPID_PKT_OK_TO_SEND_T *)
			     (&(gmpi_send_buf_ptr->buffer)));
		  gmpi_debug_assert(pkt_ptr->mode 
				    == MPID_PKT_OK_TO_SEND);
		  MPID_AINT_SET(pkt_ptr->target_ptr,
				(void *)bounce_addr);
		  
		  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)pkt_ptr,
					      (void *)
					      ((gm_up_t)pkt_ptr 
					       + sizeof(struct pkt_cksum)),
					      gmpi_send_buf_ptr->length 
					      - sizeof(struct pkt_cksum));
		}
	    }
	  else
	    {
	      return;
	    }
#else
	  unsigned int registered;
	  
	  registered = gmpi_use_interval((gm_up_t)
					 (gmpi_send_buf_ptr->source_ptr),
					 gmpi_send_buf_ptr->register_length);
	  
	  if (registered < gmpi_send_buf_ptr->register_length)
	    {
	      gmpi_unuse_interval((gm_up_t)(gmpi_send_buf_ptr->source_ptr), 
				  registered);
	      return;
	    }
#endif
	  
	  gmpi_send_buf_ptr->register_length = 0;
	}
      
      if (gmpi_send_buf_ptr->type == GMPI_MALLOC_SEND_BUF)
        {
	  gmpi_debug_assert(gmpi.send_dma_buf_free != NULL);
	  gmpi_debug_assert(gmpi.send_dma_buf_pool_remaining > 0);
	  gmpi_debug_assert(gmpi_send_buf_ptr->target_ptr == NULL);
	  gmpi_debug_assert(gmpi_send_buf_ptr->length > 0);
	  
	  gmpi_new_send_buf_ptr = gmpi.send_dma_buf_free;
	  gmpi.send_dma_buf_free = gmpi_new_send_buf_ptr->next;
	  gmpi.send_dma_buf_pool_remaining--;
	  memcpy(gmpi_new_send_buf_ptr, gmpi_send_buf_ptr, 
		 sizeof(struct gmpi_send_buf) - sizeof (unsigned int)
		 + gmpi_send_buf_ptr->length);
	  
          gmpi.send_buf_fifo_head = gmpi_new_send_buf_ptr;
          if (gmpi.send_buf_fifo_tail == gmpi_send_buf_ptr) 
            {
              gmpi.send_buf_fifo_tail = gmpi_new_send_buf_ptr;
            } 
	  
	  gmpi_debug_assert(gmpi.malloc_send_buf_allocated > 0);
	  gmpi.malloc_send_buf_allocated -= gmpi_send_buf_ptr->length;
          free(gmpi_send_buf_ptr);
	  
	  gmpi_send_buf_ptr = gmpi_new_send_buf_ptr;
	  gmpi_send_buf_ptr->type = GMPI_DMA_SEND_BUF;
	} 

      switch(gmpi_send_buf_ptr->type) 
        { 
	case GMPI_DMA_SEND_BUF:
        
          GMPI_DEBUG_GM_SEND_PRINT(0, 1, 
				   (void *)(&(gmpi_send_buf_ptr->buffer)),
				   gmpi_send_buf_ptr->length,
				   gmpi_send_buf_ptr->dest, NULL,
                                   (void *)gmpi_send_buf_ptr);
	 
          gmpi_debug_assert(gmpi.send_tokens > 0); 
	  gmpi.pending_sends[gmpi_send_buf_ptr->dest]++;
	  gm_send_with_callback(gmpi_gm_port, 
				(void *)(&(gmpi_send_buf_ptr->buffer)),
                                GMPI_PKT_GM_SIZE,
                                gmpi_send_buf_ptr->length,
				GM_LOW_PRIORITY,
				gmpi.local_node_ids[gmpi_send_buf_ptr->dest],
                                gmpi.port_ids[gmpi_send_buf_ptr->dest],
                                gmpi_send_packet_callback,
                                (void *)gmpi_send_buf_ptr);
	  
	  gmpi.pending_send_callback++;
	  gmpi.send_tokens--;
	  gmpi.send_buf_fifo_queued--;
	  GMPI_DEBUG_FIFO_SEND_REMOVE(0, gmpi.send_buf_fifo_queued);
	  gmpi.send_buf_fifo_head = gmpi_send_buf_ptr->next;
	  break;
	  
	case GMPI_PUT_DATA:
	  gmpi_debug_assert(gmpi_send_buf_ptr->source_ptr != NULL);
	  gmpi_debug_assert(gmpi_send_buf_ptr->target_ptr != NULL);
	  gmpi_debug_assert(gmpi_send_buf_ptr->shandle != NULL);
	  gmpi_debug_assert(gmpi_send_buf_ptr->length > 0);
          gmpi_debug_assert(gmpi_send_buf_ptr->buffer == 99);
	  
	  GMPI_DEBUG_GM_SEND_PRINT(1, 1, 
				   gmpi_send_buf_ptr->source_ptr,
				   gmpi_send_buf_ptr->length,
				   gmpi_send_buf_ptr->dest,
				   gmpi_send_buf_ptr->target_ptr,
                                   (void *)(gmpi_send_buf_ptr->shandle));
	  
	  GMPI_DEBUG_REGISTRATION_CHECK_SEGMENT((gm_up_t)
						gmpi_send_buf_ptr->source_ptr,
						gmpi_send_buf_ptr->length, 
						"PUT from FIFO");
	  
	  gmpi_debug_assert(gmpi.send_tokens > 0);
          gmpi.pending_sends[gmpi_send_buf_ptr->dest]++;
	  gm_directed_send_with_callback
	    (gmpi_gm_port,
	     gmpi_send_buf_ptr->source_ptr,
	     (gm_remote_ptr_t)(gm_up_t)gmpi_send_buf_ptr->target_ptr,
	     gmpi_send_buf_ptr->length,
	     GM_LOW_PRIORITY,
	     gmpi.local_node_ids[gmpi_send_buf_ptr->dest],
	     gmpi.port_ids[gmpi_send_buf_ptr->dest],
	     gmpi_put_data_callback,
	     (void *)gmpi_send_buf_ptr); 
	  
	  gmpi.pending_put_callback++;
	  gmpi.send_tokens--;
          gmpi.send_buf_fifo_queued--;
	  GMPI_DEBUG_FIFO_SEND_REMOVE(1, gmpi.send_buf_fifo_queued);
	  gmpi.send_buf_fifo_head = gmpi_send_buf_ptr->next;
	  break;
	  
	default:
	  fprintf(stderr, "[%d] gmpi_flush_fifo_send: bad send buf type\n", 
		  MPID_GM_rank);
	  gmpi_abort (0);
        }

      if (gmpi.send_buf_fifo_head == NULL)
	{
	  gmpi_debug_assert(gmpi.send_buf_fifo_tail == gmpi_send_buf_ptr);
	  gmpi.send_buf_fifo_tail = NULL;
	  gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
	  return;
	}
    }
}


gm_inline void * 
gmpi_allocate_packet(unsigned int length, struct gmpi_send_buf ** send_buf_ptr)
{
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  
  if (gmpi.send_dma_buf_free != NULL)
    {
      gmpi_debug_assert(gmpi.send_dma_buf_pool_remaining > 0);
      gmpi_send_buf_ptr = gmpi.send_dma_buf_free;
      gmpi_debug_assert(gmpi_send_buf_ptr->type == GMPI_DMA_SEND_BUF);
      gmpi.send_dma_buf_free = gmpi_send_buf_ptr->next;
      gmpi.send_dma_buf_pool_remaining--;
    }
  else
    {
      gmpi_debug_assert(gmpi.send_dma_buf_pool_remaining == 0);
      gmpi_send_buf_ptr = (struct gmpi_send_buf *)
        malloc(length + sizeof(struct gmpi_send_buf));
      gmpi_malloc_assert(gmpi_send_buf_ptr,
	                 "gmpi_allocate_packet",
			 "malloc: send buf");
      gmpi_send_buf_ptr->type = GMPI_MALLOC_SEND_BUF;
      gmpi.malloc_send_buf_allocated += length;
    }
  
  gmpi_send_buf_ptr->dest = -1;
  gmpi_send_buf_ptr->length = length;
  gmpi_send_buf_ptr->register_length = 0;
  gmpi_send_buf_ptr->source_ptr = NULL;
  gmpi_send_buf_ptr->target_ptr = NULL;
  gmpi_send_buf_ptr->shandle = NULL;
  gmpi_send_buf_ptr->next = NULL;
  gmpi_send_buf_ptr->buffer = 0;
  *send_buf_ptr = gmpi_send_buf_ptr;
  return ((void *)(&(gmpi_send_buf_ptr->buffer))); 
}


gm_inline void 
gmpi_send_packet(struct gmpi_send_buf * gmpi_send_buf_ptr, unsigned int dest)
{
  void * pkt;
  
  gmpi_debug_assert(dest < MPID_GM_size);
  gmpi_debug_assert(gmpi_send_buf_ptr != NULL);
  
  pkt = ((void *)(&(gmpi_send_buf_ptr->buffer)));
  gmpi_send_buf_ptr->dest = dest;
  
  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)pkt,
                              (void *)((gm_up_t)pkt 
                                       + sizeof(struct pkt_cksum)),
                              gmpi_send_buf_ptr->length 
                              - sizeof(struct pkt_cksum));
  GMPI_DEBUG_CHECKSUM_INFO((struct pkt_cksum *)pkt,
                           GMPI_DEBUG_CHECKSUM_SEND
                           | gmpi_send_buf_ptr->type);

  if ((gmpi.send_tokens > 0)
      && (gmpi.send_buf_fifo_head == NULL)
      && (gmpi_send_buf_ptr->type == GMPI_DMA_SEND_BUF)
      && (gmpi.dropped_sends[dest] == 0))
    {
      gmpi_debug_assert(gmpi_send_buf_ptr->target_ptr == NULL);
      gmpi_debug_assert(gmpi_send_buf_ptr->length > 0);
      gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
      
      GMPI_DEBUG_GM_SEND_PRINT(0, 0, pkt,
			       gmpi_send_buf_ptr->length,
			       dest, NULL, (void *)gmpi_send_buf_ptr);
      
      gmpi_debug_assert(gmpi.send_tokens > 0);
      gmpi.pending_sends[dest]++;
      gm_send_with_callback(gmpi_gm_port, pkt, 
			    GMPI_PKT_GM_SIZE,
			    gmpi_send_buf_ptr->length, 
			    GM_LOW_PRIORITY,
			    gmpi.local_node_ids[dest],
			    gmpi.port_ids[dest],
			    gmpi_send_packet_callback, 
			    gmpi_send_buf_ptr);

      gmpi.pending_send_callback++;
      gmpi.send_tokens--;
    }
  else
    {
      if (gmpi.send_buf_fifo_head == NULL)
	{
	  gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
	  gmpi_debug_assert(gmpi.send_buf_fifo_tail == NULL);
	  gmpi.send_buf_fifo_head = gmpi_send_buf_ptr;
	}
      else
	{
	  gmpi_debug_assert(gmpi.send_buf_fifo_tail->next == NULL);
	  gmpi.send_buf_fifo_tail->next = gmpi_send_buf_ptr;
	}
      
      GMPI_DEBUG_CHECKSUM_INFO((struct pkt_cksum *)pkt,
			       GMPI_DEBUG_CHECKSUM_SEND_QUEUED);
      gmpi.send_buf_fifo_tail = gmpi_send_buf_ptr;
      gmpi_send_buf_ptr->next = NULL;
      gmpi.send_buf_fifo_queued++;
      GMPI_DEBUG_FIFO_SEND_ADD(0, 0, gmpi.send_buf_fifo_queued);
    }
}


void 
gmpi_queue_packet_register(struct gmpi_send_buf * gmpi_send_buf_ptr,
			   void * reg_start,
			   unsigned int reg_length,
			   unsigned int dest)
{
  void * pkt;
  
  gmpi_debug_assert(dest < MPID_GM_size);
  
  gmpi_send_buf_ptr->register_length = reg_length;
  gmpi_send_buf_ptr->source_ptr = reg_start;
  gmpi_send_buf_ptr->dest = dest;
  pkt = ((void *)(&(gmpi_send_buf_ptr->buffer)));
  
  GMPI_DEBUG_CHECKSUM_COMPUTE((struct pkt_cksum *)pkt,
                              (void *)((gm_up_t)pkt 
                                       + sizeof(struct pkt_cksum)),
                              gmpi_send_buf_ptr->length 
                              - sizeof(struct pkt_cksum));
  GMPI_DEBUG_CHECKSUM_INFO((struct pkt_cksum *)pkt, 
                           GMPI_DEBUG_CHECKSUM_QUEUE_REG
			   | gmpi_send_buf_ptr->type);

  if (gmpi.send_buf_fifo_head == NULL)
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
      gmpi_debug_assert(gmpi.send_buf_fifo_tail == NULL);
      gmpi.send_buf_fifo_head = gmpi_send_buf_ptr;
    }
  else
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_tail->next == NULL);
      gmpi.send_buf_fifo_tail->next = gmpi_send_buf_ptr;
    }
  
  gmpi.send_buf_fifo_tail = gmpi_send_buf_ptr;
  gmpi_send_buf_ptr->next = NULL;
  gmpi.send_buf_fifo_queued++;
  GMPI_DEBUG_FIFO_SEND_ADD(0, 1, gmpi.send_buf_fifo_queued);
}


gm_inline void 
gmpi_put_data(MPIR_SHANDLE * shandle,
	      unsigned int dest,
	      unsigned int length,
	      void * start,
	      void * target_ptr)
{
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  unsigned int registered;
#if GM_DISABLE_REGISTRATION
  gm_up_t bounce_addr;
#endif

  gmpi_debug_assert(shandle != NULL);
  gmpi_debug_assert(start != NULL);
  gmpi_debug_assert(target_ptr != NULL);
  gmpi_debug_assert(dest < MPID_GM_size);
  gmpi_debug_assert(length > 0);
  
  gmpi_send_buf_ptr = 
    (struct gmpi_send_buf *) malloc (sizeof (struct gmpi_send_buf));
  gmpi_malloc_assert (gmpi_send_buf_ptr,
		      "gmpi_put_data",
		      "malloc: send buf for out data");

  gmpi_send_buf_ptr->type = GMPI_PUT_DATA;
  gmpi_send_buf_ptr->dest = dest;
  gmpi_send_buf_ptr->length = length;
  gmpi_send_buf_ptr->register_length = 0;
  gmpi_send_buf_ptr->source_ptr = start;
  gmpi_send_buf_ptr->target_ptr = target_ptr;
  gmpi_send_buf_ptr->shandle = shandle;
  gmpi_send_buf_ptr->buffer = 99;
  
  registered = 0;
  if ((gmpi.send_tokens > 0) 
      && (gmpi.send_buf_fifo_head == NULL)
      && (gmpi.dropped_sends[dest] == 0))
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
      
#if GM_DISABLE_REGISTRATION
      bounce_addr = gmpi_allocate_bounce_buffer(GMPI_BOUNCE_SEND, 
						(gm_up_t)start, length);
      if (bounce_addr != 0)
	{
	  memcpy((void *)bounce_addr, start, length);
	  start = (void *)bounce_addr;
	  registered = length;
	}
      else
	{
	  registered = 0;
	}
#else
      registered = gmpi_use_interval((gm_up_t)start, length);
#endif
      if (registered == length)
	{
	  GMPI_DEBUG_GM_SEND_PRINT(1, 0, start, registered, 
				   dest, target_ptr, (void *)shandle);
	  GMPI_DEBUG_REGISTRATION_CHECK_SEGMENT((gm_up_t)start, 
						registered, "PUT");
	  
          gmpi_debug_assert(gmpi.send_tokens > 0);
          gmpi.pending_sends[dest]++;
	  gm_directed_send_with_callback (gmpi_gm_port, start,
					  (gm_remote_ptr_t)(gm_up_t)target_ptr,
					  registered,
					  GM_LOW_PRIORITY,
					  gmpi.local_node_ids[dest],
					  gmpi.port_ids[dest],
					  gmpi_put_data_callback,
					  (void *)gmpi_send_buf_ptr);
	  
	  gmpi.pending_put_callback++;
	  gmpi.send_tokens--;
	  return;
	}
      else
	{
#if !GM_DISABLE_REGISTRATION
	  gmpi_unuse_interval((gm_up_t)start, registered);
#endif
	}
    }
  
  gmpi_send_buf_ptr->register_length = length;
  
  if (gmpi.send_buf_fifo_head == NULL)
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);
      gmpi_debug_assert(gmpi.send_buf_fifo_tail == NULL);
      gmpi.send_buf_fifo_head = gmpi_send_buf_ptr;
    }
  else
    {
      gmpi_debug_assert(gmpi.send_buf_fifo_tail->next == NULL);
      gmpi.send_buf_fifo_tail->next = gmpi_send_buf_ptr;
    }
  
  gmpi.send_buf_fifo_tail = gmpi_send_buf_ptr;
  gmpi_send_buf_ptr->next = NULL;
  gmpi.send_buf_fifo_queued++;
  GMPI_DEBUG_FIFO_SEND_ADD(1, 0, gmpi.send_buf_fifo_queued);
}


gm_inline int 
gmpi_packet_recv_event(MPID_Device * dev, void * buf)
{
  int from_grank, is_posted, err;
  MPIR_RHANDLE * rhandle;
  MPID_PKT_T * pkt_ptr;
 
  pkt_ptr = buf;
  err = MPI_SUCCESS;
  from_grank = pkt_ptr->head.src;
  
  GMPI_DEBUG_CHECKSUM_CHECK("Small message",
			    (void *)((gm_up_t)buf 
                                     + sizeof(struct pkt_cksum)),
                            from_grank, (struct pkt_cksum *)buf);
  DEBUG_PRINT_PKT("R received message", pkt_ptr);
    
  /* Separate the incoming messages from control messages */
  if (MPID_PKT_IS_MSG(pkt_ptr->head.mode))
    {
      DEBUG_PRINT_RECV_PKT("R rcvd msg", pkt_ptr);
      
      /* Is the message expected or not? 
	 This routine RETURNS a rhandle, creating one if the message 
	 is unexpected (is_posted == 0) */
      MPID_Msg_arrived(pkt_ptr->head.lrank,
		       pkt_ptr->head.tag,
		       pkt_ptr->head.context_id, 
		       &rhandle, &is_posted);
      
      MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt_ptr->head.msgrep);
#ifdef MPID_DEBUG_ALL
      if (MPID_DebugFlag)
	{
	  FPRINTF(MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n", 
		  MPID_GM_rank, is_posted ? "posted" : "unexpected",
		  __FILE__, __LINE__);
	}
#endif
      
      if (is_posted)
	{
	  switch (pkt_ptr->head.mode)
	    {
	    case MPID_PKT_SHORT:
	      GMPI_DEBUG_GM_RECV_PRINT("expected", "Short msg", from_grank);
	      DEBUG_TEST_FCN(dev->short_msg->recv,"dev->short->recv");
	      err = (*dev->short_msg->recv)(rhandle, from_grank, pkt_ptr);
	      break;
	      
	    case MPID_PKT_REQUEST_SEND:
	      GMPI_DEBUG_GM_RECV_PRINT("expected", "Request_to_send", 
				       from_grank);
	      DEBUG_TEST_FCN(dev->rndv->irecv,"dev->rndv->irecv");
	      err = (*dev->rndv->irecv)(rhandle, from_grank, pkt_ptr);
	      break;
	      
	    default:
	      fprintf(stderr, "[%d] Internal error: msg packet discarded "
		      "(%s:%d)\n", MPID_GM_rank, __FILE__, __LINE__);
	    }
	}
      else
	{
	  /* unexpected */
	  switch (pkt_ptr->head.mode)
	    {
	    case MPID_PKT_SHORT:
	      GMPI_DEBUG_GM_RECV_PRINT("unexpected", "Short msg", from_grank);
	      DEBUG_TEST_FCN(dev->short_msg->unex,"dev->short->unex");
	      err = (*dev->short_msg->unex)(rhandle, from_grank, pkt_ptr);
	      break;
	      
	    case MPID_PKT_REQUEST_SEND:
	      GMPI_DEBUG_GM_RECV_PRINT("unexpected", "Request_to_send", 
				       from_grank);
	      DEBUG_TEST_FCN(dev->rndv->unex,"dev->rndv->unex");
	      err = (*dev->rndv->unex)(rhandle, from_grank, pkt_ptr);
	      break;
	      
	    default:
	      fprintf(stderr, "[%d] Internal error: msg packet discarded "
		      "(%s:%d)\n", MPID_GM_rank, __FILE__, __LINE__);
	    }
	}
    }
  else
    {
      switch (pkt_ptr->head.mode)
	{
	case MPID_PKT_OK_TO_SEND:
	  GMPI_DEBUG_GM_RECV_PRINT("control", "Ok_to_send", from_grank);
	  DEBUG_TEST_FCN(dev->rndv->do_ack,"dev->rndv->do_ack");
	  err = (*dev->rndv->do_ack)(pkt_ptr, from_grank);
	  break;
	
	case MPID_PKT_DONE_SEND:
	  {
	    unsigned int chunk_size;
	    void * recvid;
	    void * done_target_ptr;
	    
	    GMPI_DEBUG_GM_RECV_PRINT("control", "Send_done", from_grank);
	    MPID_AINT_GET(recvid, pkt_ptr->done_pkt.recv_id);
	    MPID_AINT_GET(done_target_ptr, 
			  pkt_ptr->done_pkt.done_target_ptr);
	    rhandle = (MPIR_RHANDLE *)recvid;
	    gmpi_debug_assert(rhandle != NULL);
	    gmpi_debug_assert(done_target_ptr != NULL);
	    chunk_size = (rhandle->gm.current_expected
			  - rhandle->gm.current_done);
	    if (chunk_size > GMPI_MAX_PUT_LENGTH)
	      {
		chunk_size = GMPI_MAX_PUT_LENGTH;
	      }
	    
#if GM_DISABLE_REGISTRATION
	    memcpy((void *)((gm_up_t)rhandle->buf
			    + rhandle->gm.current_done),
		   done_target_ptr, chunk_size);
	    gmpi_free_bounce_buffer((gm_up_t)rhandle->buf
                                    + rhandle->gm.current_done,
                                    (unsigned int)chunk_size);
	    
#else
	    gmpi_debug_assert((gm_up_t)done_target_ptr 
			      == ((gm_up_t)rhandle->buf
				  + rhandle->gm.current_done));
	    gmpi_unuse_interval((gm_up_t)rhandle->buf
				+ rhandle->gm.current_done,
				(unsigned int)chunk_size);
#endif
	    if (gmpi.send_buf_fifo_head != NULL)
	      {
		gmpi_flush_fifo_send();
	      }
	    
	    rhandle->gm.current_done += chunk_size;
	    rhandle->gm.in_pipe--;
	    
	    gmpi_debug_assert(rhandle->gm.current_done 
			      <= rhandle->gm.current_expected);
	    if (rhandle->gm.current_done == rhandle->gm.current_expected)
	      {
		GMPI_DEBUG_CHECKSUM_CHECK("Large message",
					  (void *)(rhandle->buf),
					  from_grank,
					  &(rhandle->gm.cksum));
		rhandle->is_complete = 1;
 	        if (rhandle->finish)
		  {
		    (rhandle->finish)(rhandle);
		  }
	      }
	    else
	      {
		gmpi_debug_assert(rhandle->buf != NULL);
		gmpi_debug_assert(rhandle->send_id != NULL);
		err = MPID_CH_Rndvn_ok_to_send(rhandle);
	      }
	  }
	  break;
	  
	case MPID_PKT_ANTI_SEND:
	  MPID_SendCancelOkPacket(pkt_ptr, from_grank); 
	  break;
	  
	case MPID_PKT_ANTI_SEND_OK:
	  MPID_RecvCancelOkPacket(pkt_ptr, from_grank); 
	  break;
	  
	default:
	  {
	    unsigned char * pktp;
	    
	    fprintf(stderr, "[%d] Packet type %d (0x%x) is unknown "
		    "%s:%d!\n",  MPID_GM_rank, pkt_ptr->head.mode, 
		    pkt_ptr->head.mode,__FILE__, __LINE__);
	    pktp = (unsigned char *)pkt_ptr;
	    fprintf(stderr, "[%d]: acket dump: ptr=0x%lx buf=0x%lx "
		    "0x%x%x%x%x%x%x%x%x\n", MPID_GM_rank, 
		    (unsigned long)pkt_ptr, (unsigned long)buf,
		    pktp[0], pktp[1], pktp[2], pktp[3],
		    pktp[4], pktp[5], pktp[6], pktp[7]);
	    
	    if ((pktp[0] == 0xaa) && (pktp[1] == 0xaa)  &&
		(pktp[2] == 0xaa) && (pktp[3] == 0xaa))
	      {
		fprintf(stderr, "[%d]: GM could not translate a virtual "
			"address\n", MPID_GM_rank);
		gmpi_abort (0);
	      }
	  }
	}
    }
  
  DEBUG_PRINT_MSG("Exiting check_incoming");
  return err;
}


gm_inline int 
gmpi_net_lookup(MPID_Device * dev, int blocking)
{
  int fast, err, total;
  void * ptr;
  gm_recv_event_t *event;
  
  err = -1;
  total = 0;
  while (1)
    {
      if (blocking == MPID_NOTBLOCKING)
	{
	  event = gm_receive(gmpi_gm_port);
	}
      else
	{
	  event = (gmpi.gm_receive_mode)(gmpi_gm_port);
	}

      fast = 0;
      switch (gm_ntohc(event->recv.type))
	{
	case GM_NO_RECV_EVENT:
	  return err;
	  
	case GM_FAST_RECV_EVENT:
	case GM_FAST_HIGH_RECV_EVENT:
	case GM_FAST_PEER_RECV_EVENT:
	case GM_FAST_HIGH_PEER_RECV_EVENT:
	  fast = 1;
	  
	case GM_RECV_EVENT:
	case GM_HIGH_RECV_EVENT:
	case GM_PEER_RECV_EVENT:
	case GM_HIGH_PEER_RECV_EVENT:
	  if (fast == 1)
	    {
	      ptr = gm_ntohp(event->recv.message);
	    }
	  else
	    {
	      ptr = gm_ntohp(event->recv.buffer); 
	    }
	  
	  gmpi_debug_assert(gm_ntohc(event->recv.size) == GMPI_PKT_GM_SIZE);
	  gmpi_packet_recv_event(dev, ptr);
	  gm_provide_receive_buffer(gmpi_gm_port,
				    gm_ntohp(event->recv.buffer),
				    GMPI_PKT_GM_SIZE, GM_LOW_PRIORITY);
          total++;
	  if ((blocking == MPID_NOTBLOCKING) && (total >= 10))
	    { 
	      err = MPI_SUCCESS;
	    }
	  else
	    {
	      return MPI_SUCCESS;
	    }
	  break;
	  
	default:
          callback_flag = 0;
	  gm_unknown(gmpi_gm_port, event);
          if (callback_flag == 1)
            {
	     if (blocking == MPID_NOTBLOCKING)
	       { 
		 err = MPI_SUCCESS;
	       }
	     else
	       {
		 return MPI_SUCCESS;
	       }
	    }
	}
    }
}


int 
MPID_CH_Check_incoming(MPID_Device *dev, MPID_BLOCKING_TYPE blocking)
{
  int found;
 
  found = -1;
  GMPI_PROGRESSION_LOCK();
  found = gmpi_net_lookup(dev, blocking);
  GMPI_PROGRESSION_UNLOCK();

  return found;
}


void 
gmpi_init(int *argc,char ***argv)
{
  gm_status_t status;
  struct gmpi_send_buf * gmpi_send_buf_ptr;
  struct gmpi_send_buf * previous_send_buf_ptr;
  void * ptr;
  unsigned int i, buf_len;
  
#if GMPI_DEBUG_OUTPUT_FILE
  char output_name[256];
  
  sprintf(output_name, "%d.out", MPID_GM_rank); 
  gmpi.debug_output_filedesc = fopen(output_name, "w+");
#else
  gmpi.debug_output_filedesc = stderr;
#endif
  
  gmpi_debug_assert (gmpi_gm_port != NULL);

#if !GM_DISABLE_REGISTRATION 
  /* Check that the malloc hook has not been overloaded by another lib. */
  gmpi.malloc_hook_flag = 0;
  ptr = malloc (GMPI_MALLOC_HOOK_CHECK_SIZE * sizeof (char));
  gmpi_malloc_assert(ptr, "gmpi_init", "malloc: malloc hook test");
  ((unsigned char *) ptr)[0] = 99;
  ((unsigned char *) ptr)[GMPI_MALLOC_HOOK_CHECK_SIZE-1]=99;
  free (ptr);
  if (gmpi.malloc_hook_flag == 0) 
    {
      
      fprintf(stderr, "[%d] Malloc hook test failed: the malloc provided by MPICH-GM is not used, "
                      "it may have been overloaded by a malloc provided by another library.\n\n", 
                      MPID_GM_rank);
      gmpi_abort (0);
    }
#endif
 
  status = gm_allow_remote_memory_access(gmpi_gm_port);
  if (status != GM_SUCCESS)
    {
      gm_perror("GM error in gm_allow_remote_memory_access", status);
      fprintf(stderr, "Cannot allow remote memory access\n");
      gmpi_abort (0);
    }
  
  /* needs to save one send token for armci */
  gmpi.send_tokens = gm_num_send_tokens(gmpi_gm_port) - 1;
  gmpi.max_send_tokens = gmpi.send_tokens;
  gmpi.recv_tokens = ((gm_num_receive_tokens(gmpi_gm_port)
		       > GMPI_MAX_RECV_TOKENS) 
		      ? GMPI_MAX_RECV_TOKENS 
		      : gm_num_receive_tokens(gmpi_gm_port));
  gmpi.max_recv_tokens = gmpi.recv_tokens;
  gmpi.pending_put_callback = 0;
  gmpi.pending_send_callback = 0;
  gmpi.unexpected_short = 0;

  gmpi.send_buf_fifo_queued = 0; 
  gmpi.send_dma_buf_pool_remaining = GMPI_INITIAL_DMA_SEND_BUF; 
  gmpi.malloc_send_buf_allocated = 0;
  gmpi.send_buf_fifo_head = NULL;
  gmpi.send_buf_fifo_tail = NULL;
  
  status = gm_set_acceptable_sizes(gmpi_gm_port, GM_HIGH_PRIORITY, 0);
  if (status != GM_SUCCESS)
    {
      gm_perror("GM error in gm_set_acceptable_sizes", status);
      fprintf(stderr,
	      "Cannot restrict the acceptable sizes for High Priority\n");
      gmpi_abort (0);
    }
  
  status = gm_set_acceptable_sizes(gmpi_gm_port, GM_LOW_PRIORITY,
				   (gm_size_t)((gm_size_t)1U 
					       << GMPI_PKT_GM_SIZE)); 
  if (status != GM_SUCCESS)
    {
      gm_perror("GM error in gm_set_acceptable_sizes", status);
      fprintf(stderr, 
	      "Cannot restrict the acceptable sizes for Low Priority\n");
      gmpi_abort (0);
    }
  
  gmpi_debug_assert(gmpi.send_tokens == gmpi.max_send_tokens);
  gmpi_debug_assert(gmpi.recv_tokens == gmpi.max_recv_tokens);
  buf_len = gmpi.eager_size + sizeof(MPID_PKT_SHORT_T);
  
  for (i=0; i<gmpi.max_recv_tokens; i++)
    {
      ptr = gmpi_dma_alloc(buf_len);
      gmpi_malloc_assert(ptr, "gmpi_init",
			 "gmpi_dma_alloc: dma recv buffers");
      
      GMPI_DEBUG_DMA_MEMORY_ACQUIRE((gm_up_t)ptr,buf_len);
      GMPI_DEBUG_DMA_MEMORY_USE(buf_len);
      
      gm_provide_receive_buffer_with_tag(gmpi_gm_port, ptr, 
					 GMPI_PKT_GM_SIZE,
					 GM_LOW_PRIORITY, i+1);
    }
  
  gmpi.recv_tokens -= gmpi.max_recv_tokens;
  gmpi_debug_assert(gmpi.recv_tokens == 0);
  buf_len = (gmpi.eager_size + sizeof(MPID_PKT_SHORT_T) 
	     + sizeof(struct gmpi_send_buf));
  
  previous_send_buf_ptr = NULL;
  for (i=0; i<GMPI_INITIAL_DMA_SEND_BUF; i++)
    {
      gmpi_send_buf_ptr = (struct gmpi_send_buf *)gmpi_dma_alloc(buf_len);
      gmpi_malloc_assert(gmpi_send_buf_ptr, "gmpi_init",
			 "gmpi_dma_alloc: dma send buffers");
      GMPI_DEBUG_DMA_MEMORY_ACQUIRE((gm_up_t)gmpi_send_buf_ptr,buf_len);
      GMPI_DEBUG_DMA_MEMORY_USE(buf_len);
      
      gmpi_send_buf_ptr->type = GMPI_DMA_SEND_BUF;
      
      if (i == 0)
	{
	  gmpi.send_dma_buf_free = gmpi_send_buf_ptr;
	  previous_send_buf_ptr = gmpi_send_buf_ptr;
	}
      else
	{
	  previous_send_buf_ptr->next = gmpi_send_buf_ptr;
	  previous_send_buf_ptr = gmpi_send_buf_ptr;
	}
    }
  previous_send_buf_ptr->next = NULL;

  for (i=0; i<MPID_GM_size; i++)
    {
      gmpi.pending_sends[i] = 0;
      gmpi.dropped_sends[i] = 0;
    }
      
#if GM_DISABLE_REGISTRATION
  gmpi_bounce_buffer_init();
#else
  gmpi_regcache_init();
#endif
  GMPI_PROGRESSION_INIT();
}


void 
gmpi_finish(MPID_Device * dev)
{
  int i;
  
  while ((gmpi.send_buf_fifo_head != NULL)
	 || (gmpi.send_tokens < gmpi.max_send_tokens)
         || (gmpi.send_dma_buf_pool_remaining < GMPI_INITIAL_DMA_SEND_BUF))
    {
      (*dev->check_device)(dev, MPID_NOTBLOCKING);
    }
  GMPI_PROGRESSION_FINISH();
  
  gmpi_debug_assert(gmpi.send_dma_buf_pool_remaining 
		    == GMPI_INITIAL_DMA_SEND_BUF);
  gmpi_debug_assert(gmpi.send_buf_fifo_queued == 0);

#if GM_DISABLE_REGISTRATION
  gmpi_bounce_buffer_finish();
#endif

  GMPI_DEBUG_DMA_MEMORY_FINAL(((gmpi.eager_size
				+ sizeof(MPID_PKT_SHORT_T))
			       * gmpi.max_recv_tokens) 
			      + ((gmpi.eager_size 
				  + sizeof(MPID_PKT_SHORT_T)
				  + sizeof(struct gmpi_send_buf)) 
				 * GMPI_INITIAL_DMA_SEND_BUF));
  
  gmpi_clear_all_intervals();
  gm_close(gmpi_gm_port);
  
  for (i=0; i<MPID_GM_size; i++)
    {
      if (gmpi.host_names[i] != NULL)
	{
	  free (gmpi.host_names[i]);
	  free (gmpi.exec_names[i]);
	}
      else
	{
	  i = MPID_GM_size;
	}
    }
  
  fclose(gmpi.debug_output_filedesc);
  fflush (stdout);
  fflush (stderr);
}


void 
gmpi_printf(char *format, ...)
{
  va_list ap;
  
  va_start(ap, format);
  vfprintf(stderr,format, ap);
  va_end(ap);
}


void 
gmpi_abort (int code)
{
  static int aborted = 0;
  int sockfd, count, i;
  char buffer[32];
  
  if (aborted == 0)
    {
      if (gmpi.mpd)
	{
	  MPD_Abort (code);
	}
      else
	{
	  fflush (stderr);
	  fflush (stdout);
	  aborted = 1;
	  GMPI_PROGRESSION_FINISH();
      
	  /* get a socket */
	  sockfd = socket (AF_INET, SOCK_STREAM, 0);
	  if (sockfd < 0)
	    {
	      fprintf (stderr, "[%d] Error: Unable to open a socket !\n", 
		       MPID_GM_rank);
	      exit (-1);
	    }
	  
	  /* connect to the master */
	  i = 10;
	  while (connect (sockfd, (struct sockaddr *) (&(gmpi.master_addr)), 
			  sizeof (gmpi.master_addr)) < 0)
	    {
#ifdef WIN32
	      Sleep (1);
#else
	      sleep(1); 
#endif
	      if (i == 0)
		{
		  fprintf (stderr, "[%d] Error: Unable to connect to "
			   "the master to abort !\n", MPID_GM_rank);
		  exit (-1);
		}
	      i--;
	    }
	  
	  /* send the abort message */
	  count = 0;
	  sprintf (buffer, "<<<ABORT_%d_ABORT>>>", gmpi.magic);
	  while (count < strlen (buffer))
	    {
	      i = write (sockfd, &(buffer[count]), strlen (buffer) - count);
	      if (i < 0)
		{
		  fprintf (stderr, "[%d] Error: write to socket failed !\n", 
			   MPID_GM_rank);
		  exit (-1);;
		}
	      count += i;
	    }
	  
	  close (sockfd);
	  exit (-1);
	}
    }
}


/* Created a routine for remote PID's for TotalView debugger */
int 
gmpi_proc_info (int i, char **host, char **exename)
{
  int pid;
  
  pid = gmpi.mpi_pids[i];
  *host = gmpi.host_names[i];
  *exename = gmpi.exec_names[i];
  return (pid);
}
