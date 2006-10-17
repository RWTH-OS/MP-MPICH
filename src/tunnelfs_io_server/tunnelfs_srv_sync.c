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
* File:         tunnelfs_srv_sync.c                                       * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Synchronization with other tunnelfs io servers            *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "mpi.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_srv.h"
#include "tunnelfs_log.h"
#include "metaconfig.h"

/**
 * Correlation of process rank and filesystem domain
 */
typedef struct
{
    int rank;                   /* rank of server */
    char fs_domain[TUNNELFS_MAX_FS_DOMAINLEN];  /* filesystem domain */
}
tunnelfs_domain_info_t;

/* variables for server domain management */
/**
 * POSIX thread mutex for server schedule access
 */
static pthread_mutex_t tunnelfs_srv_server_sync_mutex =
    PTHREAD_MUTEX_INITIALIZER;
/**
 * list of domains and their servers
 */
static tunnelfs_domain_info_t *tunnelfs_servers = NULL;
static int servers_size = 0;
static int servers_last = -1;

/* variables for client domain management */
/**
 * POSIX thread mutex for client domain access
 */
static pthread_mutex_t tunnelfs_srv_client_sync_mutex =
    PTHREAD_MUTEX_INITIALIZER;
/**
 * list of clients and their filesystem domain
 */
static tunnelfs_domain_info_t *tunnelfs_clients = NULL;
static int clients_size = 0;
static int clients_last = -1;

extern MPIR_MetaConfig MPIR_meta_cfg;

/* FIXME: Weird, our unistd.h does not seem to know gethostname, therefor I
 * explicitly give the prototype here */
int gethostname(char *name, size_t len);

/**
 * Send shutdown message to slave servers
 */
void tunnelfs_srv_shutdown_slaves()
{
    int msg_id;
    int my_rank;
    int i;

    assert(tunnelfs_servers != NULL);
    assert(servers_size > 0);
    assert(servers_last >= 0);
    assert(servers_last < servers_size);

    LOCK_MPI();
    MPI_Comm_rank(MPI_COMM_META_REDUCED, &my_rank);
    UNLOCK_MPI();

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);

    for (i = 0; i <= servers_last; i++)
    {
        if (i != my_rank)
        {
            msg_id = TUNNELFS_NEXT_MSG_ID;
            ptMPI_Send(&msg_id, 1, MPI_INT, tunnelfs_servers[i].rank,
                       TUNNELFS_SERVER_SHUTDOWN, TUNNELFS_COMM_WORLD);
        }
    }

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);
}

/**
 * Wait for pending actions to finish
 */
void tunnelfs_srv_shutdown_close_pending()
{
    /* TODO:
     * check if any other messages are pending or operations have to be
     * finished, before the server can stop listening for messages */
}

/**
 * Initialize list of servers and filesystem domains
 */
void tunnelfs_srv_init_serverlist()
{
    int serverlist_size;

    int i;
    int rank_in_server;
    int rank_in_world;

    LOCK_MPI();
    MPI_Comm_size(MPI_COMM_META_REDUCED, &serverlist_size);
    UNLOCK_MPI();

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);
    ALLOC(tunnelfs_servers, serverlist_size * sizeof(tunnelfs_domain_info_t));

    LOCK_MPI();
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank_in_world);
    MPI_Comm_rank(MPI_COMM_META_REDUCED, &rank_in_server);
    UNLOCK_MPI();
    tunnelfs_servers[rank_in_server].rank = rank_in_world;

    /*  trunc global metahost number from name */
    for (i = strlen(MPIR_meta_cfg.my_metahostname) - 1; i >= 0; i--)
        if (MPIR_meta_cfg.my_metahostname[i] == '_')
            break;
    if ((i <= 0) || (i > TUNNELFS_MAX_FS_DOMAINLEN - 1))
        i = TUNNELFS_MAX_FS_DOMAINLEN - 1;

    STRNCPY(tunnelfs_servers[rank_in_server].fs_domain,
            MPIR_meta_cfg.my_metahostname, i);
    tunnelfs_servers[rank_in_server].fs_domain[i] = '\0';

    /* clear rest of string */
    for (; i < TUNNELFS_MAX_FS_DOMAINLEN; i++);
    tunnelfs_servers[rank_in_server].fs_domain[i] = '\0';

    /*
     * Insert hostname as filesystem domain
     * INFO: Be careful that a metahost definition does not accidentally match
     * a hostname, or the domains will get mixed up.
     */
