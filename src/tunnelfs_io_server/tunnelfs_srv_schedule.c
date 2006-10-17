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
* File:         tunnelfs_srv_schedule.h                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include "pario_threads.h"
#include "tunnelfs_log.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_srv.h"

/*
 * Prototypes of internal functions.
 */
int tunnelfs_srv_schedule_create_domain(char *fs_domain);
int tunnelfs_srv_schedule_find_rank(char *fs_domain, int rank);
int tunnelfs_srv_schedule_find_domain(char *fs_domain);

/* 
 * Available schedules 
 */
int tunnelfs_srv_schedule_master_only(char *fs_domain);
int tunnelfs_srv_schedule_roundrobin(char *fs_domain);

/* FIXME: mpe also has a string.h, but that doesn't define strnlen !!! */
extern size_t strnlen(__const char *__string, size_t __maxlen);

/**
 * Tunnelfs Schedule record
 */
typedef struct
{
    int *server;                    /**< list of servers in schedule */
    int size;                       /**< size of server list */
    int last;                       /**< last entry in server list */
    int current;                    /**< pointer to current entry in list */
    char *fs_domain;                /**< filesystem domain this schedule belongs to */
    int (*next) (char *fs_domain);  /**< pointer to scheduling function */
} tunnelfs_srv_schedule_t;

static tunnelfs_srv_schedule_t *server_schedules = NULL;
static int size = 0;
static int last = -1;

static pthread_mutex_t tunnelfs_srv_schedule_mutex =
    PTHREAD_MUTEX_INITIALIZER;

/**
 * Insert server into a specific schedule.
 * @param fs_domain string identifier for filesystem domain
 * @param rank rank of server in respect to TUNNELFS_COMM_WORLD
 */
void tunnelfs_srv_schedule_insert(char *fs_domain, int rank)
{
    int didx = -1;
    int sidx = -1;

    assert(fs_domain != NULL);
    assert(rank >= 0);

    pthread_mutex_lock(&tunnelfs_srv_schedule_mutex);

    if ((didx = tunnelfs_srv_schedule_find_domain(fs_domain)) == -1)
        didx = tunnelfs_srv_schedule_create_domain(fs_domain);

    if ((sidx = tunnelfs_srv_schedule_find_rank(fs_domain, rank)) == -1)
    {
        if ((server_schedules[didx].server == NULL) ||
            (server_schedules[didx].size == server_schedules[didx].last + 1))
        {
            server_schedules[didx].size += 10;
            server_schedules[didx].server =
                realloc(server_schedules[didx].server,
                        server_schedules[didx].size * sizeof(int));
        }

        assert(server_schedules[didx].server != NULL);
        assert(server_schedules[didx].size > 0);
        assert(server_schedules[didx].last >= -1);
        assert(server_schedules[didx].last + 1 < server_schedules[didx].size);

        sidx = ++server_schedules[didx].last;
        server_schedules[didx].server[sidx] = rank;
    }

    LOG("Rank %i inserted into schedule for domain %s",
        server_schedules[didx].server[sidx], fs_domain);

    pthread_mutex_unlock(&tunnelfs_srv_schedule_mutex);
}

/**
 * Create a schedule for a filesystem domain
 * @param fs_domain string identifier for filesystem domain
 * @return index of filesystem domain in global array
 */
int tunnelfs_srv_schedule_create_domain(char *fs_domain)
{
    int old_size = 0;
    int idx = -1;

    /* IMPORTANT: lock must be aquired before calling this function! Seperate
     * locking was not implemented to ensure atomic operation of querying and
     * creating a desired fs_domain-schedule! */

    if ((server_schedules == NULL) || (size == last + 1))
    {
        old_size = size;
        size += 10;
        server_schedules = realloc(server_schedules,
                                   size * sizeof(tunnelfs_srv_schedule_t));

        assert(server_schedules != NULL);
        assert(size > 0);

        /* memory is allocated, now initialize new entries */
        for (idx = old_size; idx < size; idx++)
        {
            server_schedules[idx].server = NULL;
            server_schedules[idx].size = 0;
            server_schedules[idx].last = -1;
            server_schedules[idx].current = 0;
            server_schedules[idx].fs_domain = NULL;
            server_schedules[idx].next = &tunnelfs_srv_schedule_roundrobin;
        }
    }

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);
    assert(idx == -1);

    assert(last >= -1);
    assert(last + 1 < size);
    last++;

    ALLOC(server_schedules[last].fs_domain, strnlen(fs_domain, 513));
    STRNCPY(server_schedules[last].
            fs_domain, fs_domain, strnlen(fs_domain, 512));

    LOG("Created schedule for domain: %s", server_schedules[last].fs_domain);

    return last;
}

