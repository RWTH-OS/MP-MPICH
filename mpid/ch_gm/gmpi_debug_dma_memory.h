/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_dma_memory_h
#define _gmpi_debug_dma_memory_h

extern unsigned long mem_locked_total;
extern unsigned long mem_locked_in_use;

#if GMPI_DEBUG_DMA_MEMORY

void gmpi_debug_dma_memory_alloc(void *ptr, unsigned int length);
void gmpi_debug_dma_memory_free(void *ptr);
void gmpi_debug_dma_memory_acquire(gm_up_t ptr, unsigned int size);
void gmpi_debug_dma_memory_release(gm_up_t ptr, unsigned int size);
void gmpi_debug_dma_memory_use(unsigned int size);
void gmpi_debug_dma_memory_unuse(unsigned int size);
void gmpi_debug_dma_memory_final(unsigned long static_mem_size);


#define GMPI_DEBUG_DMA_MEMORY_ALLOC(ptr,length) \
gmpi_debug_dma_memory_alloc(ptr,length)
#define GMPI_DEBUG_DMA_MEMORY_FREE(ptr) \
gmpi_debug_dma_memory_free(ptr)
#define GMPI_DEBUG_DMA_MEMORY_ACQUIRE(ptr,size) \
gmpi_debug_dma_memory_acquire(ptr,size)
#define GMPI_DEBUG_DMA_MEMORY_RELEASE(ptr,size) \
gmpi_debug_dma_memory_acquire(ptr,size)
#define GMPI_DEBUG_DMA_MEMORY_USE(size) \
gmpi_debug_dma_memory_use(size)
#define GMPI_DEBUG_DMA_MEMORY_UNUSE(size) \
gmpi_debug_dma_memory_unuse(size)
#define GMPI_DEBUG_DMA_MEMORY_FINAL(static_size) \
gmpi_debug_dma_memory_final(static_size)
#else
#define GMPI_DEBUG_DMA_MEMORY_ALLOC(ptr,length)
#define GMPI_DEBUG_DMA_MEMORY_FREE(ptr)
#define GMPI_DEBUG_DMA_MEMORY_ACQUIRE(ptr,size)
#define GMPI_DEBUG_DMA_MEMORY_RELEASE(ptr,size)
#define GMPI_DEBUG_DMA_MEMORY_USE(size)
#define GMPI_DEBUG_DMA_MEMORY_UNUSE(size)
#define GMPI_DEBUG_DMA_MEMORY_FINAL(static_size)
#endif

#endif
