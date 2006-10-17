/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_noreg_h
#define _gmpi_noreg_h


#define GMPI_BOUNCE_SEND         66
#define GMPI_BOUNCE_RECV         77
#define GMPI_BOUNCE_SEGMENT_FREE 88
#define GMPI_BOUNCE_SEGMENT_USED 99


typedef struct bounce_buffer
{
  struct bounce_buffer *next;
  gm_up_t addr;
  gm_up_t data_addr;
  unsigned int length;
  unsigned int status;
  unsigned int type;
} gmpi_bounce_buffer;


void gmpi_bounce_buffer_init(void);
void gmpi_bounce_buffer_finish(void);
gm_up_t gmpi_allocate_bounce_buffer(unsigned int type, gm_up_t data_addr, 
				    unsigned int length);
void gmpi_free_bounce_buffer(gm_up_t data_addr, unsigned int length);

#endif



