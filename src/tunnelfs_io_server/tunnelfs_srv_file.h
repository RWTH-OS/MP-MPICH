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
* File:         tunnelfs_file.h                                           * 
* Description:  File handling between client and server                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_FILE_H
#define TUNNELFS_SRV_FILE_H

#include "mpi.h"
#include "ad_tunnelfs_globals.h"

extern int tunnelfs_file_id;
#define TUNNELFS_NEXT_FILE_ID ++tunnelfs_file_id
#define TUNNELFS_CURR_FILE_ID tunnelfs_file_id

/**
 * Creating a tunnelfs handle object.
 * @param file_id tunnelfs id to reference this handle
 * @param comm_id tunnelfs id to reference the communicator associated with
 *        this handle
 * @return Pointer to MPI file handle entry within the created tunnelfs handle
 */
MPI_File *tunnelfs_srv_file_create_handle(int file_id, int comm_id);

/**
 * Deleting a tunnelfs handle object
 * @param file_id tunnelfs id to reference this handle
 */
void tunnelfs_srv_file_free_handle(int file_id);

/**
 * Check if a file handle exists for a file id
 * @param file_id ID of file handle
 * @return
 *      - 1: file handle exists
 *      - 0: file handle does not exist
 */
int tunnelfs_srv_file_handle_exists(int file_id);

/**
 * Getting MPI handle for tunnelfs file handle id
 * @param file_id tunnelfs id to reference the file handle
 * @return Pointer to MPI file handle entry within the created tunnelfs handle
 */
MPI_File *tunnelfs_srv_file_get_handle(int file_id);

/**
 * Getting tunnelfs id of the communicator associated with file_id
 * @param file_id tunnelfs id to reference the file handle
 * @return communicator id within tunnelfs
 */
int tunnelfs_srv_file_get_comm_id(int file_id);

/**
 * Define filetype access for later querying and setting
 * @param file_id file id this info is associated with
 * @param access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
void tunnelfs_srv_file_def_filetype_access(int file_id, int access);

/**
 * Set filetype access on file handle
 * @param file_id file id this info is associated with
 * @param access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
void tunnelfs_srv_file_set_filetype_access(int file_id, int access);

/**
 * Get filetype access of file handle
 * @param file_id file id this info is associated with
 * @return access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
int tunnelfs_srv_file_get_filetype_access(int file_id);

/**
 * Set mutual status on a file
 * @param file_id ID of file handle
 * @param status status of file handle
 */
void tunnelfs_srv_file_set_mutual(int file_id, int status);

/**
 * Check if all servers have this handle
 * @param file_id ID of file handle
 * @return
 *      - 0: file is on local server only
 *      - 1: file is on all servers
 */
int tunnelfs_srv_file_is_mutual(int file_id);

/**
 * Mark/unmark a file handle as a clone
 * Cloned file handles are used for parallel direct access, a la memfs
 * @param file_id ID of file handle
 * @param status Clone status of file handle
 */
void tunnelfs_srv_file_set_clone(int file_id, int status);

/**
 * Query clone status of a given file handle
 * @param file_id ID of file handle
 * @return Clone status of file handle
 */
int tunnelfs_srv_file_is_clone(int file_id);

/**
 * Mark/unmark a file handle as a opened
 * @param file_id ID of file handle
 * @param status Open/Close status of file handle
 */
void tunnelfs_srv_file_set_opened(int file_id, int status);

/**
 * Query Open/Close status of a given file handle
 * @param file_id ID of file handle
 * @return Open/Close status of file handle
 */
int tunnelfs_srv_file_is_opened(int file_id);

/**
 * Set client that gets reply for collective requests
 * @param file_id ID of file handle
 * @param client_id Rank of client
 */
void tunnelfs_srv_file_set_main_client(int file_id, int client_id);

/**
 * Get client that gets reply for collective requests
 * @param file_id ID of file handle
 * @return Rank of client
 */
int tunnelfs_srv_file_get_main_client(int file_id);

