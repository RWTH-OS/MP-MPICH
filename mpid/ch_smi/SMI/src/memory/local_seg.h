/* $Id: local_seg.h,v 1.1 2004/03/19 22:14:17 joachim Exp $ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This module supplies all basic functions to create/map/unmap/distroy  ***/
/*** memory objects for pure use within the calling process. These can     ***/
/*** never become visible to the outside.                                  ***/ 
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#ifndef __LOCAL_SEG_H
#define __LOCAL_SEG_H


#include "env/general_definitions.h"


#ifdef __cplusplus
extern "C" {
#endif



/*****************************************************************************/
/*** This functions maps a local memory segment with the specified fd to   ***/
/*** the specified address within the calling processes address space.     ***/
/*** Same possible errors than the corresponding call to unmap a UNIX      ***/
/*** segment.                                                              ***/
/*****************************************************************************/
smi_error_t _smi_map_local_segment(shseg_t* shseg);

/*****************************************************************************/
/*** This functions releases the mapping of the specified memory segment.  ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process, but can be mapped later on again.                    ***/
/*** Same possible errors than the corresponding call to unmap a UNIX      ***/
/*** segment.                                                              ***/
/*****************************************************************************/
smi_error_t _smi_unmap_local_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Creates a Unix shared segments of the specified size on the specified ***/
/*** machine. The resulting identifier is passed back.                     ***/
/*** 2xxx errors possible, indicating that the SMI was not able to         ***/
/*** _smi_allocate a local memory segment. This can have several reasons: In    ***/
/*** case of Solaris, the requested segment was larger than the maximum    ***/
/*** allowed, thare are already to many, or SMI was just not able to find  ***/
/*** a free key.                                                           ***/
/*****************************************************************************/
smi_error_t _smi_create_local_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Removes a Unix shared segment with the specified identifier.          ***/
/*** Same errors as the function that removes a UNIX shared segment.       ***/
/*****************************************************************************/
smi_error_t _smi_remove_local_segment(shseg_t* shseg);

#ifdef __cplusplus
}
#endif
  

#endif



