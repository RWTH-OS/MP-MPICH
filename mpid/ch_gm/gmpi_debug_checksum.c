/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


void gmpi_debug_checksum_compute(struct pkt_cksum * cksum_ptr,
				 void * ptr, 
				 unsigned int len)
{
  cksum_ptr->sum = gm_crc (ptr, len);
  cksum_ptr->len = len;
  cksum_ptr->info = 0;
}


void gmpi_debug_checksum_check(char * msg, void * buf,
			       unsigned int from,
			       struct pkt_cksum *cksum_ptr)
{
  struct pkt_cksum cksum_comp;
  
  gmpi_debug_checksum_compute(&cksum_comp, buf, cksum_ptr->len);
  if (cksum_comp.sum != cksum_ptr->sum)
    {
      fprintf(stderr, "[%d]: MPI-GM checksum error on %s (0x%lx/0x%lx) "
	      "from %d, info = %d\n", MPID_GM_rank, msg, 
	      cksum_comp.sum, cksum_ptr->sum, from, cksum_ptr->info);
      gmpi_abort (0);
    }
}


void gmpi_debug_checksum_copy(struct pkt_cksum * cksum_target,
			      struct pkt_cksum * cksum_source)
{
  cksum_target->sum = cksum_source->sum;
  cksum_target->len = cksum_source->len;
  cksum_target->info = cksum_source->info;
}


void gmpi_debug_checksum_info(struct pkt_cksum * cksum_ptr,
			      unsigned int info)
{
  cksum_ptr->info = cksum_ptr->info | info;
}

