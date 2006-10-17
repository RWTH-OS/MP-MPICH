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
* File:         tunnelfs_srv_sync.h                                       * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Synchronization with other tunnelfs io servers            *
*                                                                         *
**************************************************************************/
#ifndef TUNNELFS_SRV_SYNC_H
#define TUNNELFS_SRV_SYNC_H

/**
 * Send shutdown message to slave servers
 */
void tunnelfs_srv_shutdown_slaves();
/**
 * Wait for pending actions to finish
 */
void tunnelfs_srv_shutdown_close_pending();
/**
 * Initialize list of servers and filesystem domains
 */
void tunnelfs_srv_init_serverlist();
/**
 * Retrieve list of available io servers
 * @param list Address of Pointer to a list of integers. The pointer must be in
 *        a consistent state, either NULL or pointing to allocated memory that
 *        will be freed in the process.
 * @param count Address of integer to hold the number of ranks in the returned
 *        list.
 */
void tunnelfs_srv_get_serverlist(int **list, int *count);
/**
 * Get the filesystem domain defined by the metahost prefix for the local server
 * @return Null-terminated string containing the filesystem domain. 
 */
char *tunnelfs_srv_local_fs_domain();
/**
 * Get the filesystem domain defined by the metahost prefix of a remote server
 * @param rank rank of sever in respect to TUNNELFS_COMM_WORLD
 * @return Null-terminated string containing the filesystem domain. 
 */
char *tunnelfs_srv_get_fs_domain(int rank);
/**
 * Get a server for a filesystem domain
 * @param fs_domain Queried filesystem domain
 * @return rank of corresponding server in TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_fs_domain_server(char *fs_domain);
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
                                    **fs_domain, char **file);
/**
 * Return filesystem domain for a specific client
 * @param rank rank of client in TUNNELFS_COMM_WORLD
 * @return Null-terminated string identifier for client filesystem domain
 */
char *tunnelfs_srv_client_fs_domain(int rank);
/**
 * Insert client into client filesystem domain list
 * @param rank rank of client in TUNNELFS_COMM_WORLD
 * @param domain String identifier of client filesystem domain
 */
void tunnelfs_srv_client_fs_domain_insert(int rank, char *domain);
/**
 * Initialize mapping structure between communicators
 */
void tunnelfs_srv_init_map();
/**
 * Get the rank of process in MPI_COMM_META_REDUCED for provided rank in
 * TUNNELFS_COMM_WORLD
 * @param rank rank of MPI process in TUNNELFS_COMM_WORLD
 * @return corresponding rank of MPI process in MPI_COMM_META_REDUCED
 */
int tunnelfs_srv_map_w2r(int rank);
/**
 * Get the rank of process in TUNNELFS_COMM_WORLD for provided rank in
 * MPI_COMM_META_REDUCED
 * @param rank rank of MPI process in MPI_COMM_META_REDUCED
 * @return corresponding rank of MPI process in TUNNELFS_COMM_WORLD
 */
int tunnelfs_srv_map_r2w(int rank);
#endif
