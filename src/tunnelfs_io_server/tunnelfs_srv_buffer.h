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
* File:         tunnelfs_srv_buffer.h                                     * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Manangement for cache buffers for iodata transfers        *
**************************************************************************/
#ifndef _TUNNELFS_SRV_BUFFER_H
#define _TUNNELFS_SRV_BUFFER_H

/**
 * Increase buffer capacity
 * @return number of buffer blocks after capacity change
 */
int tunnelfs_srv_buffer_increase();

/**
 * Decrease buffer capacity
 * @return number of buffer blocks after capacity change
 */
int tunnelfs_srv_buffer_decrease();

/**
 * Query capacity of buffer storage
 * @return number of total buffer blocks available
 */
int tunnelfs_srv_buffer_capacity();

/**
 * Get reference to a free buffer block
 * @param queue Identifier of buffer queue
 */
void *tunnelfs_srv_buffer_get_block(int queue);

/**
 * Release a buffer block from usage
 * @param buf Referenc to buffer block
 */
void tunnelfs_srv_buffer_release_block(void *buf);

/**
 * Flush buffer blocks
 * @param queue Identifier for queue
 */
void tunnelfs_srv_buffer_flush_queue(int queue);

void tunnelfs_srv_buffer_write(void *buf, int file_id, int client,
                               MPI_Offset offset, int count,
                               MPI_Datatype datatype);

void tunnelfs_srv_buffer_send(void *buf, int dest, int tag, MPI_Comm comm,
                              int count, MPI_Datatype datatype);
#endif
