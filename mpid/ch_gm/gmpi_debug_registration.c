/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <stdlib.h>
#include "gmpi.h"


static reg_segment * start_seg_list = NULL;

void gmpi_debug_registration_use_segment(gm_up_t start, unsigned int length)
{
  reg_segment * new_seg_ptr;
  reg_segment * tmp_seg_ptr;
  reg_segment * prev_seg_ptr;
  
  new_seg_ptr = NULL;
  new_seg_ptr = (reg_segment *)malloc(sizeof(reg_segment));
  gmpi_malloc_assert(new_seg_ptr, "gmpi_use_interval",
		     "malloc: registration debug segment");
  new_seg_ptr->prev = NULL;
  new_seg_ptr->next = NULL;
  new_seg_ptr->start = 0;
  new_seg_ptr->length = 0;
  
  GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION();
  tmp_seg_ptr = start_seg_list;
  prev_seg_ptr = NULL;
  while ((tmp_seg_ptr != NULL) && (tmp_seg_ptr->start <= start))
    {
      prev_seg_ptr = tmp_seg_ptr;
      tmp_seg_ptr = tmp_seg_ptr->next;
    }
  
  if (tmp_seg_ptr != NULL)
    {
      new_seg_ptr->prev = prev_seg_ptr;
      new_seg_ptr->next = tmp_seg_ptr;
      tmp_seg_ptr->prev = new_seg_ptr;
      if (prev_seg_ptr != NULL)
	{
	  prev_seg_ptr->next = new_seg_ptr;
	}
      else
 	{	
 	  start_seg_list = new_seg_ptr;
 	}
    }
  else
    {
      if (prev_seg_ptr != NULL)
	{
          /* end of the list */
	  prev_seg_ptr->next = new_seg_ptr;
	  new_seg_ptr->prev = prev_seg_ptr;
	}
      else
	{
          /* first item in the list */
	  start_seg_list = new_seg_ptr;
	  new_seg_ptr->prev = NULL;
	}
      new_seg_ptr->next = NULL;
    }
  
  new_seg_ptr->start = start;
  new_seg_ptr->length = length;
  GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION();
}


void gmpi_debug_registration_unuse_segment(gm_up_t start, unsigned int length)
{
  reg_segment * tmp_seg_ptr;
  
  GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION();
  tmp_seg_ptr = start_seg_list;
  while ((tmp_seg_ptr != NULL) 
	 && (tmp_seg_ptr->start != start)
	 && (tmp_seg_ptr->length != length))
    {
      tmp_seg_ptr = tmp_seg_ptr->next;
    }
  
  if (tmp_seg_ptr == NULL)
    {
      fprintf(stderr, "[%d]: UNUSE_interval without corresponding segment: "
	      "start = 0x%lx, length = %ld\n",
	      MPID_GM_rank, (unsigned long)start, length);
      gmpi_abort (0);
    }
  else
    {
      if (start_seg_list == tmp_seg_ptr)
	{
	  if (tmp_seg_ptr->prev != NULL)
	    {
	      fprintf(stderr, "[%d]: gmpi_debug_registration_unuse_segment: "
		      "inconsistency in chained list\n", MPID_GM_rank);
	      gmpi_abort (0);
	    }
	  start_seg_list = tmp_seg_ptr->next;
	}

      if (tmp_seg_ptr->prev != NULL)
	{
	  tmp_seg_ptr->prev->next = tmp_seg_ptr->next;
	}
      
      if (tmp_seg_ptr->next != NULL)
	{
	  tmp_seg_ptr->next->prev = tmp_seg_ptr->prev;
	}
      
      tmp_seg_ptr->prev = NULL;
      tmp_seg_ptr->next = NULL;
      tmp_seg_ptr->start = 0;
      tmp_seg_ptr->length = 0;
      free(tmp_seg_ptr);
    }
  GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION();
}


void gmpi_debug_registration_check_segment(gm_up_t start, unsigned int length,
					   char * context)
{
  reg_segment * tmp_seg_ptr;
  
  GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION();
  tmp_seg_ptr = start_seg_list;
  while ((tmp_seg_ptr != NULL) 
	 && (tmp_seg_ptr->start != start)
	 && (tmp_seg_ptr->length != length))
    {
      tmp_seg_ptr = tmp_seg_ptr->next;
    }
  GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION();
  
  if (tmp_seg_ptr == NULL)
    {
      fprintf(stderr,
	      "[%d]: %s on unused interval: start = 0x%lx, length = %ld\n",
	      MPID_GM_rank, context, (unsigned long)start, length);
      gmpi_abort (0);
    }
}


void gmpi_debug_registration_clear_segment(gm_up_t start, unsigned int length)
{
  reg_segment * tmp_seg_ptr;
  gm_up_t last, tmp_last;

  GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION();
  last = start + length - 1;
  tmp_seg_ptr = start_seg_list;
  while (tmp_seg_ptr != NULL)
    {
      tmp_last = tmp_seg_ptr->start + tmp_seg_ptr->length - 1;
      if (((tmp_seg_ptr->start >= start) && (tmp_seg_ptr->start <= last))
	  || ((tmp_last >= start) && (tmp_last <= last)))
	{
	  fprintf(stderr, "[%d]: gmpi_clear_interval (0x%lx, %ld) "
		  "on existing interval (0x%lx, %ld)\n",
		  MPID_GM_rank, start, length,
		  (unsigned long)(tmp_seg_ptr->start), tmp_seg_ptr->length);
	  gmpi_abort (0);
	}
      
      tmp_seg_ptr = tmp_seg_ptr->next;
    } 
  GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION();
}


void gmpi_debug_registration_clear_all_segments(void)
{
  GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION();
  if (start_seg_list != NULL)
    {
      fprintf(stderr, "[%d]: gmpi_clear_all_intervals() "
	      "with intervals in use\n", MPID_GM_rank);
      gmpi_abort (0);
    }
  GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION();
}


void gmpi_debug_registration_check_align(char * msg, gm_up_t start,
					 unsigned int length)
{
  if ((start & (GM_PAGE_LEN-1)) != 0)
    {
      fprintf(stderr, "[%d]: %s on an unaligned address (0x%lx)\n",
	      MPID_GM_rank, msg, (unsigned long)start);
      gmpi_abort (0);
    }
  
  if ((length & (GM_PAGE_LEN-1)) != 0)
    {
      fprintf(stderr, "[%d]: %s on an partial page (%d)\n",
	      MPID_GM_rank, msg, length);
      gmpi_abort (0);
    }
}