#if 0
    for (i = 0; i < serverlist_size; i++)
    {
        int broadcasted_rank;

        if (i == rank_in_server)
        {
            broadcasted_rank = rank_in_world;
            gethostname(hostname, 255);
            hostname[255] = '\0';
            LOG("Local hostname is %s", hostname);
        }

        LOCK_MPI();
        MPI_Bcast(&broadcasted_rank, 1, MPI_INT, i, MPI_COMM_META_REDUCED);
        MPI_Bcast(hostname, TUNNELFS_MAX_FS_DOMAINLEN,
                  MPI_CHAR, i, MPI_COMM_META_REDUCED);
        UNLOCK_MPI();

        /* creating scheduling entry for server */
        tunnelfs_srv_schedule_insert(hostname, broadcasted_rank);
    }
#endif
    /*
     * Insert metahost prefix as filesystem domain
     */
    LOG("Local filesystem domain: %s",
        tunnelfs_servers[rank_in_server].fs_domain);

    for (i = 0; i < serverlist_size; i++)
    {
        LOG("Synchronizing server list entry");

        LOCK_MPI();
        MPI_Bcast(&(tunnelfs_servers[i].rank), 1, MPI_INT, i,
                  MPI_COMM_META_REDUCED);
        MPI_Bcast(tunnelfs_servers[i].fs_domain, TUNNELFS_MAX_FS_DOMAINLEN,
                  MPI_CHAR, i, MPI_COMM_META_REDUCED);
        UNLOCK_MPI();

        /* creating scheduling entry for server */
        tunnelfs_srv_schedule_insert(tunnelfs_servers[i].fs_domain,
                                     tunnelfs_servers[i].rank);
    }

    servers_size = serverlist_size;
    servers_last = serverlist_size - 1;

    assert(tunnelfs_servers != NULL);
    assert(servers_size > 0);
    assert(servers_last >= 0);
    assert(servers_last < servers_size);

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);
}

/**
 * Retrieve list of available io servers
 * @param list Address of Pointer to a list of integers. The pointer must be in
 *        a consistent state, either NULL or pointing to allocated memory that
 *        will be freed in the process.
 * @param count Address of integer to hold the number of ranks in the returned
 *        list.
 */
void tunnelfs_srv_get_serverlist(int **list, int *count)
{
    int idx;

    assert(tunnelfs_servers != NULL);
    assert(servers_size > 0);
    assert(servers_last >= 0);
    assert(servers_last < servers_size);

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);

    *count = servers_last + 1;

    ALLOC(*list, *count * sizeof(int));
    assert(*list != NULL);

    for (idx = 0; idx <= servers_last; idx++)
        (*list)[idx] = tunnelfs_servers[idx].rank;

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);
}

/**
 * Get the filesystem domain defined by the metahost prefix for the local server
 * @return Null-terminated string containing the filesystem domain. 
 */
char *tunnelfs_srv_local_fs_domain()
{
    int rank;
    int idx = 0;

    assert(tunnelfs_servers != NULL);
    assert(servers_size > 0);
    assert(servers_last >= 0);
    assert(servers_last < servers_size);

    LOCK_MPI();
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank);
    UNLOCK_MPI();

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);
    while ((idx <= servers_last) && (tunnelfs_servers[idx].rank != rank))
        idx++;

    assert(idx <= servers_last);

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);

    return tunnelfs_servers[idx].fs_domain;
}

/**
 * Insert client into client filesystem domain list
 * @param rank rank of client in TUNNELFS_COMM_WORLD
 * @param domain String identifier of client filesystem domain
 */
