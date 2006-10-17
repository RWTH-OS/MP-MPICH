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
* File:         tunnelfs_fileview.c                                       * 
* Description:  Handling the file views set by clients                    * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "mpi.h"
#include "mpio.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Lookup table for client file views
 */
typedef struct
{
    int client;             /**< client rank */
    int file_id;            /**< internal file id */
    MPI_Offset disp;        /**< displacement in file view */
    MPI_Datatype etype;     /**< elementary type in file view */
    MPI_Datatype filetype;  /**< filetype in file view */
}
tunnelfs_srv_fileview_t;

static tunnelfs_srv_fileview_t *tunnelfs_srv_fviews = NULL;
static int tunnelfs_srv_fviews_size = 0;
static int tunnelfs_srv_fviews_last = -1;
static int tunnelfs_srv_fviews_chunk_size = 1;

static pthread_mutex_t tunnelfs_srv_fileview_mutex =
    PTHREAD_MUTEX_INITIALIZER;


/**
 * Get index of file view in internal list
 * @param client Rank of client
 * @param file_id Internal file id
 */
static int tunnelfs_srv_fileview_find(int client, int file_id);
static int tunnelfs_srv_fileview_find(int client, int file_id)
{
    int idx = 0;

    pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

    /* sanity check */
    if ((tunnelfs_srv_fviews == NULL) || (tunnelfs_srv_fviews_size == 0))
    {
        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
        return -1;
    }

    while ((idx <= tunnelfs_srv_fviews_last) &&
           !((tunnelfs_srv_fviews[idx].client == client) &&
             (tunnelfs_srv_fviews[idx].file_id == file_id)))
        idx++;

    if (idx > tunnelfs_srv_fviews_last)
    {
        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
        return -1;
    }
    else
    {
        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
        return idx;
    }
}

/**
 * Save fileview for a client on a specific file
 * @param client Rank of client
 * @param file_id Internal file id
 * @param disp Displacement of MPI file view
 * @param etype Elementary type of MPI file view
 * @param filetype Filetype of MPI file view
 */
void tunnelfs_srv_fileview_set(int client, int file_id, MPI_Offset disp,
                               MPI_Datatype etype, MPI_Datatype filetype)
{
    int idx = -1;

    idx = tunnelfs_srv_fileview_find(client, file_id);

    if (idx == -1)
    {
        pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

        if ((tunnelfs_srv_fviews == NULL) ||
            (tunnelfs_srv_fviews_size == 0) ||
            (tunnelfs_srv_fviews_size == tunnelfs_srv_fviews_last + 1))
        {
            /* as the number of entries is significantly dependent on the
             * communicator used to open the file. Therefore, I set chunk_size to
             * the number of processes in TUNNELFS_COMM_WORLD. That will be
             * sufficient in any case */
            LOCK_MPI();
            MPI_Comm_size(TUNNELFS_COMM_WORLD,
                          &tunnelfs_srv_fviews_chunk_size);
            UNLOCK_MPI();

            tunnelfs_srv_fviews_size += tunnelfs_srv_fviews_chunk_size;
            tunnelfs_srv_fviews = realloc(tunnelfs_srv_fviews,
                                          tunnelfs_srv_fviews_size *
                                          sizeof(tunnelfs_srv_fileview_t));

            if (tunnelfs_srv_fviews == NULL)
            {
                pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
                ERR(TUNNELFS_ERR_ALLOC);
            }
        }
        idx = ++tunnelfs_srv_fviews_last;

        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
    }

    pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

    tunnelfs_srv_fviews[idx].client = client;
    tunnelfs_srv_fviews[idx].file_id = file_id;
    tunnelfs_srv_fviews[idx].disp = disp;
    tunnelfs_srv_fviews[idx].etype = etype;
    tunnelfs_srv_fviews[idx].filetype = filetype;

    pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
}

/**
 * Get saved fileview for a client on a specific file
 * @param client Rank of client
 * @param file_id Internal file id
 * @param disp Reference on displacement of MPI file view
 * @param etype Reference on elementary type of MPI file view
 * @param filetype Reference on filetype of MPI file view
 */
void tunnelfs_srv_fileview_get(int client, int file_id, MPI_Offset *disp,
                               MPI_Datatype *etype, MPI_Datatype *filetype)
{
    int idx = -1;

    idx = tunnelfs_srv_fileview_find(client, file_id);

    if (idx != -1)
    {
        pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

        *disp = tunnelfs_srv_fviews[idx].disp;
        *etype = tunnelfs_srv_fviews[idx].etype;
        *filetype = tunnelfs_srv_fviews[idx].filetype;

        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
    }
    else
    {
        *disp = 0;
        *etype = MPI_BYTE;
        *filetype = MPI_BYTE;
    }
}

/**
 * Get elemantary type of mpi file view
 * @param client Rank of client
 * @param file_id Internal file id
 * @param etype Reference on datatype variable
 */
void tunnelfs_srv_fileview_get_etype(int client, int file_id,
                                     MPI_Datatype *etype)
{
    int idx = -1;

    idx = tunnelfs_srv_fileview_find(client, file_id);

    if (idx != -1)
    {
        pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

        *etype = tunnelfs_srv_fviews[idx].etype;

        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
    }
    else
    {
        *etype = MPI_BYTE;
    }
}

/**
 * Get filetype of mpi file view
 * @param client Rank of client
 * @param file_id Internal file id
 * @param filetype Reference on datatype variable
 */
void tunnelfs_srv_fileview_get_filetype(int client, int file_id,
                                        MPI_Datatype *filetype)
{
    int idx = -1;

    idx = tunnelfs_srv_fileview_find(client, file_id);

    if (idx != -1)
    {
        pthread_mutex_lock(&tunnelfs_srv_fileview_mutex);

        *filetype = tunnelfs_srv_fviews[idx].filetype;

        pthread_mutex_unlock(&tunnelfs_srv_fileview_mutex);
    }
    else
    {
        *filetype = MPI_BYTE;
    }
}
