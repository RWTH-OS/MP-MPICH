/* $Id: unix_shmem.h,v 1.1 2004/03/19 22:14:17 joachim Exp $ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This module supplies all basic functions to deal with Unix shared     ***/
/*** segments.                                                             ***/
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#ifndef __UNIX_SHMEM_H
#define __UNIX_SHMEM_H


#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/
/*** This functions maps a unix shared segment with the specified id to    ***/
/*** the specified address within the calling processes address space.     ***/
/*** This function can return SMI_ERR_BADADR, if the mapping succeeded,    ***/
/*** but the returned address is not the desired one (the reason for this  ***/
/*** is not known), or SMI_ERR_MAP_FAILED, the the mapping failed totally. ***/
/*** Reasons for this might be, that there is not enough memory, or what   ***/
/*** else.                                                                 ***/
/*****************************************************************************/
smi_error_t _smi_map_unix_shared_segment(shseg_t* shseg);  

/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process.                                                      ***/
/*** A 2xxx error might occure but is not probable.                        ***/
/*****************************************************************************/
smi_error_t _smi_unmap_unix_shared_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Creates a Unix shared segments of the specified size on the specified ***/
/*** machine. The resulting identifier is passed back. Therefore, it is at ***/
/*** least necessary that all processes on the specified machine execute   ***/
/*** this function at the same time. For them, this function results in a  ***/
/*** global synchronization.                                               ***/
/*** 1xx errors may occure as well as SMI_ERR_NOSEGMENT in the case that   ***/
/*** no more segment is possible in the system og the function was not able***/
/*** to _smi_allocate one for other reasons. One might be that the requested    ***/
/*** segment is too large in size.                                         ***/
/*****************************************************************************/
smi_error_t _smi_create_unix_shared_segment(shseg_t* shseg);

/*****************************************************************************/
/*** Removes a Unix shared segment with the specified identifier.          ***/
/*** A 2xxx error is possible but not probable.                            ***/
/*****************************************************************************/
smi_error_t _smi_remove_unix_shared_segment(shseg_t* shseg);


#ifdef __cplusplus
}
#endif
  

#endif



