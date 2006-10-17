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
* File:         tunnelfs_srv_parts.c                                      * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  handle distribution of partial files                      *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mpi.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_srv.h"

/**
 * File handle for cache files
 */
typedef struct
{
    int file_id;                /* part of what file? */
    int client;                 /* owner */
    int dirty;                  /* has file been modified sind last flush? */
    int server;                 /* globaly responsible server for this part */
    MPI_File fh;                /* file handle for cache file */
} tunnelfs_srv_parts_t;


tunnelfs_srv_parts_t *tunnelfs_srv_parts = NULL;
static int tunnelfs_srv_parts_last = -1;
static int tunnelfs_srv_parts_size = 0;

pthread_mutex_t tunnelfs_srv_parts_mutex = PTHREAD_MUTEX_INITIALIZER;

int tunnelfs_srv_parts_search(int file_id, int client)
{
    int idx = 0;

    pthread_mutex_lock(&tunnelfs_srv_parts_mutex);

    if ((tunnelfs_srv_parts == NULL) || (tunnelfs_srv_parts_size == 0))
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return -1;
    }

    while ((idx <= tunnelfs_srv_parts_last) &&
           (idx < tunnelfs_srv_parts_size) &&
           ((tunnelfs_srv_parts[idx].file_id != file_id) ||
            (tunnelfs_srv_parts[idx].client != client)))
        idx++;

    if (idx > tunnelfs_srv_parts_last)
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return -1;
    }
    else
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return idx;
    }
}

int tunnelfs_srv_parts_find_any_client(int file_id)
{
    int idx = 0;

    pthread_mutex_lock(&tunnelfs_srv_parts_mutex);

    if ((tunnelfs_srv_parts == NULL) || (tunnelfs_srv_parts_size == 0))
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return -1;
    }

    while ((idx <= tunnelfs_srv_parts_last) &&
           (idx < tunnelfs_srv_parts_size) &&
           (tunnelfs_srv_parts[idx].file_id != file_id))
        idx++;

    if (idx > tunnelfs_srv_parts_last)
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return -1;
    }
    else
    {
        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
        return tunnelfs_srv_parts[idx].client;
    }
}

void tunnelfs_srv_parts_create(int file_id, int client, int server)
{
    int idx = -1;

    if ((idx = tunnelfs_srv_parts_search(file_id, client)) != -1)
    {
        ERR(TUNNELFS_ERR_DUPLICATE);
        return;
    }
    else
    {
        pthread_mutex_lock(&tunnelfs_srv_parts_mutex);

        if ((tunnelfs_srv_parts == NULL) ||
            (tunnelfs_srv_parts_size == tunnelfs_srv_parts_last + 1))
        {
            tunnelfs_srv_parts_size += 10;
            tunnelfs_srv_parts = realloc(tunnelfs_srv_parts,
                                         tunnelfs_srv_parts_size *
                                         sizeof(tunnelfs_srv_parts_t));
            if (tunnelfs_srv_parts == NULL)
            {
                tunnelfs_srv_parts_size -= 10;
                ERR(TUNNELFS_ERR_ALLOC);
                pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
                return;
            }
        }

        tunnelfs_srv_parts_last++;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].file_id = file_id;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].client = client;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].dirty = 0;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].server = server;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].fh = MPI_FILE_NULL;

        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
    }
}

void tunnelfs_srv_parts_destroy(int file_id, int client)
{
    int idx = tunnelfs_srv_parts_search(file_id, client);

    if (idx == -1)
    {
        ERR(TUNNELFS_ERR_NOT_FOUND);
        return;
    }
    else
    {
        pthread_mutex_lock(&tunnelfs_srv_parts_mutex);

        if (idx != tunnelfs_srv_parts_last)
        {
            memcpy(&(tunnelfs_srv_parts[idx]),
                   &(tunnelfs_srv_parts[tunnelfs_srv_parts_last]),
                   sizeof(tunnelfs_srv_parts_t));
        }

        tunnelfs_srv_parts[tunnelfs_srv_parts_last].file_id = -1;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].client = -1;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].dirty = 0;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].server = -1;
        tunnelfs_srv_parts[tunnelfs_srv_parts_last].fh = MPI_FILE_NULL;
        tunnelfs_srv_parts_last--;

        pthread_mutex_unlock(&tunnelfs_srv_parts_mutex);
    }
}

int tunnelfs_srv_parts_get_server(int file_id, int client)
{
    int idx = tunnelfs_srv_parts_search(file_id, client);

    if (idx == -1)
    {
        ERR(TUNNELFS_ERR_NOT_FOUND);
        return -1;
    }
    else
        return tunnelfs_srv_parts[idx].server;
}

MPI_File *tunnelfs_srv_parts_get_handle(int file_id, int client)
{
    int idx = tunnelfs_srv_parts_search(file_id, client);

    if (idx == -1)
    {
        ERR(TUNNELFS_ERR_NOT_FOUND);
        return NULL;
    }
    else
        return &(tunnelfs_srv_parts[idx].fh);
}
