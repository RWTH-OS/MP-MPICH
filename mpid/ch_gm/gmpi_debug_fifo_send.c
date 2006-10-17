/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


void gmpi_debug_fifo_send_add(unsigned int put, 
			      unsigned int reg, 
			      unsigned int total)
{
  if (put)
    {
      fprintf(gmpi.debug_output_filedesc, 
	      "[%d]: ADD one entry (PUT) to FIFO send "
	      "(Total = %d, ST=%d/%d)\n", MPID_GM_rank, total, 
	      gmpi.send_tokens, gmpi.max_send_tokens);
    }
  else
    {
      if (reg)
	{
	  fprintf(gmpi.debug_output_filedesc, 
		  "[%d]: ADD one entry (SMALL+reg) to FIFO send "
		  "(Total = %d, ST=%d/%d)\n", MPID_GM_rank, total, 
		  gmpi.send_tokens, gmpi.max_send_tokens);
	}
      else
	{
	  fprintf(gmpi.debug_output_filedesc, 
		  "[%d]: ADD one entry (SMALL) to FIFO send "
		  "(Total = %d, ST=%d/%d)\n", MPID_GM_rank, total, 
		  gmpi.send_tokens, gmpi.max_send_tokens);
	}
    }
  fflush(gmpi.debug_output_filedesc);
}


void gmpi_debug_fifo_send_remove(unsigned int put, unsigned int total)
{
  if (put)
    {
      fprintf(gmpi.debug_output_filedesc, 
	      "[%d]: REMOVE one entry (PUT) from FIFO send "
	      "(Total = %d, ST=%d/%d)\n", MPID_GM_rank, total, 
	      gmpi.send_tokens, gmpi.max_send_tokens);
    }
  else
    {
      fprintf(gmpi.debug_output_filedesc, 
	      "[%d]: REMOVE one entry (SMALL) from FIFO send "
	      "(Total = %d, ST=%d/%d)\n", MPID_GM_rank, total, 
	      gmpi.send_tokens, gmpi.max_send_tokens);
    }
  fflush(gmpi.debug_output_filedesc);
}