/**
 * Find index of a server in a given filesystem domain
 * @param fs_domain string identifier for filesystem domain
 * @param rank rank of server in respect to TUNNELFS_COMM_WORLD
 * @return index of server in schedule or -1 if not found
 */
int tunnelfs_srv_schedule_find_rank(char *fs_domain, int rank)
{
    int idx = -1;
    int i = 0;

    /* IMPORTANT: lock must be aquired before calling this function! Seperate
     * locking was not implemented to ensure atomic operation of querying and
     * creating a desired fs_domain-schedule! */

    assert(fs_domain != NULL);
    assert(rank >= 0);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);

    assert(idx != -1);
    if (server_schedules[idx].server == NULL)
        return -1;

    assert(server_schedules[idx].server != NULL);
    assert(server_schedules[idx].size > 0);

    while ((i <= server_schedules[idx].last) &&
           (server_schedules[idx].server[i] != rank))
        i++;

    if (i > server_schedules[idx].last)
        return -1;
    else
        return i;
}

/**
 * Find index of of filesystem domain in global schedule stucture
 * @param fs_domain string identifier of filesystem domain
 * @return index of filesystem domain in global schedule structure or -1 if not
 *         found
 */
int tunnelfs_srv_schedule_find_domain(char *fs_domain)
{
    int idx = 0;

    /* IMPORTANT: lock must be aquired before calling this function! Seperate
     * locking was not implemented to ensure atomic operation of querying and
     * creating a desired fs_domain-schedule! */

    assert(fs_domain != NULL);
    assert(size >= 0);
    assert(last >= -1);

    while ((idx <= last) &&
           (strncmp(fs_domain, server_schedules[idx].fs_domain,
                    strlen(server_schedules[idx].fs_domain)) != 0))
        idx++;

    if (idx > last)
        return -1;
    else
        return idx;
}

/**
 * Get current server from a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @return rank of server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_current(char *fs_domain)
{
    int idx = -1;
    int current_rank = -1;

    assert(fs_domain != NULL);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_schedule_mutex);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);

    assert(idx != -1);
    assert(server_schedules[idx].server != NULL);
    assert(server_schedules[idx].size > 0);
    assert(server_schedules[idx].last >= 0);
    assert(server_schedules[idx].last < server_schedules[idx].size);
    LOG("Schedule: %s Current:%i Last:%i", fs_domain,
        server_schedules[idx].current, server_schedules[idx].last);
    assert(server_schedules[idx].current <= server_schedules[idx].last);

    current_rank =
        server_schedules[idx].server[server_schedules[idx].current];

    assert(current_rank >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_schedule_mutex);

    return current_rank;
}

/**
 * Get next server from a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @return rank of server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_next(char *fs_domain)
{
    int idx = -1;
    int next_rank = TUNNELFS_GLOBAL_MASTER;

    assert(fs_domain != NULL);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);
    assert(last < size);

    pthread_mutex_lock(&tunnelfs_srv_schedule_mutex);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);
    assert(idx != -1);
    assert(server_schedules[idx].next != NULL);

    next_rank = (server_schedules[idx].next) (fs_domain);

    pthread_mutex_unlock(&tunnelfs_srv_schedule_mutex);

    return next_rank;
}

/**
 * Get list of servers in a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @param server pointer (preferably NULL) that will hold the list of servers
 * @param n pointer to integer that will hold the size of the server list
 */
void tunnelfs_srv_schedule_get_servers(char *fs_domain, int **server, int *n)
{
    int idx = -1;

    assert(fs_domain != NULL);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_schedule_mutex);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);
    assert(idx != -1);
    assert(server_schedules[idx].server != NULL);
    assert(server_schedules[idx].last >= 0);

    FREE(*server);
    *n = server_schedules[idx].last + 1;
    ALLOC(*server, *n * sizeof(int));
    memcpy(*server, server_schedules[idx].server, *n * sizeof(int));

    pthread_mutex_unlock(&tunnelfs_srv_schedule_mutex);
}

/**
 * Get list of servers excluding itself in a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @param excl_rank server to exclude
 * @param server pointer (preferably NULL) that will hold the list of servers
 * @param n pointer to integer that will hold the size of the server list
 */
