/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <stdlib.h>
#include "gmpi.h"


static gmpi_bounce_buffer *gmpi_bounce_free = NULL;
static gmpi_bounce_buffer *gmpi_bounce_used = NULL;
static gmpi_bounce_buffer *gmpi_bounce_buffers = NULL;
static unsigned int gmpi_bounce_recv_cnt = GMPI_BOUNCE_BUFFERS;


void 
gmpi_bounce_buffer_init(void)
{
  gmpi_bounce_buffer *previous_buffer = NULL;
  unsigned int i;

  gmpi_debug_assert(GM_PAGE_LEN != 0);
  if ((gmpi_bounce_buffers == NULL) 
      && (gmpi_bounce_free == NULL) 
      && (gmpi_bounce_used == NULL))
    {
      gmpi_bounce_buffers = 
	(gmpi_bounce_buffer *) malloc((gmpi.max_send_tokens 
				       + GMPI_BOUNCE_BUFFERS) 
				      * sizeof(gmpi_bounce_buffer));
      for (i = 0; i < (gmpi.max_send_tokens + GMPI_BOUNCE_BUFFERS); i++)
	{
	  gmpi_bounce_buffers[i].next = previous_buffer;
	  gmpi_bounce_buffers[i].addr =
	    (gm_up_t) gmpi_dma_alloc(GMPI_MAX_PUT_LENGTH);
	  gmpi_malloc_assert((void *)(gmpi_bounce_buffers[i].addr),
			     "gmpi_bounce_buffer_init",
			     "gmpi_dma_alloc: bounce buffer");
	  GMPI_DEBUG_DMA_MEMORY_ACQUIRE(gmpi_bounce_buffers[i].addr,
					GMPI_MAX_PUT_LENGTH);
	  GMPI_DEBUG_DMA_MEMORY_USE(GMPI_MAX_PUT_LENGTH);
	  gmpi_bounce_buffers[i].data_addr = 0;
	  gmpi_bounce_buffers[i].length = 0;
	  gmpi_bounce_buffers[i].status = GMPI_BOUNCE_SEGMENT_FREE;
	  gmpi_bounce_buffers[i].type = 0;
	  previous_buffer = &(gmpi_bounce_buffers[i]);
	}
      
      gmpi_bounce_free = previous_buffer;
    }
}


void
gmpi_bounce_buffer_finish(void)
{
  unsigned int i;
  
  if ((gmpi_bounce_buffers != NULL) 
      && ((gmpi_bounce_free != NULL) 
	  || (gmpi_bounce_used != NULL)))
    {
      for (i = 0; i < (gmpi.max_send_tokens + GMPI_BOUNCE_BUFFERS); i++)
	{
	  gmpi_dma_free((void *)(gmpi_bounce_buffers[i].addr));
	  GMPI_DEBUG_DMA_MEMORY_RELEASE(gmpi_bounce_buffers[i].addr,
					GMPI_MAX_PUT_LENGTH);
	  GMPI_DEBUG_DMA_MEMORY_UNUSE(GMPI_MAX_PUT_LENGTH);
	}
      free(gmpi_bounce_buffers);
      gmpi_bounce_buffers = NULL;
      gmpi_bounce_free = NULL;
      gmpi_bounce_used = NULL;
    }
}


gm_up_t 
gmpi_allocate_bounce_buffer(unsigned int type, gm_up_t data_addr, 
			    unsigned int length)
{
  gmpi_bounce_buffer *bounce_buffer;
  
  GMPI_DEBUG_REG_CACHE_PRINT1("Allocate bounce buffer", "length", length);
  gmpi_debug_assert(gmpi_bounce_buffers[0].addr != 0);
  
  if (gmpi_bounce_free != NULL)
    {
      if (type == GMPI_BOUNCE_RECV) {
	if (gmpi_bounce_recv_cnt > 0) {
	  gmpi_bounce_recv_cnt--;
	} else {
	  GMPI_DEBUG_REG_CACHE_PRINT0("Allocate bounce buffer: Recv limited");
	  return 0;
	}
      }
      gmpi_debug_assert(gmpi_bounce_free->status == GMPI_BOUNCE_SEGMENT_FREE);
      bounce_buffer = gmpi_bounce_free;
      bounce_buffer->data_addr = data_addr;
      bounce_buffer->length = length;
      bounce_buffer->status = GMPI_BOUNCE_SEGMENT_USED;
      bounce_buffer->type = type;
      gmpi_bounce_free = bounce_buffer->next;
      bounce_buffer->next = gmpi_bounce_used;
      gmpi_bounce_used = bounce_buffer;
      
      GMPI_DEBUG_REG_CACHE_PRINT2("Allocate bounce buffer",
				  "data_addr", data_addr,
				  "bounce_addr", bounce_buffer->addr); 
      return (bounce_buffer->addr);
    }
  else
    {	
      GMPI_DEBUG_REG_CACHE_PRINT0("Allocate bounce buffer: no luck");
      return 0;
    }

}


void 
gmpi_free_bounce_buffer(gm_up_t data_addr, unsigned int length)
{
  gmpi_bounce_buffer * bounce_buffer;
  gmpi_bounce_buffer * previous_bounce_buffer;
  
  GMPI_DEBUG_REG_CACHE_PRINT2("Free bounce buffer", "data_addr",
			      data_addr, "length", length);
  
  previous_bounce_buffer = NULL;
  bounce_buffer = gmpi_bounce_used;
  while (bounce_buffer != NULL)
    {
      if (bounce_buffer->data_addr == data_addr)
	{
	  gmpi_debug_assert(bounce_buffer->length == length);
	  gmpi_debug_assert(bounce_buffer->status == GMPI_BOUNCE_SEGMENT_USED);
	  bounce_buffer->status = GMPI_BOUNCE_SEGMENT_FREE;
	  if (previous_bounce_buffer != NULL)
	    {
	      previous_bounce_buffer->next = bounce_buffer->next;
	    }
	  else
	    {
	      gmpi_bounce_used = bounce_buffer->next;
	    }
	  bounce_buffer->next = gmpi_bounce_free;
	  gmpi_bounce_free = bounce_buffer;

	  if (bounce_buffer->type == GMPI_BOUNCE_RECV) {
	    gmpi_bounce_recv_cnt++;
	    gmpi_debug_assert(gmpi_bounce_recv_cnt <= GMPI_BOUNCE_BUFFERS);
	  }
	  
	  return;
	}
      else
	{
	  previous_bounce_buffer = bounce_buffer;
	  bounce_buffer = bounce_buffer->next;
	}
    }
  gmpi_debug_assert(0);
  gmpi_abort(0);
}
