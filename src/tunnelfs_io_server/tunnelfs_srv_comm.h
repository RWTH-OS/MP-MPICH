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
* File:         tunnelfs_srv_comm.h                                       * 
* Description:  Handling of client specific communicator ids              * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_COMM_H
#define TUNNELFS_SRV_COMM_H

/**
 * Unique id for internal communicator referencing
 */
extern int tunnelfs_srv_comm_id;

/**
 * Provide a unique local communicator id for internal referencing
 */
#define NEXT_SRV_COMM_ID    ++tunnelfs_srv_comm_id
/**
 * Get current local communicator id for internal referencing
 */
#define CURR_SRV_COMM_ID      tunnelfs_srv_comm_id

/**
 * Create a communicator with associated mpi ranks
 * @param comm_id ID the created communicator shall use
 * @param size Number of ranks in the communicator
 * @param ranks List of ranks in TUNNELFS_COMM_WORLD
 */
void tunnelfs_srv_comm_register(int comm_id, int size, int *ranks);
/**
 * Destroy a communicator with associated mpi ranks
 * @param comm_id ID the created communicator shall use
 */
void tunnelfs_srv_comm_unregister(int comm_id);
/**
 * Retrieve ranks from internal communicator reference
 * @param comm_id ID of queried communicator
 * @param size Size of the return communicator list
 * @param ranks List of ranks in the communicator
 */
void tunnelfs_srv_comm_get_ranks(int comm_id, int *size, int **ranks);

#endif