void tunnelfs_srv_schedule_get_servers_excl(char *fs_domain, int excl_rank,
                                            int **server, int *n)
{
    int idx = -1;
    int i;

    assert(fs_domain != NULL);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);

    pthread_mutex_lock(&tunnelfs_srv_schedule_mutex);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);
    assert(idx != -1);
    assert(server_schedules[idx].server != NULL);
    assert(server_schedules[idx].last >= 0);

    FREE(*server);
    /* 
     * we waste one integer but save time to check if supplied server is
     * actually in the server list at all
     */
    *n = server_schedules[idx].last + 1;
    ALLOC(*server, *n * sizeof(int));

    for (i = 0; i <= server_schedules[idx].last; i++)
    {
        if ((server_schedules[idx].server)[i] != excl_rank)
            (*server)[i] = server_schedules[idx].server[i];
    }

    pthread_mutex_unlock(&tunnelfs_srv_schedule_mutex);
}

/**
 * Schedule function for round robin scheduling
 * @param fs_domain string identifier for filesystem domain
 * @return rank of server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_roundrobin(char *fs_domain)
{
    int idx = -1;
    int next_rank = -1;

    /* IMPORTANT: Pthread locking is done in the wrapping function
     * tunnelfs_srv_schedule_next() */
    assert(fs_domain != NULL);
    assert(server_schedules != NULL);
    assert(size > 0);
    assert(last >= 0);
    assert(last < size);

    idx = tunnelfs_srv_schedule_find_domain(fs_domain);
    assert(idx != -1);
    assert(server_schedules[idx].server != NULL);

    server_schedules[idx].current += 1;
    server_schedules[idx].current %= (server_schedules[idx].last + 1);
    next_rank = server_schedules[idx].server[server_schedules[idx].current];

    assert(next_rank >= 0);

    return next_rank;
}

/**
 * Schedule function for no scheduling, always returning the global master
 * @param fs_domain string identifier for filesystem domain
 * @return rank of global master server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_master_only(char *fs_domain)
{
    LOG("Scheduling not enabled");
    return TUNNELFS_GLOBAL_MASTER;
}

/**
 * Auxilliary function to print a given distribution via the LOG interface.
 * @param list pointer to distribution list
 * @param list_size of distribution list to be printed
 */
void tunnelfs_srv_schedule_print_distribution(dist_list_t *list,
                                              int list_size)
{
    int i;

    assert(list != NULL);
    assert(list_size > 0);

    for (i = 0; i < list_size; i++)
    {
        LOG("client %i -> server %i", list[i].client, list[i].fileserver);
    }
}

/**
 * Get the default distribution for a given list of clients.
 * @param ranks list of client ranks to be distributed
 * @param n size of client list
 * @param list pointer (preferably NULL) to a distribution list
 */
void tunnelfs_srv_schedule_default_distribution(int *ranks, int n,
                                                dist_list_t **list)
{
    int i;

    assert(list != NULL);
    assert(ranks != NULL);

    for (i = 0; i < n; i++)
    {
        (*list)[i].client = ranks[i];
        (*list)[i].fileserver = TUNNELFS_GLOBAL_MASTER;
    }
}

/**
 * Get an even distribution for a given list of clients on a given list of
 * servers.
 * @param clients list of client ranks to be distributed
 * @param nc size of client list
 * @param servers list of server ranks to be used
 * @param ns size of server list
 * @param list pointer (preferably NULL) to a distribution list
 */
void tunnelfs_srv_schedule_balanced_distribution(int *clients, int nc,
                                                 int *servers, int ns,
                                                 dist_list_t **list)
{
    int i;

    assert(clients != NULL);
    assert(nc > 0);
    assert(servers != NULL);
    assert(ns > 0);

    ALLOC(*list, nc * sizeof(dist_list_t));
    assert(*list != NULL);

    for (i = 0; i < nc; i++)
    {
        (*list)[i].client = clients[i];
        (*list)[i].fileserver = servers[i % ns];
    }
}

/**
 * Get a distribution where all clients have servers in their filesystem domain
 * @param clients list of client ranks to be distributed
 * @param nc size of client list
 * @param list pointer (preferably NULL) to a distribution list
 */
void tunnelfs_srv_schedule_domain_distribution(int *clients, int nc,
                                               dist_list_t **list)
{
    int i;

    assert(clients != NULL);
    assert(nc > 0);

    ALLOC(*list, nc * sizeof(dist_list_t));
    assert(*list != NULL);

    for (i = 0; i < nc; i++)
    {
        (*list)[i].client = clients[i];
        /* check if server is available for client domain */
        if (tunnelfs_srv_schedule_find_domain
            (tunnelfs_srv_client_fs_domain(clients[i])) != -1)
            (*list)[i].fileserver =
                tunnelfs_srv_schedule_roundrobin(tunnelfs_srv_client_fs_domain
                                                 (clients[i]));
        else
            (*list)[i].fileserver =
                tunnelfs_srv_schedule_roundrobin(tunnelfs_srv_get_fs_domain
                                                 (TUNNELFS_GLOBAL_MASTER));
    }
}
