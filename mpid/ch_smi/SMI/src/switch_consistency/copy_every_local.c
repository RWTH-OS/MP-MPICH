/* $Id$ */

#define _WINNLS_

#include "env/general_definitions.h"
#include "regions/region_layout.h"
#include "proc_node_numbers/first_proc_on_node.h"


#if defined(WIN32) && defined(_M_IX86)
#define MOVE8(s, d) { __asm { mov eax, dword ptr [s] } \
					  __asm { fild qword ptr [eax] } \
					  __asm { mov eax, dword ptr [d] } \
					  __asm { fistp qword ptr [eax] } }
#else
#define MOVE8(s, d) { *d = *s; }
#endif

   

/*****************************************************************************/
/*** This function take a globally distributed and shared region and it's  ***/
/*** corresponding counterpart region. It fills the contents off all       ***/
/*** segments of the distributed shared segment, that are owned by the     ***/
/*** executing process, with the corresponding  data of the local          ***/
/*** counterpart region.                                                   ***/
/*****************************************************************************/
smi_error_t _smi_copy_every_local(int global_region_id, int local_region_id)
{  
  size_t        i,j;                       /* loop counters                      */
  smi_rlayout_t* distributed_layout;       /* description of the detailed memory */
  smi_rlayout_t* local_layout;             /* layout of the two regions          */
  volatile double* from_ptr;
  volatile double* to_ptr;
  size_t  it; 
  volatile double* s;
  volatile double* d;

  
  /* look-up the region layouts */
  SMI_Region_layout(global_region_id, &distributed_layout);
  SMI_Region_layout(local_region_id, &local_layout);
  
  /* check all segments, whether this process is the owner */
  for(i=0;i<distributed_layout->nsegments;i++)
    if (   distributed_layout->seg_machine[i] == _smi_my_machine_rank
	    && _smi_my_proc_rank == _smi_first_proc_on_node(_smi_my_machine_rank))
      {
	/* copy */
	
	to_ptr  = (double*)(distributed_layout->seg_adr[i]);
	from_ptr = (double*)((size_t)(local_layout->adr)
			+ ((size_t)to_ptr-(size_t)(distributed_layout->adr)));
	it = distributed_layout->seg_size[i] / sizeof(double);

	for(j=0;j<it;j++)
	{
		s = &(from_ptr[j]);
		d = &(to_ptr[j]);
		MOVE8(s, d);
	}
	   
      }

  
  free(distributed_layout);
  free(local_layout);
  
  return(SMI_SUCCESS);
}



