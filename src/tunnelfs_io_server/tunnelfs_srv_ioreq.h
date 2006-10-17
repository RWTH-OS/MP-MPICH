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
* File:         tunnelfs_srv_ioreq.h                                      * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_IOREQ_H
#define TUNNELFS_SRV_IOREQ_H

/**
 * Create a pending io request for a client
 * @param client Rank of client
 * @param file_id Internal file id
 * @param num_blocks Number of pending io blocks
 * @param count Count parameter of io call
 * @param ptr_type Pointer type of io call
 * @param offset Offset of io call
 * @param iobuf_offset Offset to internal io buffer
 * @param msg_id Message id of initial client request
 */
void tunnelfs_srv_pending_io_set(int client, int file_id, int num_blocks,
                                 int count, int ptr_type, MPI_Offset offset,
                                 int msg_id);

/**
 * Get pending io request parameters for a client
 * @param client Rank of client
 * @param file_id Internal file id
 * @param num_blocks Number of pending io blocks
 * @param count Count parameter of io call
 * @param ptr_type Pointer type of io call
 * @param offset Offset of io call
 * @param iobuf_offset Offset to internal io buffer
 * @param msg_id Message id of initial client request
 */
void tunnelfs_srv_pending_io_get(int client, int *file_id, int *count,
                                 int *ptr_type, MPI_Offset *offset,
                                 int *iobuf_offset, int *msg_id);

/**
 * Increase internal buffer offset for pending io
 * @param client Rank of client
 * @param offset Increment to internal buffer
 */
void tunnelfs_srv_pending_io_inc_offset(int client, int offset);

/**
 * Decrease the number of pending io blocks
 * @param client Rank of client
 */
void tunnelfs_srv_pending_io_dec(int client);

/**
 * Get number of pending io blocks for a client
 * @param client Rank of client
 * @return number of pending io blocks
 */
int tunnelfs_srv_pending_io_num_blocks(int client);

/**
 * Check if pending io is completed
 * @param client Rank of client
 * @return Boolean value
 *      - 0: Still io blocks pending
 *      - 1: All pending io blocks have been received
 */
int tunnelfs_srv_pending_io_is_done(int client);

#endif
