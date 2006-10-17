/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "env/general_definitions.h"
#include "switch_to_replication_fast.h"
#include "switch_to_replication.h"
#include "synchronization/barrier.h"
#include "synchronization/mutex.h"
#include "regions/create_shreg.h"
#include "memory/shmem.h"
#include "switch_consistency/copy.h"
#include "loop_splitting/loop_split.h"
#include "utility/general.h"





/********************************************************************************/
/*** ***/
/********************************************************************************/
smi_error_t SMI_Switch_to_replication_fast(int id, int mode, int param1, int param2, int param3, void** start)
 {
   DSECTION("SMI_Switch_to_replication_fast");
   region_t* region;
   int       counterpart_id;
   char*     counterpart_address;
   region_t* counterpart_region;
   int local_lower, local_upper, global_lower, global_upper, offset,size;
   device_t device;
   smi_error_t error;

   DSECTENTRYPOINT;
   
#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */

   if (_smi_nbr_procs == 1) {
     DSECTLEAVE; return(SMI_SUCCESS);
   }

   /* look up the specified region into the 'mis.region' array; */
   /* this includes a parameter check at the same time          */
   
   error = _smi_id_to_region_struct(id, &region);
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);
   
   /* check, if already replicated */
   if (region->replication == true) {
       DNOTICE("already in state 'replicated => return");
       DSECTLEAVE; return(SMI_SUCCESS);
   }


   /* check, if already a counterpart region exists, if not, generate one */

   if (region->counterpart_id == -1) {
       smi_region_info_t region_info;
       /* I must generate one */
       device = (mode & SMI_REP_ONE_PER_NODE) ? DEV_SMP : DEV_LOCAL;
       
       region_info.size = region->size;
       region_info.owner = _smi_my_proc_rank;
       error = _smi_create_region_data_structure(SMI_SHM_UNDIVIDED, &region_info,
						 &counterpart_id, &counterpart_address,
						 &counterpart_region, device,0);
       ASSERT_R((error==SMI_SUCCESS),"_smi_create_region_data_structure failed",error);
       
       region->counterpart_id = counterpart_id;
       
       counterpart_region->counterpart_id = region->id;
       counterpart_region->replication = true;
   } else {
      /* I just have to look it up */
      counterpart_id = region->counterpart_id;
      error = _smi_id_to_region_struct(counterpart_id, &counterpart_region);
      ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct",error);
      counterpart_address = counterpart_region->addresses[0];
    }

#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T2,NULL);
   fprintf(stderr,"1111 %.3f\n",TIME());
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */

   /* copy memory from shared region into each local segment */
   if (mode & SMI_REP_EVERYTHING) {
      _smi_copy_from_to_double(region->addresses[0], counterpart_region->addresses[0], 
			       region->size, mode);
   } else 
       if (mode & SMI_REP_LOCAL_AND_BEYOND) {
	   error = SMI_Loop_index_range(param2, &global_lower, &global_upper, SMI_LOOP_GET_GLOBAL);
	   error = SMI_Loop_index_range(param2, &local_lower, &local_upper, SMI_LOOP_GET_LOCAL);
	   offset = imax (0, param1 * (local_lower-param3-global_lower));
	   size = param1 * (local_upper-local_lower+1
			    + imin (local_lower - global_lower, param3)
			    + imin (global_upper - local_upper, param3));
	   _smi_copy_from_to_double((void*)((size_t)(region->addresses[0]) + (size_t)offset),
				    (void*)((size_t)(counterpart_region->addresses[0]) + (size_t)offset),
				    size, mode);
       } else 
	   if (mode & SMI_REP_NOTHING) {
	       ;
	   } else {
	       ASSERT_R((0),"Invalid mode",SMI_ERR_PARAM);
	   }
   
#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T2,NULL);
   fprintf(stderr,"2222 %.3f\n",TIME());
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */


   /* exchange ids and addresses of both regions */

   counterpart_region->id             = region->id;
   region->id                         = counterpart_id;
   counterpart_region->counterpart_id = region->id;
   region->counterpart_id             = counterpart_region->id;
   *start = counterpart_region->addresses[0];

#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T2,NULL);
   fprintf(stderr,"6666 %.3f\n",TIME());
#endif /* _SMI_DO_TIME_TEST_ */
   
   SMI_Barrier();
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
  





