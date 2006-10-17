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
* File:         tunnelfs_srv_buffer.c                                     * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Functions for managing the buffer buffers for client io    *
*               data                                                      *
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <assert.h>
#include <pthread.h>
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"
#include "tunnelfs_srv_fileview.h"

#define BUFFER_SIZE 20

typedef enum
{ net, io } tunnelfs_btype_t;

typedef struct
{
    int queue;
    tunnelfs_btype_t type;
    union
    {
        struct
        {
            int file_id;
            int client;
            MPI_Offset offset;
        } io;
        struct
        {
            int dest;
            int tag;
            MPI_Comm comm;
        } net;
    } param;
    int count;
    MPI_Datatype datatype;
    int reserved;
    int used;
    int bytes;
    void *data;
} tunnelfs_buffer_block_t;

static tunnelfs_buffer_block_t *buffer = NULL;
static int buffer_capacity = 0;
static int buffer_initialized = 0;

static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t buffer_reservable = PTHREAD_COND_INITIALIZER;

static void tunnelfs_srv_buffer_init();
static int tunnelfs_srv_buffer_full();
static void tunnelfs_srv_buffer_flush_all();
static void tunnelfs_srv_buffer_flush_entry(int index);

static void tunnelfs_srv_buffer_init()
{
    int i;

    /* INFO:
     * Locking has to be done outside of this function!
     */

    /* allocate overall buffer list */
    ALLOC(buffer, BUFFER_SIZE * sizeof(tunnelfs_buffer_block_t));

    for (i = 0; i < BUFFER_SIZE; i++)
    {
        /* buffer needs manual setting to NULL */
        buffer[i].data = NULL;
        /* allocate data buffer for block i */
        ALLOC(buffer[i].data, TUNNELFS_MAX_MSG_SIZE);
        buffer[i].queue = -1;
        buffer[i].reserved = 0;
        buffer[i].used = 0;
    }
    buffer_capacity = BUFFER_SIZE;
    buffer_initialized = 1;
}

/**
 * Increase buffer capacity
 * @return number of buffer blocks after capacity change
 */
int tunnelfs_srv_buffer_increase()
{
    int tmp;

    pthread_mutex_lock(&buffer_mutex);
    tmp = buffer_capacity;
    pthread_mutex_unlock(&buffer_mutex);

    return tmp;
}

/**
 * Decrease buffer capacity
 * @return number of buffer blocks after capacity change
 */
int tunnelfs_srv_buffer_decrease()
{
    int tmp;

    pthread_mutex_lock(&buffer_mutex);
    tmp = buffer_capacity;
    pthread_mutex_unlock(&buffer_mutex);

    return tmp;
}

/**
 * Query capacity of buffer storage
 * @return number of total buffer blocks available
 */
int tunnelfs_srv_buffer_capacity()
{
    int tmp;

    pthread_mutex_lock(&buffer_mutex);
    tmp = buffer_capacity;
    pthread_mutex_unlock(&buffer_mutex);

    return tmp;
}

/**
 * Get reference to a free buffer block
 * @param queue Identifier of buffer queue
 * @return Reference of valid data buffer of NULL
 */
void *tunnelfs_srv_buffer_get_block(int queue)
{
    int idx = 0;
    void *ret = NULL;

    pthread_mutex_lock(&buffer_mutex);
    if (!buffer_initialized)
        tunnelfs_srv_buffer_init();

    if (tunnelfs_srv_buffer_full())
        tunnelfs_srv_buffer_flush_all();

    do
    {
        idx = 0;
        while ((idx < buffer_capacity) && (buffer[idx].reserved == 1))
            idx++;

        if (idx >= buffer_capacity)
        {
            LOG("Waiting for reservable buffer");
            pthread_cond_wait(&buffer_reservable, &buffer_mutex);
        }
    }
    while (idx >= buffer_capacity);

    assert(idx < buffer_capacity);

    buffer[idx].queue = queue;
    buffer[idx].reserved = 1;
    ret = buffer[idx].data;

    pthread_mutex_unlock(&buffer_mutex);

    LOG("Buffer address: %x", ret);

    return ret;
}

/**
 * Release a buffer block from usage
 * @param buf Referenc to buffer block
 */
void tunnelfs_srv_buffer_release_block(void *buf)
{
    int idx = 0;

    pthread_mutex_lock(&buffer_mutex);

    assert(buffer_initialized);

    while ((idx < buffer_capacity) && (buffer[idx].data != buf))
        idx++;

    assert(idx < buffer_capacity);

    LOG("Releasing block %i with buffer %x", idx, buffer[idx].data);

    buffer[idx].reserved = 0;
    buffer[idx].used = 0;
    pthread_cond_signal(&buffer_reservable);

    pthread_mutex_unlock(&buffer_mutex);
}

/**
 * Check if buffer space is available
 * @return Number of used buffer blocks
 */
static int tunnelfs_srv_buffer_full()
{
    int idx;
    int num_free = 0;

    for (idx = 0; idx < buffer_capacity; idx++)
        if (buffer[idx].reserved == 0)
            num_free++;

    return (buffer_capacity - num_free);
}

