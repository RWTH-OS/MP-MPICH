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
* File:         tunnelfs_srv_schedule.c                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_SCHEDULE_H
#define TUNNELFS_SRV_SCHEDULE_H

/**
 * Insert server into a specific schedule.
 * @param fs_domain string identifier for filesystem domain
 * @param rank rank of server in respect to TUNNELFS_COMM_WORLD
 */
void tunnelfs_srv_schedule_insert(char *fs_domain, int rank);
/**
 * Get current server from a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @return rank of server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_current(char *fs_domain);
/**
 * Get next server from a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @return rank of server in respect to TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_schedule_next(char *fs_domain);
/**
 * Get list of servers in a given schedule
 * @param string identifier of filesystem domain
 * @param server pointer (preferably NULL) that will hold the list of servers
 * @param n pointer to integer that will hold the size of the server list
 */
void tunnelfs_srv_schedule_get_servers(char *fs_domain, int **server, int *n);
/**
 * Get list of servers excluding itself in a given schedule
 * @param fs_domain string identifier of filesystem domain
 * @param excl_rank server to exclude from list
 * @param server pointer (preferably NULL) that will hold the list of servers
 * @param n pointer to integer that will hold the size of the server list
 */
void tunnelfs_srv_schedule_get_servers_excl(char *fs_domain, int excl_rank,
                                            int **server, int *n);
/**
 * Auxilliary function to print a given distribution via the LOG interface.
 * @param list pointer to distribution list
 * @param size of distribution list to be printed
 */
void tunnelfs_srv_schedule_print_distribution(dist_list_t *list, int size);
/**
 * Get the default distribution for a given list of clients.
 * @param ranks list of client ranks to be distributed
 * @param n size of client list
 * @param list pointer (preferably NULL) to a distribution list
 */
void tunnelfs_srv_schedule_default_distribution(int *ranks, int n,
                                                dist_list_t **list);
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
                                                 dist_list_t **list);
/**
 * Get a distribution where all clients have servers in their filesystem domain
 * @param clients list of client ranks to be distributed
 * @param nc size of client list
 * @param list pointer (preferably NULL) to a distribution list
 */
void tunnelfs_srv_schedule_domain_distribution(int *clients, int nc,
                                               dist_list_t **list);
#endif
