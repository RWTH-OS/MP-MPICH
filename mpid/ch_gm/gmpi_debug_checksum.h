/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_checksum_h
#define _gmpi_debug_checksum_h

#ifndef GM_ORIGINAL
/* does not work with mp-mpich */
#define GMPI_DEBUG_CHECKSUM 0
#endif

#include "gmpi_debug_checksum_type.h"

#if GMPI_DEBUG_CHECKSUM 

void gmpi_debug_checksum_compute(struct pkt_cksum *cksum_ptr,
				 void *buf,
				 unsigned int len);
void gmpi_debug_checksum_check(char *msg, void *buf,
			       unsigned int from,
			       struct pkt_cksum *cksum_ptr);
void gmpi_debug_checksum_copy(struct pkt_cksum *cksum_target,
			      struct pkt_cksum *cksum_source);
void gmpi_debug_checksum_info(struct pkt_cksum * cksum_ptr,
			      unsigned int info);
     

#define GMPI_DEBUG_CHECKSUM_SMALL struct pkt_cksum cksum_small;
#define GMPI_DEBUG_CHECKSUM_LARGE struct pkt_cksum cksum_large;
#define GMPI_DEBUG_CHECKSUM_COMPUTE(cksum_ptr,buf,len) \
gmpi_debug_checksum_compute(cksum_ptr, buf, len)
#define GMPI_DEBUG_CHECKSUM_CHECK(msg,buf,from,cksum_ptr) \
gmpi_debug_checksum_check(msg, buf, from, cksum_ptr)
#define GMPI_DEBUG_CHECKSUM_COPY(cksum_target,cksum_source) \
gmpi_debug_checksum_copy(cksum_target, cksum_source)
#define GMPI_DEBUG_CHECKSUM_INFO(cksum_ptr,info) \
gmpi_debug_checksum_info(cksum_ptr, info)
#else
#define GMPI_DEBUG_CHECKSUM_SMALL 
#define GMPI_DEBUG_CHECKSUM_LARGE 
#define GMPI_DEBUG_CHECKSUM_COMPUTE(cksum_ptr,buf,len)  
#define GMPI_DEBUG_CHECKSUM_CHECK(msg,buf,from,cksum_ptr) 
#define GMPI_DEBUG_CHECKSUM_COPY(cksum_target,cksum_source) 
#define GMPI_DEBUG_CHECKSUM_INFO(cksum_ptr,info) 
#endif

#endif
