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
* File:         tunnelfs_srv_comm.c                                       * 
* Description:  Handling of client specific communicator ids              * 
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
#include "ad_tunnelfs.h"
#include "tunnelfs_srv_comm.h"

/**
 * Internal representation of client communicators
 */
typedef struct
{
    int comm_id;    /**< internal communicator id */
    int size;       /**< size of process list */
    int *ranks;     /**< list of mpi ranks */
}
tunnelfs_client_comm_t;

/* List of communicator representations */
static tunnelfs_client_comm_t *client_comms = NULL;
static int _size = 0;
static int _last = -1;
static int chunk_size = 10;

int tunnelfs_srv_comm_id = 0;

static pthread_mutex_t tunnelfs_srv_comm_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * Get index of communicator in local list
 * @param comm_id ID of queried communicator
 * @return index of communicator in local list, -1 if not found
 */
int tunnelfs_srv_comm_get_idx(int comm_id);
int tunnelfs_srv_comm_get_idx(int comm_id)
{
    int i = 0;

    assert(comm_id > 0);
    assert(client_comms != NULL);
    assert(_size > 0);

    if (_last == -1)
        return _last;

    assert(_last >= 0);

    /* INFO:
     * Mutex locking has to be handled outside to ensure atomic find and alter
     * operation ! */

    while ((i <= _last) && (i < _size)
           && (client_comms[i].comm_id != comm_id))
        i++;

    if (i > _last)
        return -1;
    else
        return i;
}

/**
 * Create a communicator with associated mpi ranks
 * @param comm_id ID the created communicator shall use
 * @param size Number of ranks in the communicator
 * @param ranks List of ranks in TUNNELFS_COMM_WORLD
 */
void tunnelfs_srv_comm_register(int comm_id, int size, int *ranks)
{
    int idx = -1;

    assert(comm_id > 0);
    assert(size > 0);
    assert(ranks != NULL);
    pthread_mutex_lock(&tunnelfs_srv_comm_mutex);

    if ((client_comms == NULL) || (_size == 0) || (_size == _last + 1))
    {
        _size += chunk_size;
        client_comms = realloc(client_comms,
                               _size * sizeof(tunnelfs_client_comm_t));
    }

    assert(client_comms != NULL);

    /* making sure comm is not registered twice */
    idx = tunnelfs_srv_comm_get_idx(comm_id);
    assert(idx == -1);

    _last++;
    client_comms[_last].comm_id = comm_id;
    client_comms[_last].size = size;
    client_comms[_last].ranks = ranks;

    pthread_mutex_unlock(&tunnelfs_srv_comm_mutex);
}

/**
 * Destroy a communicator with associated mpi ranks
 * @param comm_id ID the created communicator shall use
 */
void tunnelfs_srv_comm_unregister(int comm_id)
{
    int idx = -1;

    assert(comm_id > 0);
    assert(client_comms != NULL);
    assert(_size > 0);
    assert(_last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_comm_mutex);

    idx = tunnelfs_srv_comm_get_idx(comm_id);
    assert(idx != -1);

    if (idx < _last)
        memcpy(&(client_comms[idx]),
               &(client_comms[_last]), sizeof(tunnelfs_client_comm_t));

    client_comms[_last].comm_id = -1;
    client_comms[_last].size = 0;
    FREE(client_comms[_last].ranks);
    _last--;

    pthread_mutex_unlock(&tunnelfs_srv_comm_mutex);
}

/**
 * Retrieve ranks from internal communicator reference
 * @param comm_id ID of queried communicator
 * @param size Size of the return communicator list
 * @param ranks List of ranks in the communicator
 */
void tunnelfs_srv_comm_get_ranks(int comm_id, int *size, int **ranks)
{
    int idx = -1;

    assert(comm_id > 0);
    assert(client_comms != NULL);
    assert(_size > 0);
    assert(_last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_comm_mutex);

    idx = tunnelfs_srv_comm_get_idx(comm_id);

    assert(idx >= 0);
    assert(client_comms[idx].size > 0);
    assert(client_comms[idx].ranks != NULL);

    *size = client_comms[idx].size;
    *ranks = client_comms[idx].ranks;

    pthread_mutex_unlock(&tunnelfs_srv_comm_mutex);
}
