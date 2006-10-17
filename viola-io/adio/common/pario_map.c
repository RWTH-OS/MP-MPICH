/**************************************************************************
* VIOLA Parallel IO (ParIO)                                               *
***************************************************************************
*                                                                         *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              *
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         *
* See COPYRIGHT notice in base directory for details                      *
***************************************************************************
***************************************************************************
* File:         pario_map.c                                               *
* Description:  Mapping between TUNNELFS_COMM_WORLD                       *
*               MPI_COMM_META_REDUCED                                     *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"
#include "ad_tunnelfs_common.h"
#include "ad_tunnelfs.h"
#include "pario_map.h"
#include "pario_threads.h"
#include <assert.h>
#include "../../../src/tunnelfs_io_server/tunnelfs_log.h"

/**
 * Mapping from higher level and lower level communicator
 */
static int *pario_rank_map = NULL;
static int pario_map_size = 0;
static int pario_map_initialized = 0;

static pthread_mutex_t pario_map_mutex = PTHREAD_MUTEX_INITIALIZER;

static int *pario_rank_tree = NULL;

/**
 * Initialize mapping structure between communicators
 */
void pario_init_map()
{
    MPI_Group world_group;
    MPI_Group reduced_group;
    int *reduced_ranks;
    int i;

    LOCK_MPI();
    MPI_Comm_size(MPI_COMM_META_REDUCED, &pario_map_size);

    ALLOC(reduced_ranks, pario_map_size * sizeof(int));

    /* we want to translate all ranks */
    for (i = 0; i < pario_map_size; i++)
        reduced_ranks[i] = i;

    MPI_Comm_group(TUNNELFS_COMM_WORLD, &world_group);
    MPI_Comm_group(MPI_COMM_META_REDUCED, &reduced_group);

    pthread_mutex_lock(&pario_map_mutex);
    if (pario_rank_map == NULL)
        ALLOC(pario_rank_map, pario_map_size * sizeof(int));

    MPI_Group_translate_ranks(reduced_group, pario_map_size, reduced_ranks,
                              world_group, pario_rank_map);

    pthread_mutex_unlock(&pario_map_mutex);
    
    UNLOCK_MPI();
    FREE(reduced_ranks);
    pario_map_initialized = 1;
}

/**
 * Get the rank of process in MPI_COMM_META_REDUCED for provided rank in
 * TUNNELFS_COMM_WORLD
 * @param rank rank of MPI process in TUNNELFS_COMM_WORLD
 * @return corresponding rank of MPI process in MPI_COMM_META_REDUCED
 */
int pario_map_w2r(int rank)
{
    int i = 0;

    if (!pario_map_initialized) pario_init_map();

    assert(pario_rank_map != NULL);
    assert(rank >= 0);

    pthread_mutex_lock(&pario_map_mutex);
    while ((i < pario_map_size) && (pario_rank_map[i] != rank))
        i++;
    pthread_mutex_unlock(&pario_map_mutex);

    assert(i < pario_map_size);

    return i;
}

/**
 * Get the rank of process in TUNNELFS_COMM_WORLD for provided rank in
 * MPI_COMM_META_REDUCED
 * @param rank rank of MPI process in MPI_COMM_META_REDUCED
 * @return corresponding rank of MPI process in TUNNELFS_COMM_WORLD
 */
int pario_map_r2w(int rank)
{
    if (!pario_map_initialized) pario_init_map();

    assert(pario_rank_map != NULL);
    assert(rank >= 0);
    assert(rank < pario_map_size);

    return pario_rank_map[rank];
}
