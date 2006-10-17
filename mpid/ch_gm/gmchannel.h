/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmchannel_h
#define _gmchannel_h

#include "channel.h"
#include "gmpi.h"

#ifdef GM_ORGIGINAL
#define MPID_SendControl(pkt,size,dest)                     \
{                                                           \
  struct gmpi_send_buf *gmpi_send_buf_ptr;                  \
  void *tmp_pkt;                                            \
                                                            \
  tmp_pkt = gmpi_allocate_packet (size * sizeof (char),     \
                                  &gmpi_send_buf_ptr);      \
  memcpy (tmp_pkt, pkt, size * sizeof (char));              \
  gmpi_send_packet(gmpi_send_buf_ptr, dest);                \
}

#define MPID_SendControlBlock(pkt,size,dest) .

#define MPIDPATCHLEVEL 2.0

#define MPID_DRAIN_INCOMING \
    while (MPID_DeviceCheck( MPID_NOTBLOCKING ) != -1) ;
#ifdef MPID_TINY_BUFFERS 
#define MPID_DRAIN_INCOMING_FOR_TINY(is_non_blocking) \
{if (is_non_blocking) {MPID_DRAIN_INCOMING;}}
#else
#define MPID_DRAIN_INCOMING_FOR_TINY(is_non_blocking)
#endif

/* 
   These macros control the conversion of packet information to a standard
   representation.  On homogeneous systems, these do nothing.
 */
#ifdef MPID_HAS_HETERO
#define MPID_PKT_PACK(pkt,size,dest) MPID_CH_Pkt_pack((MPID_PKT_T*)(pkt),size,dest)
#define MPID_PKT_UNPACK(pkt,size,src) MPID_CH_Pkt_unpack((MPID_PKT_T*)(pkt),size,src)
#else
#define MPID_PKT_PACK(pkt,size,dest) 
#define MPID_PKT_UNPACK(pkt,size,src) 
#endif

#endif /* GM_ORIGINAL */

MPID_Device *MPID_CH_InitSelfMsg(int * argc, char ***argv, int short_len, int long_len);

#endif /* _gmchannel_h */


