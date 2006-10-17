/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_checksum_type_h
#define _gmpi_debug_checksum_type_h

struct pkt_cksum
{
  unsigned long sum;
  unsigned int len;
  unsigned int info;
};

#define GMPI_DEBUG_CHECKSUM_SEND 16
#define GMPI_DEBUG_CHECKSUM_SEND_QUEUED 32
#define GMPI_DEBUG_CHECKSUM_QUEUE_REG 64

#endif

