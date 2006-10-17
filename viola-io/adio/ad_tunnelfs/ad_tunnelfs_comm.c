/**************************************************************************
* TunnelFS                                                                * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_tunnelfs_comm.c                                        * 
* Description:                                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdlib.h>
#include "ad_tunnelfs.h"

/**
 * Tunnelfs client side communicator tracking
 */
typedef struct
{
    int id;
    MPI_Comm comm;
    int transfered;
}
tunnelfs_comm_t;

/**
 * list of communicator stuctures 
 */
static tunnelfs_comm_t *tunnelfs_comms;
/* size of linear list */
static int tunnelfs_comms_size = 0;
/* last entry in list */
static int tunnelfs_comm_last = -1;
/* increment buffer size in  steps of chunk_size */
static int tunnelfs_comms_chunk_size = 10;

static int tunnelfs_comm_id = 0;

/**
 * Register MPI communicator under a certain comm id
 * @param comm_id internal communicator id
 * @param comm MPI communicator handle
 * @return if successful the communicator id is returned, else -1.
 */
int tunnelfs_comm_register(int comm_id, MPI_Comm comm)
{
    if (tunnelfs_comm_storage_sufficient() != TUNNELFS_FAILURE)
    {
        tunnelfs_comm_last++;
        tunnelfs_comms[tunnelfs_comm_last].id = comm_id;
        tunnelfs_comms[tunnelfs_comm_last].comm = comm;
        tunnelfs_comms[tunnelfs_comm_last].transfered = 0;
        /*
           fprintf(stderr, "Registered comm: %i\n", comm_id);
         */
        return comm_id;
    }
    else
        return -1;
}

/**
 * Delete internal communicator reference
 * @param comm_id internal communicator id
 */
void tunnelfs_comm_unregister(int comm_id)
{
    int idx = -1;
    if ((idx = tunnelfs_comm_get_idx4id(comm_id)) != -1)
    {
        if (tunnelfs_comm_last != 0)
        {
            memcpy(&(tunnelfs_comms[idx]),
                   &(tunnelfs_comms[tunnelfs_comm_last]),
                   sizeof(tunnelfs_comm_t));
        }
        tunnelfs_comm_last--;
    }
    else
        fprintf(stderr, "Client could not unregister comm: %i\n", comm_id);
}

/**
 * Get internal communicator id for a given MPI communicator handle
 * @param comm MPI communicator handle
 * @return internal communicator id
 */
int tunnelfs_comm_get_id(MPI_Comm comm)
{
    int search_idx = 0;

    if ((search_idx = tunnelfs_comm_get_idx4comm(comm)) != -1)
        return tunnelfs_comms[search_idx].id;
    else
        return -1;
}

/**
 * Get MPI communicator handle for internal communicator id
 * @param id internal communicator id
 * @return MPI communicator handle
 */
MPI_Comm tunnelfs_comm_get_comm(int id)
{
    int search_idx = -1;

    if ((search_idx = tunnelfs_comm_get_idx4id(id)) != TUNNELFS_FAILURE)
    {
        return tunnelfs_comms[search_idx].comm;
    }
    else
    {
        return MPI_COMM_NULL;
    }
}

/**
 * Check if there still is sufficient space for storing an additional entry.
 * @return boolean value, indicating sufficient storage
 */
int tunnelfs_comm_storage_sufficient()
{
    if (tunnelfs_comms_size == 0)
    {
        tunnelfs_comms_size = tunnelfs_comms_chunk_size;
        tunnelfs_comms = (tunnelfs_comm_t *) calloc(tunnelfs_comms_size,
                                                    sizeof(tunnelfs_comm_t));
        if (tunnelfs_comms == NULL)
            return TUNNELFS_FAILURE;
        else
            return TUNNELFS_SUCCESS;
    }
    else if (tunnelfs_comm_last == (tunnelfs_comms_size - 1))
    {
        tunnelfs_comm_t *temp = tunnelfs_comms;

        tunnelfs_comms_size += tunnelfs_comms_chunk_size;
        tunnelfs_comms = realloc(tunnelfs_comms, tunnelfs_comms_size *
                                 sizeof(tunnelfs_comm_t));

        if (tunnelfs_comms == NULL)
        {
            tunnelfs_comms = temp;
            tunnelfs_comms_size -= tunnelfs_comms_chunk_size;
            return TUNNELFS_FAILURE;
        }
        return TUNNELFS_SUCCESS;
    }
    else
    {
        return TUNNELFS_SUCCESS;
    }
}

/**
 * Get index in internal list for a given MPI communicator handle
 * @param comm MPI communicator handle
 * @return index of entry in internal list
 */
int tunnelfs_comm_get_idx4comm(MPI_Comm comm)
{
    int i = 0;

    /* sanity check */
    if (tunnelfs_comms == NULL)
        return -1;

    while ((i <= tunnelfs_comm_last) && (tunnelfs_comms[i].comm != comm))
        i++;

    if (i > tunnelfs_comm_last)
        return -1;
    else
        return i;
}

