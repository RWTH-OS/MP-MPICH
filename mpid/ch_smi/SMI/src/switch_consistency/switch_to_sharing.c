/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "switch_to_sharing.h"
#include "init_switching.h"
#include "synchronization/barrier.h"
#include "loop_splitting/loop_split.h"
#include "utility/general.h"
#include "switch_to_replication.h"
#include "memory/shmem.h"
#include "copy.h"
#include "combine_add.h"
#include "copy_every_local.h"

  

  
/********************************************************************************/
/***                  ***/
/********************************************************************************/
smi_error_t _smi_combine_loop_splitting(int elem_size, int loop_id, region_t* local,
			       region_t* shared)
 {
   DSECTION("_smi_combine_loop_splitting");
   smi_error_t error;
   int from, to;    /* elements of the array that is contained in the shared    */
                    /* region with that the local process participates in the   */
                    /* shared region                                            */
   
   DSECTENTRYPOINT;

   error = SMI_Loop_index_range(loop_id, &from, &to, SMI_LOOP_GET_LOCAL);
   ASSERT_R((error==0),"SMI_Loop_index_range failed",error);
 
   memcpy((char*)((size_t)(shared->addresses[0])+(size_t)(from*elem_size)),
	  (char*)((size_t)(local->addresses[0])+(size_t)(from*elem_size)),
	  (to-from+1)*elem_size);
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }




  
  
/********************************************************************************/
/***                  ***/
/********************************************************************************/
smi_error_t SMI_Switch_to_sharing(int id, int comb_mode, int comb_param1, int comb_param2)
 {
   DSECTION("SMI_Switch_to_sharing");
   region_t* region;
   region_t* counterpart_region;
   int       i;
   char*     tmp_address;
   int       tmp_id;
   smi_error_t   error;

   DSECTENTRYPOINT;

   if (_smi_nbr_procs == 1) {
     DSECTLEAVE
       return(SMI_SUCCESS);
   }


   /* look up the specified region and it's counterpart into the 'mis.region' */
   /* array; this includes a parameter check at the same time                 */

   error = _smi_id_to_region_struct(id, &region);
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);

   /* check if replicated; if not, nothing has to be done */
   if (region->replication != true)
     {
       DSECTLEAVE
	 return(SMI_SUCCESS);
     }


   error = _smi_id_to_region_struct(region->counterpart_id, &counterpart_region);
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error)

   /* Synchronize, because nobody is allowed to unmap a shared */
   /* region that might be still in use by another process     */
   SMI_Barrier();




   /* release mapping of the shared region and the local memory segment */
   
   for(i=0;i<counterpart_region->no_segments;i++)
    {
      error = _smi_unmap_shared_segment(counterpart_region->seg[i]);
      ASSERT_R((error==SMI_SUCCESS),"_smi_unmap_shared_segment failed",error);
    }

   error = _smi_unmap_shared_segment(region->seg[0]);
   ASSERT_R((error==SMI_SUCCESS),"_smi_unmap_shared_segment failed", error);

   SMI_Barrier();
   
   /* exchange ids and addresses of both regions */

   tmp_address                 = region->addresses[0];
   tmp_id                      = region->id;
   region->addresses[0]        = counterpart_region->addresses[0];
   region->id                  = counterpart_region->id;
   counterpart_region->addresses[0] = tmp_address;
   counterpart_region->id      = tmp_id;
   region->counterpart_id      = counterpart_region->id;
   counterpart_region->counterpart_id = region->id;
      
   region->seg[0]->address = region->addresses[0];
   counterpart_region->seg[0]->address = counterpart_region->addresses[0];
   for(i=1;i<counterpart_region->no_segments;i++)
      counterpart_region->seg[i]->address =
	 (char*)((size_t)(counterpart_region->seg[i-1]->address)
	 + (size_t)counterpart_region->seg[i-1]->size);
   


   /* map both regions again */
   
   error = _smi_map_shared_segment(region->seg[0]);
   ASSERT_R((error==SMI_SUCCESS),"_smi_map_shared_segment failed",error);

   for(i=0;i<counterpart_region->no_segments;i++)
    {
      error = _smi_map_shared_segment(counterpart_region->seg[i]);
      ASSERT_R((error==SMI_SUCCESS),"_smi_map_shared_segment failed",error);
    }

   SMI_Barrier();
   


   /****************/
   /* combine data */
   /****************/

   if (comb_mode & SMI_SHR_SINGLE_SOURCE)
    {
      if (_smi_my_proc_rank == comb_param1
	  || ((comb_mode & SMI_REP_ONE_PER_NODE) && _smi_my_machine_rank == _smi_machine_rank[comb_param1]))
	 _smi_copy_from_to(region->addresses[0], counterpart_region->addresses[0], 
			   region->size, comb_mode);
    }
   else if (comb_mode & SMI_SHR_ADD)
      _smi_combine_add(counterpart_region, region, comb_param1, comb_param2,
		  comb_mode);
   else if (comb_mode & SMI_SHR_LOOP_SPLITTING)
      _smi_combine_loop_splitting(comb_param1, comb_param2, counterpart_region, region);
   else if (comb_mode & SMI_SHR_NOTHING)
      ;
   else if (comb_mode & SMI_SHR_EVERY_LOCAL)
     _smi_copy_every_local(counterpart_region->id, region->id); 
   else
    {
      ASSERT_R((0),"Invalid combine-mode",SMI_ERR_PARAM);
    }


   /* Synchronize, so that nobody uses a shared segment, before it */
   /* is mapped and filled with the desired contents.              */
   SMI_Barrier();

   DSECTLEAVE
     return(SMI_SUCCESS);
 }
  

  
