/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "env/general_definitions.h"
#include "regions/region_layout.h"
#include "synchronization/barrier.h"
#include "dyn_mem/dyn_mem.h"
#include "proc_node_numbers/first_proc_on_node.h" 
#include "synchronization/progress.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


#if defined(WIN32) && defined(_M_IX86)
#define MOVE8(s, d) { __asm { mov eax, dword ptr [s] } \
					  __asm { fild qword ptr [eax] } \
					  __asm { mov eax, dword ptr [d] } \
					  __asm { fistp qword ptr [eax] } }
#else
#define MOVE8(s, d) { *d = *s; }
#endif




#define SMI_COPY_BUFFER_SIZE 65536 /* This constant determines the chunksize with */
                                   /* that data is copies. As larger it is, as    */
                                   /* better the performance will be. But also    */
                                   /* the usage of rare internal shared memory    */
                                   /* grows.                                      */ 


static double**   buffer;
static boolean buffer__smi_allocated = false;
static int progress_counter;



/*****************************************************************************/
/*** This function copies a globally shared but physically distributed     ***/
/*** region to a local region within each process. Therefore a set of      ***/
/*** globally accessable buffers are used, one local to each machine.      ***/
/*****************************************************************************/
smi_error_t _smi_copy_globally_distributed_to_local(int global_region_id,
					   int local_region_id)
{  
  DSECTION("_smi_copy_globally_distributed_to_local");
  smi_error_t error;
  size_t       i, j, k;							   /* loop counters                      */
  smi_rlayout_t* distributed_layout;               /* description of the detailed memory */
  smi_rlayout_t* local_layout;                     /* layout of the two regions          */
  void*      own_global_segment_start = NULL;  /* start address and size of a segment*/ 
  size_t     own_global_segment_size = 0;      /* and it's size of the entire copy   */
  void*      other_global_segment_start = NULL;/* proceedure; once for the own source*/
  size_t     other_global_segment_size = 0;    /* segment, once for the remote       */
                                               /* source segment                     */
  double*       ptr;                              /* tmp var                            */
  int           other_machine_rank;
  size_t        no_iterations;                 /* specifies how many                 */
                                               /* SMI_COPY_BUFFER_SIZE-byte blocks   */
                                               /* the largest segment of the distrib.*/
                                               /* region contains                    */
  int tmp_machine, tmp_proc;
  size_t act_inner_iterations;
  size_t remaining_bytes; 
  size_t tmp_it;
 
  double* tmp_ptr;
  size_t no_inner_iterations = SMI_COPY_BUFFER_SIZE/sizeof(double);
  double* s;
  double* d;

  DSECTENTRYPOINT;
  
  /* _smi_allocate buffer space, if not already done */
  if (buffer__smi_allocated == false)
    {
      buffer__smi_allocated = true;
      
      error = SMI_Init_PC(&progress_counter);
      ASSERT_R((error==SMI_SUCCESS),"Could not init progress-counter",error);
      
      ALLOCATE( buffer,  double**, _smi_nbr_machines * sizeof(double*) );
      for(i=0;i<(size_t)_smi_nbr_machines;i++)
	{
	  tmp_proc = _smi_first_proc_on_node((int) i);
	  error = SMI_Cmalloc(SMI_COPY_BUFFER_SIZE, tmp_proc|INTERNAL, 
			      (char**)(&(buffer[i])));
	  ASSERT_R((error==SMI_SUCCESS),"Could not allocate memory",error);
	}
    }
  
  
  /* look-up the region layouts */
  SMI_Region_layout(global_region_id, &distributed_layout);
  SMI_Region_layout(local_region_id, &local_layout);
   
  /* determine, how many SMI_COPY_BUFFER_SIZE-byte blocks a continuous   */
  /* piece of the region is made at maximum of, that is all located on   */
  /* the same owner this is the number of iterations that have to be     */
  /* performed per segment                                               */
  
  no_iterations = 0;
  tmp_it = 0;
  tmp_machine = -1;
  for(i=0; i<distributed_layout->nsegments; i++)
    {
      if (distributed_layout->seg_machine[i] != tmp_machine)
	{
	  if (tmp_it>no_iterations) no_iterations = tmp_it;
	  tmp_it      =	0; 
	  tmp_machine = distributed_layout->seg_machine[i];
	}
      tmp_it +=	distributed_layout->seg_size[i] / SMI_COPY_BUFFER_SIZE;
      if (distributed_layout->seg_size[i] % SMI_COPY_BUFFER_SIZE != 0)
	tmp_it++;
    }
  if (tmp_it > no_iterations) no_iterations = tmp_it;
  
  
  /* search for local segment of the distributed region;            */
  /* only the first process on each node is responsible for sending */
  /* to indicate this, all other processes than the first on a      */
  /* machine get a zero length.                                     */
  
  for(i=0;i<distributed_layout->nsegments;i++)
    if (distributed_layout->seg_machine[i] == _smi_my_machine_rank)
      {
	if (own_global_segment_start == NULL)
	  {
	    own_global_segment_start = distributed_layout->seg_adr[i];	
	    own_global_segment_size  = distributed_layout->seg_size[i];
	  }
	else
	  own_global_segment_size += distributed_layout->seg_size[i];	
      }
  if (_smi_my_proc_rank != _smi_first_proc_on_node(_smi_my_machine_rank))
    own_global_segment_size = 0;

  
  
  for (i=0;i<(size_t)_smi_nbr_machines;i++)
    {
      /* each first process of one node sends to the process on node */
      /* '(my_machine_rank+i)%no_machins' all it's local share of    */
      /* the globally distributed region via the destination's       */
      /* machine's buffer.                                           */
      
      
      /* first, each process informs itself about the starting      */
      /* address and the size of the segment that it receives in    */
      /* this iteration                                             */
      
      other_global_segment_start = NULL;
      other_global_segment_size  = 0;    
      other_machine_rank         = (int)(_smi_my_machine_rank-i+_smi_nbr_machines)%_smi_nbr_machines;
      
      for(j=0;j<distributed_layout->nsegments;j++)	  
	if (distributed_layout->seg_machine[j] == other_machine_rank)
	  {
	    if (other_global_segment_start == NULL)
	      {		
		other_global_segment_start = distributed_layout->seg_adr[j];
		other_global_segment_size  = distributed_layout->seg_size[j];
	      }
	    else
	      other_global_segment_size += distributed_layout->seg_size[j];
	  }
     
      /* send own, receive remote segment */
      for(j=0;j<no_iterations;j++)
	{
	  /* send the j-th block, we do this with operations that */
	  /* copy the amount of one 'double' per operation        */
	  /* in the case that the sender is executing on the      */
	  /* receiving machine itself, this can be omitted        */

	  if (j*SMI_COPY_BUFFER_SIZE < own_global_segment_size && i!=0)
	    {
	      remaining_bytes = own_global_segment_size - j*SMI_COPY_BUFFER_SIZE;
	      act_inner_iterations = remaining_bytes / sizeof(double);
	      if (act_inner_iterations > no_inner_iterations)
		act_inner_iterations = no_inner_iterations;
	      
	      ptr = (double*)((size_t)own_global_segment_start 
			      + (size_t)(j*SMI_COPY_BUFFER_SIZE));
	      tmp_ptr = buffer[(_smi_my_machine_rank+i)%_smi_nbr_machines];
	      
	      for(k=0;k<act_inner_iterations;k++)
		  {
			  s = &(ptr[k]);
			  d = &(tmp_ptr[k]);
			  MOVE8(s, d);
		  }
	    }
	  
	  
	  SMI_Increment_PC(progress_counter,1);
	  SMI_Wait_collective_PC(progress_counter, SMI_OWNPC);
	  
	  /* receive the j-th block */
	  
	  if (j*SMI_COPY_BUFFER_SIZE < other_global_segment_size)
	    {
	      remaining_bytes = other_global_segment_size - j*SMI_COPY_BUFFER_SIZE;
	      act_inner_iterations = remaining_bytes / sizeof(double);
	      if (act_inner_iterations > no_inner_iterations)
		act_inner_iterations = no_inner_iterations;
	      
	      ptr = (double*)((size_t)(local_layout->adr)
			      + ((size_t)other_global_segment_start
			      - (size_t)(distributed_layout->adr))
			      + (size_t)(j*SMI_COPY_BUFFER_SIZE));
	      if (i==0)
		tmp_ptr = (double*)( (size_t)own_global_segment_start 
				      + (size_t)(j*SMI_COPY_BUFFER_SIZE));
	      else
		tmp_ptr = buffer[_smi_my_machine_rank];
	 
	      for(k=0;k<act_inner_iterations;k++)
		ptr[k] = tmp_ptr[k];
	    }
	  
	  SMI_Increment_PC(progress_counter,1);
	  SMI_Wait_collective_PC(progress_counter, SMI_OWNPC);
	}
    }
   
  
  free(distributed_layout);
  free(local_layout);
  
  DSECTLEAVE
    return(SMI_SUCCESS);
}



