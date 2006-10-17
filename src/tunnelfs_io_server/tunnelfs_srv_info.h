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
* File:         tunnelfs_srv_info.h                                       * 
* Description:  Info handling                                             * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_INFO_H
#define TUNNELFS_SRV_INFO_H

/**
 * Create an MPI_Info object from a message buffer
 * @param buffer message buffer
 * @param buffer_size size of message buffer
 * @param position Reference to position indicator
 * @param count Number of info entries
 * @param info Reference to MPI_Info handle
 */
void tunnelfs_srv_info_create(void *buffer, int buffer_size, int *position,
                              int count, MPI_Info *info);

/**
 * Pack MPI_Info object into a message buffer
 * @param buffer message buffer
 * @param buffer_size size of message buffer
 * @param position Reference to position indicator
 * @param count Number of info entries
 * @param info MPI_Info handle
 */
void tunnelfs_srv_info_pack(void *buffer, int buffer_size, int *position,
                            int count, MPI_Info info);

/**
 * Check if a key is set to a specific value
 * @param info MPI_Info handle
 * @param key String identifier for key
 * @param val String identifier for value
 * @return Boolean value
 *      - 1: Key is set to val in info object
 *      - 0: Key is not set to val in info object
 */
int tunnelfs_srv_info_is_set(MPI_Info info, char *key, char *val);

/**
 * Get value of a specific key of the info object
 * @param info MPI_Info handle
 * @param key String identifier for key
 * @return String identifier for value
 */
char *tunnelfs_srv_info_get(MPI_Info info, char *key);

/**
 * Evaluate a given info object for relevant settings
 * @param info MPI_Info handle
 * @param file_id Internal file id
 */
void tunnelfs_srv_info_eval(MPI_Info info, int file_id);

#endif