void tunnelfs_srv_client_fs_domain_insert(int rank, char *domain)
{
    assert(rank >= 0);
    assert(domain != NULL);

    pthread_mutex_lock(&tunnelfs_srv_client_sync_mutex);

    if ((tunnelfs_clients == NULL) || (clients_size == clients_last + 1))
    {
        /* allocate more space */
        clients_size += 10;
        tunnelfs_clients = realloc(tunnelfs_clients, clients_size *
                                   sizeof(tunnelfs_domain_info_t));
    }

    assert(tunnelfs_clients != NULL);
    assert(clients_size > 0);
    assert(clients_last + 1 < clients_size);

    clients_last++;
    tunnelfs_clients[clients_last].rank = rank;
    STRNCPY(tunnelfs_clients[clients_last].fs_domain, domain,
            TUNNELFS_MAX_FS_DOMAINLEN);

    pthread_mutex_unlock(&tunnelfs_srv_client_sync_mutex);
}

/**
 * Return filesystem domain for a specific client
 * @param rank rank of client in TUNNELFS_COMM_WORLD
 * @return Null-terminated string identifier for client filesystem domain
 */
char *tunnelfs_srv_client_fs_domain(int rank)
{
    int idx = 0;

    assert(rank >= 0);
    assert(tunnelfs_clients != NULL);
    assert(clients_last >= 0);
    assert(clients_size > 0);

    pthread_mutex_lock(&tunnelfs_srv_client_sync_mutex);

    while ((idx <= clients_last) && (tunnelfs_clients[idx].rank != rank))
        idx++;

    assert(idx <= clients_last);

    pthread_mutex_unlock(&tunnelfs_srv_client_sync_mutex);

    return tunnelfs_clients[idx].fs_domain;
}

/**
 * Get the filesystem domain defined by the metahost prefix of a remote server
 * @param rank rank of sever in respect to TUNNELFS_COMM_WORLD
 * @return Null-terminated string containing the filesystem domain. 
 */
char *tunnelfs_srv_get_fs_domain(int rank)
{
    int idx = 0;

    assert(rank >= 0);
    assert(tunnelfs_servers != NULL);
    assert(servers_last >= 0);
    assert(servers_size > 0);

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);

    while ((idx <= servers_last) && (tunnelfs_servers[idx].rank != rank))
        idx++;

    assert(idx <= servers_last);

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);

    return tunnelfs_servers[idx].fs_domain;
}

/**
 * Get a server for a filesystem domain
 * @param fs_domain Queried filesystem domain
 * @return rank of corresponding server in TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_fs_domain_server(char *fs_domain)
{
    int idx = 0;

    assert(fs_domain != NULL);
    assert(tunnelfs_servers != NULL);
    assert(servers_last >= 0);
    assert(servers_size > 0);

    pthread_mutex_lock(&tunnelfs_srv_server_sync_mutex);

    while ((idx <= servers_last) &&
           (strcmp(tunnelfs_servers[idx].fs_domain, fs_domain) != 0))
        idx++;


    if (idx > servers_last)
        idx = -1;

    pthread_mutex_unlock(&tunnelfs_srv_server_sync_mutex);

    return tunnelfs_servers[idx].rank;
}

/**
 * Tokenize a filename into the parts of filesyste, filesystem domain, and
 * filename, each seperated by a colon.
 * @param filename Provided filename to be analyzed
 * @param filesystem Returned string identifier for filesystem. Needs to be NULL
 *        of pointing to allocated memory that will be freed in the process.
 * @param fs_domain Returned string identifier for filesystem domain. Needs to
 *        be NULL of pointing to allocated memory that will be freed in the 
 *        process.
 * @param file Returned string identifier for filename with truncated
 *        filesystem and filesystem domain identifiers. Needs to
 *        be NULL of pointing to allocated memory that will be freed in the 
 *        process.
 */
