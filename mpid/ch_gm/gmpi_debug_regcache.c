/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


void gmpi_debug_reg_cache_print0(char *msg)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: %s\n", MPID_GM_rank, msg);
  fflush(gmpi.debug_output_filedesc);
}


void gmpi_debug_reg_cache_print1(char *msg, char *label1, unsigned long value1)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: %s, %s=0x%lx\n", 
	  MPID_GM_rank, msg, label1, value1);
  fflush(gmpi.debug_output_filedesc);
}


void gmpi_debug_reg_cache_print2(char *msg, char *label1, unsigned long value1,
				 char *label2, unsigned long value2)
{
  fprintf(gmpi.debug_output_filedesc, "[%d]: %s, %s=0x%lx, %s=0x%lx\n", 
	  MPID_GM_rank, msg, label1, value1, label2, value2);
  fflush(gmpi.debug_output_filedesc);
}

