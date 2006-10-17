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
* File:         tunnelfs_srv_ioreq.c                                      * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "mpi.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_srv.h"
#include "tunnelfs_log.h"

/**
 * Pending I/O information
 */
typedef struct
{
    int client;         /**< client rank */
    int file_id;        /**< Internal file id */
    int num_blocks;     /**< number of total io blocks */
    int count;          /**< Count parameter of io call */
    int ptr_type;       /**< Pointer type of io call */
    MPI_Offset offset;  /**< Offset of io call */
    void *iobuf;        /**< depricated: maybe used for buffering!! */
    int iobuf_offset;   /**< Internal offset to beginnng of buffer */
    int msg_id;         /**< Message id of initial client request */
}
tunnelfs_srv_io_pending_t;

static tunnelfs_srv_io_pending_t *pending_blocks = NULL;
static int _size = 0;
static int _last = -1;
static int chunk_size = 10;

static pthread_mutex_t tunnelfs_srv_ioreq_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Get index of a pending io request in internal list
 * @param client Rank of client
 * @return Index of request in internal list
 */
static int tunnelfs_srv_pending_io_find(int client);
static int tunnelfs_srv_pending_io_find(int client)
{
    int i = 0;

    /* INFO:
     * Mutex locking has to be handled outside of this function to ensure
     * atomic find and alter operations */

    assert(client >= 0);
    assert(pending_blocks != NULL);
    assert(_size > 0);
    assert(_last < _size);

    while ((i <= _last) && (i < _size)
           && (pending_blocks[i].client != client))
        i++;

    if (i > _last)
        return -1;
    else
        return i;
}

/**
 * Create a pending io request for a client
 * @param client Rank of client
 * @param file_id Internal file id
 * @param num_blocks Number of pending io blocks
 * @param count Count parameter of io call
 * @param ptr_type Pointer type of io call
 * @param offset Offset of io call
 * @param msg_id Message id of initial client request
 */
void tunnelfs_srv_pending_io_set(int client, int file_id, int num_blocks,
                                 int count, int ptr_type, MPI_Offset offset,
                                 int msg_id)
{
    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    assert(client >= 0);
    assert(file_id > 0);
    assert(num_blocks > 0);
    assert(count > 0);
    assert(offset >= 0);

    if ((pending_blocks == NULL) || (_size == 0) || (_size == _last + 1))
    {
        _size += chunk_size;
        pending_blocks = realloc(pending_blocks, _size *
                                 sizeof(tunnelfs_srv_io_pending_t));
        if (pending_blocks == NULL)
            ERR(TUNNELFS_ERR_ALLOC);
    }

    assert(pending_blocks != NULL);
    assert(_last + 1 < _size);
    assert(_size > 0);

    _last++;
    pending_blocks[_last].client = client;
    pending_blocks[_last].file_id = file_id;
    pending_blocks[_last].num_blocks = num_blocks;
    pending_blocks[_last].count = count;
    pending_blocks[_last].ptr_type = ptr_type;
    pending_blocks[_last].offset = offset;
    pending_blocks[_last].iobuf = NULL;
    pending_blocks[_last].iobuf_offset = 0;
    pending_blocks[_last].msg_id = msg_id;

    pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);
}

/**
 * Get pending io request parameters for a client
 * @param client Rank of client
 * @param file_id Internal file id
 * @param count Count parameter of io call
 * @param ptr_type Pointer type of io call
 * @param offset Offset of io call
 * @param iobuf_offset Offset to internal io buffer
 * @param msg_id Message id of initial client request
 */
void tunnelfs_srv_pending_io_get(int client, int *file_id, int *count,
                                 int *ptr_type, MPI_Offset *offset,
                                 int *iobuf_offset, int *msg_id)
{
    int idx = -1;

    assert(client >= 0);
    assert(pending_blocks != NULL);
    assert(_size > 0);
    assert(_last < _size);

    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    idx = tunnelfs_srv_pending_io_find(client);
    assert(idx != -1);

    *file_id = pending_blocks[idx].file_id;
    *count = pending_blocks[idx].count;
    *ptr_type = pending_blocks[idx].ptr_type;
    *offset = pending_blocks[idx].offset;
    *iobuf_offset = pending_blocks[idx].iobuf_offset;
    *msg_id = pending_blocks[idx].msg_id;

    pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);
}

/**
 * Increase internal buffer offset for pending io
 * @param client Rank of client
 * @param offset Increment to internal buffer
 */
void tunnelfs_srv_pending_io_inc_offset(int client, int offset)
{
    int idx = -1;

    assert(client >= 0);
    assert(offset > 0);
    assert(pending_blocks != NULL);
    assert(_last < _size);

    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    idx = tunnelfs_srv_pending_io_find(client);
    assert(idx != -1);

    pending_blocks[idx].offset += offset;
    pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);
}

/**
 * Decrease the number of pending io blocks
 * @param client Rank of client
 */
void tunnelfs_srv_pending_io_dec(int client)
{
    int idx = -1;

    assert(client >= 0);
    assert(pending_blocks != NULL);
    assert(_size > 0);
    assert(_last >= 0);
    assert(_last < _size);

    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    idx = tunnelfs_srv_pending_io_find(client);
    assert(idx != -1);

    pending_blocks[idx].num_blocks--;
    assert(pending_blocks[idx].num_blocks >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);
}

/**
 * Get number of pending io blocks for a client
 * @param client Rank of client
 * @return number of pending io blocks
 */
int tunnelfs_srv_pending_io_num_blocks(int client)
{
    int idx = -1;

    assert(client >= 0);
    assert(pending_blocks != NULL);
    assert(_size > 0);
    assert(_last >= 0);
    assert(_last < _size);

    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    idx = tunnelfs_srv_pending_io_find(client);
    assert(idx != -1);

    pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);

    return pending_blocks[idx].num_blocks;
}

/**
 * Check if pending io is completed
 * @param client Rank of client
 * @return Boolean value
 *      - 0: Still io blocks pending
 *      - 1: All pending io blocks have been received
 */
int tunnelfs_srv_pending_io_is_done(int client)
{
    int idx = -1;

    assert(client >= 0);
    assert(pending_blocks != NULL);
    assert(_size > 0);
    assert(_last >= 0);
    assert(_last < _size);

    pthread_mutex_lock(&tunnelfs_srv_ioreq_mutex);

    idx = tunnelfs_srv_pending_io_find(client);
    assert(idx != -1);

    if (pending_blocks[idx].num_blocks == 0)
    {
        LOG("Removing pending io for client %i", pending_blocks[idx].client);

        memcpy(&(pending_blocks[idx]),
               &(pending_blocks[_last]), sizeof(tunnelfs_srv_io_pending_t));
        _last--;

        pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);

        return 1;
    }
    else
    {
        pthread_mutex_unlock(&tunnelfs_srv_ioreq_mutex);
        return 0;
    }
}
