/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <unistd.h>
#include "gmpi.h"

unsigned long mem_locked_total = 0;
unsigned long mem_locked_in_use = 0;


void gmpi_debug_dma_memory_alloc(void *ptr, unsigned int length)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: gm_dma_malloc ptr=0x%lx len=%d\n",
	  MPID_GM_rank, (unsigned long)ptr, length);
}


void gmpi_debug_dma_memory_free(void *ptr)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: gm_dma_free ptr=0x%lx",
	  MPID_GM_rank, (unsigned long)ptr);
}


void gmpi_debug_dma_memory_acquire(gm_up_t ptr, unsigned int size)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: GM Register ptr=0x%lx len=%d\n",
	  MPID_GM_rank, (unsigned long)ptr, size);
  fflush(gmpi.debug_output_filedesc);
  mem_locked_total += size;
}


void gmpi_debug_dma_memory_release(gm_up_t ptr, unsigned int size)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: GM Deregister ptr=0x%lx len=%d\n",
	  MPID_GM_rank, (unsigned long)ptr, size);
  fflush(gmpi.debug_output_filedesc);
  mem_locked_total -= size;
}


void gmpi_debug_dma_memory_use(unsigned int size)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: DMA memory in use: %ld (+%d) \n",
	  MPID_GM_rank, mem_locked_in_use+size, size);
  fflush(gmpi.debug_output_filedesc);
  mem_locked_in_use += size;
}


void gmpi_debug_dma_memory_unuse(unsigned int size)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: DMA memory in use: %ld (-%d) \n",
	  MPID_GM_rank, mem_locked_in_use-size, size);
  fflush(gmpi.debug_output_filedesc);
  mem_locked_in_use -= size;
}


void gmpi_debug_dma_memory_final(unsigned long static_mem_size)
{
  usleep((MPID_GM_rank*100000)+1000000);
  fprintf(stderr, "[%d]: DMA Memory usage: Total  = %ld Bytes, "
	  "In use = %ld Bytes\n",
	  MPID_GM_rank, mem_locked_total - static_mem_size,
	  mem_locked_in_use - static_mem_size);
}

