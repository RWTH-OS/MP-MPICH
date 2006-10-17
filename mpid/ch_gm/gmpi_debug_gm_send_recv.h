/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_gm_send_recv_h
#define _gmpi_debug_gm_send_recv_h


#if GMPI_DEBUG_GM_SEND

void gmpi_debug_gm_send_print(int put, int fifo, void * ptr,
                              unsigned long length,
                              unsigned int dest, 
                              void * extra, void *extra2);
void gmpi_debug_gm_send_finish(unsigned int put, void * ptr,
			       unsigned int length,
			       unsigned int dest, void *extra);

#define GMPI_DEBUG_GM_SEND_PRINT(put,fifo,ptr,length,dest,extra,extra2) \
gmpi_debug_gm_send_print(put, fifo, ptr, length, dest, extra, extra2)
#define GMPI_DEBUG_GM_SEND_FINISH(put,ptr,length,dest,extra) \
gmpi_debug_gm_send_finish(put, ptr, length, dest, extra)
#else
#define GMPI_DEBUG_GM_SEND_PRINT(put,fifo,ptr,length,dest,extra,extra2)
#define GMPI_DEBUG_GM_SEND_FINISH(put,ptr,length,dest,extra)
#endif


#if GMPI_DEBUG_GM_RECV

void gmpi_debug_gm_recv_print(char *expected, char *type,
                              unsigned int from);

#define GMPI_DEBUG_GM_RECV_PRINT(expected,type,from) \
gmpi_debug_gm_recv_print(expected,type,from)
#else
#define GMPI_DEBUG_GM_RECV_PRINT(expected,type,from)
#endif

#endif
