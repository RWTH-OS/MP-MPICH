/* $Id: ensure_consistency.c,v 1.1 2004/03/19 22:14:21 joachim Exp $ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "ensure_consistency.h"
#include "synchronization/barrier.h"
#include "copy_gl_dist_local.h"
#include "copy.h"
#include "combine_add.h"
#include "copy_every_local.h"
#include "switch_to_sharing.h"
#include "switch_to_replication.h"


  
/********************************************************************************/
/*** Not very efficiently implemented, but it works.                          ***/
/********************************************************************************/
smi_error_t SMI_Ensure_consistency(int id, int comb_mode, int comb_param1, int comb_param2)
 {
   DSECTION("SMI_Ensure_consistency");
   region_t* region;
   region_t* counterpart_region;
   smi_error_t error;
  					
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
   ASSERT_R((error==SMI_SUCCESS),"_smi_id_to_region_struct failed",error);

   /* Synchronize, because nobody is allowed to unmap a shared */
   /* region that might be still in use by another process     */
   SMI_Barrier();


   /****************/
   /* combine data */
   /****************/

   if (comb_mode & SMI_SHR_SINGLE_SOURCE)
    {
      if (   _smi_my_proc_rank == comb_param1
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

   _smi_copy_globally_distributed_to_local(region->counterpart_id, id);


   SMI_Barrier();

   DSECTLEAVE
     return(SMI_SUCCESS);
}