/**
 * Get index in internal list for a given communicator id
 * @param id internal communcator id
 * @return index of entry in internal list
 */
int tunnelfs_comm_get_idx4id(int id)
{
    int i = 0;

    if (tunnelfs_comms == NULL)
        return -1;
    if (tunnelfs_comm_last < 0)
        return -1;

    while ((i <= tunnelfs_comm_last) && (tunnelfs_comms[i].id != id))
        i++;

    if (i > tunnelfs_comm_last)
        return -1;
    else
        return i;
}

/**
 * Create a tunnelfs communicator setup message
 * @param id internal communicator id
 * @param buf Reference of message buffer
 * @param buf_size Reference size of message buffer
 * @param position Reference to position indicator
 * @param msg_id Message id to use
 */
void tunnelfs_comm_serialize_by_id(int id, void **buf, int *buf_size,
                                   int *position, int *msg_id)
{
    int i = 0;
    int pack_size = 0;
    int comm_size = 0;
    /*int comm_id = -1; */
    MPI_Comm comm;
    MPI_Group world_group;
    MPI_Group local_group;
    int *local_ranks = NULL;
    int *global_ranks = NULL;
    int var_id = 0;
    /*int send_id = 0; */

    /* get group of my world */
    MPI_Comm_group(TUNNELFS_COMM_WORLD, &world_group);

    /* get size of respective communicator */
    comm = tunnelfs_comm_get_comm(id);

    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_group(comm, &local_group);

    ALLOC(local_ranks, comm_size * sizeof(int));
    ALLOC(global_ranks, comm_size * sizeof(int));

    for (i = 0; i < comm_size; i++)
        local_ranks[i] = i;

    MPI_Group_translate_ranks(local_group, comm_size, local_ranks,
                              world_group, global_ranks);

    /* calculate buffer size */
    MPI_Pack_size(4 + comm_size, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
    tunnelfs_adjust_buffer(buf, buf_size, pack_size);

    *msg_id = TUNNELFS_NEXT_MSG_ID;
    MPI_Pack(msg_id, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    var_id = TUNNELFS_VAR_COMMUNICATOR;
    MPI_Pack(&var_id, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&id, 1, MPI_INT, *buf, *buf_size, position, TUNNELFS_COMM_WORLD);
    MPI_Pack(&comm_size, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(global_ranks, comm_size, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);

    FREE(local_ranks);
    FREE(global_ranks);
}

/**
 * Create a tunnelfs communicator setup message
 * @param comm MPI communicator handle
 * @param buf Reference of message buffer
 * @param buf_size Reference size of message buffer
 * @param position Reference to position indicator
 * @param msg_id Message id to use
 */
void tunnelfs_comm_serialize_by_comm(MPI_Comm comm, void **buf,
                                     int *buf_size, int *position,
                                     int *msg_id)
{
    int i = 0;
    int pack_size = 0;
    int comm_size = 0;
    int comm_id = -1;
    MPI_Group world_group;
    MPI_Group local_group;
    int *local_ranks = NULL;
    int *global_ranks = NULL;
    int var_id = 0;
    /*int send_id = 0; */

    /* get group of my world */
    MPI_Comm_group(TUNNELFS_COMM_WORLD, &world_group);

    /* get size of respective communicator */
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_group(comm, &local_group);

    ALLOC(local_ranks, comm_size * sizeof(int));
    ALLOC(global_ranks, comm_size * sizeof(int));

    for (i = 0; i < comm_size; i++)
        local_ranks[i] = i;

    MPI_Group_translate_ranks(local_group, comm_size, local_ranks,
                              world_group, global_ranks);

    /* calculate buffer size */
    MPI_Pack_size(4 + comm_size, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
    tunnelfs_adjust_buffer(buf, buf_size, pack_size);

    *msg_id = TUNNELFS_NEXT_MSG_ID;
    MPI_Pack(msg_id, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    var_id = TUNNELFS_VAR_COMMUNICATOR;
    MPI_Pack(&var_id, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    comm_id = tunnelfs_comm_get_id(comm);
    MPI_Pack(&comm_id, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&comm_size, 1, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(global_ranks, comm_size, MPI_INT, *buf, *buf_size, position,
             TUNNELFS_COMM_WORLD);

    FREE(local_ranks);
    FREE(global_ranks);
}

/** 
 * Check if a communicator has already been transfered
 * @param id internal communicator id
 * @return boolean value
 */
int tunnelfs_comm_is_transfered(int id)
{
    int idx;

    idx = tunnelfs_comm_get_idx4id(id);
    if (idx != -1)
    {
        return tunnelfs_comms[idx].transfered;
    }
    else
    {
        return 0;
    }
}

/**
 * Mark a communicator as transfered
 * @param id internal communicator id
 */
void tunnelfs_comm_set_transfered(int id)
{
    int idx;

    idx = tunnelfs_comm_get_idx4id(id);
    if (idx != -1)
    {
        tunnelfs_comms[idx].transfered = 1;
    }
    else
    {
        fprintf(stderr, "Could not modify communicator id: %i\n", id);
    }
}
