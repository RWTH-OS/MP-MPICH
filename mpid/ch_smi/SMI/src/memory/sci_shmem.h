/* $Id: sci_shmem.h,v 1.1 2004/03/19 22:14:17 joachim Exp $ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This module supplies all basic functions to deal with SCI shared      ***/
/*** segments.                                                             ***/
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#ifndef __SCI_SHMEM_H
#define __SCI_SHMEM_H


#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* If a segment can not be removed because it's busy: nbr of retries and delay (in us) 
   between each retry. */
#define MAX_BUSY_RETRY    0
#define BUSY_RETRY_DELAY  0


/*****************************************************************************/
/*** Initialization of some internal data structures.                      ***/
/*** Error-Codes: Systemerrors			      			   ***/
/***              MPI Error	        				   ***/
/***              SMI_ERR_NODEVICE		      			   ***/
/*****************************************************************************/
smi_error_t _smi_init_sci_subsystem(void);
 
/*****************************************************************************/
/*** This functions maps a SCI shared segment with the specified id in the ***/
/*** calling processes address space.                                      ***/
/*** Error-Code: Systemerrors		        			   ***/
/***             SMI_ERR_PARAM			      			   ***/
/***             SMI_ERR_NODEVICE		      			   ***/
/*****************************************************************************/
smi_error_t _smi_map_sci_shared_segment(shseg_t* shseg);
 
/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process.                                                      ***/
/*** Error-Code: Systemerrors			       			   ***/
/***             SMI_ERR_PARAM			      			   ***/
/***             SMI_ERR_NODEVICE		      			   ***/
/*****************************************************************************/
smi_error_t _smi_unmap_sci_shared_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Creates a SCI shared segments of the specified size on the specified  ***/
/*** machine. The resulting identifier is passed back. Therefore, this     ***/
/*** function results in a global synchronization.                         ***/
/*** Error-Codes: Systemerrors                                             ***/
/*** Error-Code: Systemerrors			    			   ***/
/***             SMI_ERR_NODEVICE		     			   ***/
/***             MPI Error		       				   ***/
/*****************************************************************************/
smi_error_t _smi_create_sci_shared_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Removes a SCI shared segment with the specified identifier.           ***/
/*** Error-Code: Systemerrors			     			   ***/
/*****************************************************************************/
smi_error_t _smi_remove_sci_shared_segment(shseg_t* shseg);


#ifdef __cplusplus
}
#endif
  

#endif



