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
* File:         tunnelfs_srv_coll.c                                       * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Manangement for collective operations, that cannot be     *
*               handled by a single client                                *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "ad_tunnelfs.h"
#include "tunnelfs_srv.h"
#include "tunnelfs_log.h"
#include "mpi.h"

/**
 * Structure for pseudo collective operations
 */
typedef struct
{
    int file_id;                /**< file id */
    int magic_cookie;           /**< unique identifier */
    int n;                      /**< number of total processes */
    int pending;                /**< number of pending processes */
    int clnt_msg_id;            /**< client msg id to be used for answering */
}
tunnelfs_collops_t;

/**
 * The value of the current magic cookie for pseudo collective operations
 */
int tunnelfs_srv_coll_id = 0;

static tunnelfs_collops_t *collops = NULL;
static int size = 0;
static int last = -1;
static int chunk_size = 10;

static pthread_mutex_t tunnelfs_srv_coll_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Find index of an ongoing pseudo collective operation
 * @param file_id File ID associated with the operation
 * @param magic_cookie Unique identifier for operation
 * @return index Index of pseudo collective operation
 */
int tunnelfs_srv_collop_find(int file_id, int magic_cookie);
int tunnelfs_srv_collop_find(int file_id, int magic_cookie)
{
    int idx = 0;

    /* INFO:
     * mutex locking has to be handled outside of this function to ensure
     * atomic search and alter operations! */

    assert(file_id > 0);
    assert(collops != NULL);

    /* sanity check */
    if (collops == NULL)
        return -1;

    while ((idx <= last) &&
           ((collops[idx].file_id != file_id) ||
            (collops[idx].magic_cookie != magic_cookie)))
        idx++;

    if (idx > last)
        return -1;
    else
        return idx;
}

/**
 * Start tracking of pseudo collective operation
 * @param file_id File ID to associate with file operation
 * @param magic_cookie Unique identifier for this operation
 * @param n Number of processes participating (implizitely, the number of
 *          answers expected before completion)
 * @param clnt_msg_id Message ID of client request for reply message
 */
void tunnelfs_srv_collop_start(int file_id, int magic_cookie, int n,
                               int clnt_msg_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(n > 0);

    pthread_mutex_lock(&tunnelfs_srv_coll_mutex);

    if ((collops == NULL) || (size == 0) || (size == last + 1))
    {
        int i;

        size += chunk_size;
        collops = realloc(collops, size * sizeof(tunnelfs_collops_t));
        if (collops == NULL)
        {
            pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);
            ERR(TUNNELFS_ERR_ALLOC);
        }

        /* initialize new fields */
        for (i = last + 1; i < size; i++)
        {
            collops[i].file_id = 0;
            collops[i].magic_cookie = 0;
            collops[i].n = 0;
            collops[i].pending = 0;
            collops[i].clnt_msg_id = 0;
        }
    }

    assert(collops != NULL);
    assert(last + 1 < size);

    /* making sure this is not started twice */
    idx = tunnelfs_srv_collop_find(file_id, magic_cookie);
    assert(idx == -1);

    last++;
    collops[last].file_id = file_id;
    collops[last].magic_cookie = magic_cookie;
    collops[last].n = n;
    collops[last].pending = n;
    collops[last].clnt_msg_id = clnt_msg_id;

    pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);

    LOG("Collective Operation %i handling started for file %i and %i procs",
        magic_cookie, file_id, n);
}

/**
 * Decrease the number of pending answers for the operation
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
void tunnelfs_srv_collop_done(int file_id, int magic_cookie)
{
    int idx = -1;

    assert(file_id > 0);
    assert(collops != NULL);

    pthread_mutex_lock(&tunnelfs_srv_coll_mutex);

    idx = tunnelfs_srv_collop_find(file_id, magic_cookie);

    assert(idx >= 0);

    collops[idx].pending--;

    LOG("Collective Operation %i for file %i is still waiting for %i procs",
        magic_cookie, file_id, collops[idx].pending);
    pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);
}

/**
 * Delete pseudo collective operation from buffers if finished
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
void tunnelfs_srv_collop_free(int file_id, int magic_cookie)
{
    int idx = -1;

    assert(file_id > 0);
    assert(collops != NULL);

    pthread_mutex_lock(&tunnelfs_srv_coll_mutex);

    idx = tunnelfs_srv_collop_find(file_id, magic_cookie);

    assert(idx >= 0);

    if (size == 1)
    {
        /* no memcpy */
        last--;
    }
    else
    {
        memcpy(&(collops[idx]), &(collops[last]), sizeof(tunnelfs_collops_t));
        last--;
    }

    LOG("Collective Operation %i handling completed for file %i",
        magic_cookie, file_id);

    pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);
}

/**
 * Get the number of pending replies for an operation
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 * @return Number of pending replies
 */
int tunnelfs_srv_collop_num_pending(int file_id, int magic_cookie)
{
    int idx = -1;

    assert(file_id > 0);
    assert(collops != NULL);

    pthread_mutex_lock(&tunnelfs_srv_coll_mutex);

    idx = tunnelfs_srv_collop_find(file_id, magic_cookie);

    pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);

    assert(idx >= 0);

    return collops[idx].pending;
}

/**
 * Get message id of initial client request
 * @param file_id File ID associated with operation
 * @param magic_cookie Unique identifier for this operation
 */
int tunnelfs_srv_collop_get_clnt_msg_id(int file_id, int magic_cookie)
{
    int idx = -1;

    assert(file_id > 0);
    assert(collops != NULL);

    pthread_mutex_lock(&tunnelfs_srv_coll_mutex);

    idx = tunnelfs_srv_collop_find(file_id, magic_cookie);

    pthread_mutex_unlock(&tunnelfs_srv_coll_mutex);

    assert(idx >= 0);

    return collops[idx].clnt_msg_id;
}
