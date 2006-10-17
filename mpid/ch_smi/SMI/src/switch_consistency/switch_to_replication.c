/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "switch_to_replication.h"
#include "synchronization/barrier.h"
#include "synchronization/mutex.h"
#include "regions/create_shreg.h"
#include "memory/shmem.h"
#include "switch_consistency/copy.h"
#include "loop_splitting/loop_split.h"
#include "utility/general.h"
#include "copy_gl_dist_local.h"



/********************************************************************************/
/*** Internal function: searchs in the 'mis.region' array for the region that ***/
/*** matches the specified id.                                                ***/
/********************************************************************************/
smi_error_t _smi_id_to_region_struct(int id, region_t** region)
 {
   REMDSECTION("_smi_id_to_region_struct");
   int i;

   DSECTENTRYPOINT;

   *region = NULL;

   SMI_LOCK(&_smi_mis_lock);
   for (i=0;i<_smi_mis.no_regions;i++)
      if (_smi_mis.region[i]->id == id)
	 *region = _smi_mis.region[i];
   SMI_UNLOCK(&_smi_mis_lock);

   ASSERT_R((*region!=NULL),"Region is not a valid parameter",SMI_ERR_PARAM);
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }


/********************************************************************************/
/*** ***/
/********************************************************************************/
smi_error_t SMI_Switch_to_replication(int id, int mode, int param1, int param2, int param3)
 {
   DSECTION("SMI_Switch_to_replication");
   region_t* region;
   int       counterpart_id;
   char*     counterpart_address;
   region_t* counterpart_region;
   int       i;
   int local_lower, local_upper, global_lower, global_upper, offset,size;
   device_t device;
   smi_error_t error;

   DSECTENTRYPOINT;

   if (_smi_nbr_procs == 1) {
     DSECTLEAVE
       return(SMI_SUCCESS);
   }

   /* look up the specified region into the 'mis.region' array; */
   /* this includes a parameter check at the same time          */
   
   error = _smi_id_to_region_struct(id, &region);
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);
  
   /* check, if already replicated */

   if (region->replication == true)
    {
      DNOTICE(" already in state 'replicated => return");
      DSECTLEAVE
	return(SMI_SUCCESS);
    }


   /* check, if already a counterpart region exists, if not, generate one */

   if (region->counterpart_id == -1) {
       smi_region_info_t region_info;

      /* I must generate one */
      if (mode & SMI_REP_ONE_PER_NODE)
	 device = DEV_SMP;
      else
	 device = DEV_LOCAL;
      
      region_info.size = region->size;
      region_info.owner = _smi_my_proc_rank;
      region_info.nbr_sgmts = region->no_segments;
      error = _smi_create_region_data_structure(SMI_SHM_UNDIVIDED, &region_info,
					   &counterpart_id, &counterpart_address,
					   &counterpart_region, device, SHREG_NONFIXED);
      ASSERT_R((error==SMI_SUCCESS),"_smi_create_region_data_structure failed",error);
    
      region->counterpart_id = counterpart_id;
      
      counterpart_region->counterpart_id = region->id;
      counterpart_region->replication = true;
    }
   else
    {
      /* I just have to look it up */
      counterpart_id = region->counterpart_id;
      error = _smi_id_to_region_struct(counterpart_id, &counterpart_region);
      ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);
     
      counterpart_address = counterpart_region->addresses[0];
    }


   /* synchronize before copying any data to make sure that each process */
   /* has reached this point                                             */
   SMI_Barrier();
   
 
   /* copy memory from shared region into each local segment */
   if (mode & SMI_REP_EVERYTHING)
    {
      /*_smi_copy_from_to(region->address, counterpart_region->address, region->size, mode);*/
	  _smi_copy_globally_distributed_to_local(region->id, region->counterpart_id);
    }
   else if (mode & SMI_REP_LOCAL_AND_BEYOND)
    {
      error = SMI_Loop_index_range(param2, &global_lower, &global_upper, SMI_LOOP_GET_GLOBAL);
      error = SMI_Loop_index_range(param2, &local_lower, &local_upper, SMI_LOOP_GET_LOCAL);
      offset = imax (0, param1 * (local_lower-param3-global_lower));
      size = param1 * (local_upper-local_lower+1
		       + imin (local_lower - global_lower, param3)
		       + imin (global_upper - local_upper, param3));
      _smi_copy_from_to((void*)((size_t)(region->addresses[0]) + (size_t)offset),
		   (void*)((size_t)(counterpart_region->addresses[0])
			   + (size_t)offset),
		   size, mode);
    }
   else if (mode & SMI_REP_NOTHING)
    {
      ;
    }
   else
    {
      ASSERT_R((0),"Invalid mode",SMI_ERR_PARAM);
    }
  

   /* release mapping of the shared region and the local memory segment */
   
   for(i=0;i<region->no_segments;i++)
    {
      error = _smi_unmap_shared_segment(region->seg[i]);
      ASSERT_R((error==SMI_SUCCESS),"Could not unmap segment",error);
    }
 
   error = _smi_unmap_shared_segment(counterpart_region->seg[0]);
   ASSERT_R((error==SMI_SUCCESS),"Could not unmap segment",error);

   /* exchange ids and addresses of both regions */
   
   counterpart_region->addresses[0]   = region->addresses[0];
   counterpart_region->id             = region->id;
   region->addresses[0]               = counterpart_address;
   region->id                         = counterpart_id;
   counterpart_region->counterpart_id = region->id;
   region->counterpart_id             = counterpart_region->id;

   counterpart_region->seg[0]->address = counterpart_region->addresses[0];
   region->seg[0]->address = region->addresses[0];
   for(i=1;i<region->no_segments;i++)
      region->seg[i]->address = (char*)((size_t)(region->seg[i-1]->address)
					+ (size_t)region->seg[i-1]->size);

   
   /* map both regions again */
   
   error = _smi_map_shared_segment(counterpart_region->seg[0]);
   ASSERT_R((error==SMI_SUCCESS),"Could not map segment",error);

   for(i=0;i<region->no_segments;i++)
    {
      error = _smi_map_shared_segment(region->seg[i]);
      ASSERT_R((error==SMI_SUCCESS),"Could not map segment",error);
    }
   
   /* synchonize once more to make sure that no processes uses this */
   /* data before everything has finished                           */
   SMI_Barrier();
  
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
  