/**
 * Flush complete buffer
 */
static void tunnelfs_srv_buffer_flush_all()
{
    int idx;

    for (idx = 0; idx < buffer_capacity; idx++)
    {
        if (buffer[idx].used)
            tunnelfs_srv_buffer_flush_entry(idx);
    }
}

/**
 * Flush buffer blocks of a queue
 * @param queue Identifier for queue
 */
void tunnelfs_srv_buffer_flush_queue(int queue)
{
    int idx = -1;

    pthread_mutex_lock(&buffer_mutex);

    while (++idx < buffer_capacity)
    {
        if ((buffer[idx].queue == queue) && (buffer[idx].used))
            tunnelfs_srv_buffer_flush_entry(idx);
    }

    pthread_mutex_unlock(&buffer_mutex);
}

static void tunnelfs_srv_buffer_flush_entry(int idx)
{
    /* INFO:
     * Locking is done outside of this function
     */
    assert(buffer[idx].data != NULL);
    assert(buffer[idx].reserved == 1);
    assert(buffer[idx].used == 1);

    LOG("Flushing block %i with buffer %x", idx, buffer[idx].data);

    switch (buffer[idx].type)
    {
    case net:
        {
            LOG("Sending block to rank %i", buffer[idx].param.net.dest);
            ptMPI_Send(buffer[idx].data, buffer[idx].count,
                       buffer[idx].datatype, buffer[idx].param.net.dest,
                       buffer[idx].param.net.tag, buffer[idx].param.net.comm);
            break;
        }
    case io:
        {
            MPI_Offset disp;
            MPI_Datatype etype;
            MPI_Datatype ftype;
            MPI_Status status;
            MPI_File *fh;

            fh = tunnelfs_srv_file_get_handle(buffer[idx].param.io.file_id);
            assert(fh != NULL);

            LOCK_MPI();
            tunnelfs_srv_fileview_get(buffer[idx].param.io.client,
                                      buffer[idx].param.io.file_id,
                                      &disp, &etype, &ftype);

            MPI_File_set_view(*fh, disp, etype, ftype, "native",
                              MPI_INFO_NULL);
            LOG("Writing buffer %x to offset %Li", buffer[idx].data,
                buffer[idx].param.io.offset);
            MPI_File_write_at(*fh, buffer[idx].param.io.offset,
                              buffer[idx].data, buffer[idx].count,
                              buffer[idx].datatype, &status);
            UNLOCK_MPI();
            break;
        }
    default:
        {
            LOG("UNKNOWN MESSAGE TYPE %i", buffer[idx].type);
            exit(-1);
            break;
        }
    }
    LOG("Releasing block %i", idx);
    buffer[idx].reserved = 0;
    buffer[idx].used = 0;
    pthread_cond_signal(&buffer_reservable);
}

void tunnelfs_srv_buffer_write(void *buf, int file_id, int client,
                               MPI_Offset offset, int count,
                               MPI_Datatype datatype)
{
    /* buf already contains the data! */
    int idx = 0;

    pthread_mutex_lock(&buffer_mutex);
    /* 
     * buf already has to contain the data, thus the buffers must be
     * initialized
     */
    assert(buffer_initialized);

    while ((idx < buffer_capacity) && (buffer[idx].data != buf))
        idx++;

    assert(idx < buffer_capacity);
    assert(buffer[idx].reserved == 1);

    buffer[idx].used = 1;
    buffer[idx].type = io;
    buffer[idx].param.io.file_id = file_id;
    buffer[idx].param.io.client = client;

    LOG("Buffering block %i in buffer %x with offset %Li", idx, buf, offset);
    buffer[idx].param.io.offset = offset;
    buffer[idx].count = count;
    buffer[idx].datatype = datatype;

    if (tunnelfs_srv_buffer_full())
        tunnelfs_srv_buffer_flush_all();

    pthread_mutex_unlock(&buffer_mutex);

}

void tunnelfs_srv_buffer_send(void *buf, int dest, int tag, MPI_Comm comm,
                              int count, MPI_Datatype datatype)
{
    /* buf already contains the data! */
    int idx = 0;

    pthread_mutex_lock(&buffer_mutex);
    /*
     * buf already has to contain the data, thus the buffers must be
     * initialized
     */
    assert(buffer_initialized);

    while ((idx < buffer_capacity) && (buffer[idx].data != buf))
        idx++;

    assert(idx < buffer_capacity);
    assert(buffer[idx].reserved == 1);

    buffer[idx].used = 1;
    buffer[idx].type = net;
    buffer[idx].param.net.dest = dest;
    buffer[idx].param.net.tag = tag;
    buffer[idx].param.net.comm = comm;
    buffer[idx].count = count;
    buffer[idx].datatype = datatype;

    if (tunnelfs_srv_buffer_full())
        tunnelfs_srv_buffer_flush_all();

    pthread_mutex_unlock(&buffer_mutex);
}
