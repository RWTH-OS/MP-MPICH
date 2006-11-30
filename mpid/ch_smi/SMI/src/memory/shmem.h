/* $Id$ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This module supplies all basic functions to deal with (shared)        ***/
/*** segments, not depending on which device is used to implement them.    ***/
/*** Three types are supplied, which differ in the range of visibility.    ***/
/***                                                                       ***/
/***   LOCAL : just visible to the local process                           ***/
/***                                                                       ***/
/***   SMP   : visible to all processes within the SMP                     ***/
/***                                                                       ***/
/***   GLOBAL: visible to all processes                                    ***/
/***                                                                       ***/
/*** All types can be unmapped and mapped again, without loosing their     ***/
/*** contents.                                                             ***/
/*** This module requires, that the message-passing subsystem has already  ***/
/*** been set-up.                                                          ***/
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#ifndef __SHMEM_H
#define __SHMEM_H


#include "env/general_definitions.h"

extern _smi_FIFO_t _smi_busy_regions_fifo;
extern _smi_FIFO_t _smi_busy_regions_fifo_2;

#ifdef __cplusplus
extern "C" {
#endif



/*****************************************************************************/
/*** This functions maps the specified shared segment into the virtual     ***/
/*** address space of the calling process. 'address' must be set prior to  ***/
/*** this call in the 'shseg' structure. All other required information    ***/
/*** are already set by the preceeding '_smi_create_shared_segment' call. It is ***/
/*** not required, that processes, sharing a segment, perform a call to    ***/
/*** this function collectively.                                           ***/ 
/*****************************************************************************/
smi_error_t _smi_map_shared_segment(shseg_t* shseg);


  
/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process. It is not required, that processes, sharing a        ***/
/*** segment, perform a call to this function collectively.                ***/
/*****************************************************************************/
smi_error_t _smi_unmap_shared_segment(shseg_t* shseg);

  

/*****************************************************************************/
/*** This function creates a segment. The 'device' component of the        ***/
/*** structure 'shseg' must specify the desired range of visibility. All   ***/
/*** processes in the potential range of visibility must call this         ***/
/*** function collectively. Furthermore, the components 'machine', and     ***/
/*** 'size' must be specified. This function fills-in the components       ***/
/*** 'owner', 'id' and 'fd' (if required for the specified device).        ***/
/*****************************************************************************/
smi_error_t _smi_create_shared_segment(shseg_t* shseg);

  
  
/*****************************************************************************/
/*** Removes the specified shared segment. Before This function is called, ***/
/*** all processes in the range of visibility must haved called            ***/
/*** '_smi_unmap_shared_segment'. This function need not to be called from all  ***/
/*** participating processes collectively.                                 ***/
/*****************************************************************************/
smi_error_t _smi_remove_shared_segment(shseg_t* shseg);


  
/*****************************************************************************/
/*** This function performs all necessary initializations for the later    ***/
/*** usage of memory segments of various types. It must be called before   ***/
/*** any other function from this module.                                  ***/
/*****************************************************************************/
smi_error_t _smi_init_shared_segment_subsystem(void);

#ifdef __cplusplus
}
#endif


#endif