void tunnelfs_srv_tokenize_filename(char *filename, char **filesystem, char
                                    **fs_domain, char **file)
{
    char *tmp_filename = NULL;
    char *rest = NULL;
    char *rest2 = NULL;
    int length = 0;

    assert(filename != NULL);

    ALLOC(tmp_filename, (strlen(filename) + 1));
    strcpy(tmp_filename, filename);

    rest = strchr(tmp_filename, (int) ':');
    if (rest == NULL)
    {
        /* filename has no further entries separated with ':',
         * therefore we assume default filesystem domain and
         * default filesystem */

        LOG("No special prefixes found. Using defaults.");

        ALLOC(*filesystem, 8);
        STRNCPY(*filesystem, "default", 7);

        ALLOC(*fs_domain, strlen(tunnelfs_srv_local_fs_domain()) + 1);
        STRNCPY(*fs_domain, tunnelfs_srv_local_fs_domain(),
                strlen(tunnelfs_srv_local_fs_domain()) + 1);

        ALLOC(*file, (strlen(filename) + 1));
        STRNCPY(*file, filename, (strlen(filename) + 1));
    }
    else
    {
        /* filesystem and/or filesystem domain are specified */
        length = rest - tmp_filename;

        ALLOC(*filesystem, (length + 1));
        STRNCPY(*filesystem, tmp_filename, length);

        if ((strncmp(*filesystem, "ufs", 3)   == 0) ||
            (strncmp(*filesystem, "nfs", 3)   == 0) ||
            (strncmp(*filesystem, "pvfs", 4)  == 0) ||
            (strncmp(*filesystem, "pvfs2", 5) == 0) ||
            (strncmp(*filesystem, "piofs", 5) == 0) ||
            (strncmp(*filesystem, "pfs", 3)   == 0) ||
            (strncmp(*filesystem, "memfs", 5) == 0))
        {
            /* we guessed correctly and now we just have to set the rest
             * accordingly */

            LOG("Prefix for filesystem found: %s", *filesystem);

            /* fs_domain is assumed to be the local one */
            ALLOC(*fs_domain, strlen(tunnelfs_srv_local_fs_domain()) + 1);
            strcpy(*fs_domain, tunnelfs_srv_local_fs_domain());

            /* the rest of the string is the filename */
            ALLOC(*file, strlen(rest + 1) + 1);
            STRNCPY(*file, (rest + 1), strlen(rest));
        }
        else
        {
            /* false guess, fs domain was given */
            *fs_domain = *filesystem;
            *filesystem = NULL;

            /* get next token */
            rest2 = strchr((rest + 1), (int) ':');
            if (rest2 == NULL)
            {
                /* no ':' found */

                LOG("Filesystem domain found, but no filesystem specified.");
                
                /* set filesystem to default */
                ALLOC(*filesystem, 8);
                strcpy(*filesystem, "default");

                /* copy filename */
                length = strlen((rest + 1));
                ALLOC(*file, length + 1);
                STRNCPY(*file, (rest + 1), length);

            }
            else
            {
                length = rest2 - (rest + 1);

                ALLOC(*filesystem, length + 1);
                STRNCPY(*filesystem, (rest + 1), length);

                if ((strncmp(*filesystem, "ufs", 3) == 0) ||
                    (strncmp(*filesystem, "nfs", 3) == 0) ||
                    (strncmp(*filesystem, "pvfs", 4) == 0) ||
                    (strncmp(*filesystem, "pvfs2", 5) == 0) ||
                    (strncmp(*filesystem, "piofs", 5) == 0) ||
                    (strncmp(*filesystem, "pfs", 3) == 0) ||
                    (strncmp(*filesystem, "memfs", 5) == 0))
                {
                    /* we guessed correctly and now we just have to set the rest
                     * accordingly */
                    
                    LOG("Filesystem domain and filesystem found.");
                    
                    ALLOC(*file, strlen(rest2 + 1) + 1);
                    STRNCPY(*file, (rest2 + 1), strlen(rest2 + 1));
                }
                else
                {
                    LOG("Unknown filesystem \"%s\" specified!\n",
                        *filesystem);
                }

                length = strlen((rest2 + 1));
                ALLOC(*file, length + 1);
                STRNCPY(*file, (rest2 + 1), length);
            }
        }
    }

    FREE(tmp_filename);
}
