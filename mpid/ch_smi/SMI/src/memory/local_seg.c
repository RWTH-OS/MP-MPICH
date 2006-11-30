/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "local_seg.h"
#include "env/general_definitions.h"
#include "unix_shmem.h"
#include "utility/general.h"
#include "shseg_key.h"


static key_t SHMSEG_KEY = 1; /* This is a supposed key for a to be created       */
                             /* shared memory segment. The keys are tested       */
                             /* incrementally whether the creation of a segment  */
                             /* for this key is possible                         */


/*****************************************************************************/
/*** This functions maps a unix shared segment with the specified id to    ***/
/*** the specified address within the calling processes address space.     ***/
/*** Same possible errors than the corresponding call to unmap a UNIX      ***/
/*** segment.                                                              ***/
/*****************************************************************************/
smi_error_t _smi_map_local_segment(shseg_t* shseg)
 {
   return(_smi_map_unix_shared_segment(shseg));
 }


  
/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process.                                                      ***/
/*** Same possible errors than the corresponding call to unmap a UNIX      ***/
/*** segment.                                                              ***/  
/*****************************************************************************/
smi_error_t _smi_unmap_local_segment(shseg_t* shseg)
 {
   return(_smi_unmap_unix_shared_segment(shseg));
 }


  

/*****************************************************************************/
/*** Creates a Unix shared segments of the specified size on the specified ***/
/*** machine. The resulting identifier is passed back.                     ***/
/*** 2xxx errors possible, indicating that the SMI was not able to         ***/
/*** _smi_allocate a local memory segment. This can have several reasons: In    ***/
/*** case of Solaris, the requested segment was larger than the maximum    ***/
/*** allowed, thare are already to many, or SMI was just not able to find  ***/
/*** a free key.                                                           ***/
/*****************************************************************************/
smi_error_t _smi_create_local_segment(shseg_t* shseg)
 {
   DSECTION("_smi_create_local_segment");
   int npm,lr,key;

   DSECTENTRYPOINT;
   
   npm = _smi_no_processes_on_machine(_smi_my_machine_rank);
   lr = _smi_local_rank(_smi_my_proc_rank);
   shseg->owner = _smi_my_proc_rank;

   /* To do so: try several keys as long as the call */
   /* fails only because the key does already exist. */
   
   do
    {
      /* This is the place where we have to choose a key to try with it the */
      /* allocation of a unix shared segment. Because several processes do  */
      /* this, we have to ensure that no two processes on the same machine  */
      /* use the same key. The strategy is as follows: Each process         */
      /* requests its local rank on the machine on that it is executed,     */
      /* 'lr', and the total number of processes on this machine,'t'. Each  */
      /* process only uses key's of the form: '(t+1)*k+lr', which are then  */
      /* distinct among all processes on the same machine.                  */

      SHMSEG_KEY++;
      key = (npm+1)*SHMSEG_KEY + lr;
	  key = _smi_modify_key_ever(key);
      shseg->id = rs_shmget(key, shseg->size, IPC_CREAT|IPC_EXCL|0600);
    } while (shseg->id==-1 && key<9999);
   
   ASSERT_R((shseg->id != -1),"Could not get shared memory",2000+errno);
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
   
   


  
   
  
/*****************************************************************************/
/*** Removes a Unix shared segment with the specified identifier.          ***/
/*** Same errors as the function that removes a UNIX shared segment.       ***/
/*****************************************************************************/
smi_error_t _smi_remove_local_segment(shseg_t* shseg)
 {
   return(_smi_remove_unix_shared_segment(shseg));
 }








