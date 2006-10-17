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
* File:         tunnelfs_srv_comminfo.c                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  encapsulate communicator information                      *
*                                                                         *
**************************************************************************/
#include <assert.h>
#include "mpi.h"
#include "adio.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Fill comminfo structure with values of a given communicator
 * @param comm_id ID of queried communicator
 * @param ci Reference on comminfo structure
 */
void tunnelfs_srv_comminfo_create(int comm_id, tunnelfs_comminfo_t *ci)
{
    assert(comm_id > 0);
    assert(ci != NULL);

    ci->comm_id = comm_id;
    tunnelfs_srv_comm_get_ranks(comm_id, &(ci->size), &(ci->ranks));
}

/**
 * Pack comminfo structure into an mpi message buffer
 * @param buf MPI message buffer
 * @param size size of message buffer
 * @param position Reference of position indicator
 * @param ci Reference to comminfo structure
 */
void tunnelfs_srv_comminfo_pack(void *buf, int size, int *position,
                                tunnelfs_comminfo_t *ci)
{
    assert(buf != NULL);
    assert(size > 0);
    assert(position >= 0);

    int var_id = TUNNELFS_VAR_COMMUNICATOR;

    LOCK_MPI();

    MPI_Pack(&var_id, 1, MPI_INT, buf, size, position, TUNNELFS_COMM_WORLD);
    MPI_Pack(&(ci->comm_id), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&(ci->size), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(ci->ranks, ci->size, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);

    UNLOCK_MPI();
}

/**
 * Unpack comminfo structure from an mpi message buffer
 * @param buf MPI message buffer
 * @param size size of message buffer
 * @param position Reference of position indicator
 * @param ci Reference to comminfo structure
 */
void tunnelfs_srv_comminfo_unpack(void *buf, int size, int *position,
                                  tunnelfs_comminfo_t *ci)
{
    /** TODO: Implementation! */
}

/**
 * Send communicator info to a specific io server
 * @param comm_id ID of queried communicator
 * @param rank Rank of IO Server
 */
void tunnelfs_srv_comminfo_send(int comm_id, int rank)
{
    void *setup_msg = NULL;
    int setup_msg_size = 0;
    int pack_size = 0;
    int send_id = 0;
    int position = 0;
    tunnelfs_comminfo_t ci;

    assert(comm_id > 0);
    assert(rank > 0);

    LOG("Sending communicator with id %i to %i", comm_id, rank);

    tunnelfs_srv_comminfo_create(comm_id, &ci);

    LOCK_MPI();
    MPI_Pack_size(4 + ci.size, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
    UNLOCK_MPI();

    ALLOC(setup_msg, pack_size);

    setup_msg_size = pack_size;

    send_id = TUNNELFS_NEXT_MSG_ID;
    MPI_Pack(&send_id, 1, MPI_INT,
             setup_msg, setup_msg_size, &position, TUNNELFS_COMM_WORLD);

    tunnelfs_srv_comminfo_pack(setup_msg, setup_msg_size, &position, &ci);

    ptMPI_Send(setup_msg, position,
               MPI_PACKED, rank, TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);

    FREE(setup_msg);
}
