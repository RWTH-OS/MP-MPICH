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
* File:         tunnelfs_srv_coll.h                                       * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Manangement for collective operations, that cannot be     *
*               handled by a single client                                *
**************************************************************************/
#ifndef TUNNELFS_SRV_COLL_H
#define TUNNELFS_SRV_COLL_H

/**
 * Return current value of magic cookie
 */
#define TUNNELFS_CURR_COLL_ID tunnelfs_srv_coll_id;
/**
 * Provide a new value for magic cookie
 */
#define TUNNELFS_NEXT_COLL_ID ++tunnelfs_srv_coll_id;

/**
 * The value of the current magic cookie for pseudo collective operations
 */
extern int tunnelfs_srv_coll_id;

/**
 * Start tracking of pseudo collective operation
 * @param file_id File ID to associate with file operation
 * @param magic_cookie Unique identifier for this operation
 * @param n Number of processes participating (implizitely, the number of
 *          answers expected before completion)
 * @param clnt_msg_id Message ID of client request for reply message
 */
void tunnelfs_srv_collop_start(int file_id, int magic_cookie, int n,
                               int clnt_msg_id);
/**
 * Decrease the number of pending answers for the operation
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
void tunnelfs_srv_collop_done(int file_id, int magic_cookie);
/**
 * Delete pseudo collective operation from buffers if finished
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
void tunnelfs_srv_collop_free(int file_id, int magic_cookie);
/**
 * Get the number of pending replies for an operation
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 * @return Number of pending replies
 */
int tunnelfs_srv_collop_num_pending(int file_id, int magic_cookie);
/**
 * Get message id of initial client request
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
int tunnelfs_srv_collop_get_clnt_msg_id(int file_id, int magic_cookie);
#endif
