/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_fifo_send_h
#define _gmpi_debug_fifo_send_h


#if GMPI_DEBUG_FIFO_SEND

void gmpi_debug_fifo_send_add(unsigned int put, 
			      unsigned int reg, 
			      unsigned int total);
void gmpi_debug_fifo_send_remove(unsigned int put, 
				 unsigned int total);


#define GMPI_DEBUG_FIFO_SEND_ADD(put,reg,total) \
gmpi_debug_fifo_send_add(put,reg, total)
#define GMPI_DEBUG_FIFO_SEND_REMOVE(put,total) \
gmpi_debug_fifo_send_remove(put, total)
#else
#define GMPI_DEBUG_FIFO_SEND_ADD(put,reg,total)
#define GMPI_DEBUG_FIFO_SEND_REMOVE(put,total)
#endif

#endif
