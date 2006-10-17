/* $Id$ */

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "unix_shmem.h"
#include "local_seg.h"
#include "sci_shmem.h"
#include "utility/statistics.h"
#include "message_passing/lowlevelmp.h"

/* global export */
_smi_FIFO_t _smi_busy_regions_fifo;
_smi_FIFO_t _smi_busy_regions_fifo_2;


/*****************************************************************************/
/*** This functions maps the specified shared segment into the virtual     ***/
/*** address space of the calling process. 'address' must be set prior to  ***/
/*** this call in the 'shseg' structure. All other required information    ***/
/*** are already set by the preceeding '_smi_create_shared_segment' call. It is ***/
/*** not required, that processes, sharing a segment, perform a call to    ***/
/*** this function collectively.                                           ***/ 
/*****************************************************************************/
smi_error_t _smi_map_shared_segment(shseg_t* shseg)
{
    int remote_rank, remote_error;
    boolean all_OK;
    smi_error_t error;
     
    switch(shseg->device) {
    case DEV_SMP: 
	error = _smi_map_unix_shared_segment(shseg);
	break;
    case DEV_LOCAL: 
	error = _smi_map_local_segment(shseg);
	break;
    case DEV_GLOBAL: 
	error = _smi_map_sci_shared_segment(shseg);
	break;
    default: 
	return(SMI_ERR_NODEVICE);
    }
    
    /* error checking for all non-asynchronous segments  */
    if (!(shseg->flags & SHREG_ASYNC)) {
	if (!(shseg->flags & SHREG_PT2PT)) {
	    all_OK = _smi_ll_all_true(error == SMI_SUCCESS);
	} else {
	    remote_rank = (shseg->owner == _smi_my_proc_rank) ? shseg->partner : shseg->owner;
	    SMI_Sendrecv(&error, &remote_error, sizeof(int), remote_rank);
	    all_OK = (remote_error == SMI_SUCCESS);
	}
	if (!all_OK && (error == SMI_SUCCESS)) {
	    _smi_unmap_shared_segment(shseg);
	    error = SMI_ERR_MAPFAILED;
	}
    }
    
    return(error);
}


/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process. It is not required, that processes, sharing a        ***/
/*** segment, perform a call to this function collectively.                ***/
/*****************************************************************************/
smi_error_t _smi_unmap_shared_segment(shseg_t* shseg)
{
   smi_error_t error;
   switch(shseg->device) {
   case DEV_SMP: 
       error = _smi_unmap_unix_shared_segment(shseg);
       break;
   case DEV_LOCAL: 
       error = _smi_unmap_local_segment(shseg);
       break;
   case DEV_GLOBAL: error = _smi_unmap_sci_shared_segment(shseg);
       break; 
   default: 
       error = SMI_ERR_NODEVICE;
   }
   
   return(error);
 }

  

/*****************************************************************************/
/*** This function creates a segment. The 'device' component of the        ***/
/*** structure 'shseg' must specify the desired range of visibility. All   ***/
/*** processes in the potential range of visibility must call this         ***/
/*** function collectively. Furthermore, the components 'machine', and     ***/
/*** 'size' must be specified. This function fills-in the components       ***/
/*** 'owner', 'id' and 'fd' (if required for the specified device).        ***/
/*****************************************************************************/

smi_error_t _smi_create_shared_segment(shseg_t* shseg)
{
    int remote_rank, remote_error;
    boolean all_OK;
    smi_error_t error;

    switch(shseg->device) {
    case DEV_SMP: 
	/* no delayed connection for local segments */
	shseg->flags &= ~SHREG_DELAYED;
	error = _smi_create_unix_shared_segment(shseg);
	break;
    case DEV_LOCAL: error = _smi_create_local_segment(shseg);
	break;
    case DEV_GLOBAL: 
	error = _smi_create_sci_shared_segment(shseg);
	break;    
    default: 
	return(SMI_ERR_NODEVICE);
    }
    
   /* error checking for all non-asynchronous segments  */
   if (!(shseg->flags & SHREG_ASYNC)) {
       if (!(shseg->flags & SHREG_PT2PT)) {
	   /* collective segment creation */
	   all_OK = _smi_ll_all_true(error == SMI_SUCCESS);
       } else {
	   /* pt2pt segment creation */	   
	   remote_rank = (shseg->owner == _smi_my_proc_rank) ? shseg->partner : shseg->owner;
	   SMI_Sendrecv(&error, &remote_error, sizeof(int), remote_rank);
	   all_OK = (remote_error == SMI_SUCCESS);
       }
       if (!all_OK && (error == SMI_SUCCESS)) {
	   _smi_remove_shared_segment(shseg);
	   error = SMI_ERR_NOSEGMENT;
       }
   }
   
   return(error);
}

  
  
/*****************************************************************************/
/*** Removes the specified shared segment. Before This function is called, ***/
/*** all processes in the range of visibility must haved called            ***/
/*** _smi_unmap_shared_segment'. This function need not to be called from  ***/
/*** all participating processes collectively.                             ***/
/*****************************************************************************/
smi_error_t _smi_remove_shared_segment(shseg_t* shseg)
{
    smi_error_t error;
    switch(shseg->device) {
    case DEV_SMP: 
	error = _smi_remove_unix_shared_segment(shseg);
	break;
    case DEV_LOCAL: 
	error = _smi_remove_local_segment(shseg);
	break;
    case DEV_GLOBAL: 
	error = _smi_remove_sci_shared_segment(shseg);
	break;  
    default: 
	error = SMI_ERR_NODEVICE;
    }
    
    return(error);
}


  
  
/*****************************************************************************/
/*** This function performs all necessary initializations for the later    ***/
/*** usage of memory segments of various types. It must be called before   ***/
/*** any other function from this module.                                  ***/
/*****************************************************************************/
smi_error_t _smi_init_shared_segment_subsystem()
{
    smi_error_t error = SMI_SUCCESS;
    /* the SCI network is only required and therefore needs only   */
    /* be initialized, if not all processes reside on the same SMP */
#ifndef NO_SISCI
    if (_smi_all_on_one == false) {
	error = _smi_init_sci_subsystem();
    }
#endif
    
    /* Create a fifo-queue for regions which could not be de-allocated because 
       they are still busy. */
    error = _smi_FIFO_init (&_smi_busy_regions_fifo, SMI_FIFO_THREADSAFE);
    error = _smi_FIFO_init (&_smi_busy_regions_fifo_2, SMI_FIFO_THREADSAFE);
    
    return (error);
}














