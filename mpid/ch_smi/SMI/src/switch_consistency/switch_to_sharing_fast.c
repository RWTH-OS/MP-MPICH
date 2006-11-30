/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "env/general_definitions.h"
#include "switch_to_sharing_fast.h"
#include "switch_to_sharing.h"
#include "init_switching.h"
#include "synchronization/barrier.h"
#include "loop_splitting/loop_split.h"
#include "utility/general.h"
#include "switch_to_replication.h"
#include "memory/shmem.h"
#include "copy.h"
#include "combine_add.h" 

 


  
  
/********************************************************************************/
/***                  ***/
/********************************************************************************/
smi_error_t SMI_Switch_to_sharing_fast(int id, int comb_mode, int comb_param1, int comb_param2, void** start)
 {
   DSECTION("SMI_Switch_to_sharing_fast");
   region_t* region;
   region_t* counterpart_region;
   int       tmp_id;
   smi_error_t   error;

   DSECTENTRYPOINT;

   if (_smi_nbr_procs == 1) {
     DSECTLEAVE
       return(SMI_SUCCESS);
   }

#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */

   
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
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);

#ifdef _SMI_DO_TIME_TEST_
   ettimeofday(&T2,NULL);
   fprintf(stderr,"AAAA %.3f\n",TIME2());
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */
   
   
   /* exchange ids and addresses of both regions */
   
   tmp_id                      = region->id;
   region->id                  = counterpart_region->id;
   counterpart_region->id      = tmp_id;
   region->counterpart_id      = counterpart_region->id;
   counterpart_region->counterpart_id = region->id;
   *start = counterpart_region->addresses[0];

#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T2,NULL);
   fprintf(stderr,"DDDD %.3f\n",TIME2());
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */
   

   /****************/
   /* combine data */
   /****************/
   
   if (comb_mode & SMI_SHR_SINGLE_SOURCE)
    {
      if (   _smi_my_proc_rank == comb_param1
	  || ((comb_mode & SMI_REP_ONE_PER_NODE) && _smi_my_machine_rank == _smi_machine_rank[comb_param1]))
	 _smi_copy_from_to_double(region->addresses[0], counterpart_region->addresses[0], 
				  region->size, comb_mode);
    }
   else if (comb_mode & SMI_SHR_ADD)
      _smi_combine_add(counterpart_region, region, comb_param1, comb_param2,
		  comb_mode);
   else if (comb_mode & SMI_SHR_LOOP_SPLITTING)
      _smi_combine_loop_splitting(comb_param1, comb_param2, counterpart_region, region);
   else if (comb_mode & SMI_SHR_NOTHING)
      ;
   else
    {
      ASSERT_R((0),"Invalid combine-mode",SMI_ERR_PARAM);
    }

#ifdef _SMI_DO_TIME_TEST_
   gettimeofday(&T2,NULL);
   fprintf(stderr,"GGGG %.3f\n",TIME2());
   gettimeofday(&T1,NULL);
#endif /* _SMI_DO_TIME_TEST_ */

   /* Synchronize, so that nobody uses a shared segment, before it */
   /* is mapped and filled with the desired contents.              */
   SMI_Barrier();
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
  

  
