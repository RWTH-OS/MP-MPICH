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
* File:         ad_tunnelfs_server.c                                      * 
* Description:  Server control for tunnelfs client side                   * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"

static void tunnelfs_server_check_fileservers();
static int tunnelfs_server_find_fileserver(int file_id);

/**
 * Correlation structure for internal file ids to file servers
 */
typedef struct
{
    int file_id;
    int rank;
}
tunnelfs_fileserver_t;

static tunnelfs_fileserver_t *tunnelfs_fileservers = NULL;
static int tunnelfs_fileservers_size = 0;
static int tunnelfs_fileservers_last = -1;

static int tunnelfs_local_master = 0;

/**
 * determine the global master io server for a new request 
 * @return rank of global master tunnelfs server
 */
int tunnelfs_server_get_global_master()
{
    return TUNNELFS_GLOBAL_MASTER;
}

/** 
 * determine the master io server for a given file_id 
 * @return rank of master tunnelfs server for the given file
 */
int tunnelfs_server_get_file_master(int file_id)
{
    int idx;

    if ((idx = tunnelfs_server_find_fileserver(file_id)) != -1)
        return tunnelfs_fileservers[idx].rank;
    else
    {
        tunnelfs_server_set_file_master(file_id,
                                        tunnelfs_server_get_global_master());
        return tunnelfs_server_get_global_master();
    }
}

/**
 * set new file master io server for a client process 
 * @param file_id internal file id
 * @param master rank of master fileserver for this file
 */
void tunnelfs_server_set_file_master(int file_id, int master)
{
    int idx;

    tunnelfs_server_check_fileservers();

    if ((idx = tunnelfs_server_find_fileserver(file_id)) != -1)
    {
        tunnelfs_fileservers[idx].rank = master;
    }
    else
    {
        tunnelfs_fileservers_last++;
        tunnelfs_fileservers[tunnelfs_fileservers_last].rank = master;
        tunnelfs_fileservers[tunnelfs_fileservers_last].file_id = file_id;
    }
}

/**
 * Unset new file master io server for a client process 
 * @param file_id internal file id
 */
void tunnelfs_server_unset_file_master(int file_id)
{
    int idx;

    tunnelfs_server_check_fileservers();

    if ((idx = tunnelfs_server_find_fileserver(file_id)) != -1)
    {
        tunnelfs_fileservers[idx].file_id =
            tunnelfs_fileservers[tunnelfs_fileservers_last].file_id;
        tunnelfs_fileservers[idx].rank =
            tunnelfs_fileservers[tunnelfs_fileservers_last].rank;
        tunnelfs_fileservers_last--;
    }
}

/**
 * Check if fileserver structure has sufficient space for a new entry
 */
static void tunnelfs_server_check_fileservers()
{
    if ((tunnelfs_fileservers == NULL) ||
        (tunnelfs_fileservers_size == 0) ||
        (tunnelfs_fileservers_size == tunnelfs_fileservers_last + 1))
    {
        tunnelfs_fileservers_size += 10;
        tunnelfs_fileservers = realloc(tunnelfs_fileservers,
                                       tunnelfs_fileservers_size *
                                       sizeof(tunnelfs_fileserver_t));
        if (tunnelfs_fileservers == NULL)
            ERR(TUNNELFS_ERR_ALLOC);
    }
}

/**
 * Get index for a fileserver entry for a given file
 * @param file_id internal file id
 * @return index of entry in internal list
 */
static int tunnelfs_server_find_fileserver(int file_id)
{
    int idx = 0;

    tunnelfs_server_check_fileservers();

    while ((idx <= tunnelfs_fileservers_last) &&
           (idx < tunnelfs_fileservers_size) &&
           (tunnelfs_fileservers[idx].file_id != file_id))
        idx++;

    if (idx > tunnelfs_fileservers_last)
        return -1;
    else
        return idx;
}
