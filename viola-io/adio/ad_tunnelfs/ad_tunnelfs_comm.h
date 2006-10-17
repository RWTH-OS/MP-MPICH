/**************************************************************************
* TunnelFS                                                                * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_tunnelfs_comm.h                                        * 
* Description:                                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_COMM_H
#define AD_TUNNELFS_COMM_H

#define NEXT_COMM_ID ++tunnelfs_comm_id
#define CURR_COMM_ID   tunnelfs_comm_id

extern int tunnelfs_comm_id;

/**
 * Register MPI communicator under a certain comm id
 * @param comm_id internal communicator id
 * @param comm MPI communicator handle
 * @return if successful the communicator id is returned, else -1.
 */
int tunnelfs_comm_register(int comm_id, MPI_Comm comm);

/**
 * Delete internal communicator reference
 * @param comm_id internal communicator id
 */
void tunnelfs_comm_unregister(int comm_id);

/**
 * Get internal communicator id for a given MPI communicator handle
 * @param comm MPI communicator handle
 * @return internal communicator id
 */
int tunnelfs_comm_get_id(MPI_Comm comm);

/**
 * Get MPI communicator handle for internal communicator id
 * @param id internal communicator id
 * @return MPI communicator handle
 */
MPI_Comm tunnelfs_comm_get_comm(int id);

/**
 * Create a tunnelfs communicator setup message
 * @param id internal communicator id
 * @param buf Reference of message buffer
 * @param buf_size Reference size of message buffer
 * @param position Reference to position indicator
 * @param msg_id Message id to use
 */
void tunnelfs_comm_serialize_by_id(int id, void **buf, int *buf_size,
                                   int *position, int *msg_id);

/**
 * Create a tunnelfs communicator setup message
 * @param id MPI communicator handle
 * @param buf Reference of message buffer
 * @param buf_size Reference size of message buffer
 * @param position Reference to position indicator
 * @param msg_id Message id to use
 */
void tunnelfs_comm_serialize_by_comm(MPI_Comm comm, void **buf,
                                     int *buf_size, int *position,
                                     int *msg_id);

/** 
 * Check if a communicator has already been transfered
 * @param id internal communicator id
 * @return boolean value
 */
int tunnelfs_comm_is_transfered(int id);

/**
 * Mark a communicator as transfered
 * @param id internal communicator id
 */
void tunnelfs_comm_set_transfered(int id);

#endif
