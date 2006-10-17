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
* File:         tunnelfs_srv_fptr.c                                       * 
* Description:  Handling for shared and individual file pointers          * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"
#include "mpio.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"
#include "pario_threads.h"

/**
 * Shared filepointer for file handles
 */
typedef struct
{
    int file_id;                /**< Internal file id */
    ADIO_Offset offset;         /**< byte offset from beginning of file */
}
tunnelfs_srv_shared_fp_t;

static tunnelfs_srv_shared_fp_t *shared_fp = NULL;
static int size = 0;
static int last = -1;
static int chunk_size = 10;

static pthread_mutex_t tunnelfs_srv_fptr_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Get index of filepointer in internal list
 * @param file_id Internal file id
 * @param raise_error Boolean flag to raise an error if pointer is not found
 * @return Index of shared filepointer in internal list
 */
int tunnelfs_srv_fptr_get_idx(int file_id, int raise_error);
int tunnelfs_srv_fptr_get_idx(int file_id, int raise_error)
{
    int i = 0;

    if (shared_fp == NULL)
    {
        if (raise_error)
            ERR(TUNNELFS_ERR_NULL_POINTER);
        return -1;
    }

    while ((i <= last) && !(shared_fp[i].file_id == file_id))
        i++;

    if (i > last)
    {
        if (raise_error)
            ERR(TUNNELFS_ERR_NOT_FOUND);
        return -1;
    }
    else
    {
        return i;
    }
}

/**
 * Get the value of a shared filepointer
 * @param file_id Internal file id
 * @return Current value of shared file pointer
 */
ADIO_Offset tunnelfs_srv_fptr_get_shared(int file_id)
{
    int idx = -1;
    ADIO_Offset ret = 0;

    pthread_mutex_lock(&tunnelfs_srv_fptr_mutex);

    if ((idx = tunnelfs_srv_fptr_get_idx(file_id, 0)) == -1)
    {
        /* no shared file pointer registered, let's check the main server */
        MPI_Status status;
        void *send_buf = NULL;
        int send_buf_size = 0;
        int pack_size = 0;
        int position = 0;
        int main_server = -1;
        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int var_id = TUNNELFS_REQ_SHARED_PTR;

        LOG("Requesting shared file pointer from main server");

        LOCK_MPI();
        MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
        UNLOCK_MPI();

        tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

        LOCK_MPI();
        MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        UNLOCK_MPI();

        main_server = tunnelfs_srv_file_get_main_server(file_id);
        ptMPI_Send(send_buf, position, MPI_PACKED, main_server,
                   TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);

        /* TODO: make sure the tag is not handled by another thread! */
        ptMPI_Recv(&ret, 1, TUNNELFS_OFFSET, main_server, TUNNELFS_REPLY,
                   TUNNELFS_COMM_WORLD, &status);
    }
    else
    {
        LOG("Found local shared fp at 0x%Lx", shared_fp[idx].offset);
        ret = shared_fp[idx].offset;
    }

    pthread_mutex_unlock(&tunnelfs_srv_fptr_mutex);

    return ret;
}

/**
 * Set the value of a shared filepointer
 * @param file_id Internal file id
 * @param offset Current value of shared filepointer
 */
void tunnelfs_srv_fptr_set_shared(int file_id, ADIO_Offset offset)
{
    int idx = -1;

    pthread_mutex_lock(&tunnelfs_srv_fptr_mutex);

    if ((idx = tunnelfs_srv_fptr_get_idx(file_id, 0)) == -1)
    {
        /* no shared file pointer registered, let's check the main server */
        void *send_buf = NULL;
        int send_buf_size = 0;
        int pack_size = 0;
        int temp_size = 0;
        int position = 0;
        int main_server = -1;
        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int var_id = TUNNELFS_REQ_SET_SHARED_PTR;

        LOG("Requesting shared file pointer from main server");

        LOCK_MPI();
        MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
        MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &temp_size);
        pack_size += temp_size;
        UNLOCK_MPI();

        tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

        LOCK_MPI();
        MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&offset, 1, TUNNELFS_OFFSET, send_buf, send_buf_size,
                 &position, TUNNELFS_COMM_WORLD);
        UNLOCK_MPI();

        main_server = tunnelfs_srv_file_get_main_server(file_id);
        ptMPI_Send(send_buf, position, MPI_PACKED, main_server,
                   TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);
    }
    else
        shared_fp[idx].offset = offset;

    pthread_mutex_unlock(&tunnelfs_srv_fptr_mutex);
}

