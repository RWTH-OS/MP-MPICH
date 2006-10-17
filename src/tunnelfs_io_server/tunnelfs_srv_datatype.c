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
* File:         tunnelfs_datatype.c                                       * 
* Description:  Handling of client side datatypes                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "mpio.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Lookup table for internal to mpi type id conversions
 */
typedef struct
{
    int client;         /**< rank of client that defined this type */
    int id;             /**< internal datatype id */
    MPI_Datatype type;  /**< MPI Datatype handle */
    int size;           /**< Size of datatype */
}
tunnelfs_srv_datatype_t;

static tunnelfs_srv_datatype_t *client_dtypes = NULL;
static int _size = 0;
static int _last = -1;
static int _chunk_size = 1;

static int tunnelfs_client_dtypes_initialized = 0;

/**
 * POSIX thread mutex to synchronize access to datatype list
 */
static pthread_mutex_t tunnelfs_srv_datatype_mutex =
    PTHREAD_MUTEX_INITIALIZER;

void tunnelfs_srv_datatype_init();
void tunnelfs_srv_datatype_init()
{
    /* INFO:
     * Locking has to be done outside of this function!
     */

    /*
     * Memory should be allocated for 10 datatypes per client en block
     */
    LOCK_MPI();
    MPI_Comm_size(TUNNELFS_COMM_WORLD, &_chunk_size);
    UNLOCK_MPI();
    _chunk_size *= 10;

    _size = _chunk_size * 4;
    ALLOC(client_dtypes, _size * sizeof(tunnelfs_srv_datatype_t));

    assert(client_dtypes != NULL);

    /* 
     * Initializing named types
     */
    for (_last = 0; _last < TUNNELFS_NUM_NAMED_TYPES; _last++)
    {
        client_dtypes[_last].client = -1;       /* named types are equal on all
                                                   clients */
        client_dtypes[_last].id = _last;
        client_dtypes[_last].type = _last;
        if (_last > 0)
        {
            LOCK_MPI();
            MPI_Type_size(_last, &(client_dtypes[_last].size));
            UNLOCK_MPI();
        }
    }

    tunnelfs_client_dtypes_initialized = 1;
}

/**
 * Find index of an entry for a client and datatype
 * @param client Rank of client that defined the type
 * @param type_id Internal id for datatype
 * @return Index in datatype list
 */
static int tunnelfs_srv_datatype_find(int client, int type_id);
static int tunnelfs_srv_datatype_find(int client, int type_id)
{
    int idx = -1;

    /* INFO:
     * Mutex locking has to be handled outside of this function to ensure
     * atomic find and alter operations */

    assert(client >= -1);
    assert(type_id > 0);

    if (!tunnelfs_client_dtypes_initialized)
        tunnelfs_srv_datatype_init();

    assert(client_dtypes != NULL);
    assert(_size > 0);

    while ((idx <= _last) &&
           !((client_dtypes[idx].client == client) &&
             (client_dtypes[idx].id == type_id)))
        idx++;

    if (idx > _last)
        return -1;
    else
        return idx;
}


/** 
 * Create entry in datatype structure and return reference to MPI datatype
 * handle for subsequent MPI_Type_create call
 * @param client Rank of client that define the type
 * @param type_id Internal id for datatype
 * @return Reference to MPI Datatype handle. 
 */
MPI_Datatype *tunnelfs_srv_datatype_prepare(int client, int type_id)
{
    int idx = -1;

    assert(client >= 0);
    assert(type_id > 0);

    pthread_mutex_lock(&tunnelfs_srv_datatype_mutex);

    if (!tunnelfs_client_dtypes_initialized)
        tunnelfs_srv_datatype_init();

    assert(client_dtypes != NULL);
    assert(_size > 0);

    if (_size == _last + 1)
    {
        _size += _chunk_size;
        client_dtypes = realloc(client_dtypes,
                                _size * sizeof(tunnelfs_srv_datatype_t));
    }

    assert(client_dtypes != NULL);
    assert(_size > 0);
    assert(_last + 1 < _size);

    /* making sure datatype is not registered twice */
    idx = tunnelfs_srv_datatype_find(client, type_id);
    assert(idx == -1);

    _last++;
    client_dtypes[_last].client = client;
    client_dtypes[_last].id = type_id;

    pthread_mutex_unlock(&tunnelfs_srv_datatype_mutex);
    /* FIXME: With threads it is not a wise thing to return pointers anymore */
    return &(client_dtypes[_last].type);
}

/** 
 * Calculate parameters to avoid MPI locking for obtaining them later
 * @param dtype Handle of committed MPI datatype
 */
void tunnelfs_srv_datatype_calc_param(MPI_Datatype dtype)
{
    int idx = -1;

    pthread_mutex_lock(&tunnelfs_srv_datatype_mutex);

    assert(client_dtypes != NULL);
    assert(_size > 0);
    assert(_last >= 0);
    assert(_last < _size);

    while ((idx <= _last) && (client_dtypes[idx].type != dtype))
        idx++;

    assert(idx >= 0);
    assert(idx <= _last);

    LOCK_MPI();
    MPI_Type_size(client_dtypes[idx].type, &(client_dtypes[idx].size));
    UNLOCK_MPI();

    pthread_mutex_unlock(&tunnelfs_srv_datatype_mutex);
}

/** 
 * Get MPI datatype for a given client and type id
 * @param client Rank of client that define the type
 * @param type_id Internal id for datatype
 * @param mpi_type Reference to MPI Datatype handle. 
 */
void tunnelfs_srv_datatype_get_type(int client, int type_id,
                                    MPI_Datatype *mpi_type)
{
    int idx = -1;

    assert(client >= 0);
    assert(type_id > 0);

    /* if type_id is of a named type, ignore the client */
    if (type_id < TUNNELFS_NUM_NAMED_TYPES)
        client = -1;

    pthread_mutex_lock(&tunnelfs_srv_datatype_mutex);

    /* derived datatypes are client specific */
    idx = tunnelfs_srv_datatype_find(client, type_id);
    assert(idx >= 0);
    assert(idx <= _last);

    *mpi_type = client_dtypes[idx].type;

    pthread_mutex_unlock(&tunnelfs_srv_datatype_mutex);
}

void tunnelfs_srv_datatype_get_size(MPI_Datatype dtype, int *type_size)
{
    int idx = 0;

    pthread_mutex_lock(&tunnelfs_srv_datatype_mutex);

    if (!tunnelfs_client_dtypes_initialized)
        tunnelfs_srv_datatype_init();

    assert(client_dtypes != NULL);
    assert(_last >= 0);
    assert(_size > 0);
    assert(_last < _size);

    while ((idx <= _last) && (client_dtypes[idx].type != dtype))
        idx++;

    assert(idx >= 0);
    assert(idx <= _last);

    *type_size = client_dtypes[idx].size;
    pthread_mutex_unlock(&tunnelfs_srv_datatype_mutex);
}
