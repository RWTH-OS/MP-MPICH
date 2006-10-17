/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


void gmpi_debug_gm_send_print(int put, int fifo, void * ptr,
			      unsigned long length,
			      unsigned int dest, 
                              void *extra, void *extra2)
{
  if (put)
    {
      if (fifo)
	{
	  fprintf(gmpi.debug_output_filedesc,
		  "[%d]: Send PUT packet (buf %p, %ld B) "
		  "to %d from FIFO (Remote_ptr=%p) shandle=%p\n",
		  MPID_GM_rank, ptr, length, dest, extra,
                  extra2);
	}
      else
	{
	  fprintf(gmpi.debug_output_filedesc,
		  "[%d]: Send PUT packet (buf %p, %ld B) "
		  "to %d (Remote_ptr=%p) shandle=%p\n",
		  MPID_GM_rank, ptr, length, dest, 
                  extra, extra2); 
	}
    }
  else
    {
      if (fifo)
	{
	  fprintf(gmpi.debug_output_filedesc,
		  "[%d]: Send SMALL packet (buf %p, %ld B) "
		  "to %d from FIFO send_buf_ptr=%p\n",
		  MPID_GM_rank, ptr, length, dest, extra2);
	}
      else
	{
	  fprintf(gmpi.debug_output_filedesc, 
		  "[%d]: Send SMALL packet (buf %p, %ld B) "
		  "to %d send_buf_ptr=%p\n", 
		  MPID_GM_rank, ptr, length, dest, extra2);
	}
    }
  fflush(gmpi.debug_output_filedesc);
}


void gmpi_debug_gm_send_finish(unsigned int put,
			       void * ptr,
			       unsigned int length,
			       unsigned int dest,
                               void *extra)
{
  if (put)
    {
      fprintf(gmpi.debug_output_filedesc, 
	      "[%d]: Send completion PUT packet (buf %p, %d B) "
	      "to %d shandle=%p\n", MPID_GM_rank, ptr, length, 
              dest, extra);
    }
  else
    {
      fprintf(gmpi.debug_output_filedesc, 
	      "[%d]: Send completion SMALL packet (buf %p, %d B) "
	      "to %d send_buf_ptr=%p\n", MPID_GM_rank, ptr, 
              length, dest, extra);
    }
  fflush(gmpi.debug_output_filedesc);
}

void gmpi_debug_gm_recv_print(char * expected, char * type, unsigned int from)
{ 
  fprintf(gmpi.debug_output_filedesc, "[%d]: Received %s %s from %d\n",
	  MPID_GM_rank, expected, type, from);
  fflush(gmpi.debug_output_filedesc);
}