/**
 * Create a shared filepointer for a file id
 * @param file_id Internal file id
 */
void tunnelfs_srv_fptr_register(int file_id)
{
    assert(file_id > 0);

    pthread_mutex_lock(&tunnelfs_srv_fptr_mutex);

    /* insert shared pointer */
    if ((shared_fp == NULL) || (size == 0) || (size == last + 1))
    {
        size += chunk_size;
        shared_fp = realloc(shared_fp,
                            size * sizeof(tunnelfs_srv_shared_fp_t));
    }

    assert(shared_fp != NULL);

    last++;
    shared_fp[last].file_id = file_id;
    shared_fp[last].offset = 0;

    LOG("registered shared file pointer");

    pthread_mutex_unlock(&tunnelfs_srv_fptr_mutex);
}

/**
 * Delete a shared filepoint for a file id
 * @param file_id Internal file id
 */
void tunnelfs_srv_fptr_unregister(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(shared_fp != NULL);
    assert(size > 0);
    assert(last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_fptr_mutex);

    idx = tunnelfs_srv_fptr_get_idx(file_id, 1);

    assert(idx >= 0);

    if (idx < last)
    {
        memcpy(&(shared_fp[idx]), &(shared_fp[last]),
               sizeof(tunnelfs_srv_shared_fp_t));
    }

    shared_fp[last].file_id = 0;
    shared_fp[last].offset = 0;
    last--;

    pthread_mutex_unlock(&tunnelfs_srv_fptr_mutex);

    LOG("unregistered shared file pointer");
}

/**
 * Get current shared filepointer value and increase it atomically for
 * subsequent queries
 * @param file_id Internal file id
 * @param inc Increment for offset
 * @return Offset before the increment
 */
ADIO_Offset tunnelfs_srv_fptr_getset_shared_atomic(int file_id,
                                                   ADIO_Offset inc)
{
    int idx = -1;
    ADIO_Offset ret = 0;

    assert(file_id > 0);
    assert(inc >= 0);

    pthread_mutex_lock(&tunnelfs_srv_fptr_mutex);

    if ((idx = tunnelfs_srv_fptr_get_idx(file_id, 0)) == -1)
    {
        LOG("Requesting and incrementing shared file pointer");

        /* no shared file pointer registered, let's check the main server */
        void *send_buf = NULL;
        int send_buf_size = 0;
        int pack_size = 0;
        int temp_size = 0;
        int position = 0;
        int main_server = -1;
        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int var_id = TUNNELFS_REQ_GETSET_SHARED_PTR;
        MPI_Status status;

        LOCK_MPI();
        MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
        MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &temp_size);
        pack_size += temp_size;
        UNLOCK_MPI();

        tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

        LOCK_MPI();
        MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size, &position,
                 TUNNELFS_COMM_WORLD);
        MPI_Pack(&inc, 1, TUNNELFS_OFFSET, send_buf, send_buf_size,
                 &position, TUNNELFS_COMM_WORLD);
        UNLOCK_MPI();

        main_server = tunnelfs_srv_file_get_main_server(file_id);
        LOG("Server for file %i is %i", file_id, main_server);

        ptMPI_Send(send_buf, position, MPI_PACKED, main_server,
                   TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);

        /* TODO: make sure the tag is not handled by another thread! */
        ptMPI_Recv(&ret, 1, TUNNELFS_OFFSET, main_server, TUNNELFS_REPLY,
                   TUNNELFS_COMM_WORLD, &status);

        FREE(send_buf);
    }
    else
    {
        ret = shared_fp[idx].offset;
        LOG("Found local shared fp at 0x%Lx", ret);
        shared_fp[idx].offset += inc;
    }

    pthread_mutex_unlock(&tunnelfs_srv_fptr_mutex);

    return ret;
}
