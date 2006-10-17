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
* File:         tunnelfs_srv_comminfo.h                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  encapsulate file information                              *
*                                                                         *
**************************************************************************/
#ifndef _TUNNELFS_SRV_COMMINFO_H
#define _TUNNELFS_SRV_COMMINFO_H

/**
 * Interface structure to internal communicator representation
 */
typedef struct
{
    int comm_id;    /**< internal communicator id */
    int size;       /**< size of process list */
    int *ranks;     /**< list of mpi ranks */
} tunnelfs_comminfo_t;

/**
 * Fill comminfo structure with values of a given communicator
 * @param comm_id ID of queried communicator
 * @param ci Reference on comminfo structure
 */
void tunnelfs_srv_comminfo_create(int comm_id, tunnelfs_comminfo_t *ci);
/**
 * Pack comminfo structure into an mpi message buffer
 * @param buf MPI message buffer
 * @param size size of message buffer
 * @param position Reference of position indicator
 * @param ci Reference to comminfo structure
 */
void tunnelfs_srv_comminfo_pack(void *buf, int size, int *position,
                                tunnelfs_comminfo_t *ci);
/**
 * Unpack comminfo structure from an mpi message buffer
 * @param buf MPI message buffer
 * @param size size of message buffer
 * @param position Reference of position indicator
 * @param ci Reference to comminfo structure
 */
void tunnelfs_srv_comminfo_unpack(void *buf, int size, int *position,
                                  tunnelfs_comminfo_t *ci);
/**
 * Send communicator info to a specific io server
 * @param comm_id ID of queried communicator
 * @param rank Rank of IO Server
 */
void tunnelfs_srv_comminfo_send(int comm_id, int rank);

#endif