/**
 * Set server that is responsible for a file handle
 * @param file_id ID of file handle
 * @param server_id Rank of main server
 */
void tunnelfs_srv_file_set_main_server(int file_id, int server_id);

/**
 * Get server that is responsible for a file handle
 * @param file_id ID of file handle
 * @return Rank of main server
 */
int tunnelfs_srv_file_get_main_server(int file_id);

/**
 * Associate a file distribution with the given file handle
 * @param file_id ID of file handle
 * @param list Reference of the distribution list
 * @param list_size Size of the distribution list
 */
void tunnelfs_srv_file_set_distribution(int file_id, dist_list_t *list,
                                        int size);

/**
 * Get the file distribution of the given file handle
 * @param file_id ID of file handle
 * @param list Reference of the distribution list
 * @param list_size Size of the distribution list
 */
void tunnelfs_srv_file_get_distribution(int file_id, dist_list_t **list,
                                        int *size);

/**
 * Get a list of all servers sharing a file handle
 * @param file_id ID of file handle
 * @param servers Reference on server list
 * @param server_size Reference on size of server list
 */
void tunnelfs_srv_file_get_serverlist(int file_id, int **servers, int *size);

/**
 * Get server for a specific client
 * @param file_id ID of file handle
 * @param client Rank of client
 * @return Rank of corresponding server
 */
int tunnelfs_srv_file_get_server(int file_id, int client);

/**
 * Get magic cookie of client operation
 * @param file_id ID of file handle
 * @return magic cookie of client operation
 */
int tunnelfs_srv_file_get_client_cookie(int file_id);

/**
 * Set filename on internal structures
 * @param file_id ID of file handle
 * @param filename Name of the file
 */
void tunnelfs_srv_file_set_name(int file_id, char *filename);

/**
 * Get filename of a file handle
 * @param file_id ID of file handle
 * @return Name of the file
 */
char *tunnelfs_srv_file_get_name(int file_id);

/**
 * Create a name for the tunnelfs cache file used in cached io
 * @param prefix Prefix for filename
 * @param rank Client rank the cache file is for
 * @param width width for implicit leading_zero call
 * @return filename of cache file
 */
char *tunnelfs_srv_file_create_cache_name(char *prefix, int rank, int width);

/**
 * Define a future server behaviour for file handling
 * @param file_id ID of file handle
 * @param behaviour Server behaviour for requests
 */
void tunnelfs_srv_file_def_behaviour(int file_id, int behaviour);

/**
 * Set the future server behaviour for file handling
 * @param file_id ID of file handle
 * @param behaviour Server behaviour for requests
 */
void tunnelfs_srv_file_set_behaviour(int file_id, int behaviour);

/**
 * Get the current server behaviour for file handling
 * @param file_id ID of file handle
 * @return Server behaviour for requests
 */
int tunnelfs_srv_file_get_behaviour(int file_id);

/**
 * Define distribution type for a file handle
 * @param file_id ID for file handle
 * @param dist_type Type of distribution
 */
void tunnelfs_srv_file_def_dist_type(int file_id, int dist_type);

/**
 * Set distribution type for a file handle
 * @param file_id ID for file handle
 * @param dist_type Type of distribution
 */
void tunnelfs_srv_file_set_dist_type(int file_id, int dist_type);

/**
 * Get distribution type for a file handle
 * @param file_id ID for file handle
 * @return Type of distribution
 */
int tunnelfs_srv_file_get_dist_type(int file_id);

/**
 * Calculate distribution of a file handle
 * @param file_id ID for file handle
 * @param clnt_msg_id Message id of the client request needed for reply
 */
void tunnelfs_srv_file_distribute(int file_id, int clnt_msg_id);

/**
 * Clone handle on all servers
 * @param file_id ID for file handle
 * @param clnt_msg_id Message id of the client request needed for reply
 */
void tunnelfs_srv_file_clone_handle(int file_id, int clnt_msg_id);

/**
 * Clone distribution on all servers
 * @param file_id ID for file handle
 */
void tunnelfs_srv_file_clone_distribution(int file_id);

#endif
