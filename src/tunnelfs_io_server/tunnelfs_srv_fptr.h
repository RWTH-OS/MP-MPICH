/**************************************************************************
* TunnelFS Server                                                         * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         tunnelfs_srv_fptr.h                                       * 
* Description:  Handling for shared and individual file pointers          * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_FPTR_H
#define TUNNELFS_SRV_FPTR_H

#include "adio.h"

ADIO_Offset tunnelfs_srv_fptr_get_shared(int file_id);

/**
 * Set the value of a shared filepointer
 * @param file_id Internal file id
 * @param offset Current value of shared filepointer
 */
void tunnelfs_srv_fptr_set_shared(int file_id, ADIO_Offset offset);

/**
 * Create a shared filepointer for a file id
 * @param file_id Internal file id
 */
void tunnelfs_srv_fptr_register(int file_id);

/**
 * Delete a shared filepoint for a file id
 * @param file_id Internal file id
 */
void tunnelfs_srv_fptr_unregister(int file_id);

/**
 * Get current shared filepointer value and increase it atomically for
 * subsequent queries
 * @param file_id Internal file id
 * @param inc Increment for offset
 * @return Offset before the increment
 */
ADIO_Offset tunnelfs_srv_fptr_getset_shared_atomic(int file_id,
                                                   ADIO_Offset inc);
#endif
