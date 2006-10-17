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
* File:         tunnelfs_srv_file.c                                       * 
* Description:  File handling between client and server                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "mpio.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Short for a safe test to string equality
 */
#define STR_EQ(str1, str2) (strncmp( ((str1 != NULL) ? str1 : "\0"), \
                                     ((str2 != NULL) ? str2 : "\0"), \
                                     (str2 != NULL) ? strlen(str2) : 0 ) == 0)
/**
 * Tunnelfs File Handle
 */
typedef struct
{
    MPI_File fh;                /**< MPI_File handle */
    char *filename;             /**< Lokal copy of filename */
    int file_id;                /**< global file id */
    int comm_id;                /**< communicator id */
    int client_cookie;          /**< magic cookie for collective client ops */
    int main_client;            /**< client doing open/close handling */
    int main_server;            /**< server keeping main copy */
    int mutual;                 /**< are file handles also somewhere else?  */
    int file_system;            /**< ADIO filesystem identifier */
    int def_filetype_access;    /**< filetype access defined in last info */
    int def_server_behaviour;   /**< server behaviour defined in last info */
    int def_dist_type;          /**< distribution type defined in last info */
    int cur_filetype_access;    /**< filetype access currently set */
    int cur_server_behaviour;   /**< server behaviour currently set */
    int cur_dist_type;          /**< distribution type currently set */
    dist_list_t *distribution;  /**< current client/server distribution */
    int distribution_size;      /**< number of entries in distribution */
    int opened;                 /**< Open/Close flag */
    int is_clone;               /**< Clone flag */
}
tunnelfs_handle_t;

/* FIXME: mpe also has a string.h, but that doesn't define strnlen !!! */
extern size_t strnlen(__const char *__string, size_t __maxlen);
extern int snprintf(char *__string, size_t __maxlen, const char *format, ...);
extern char *strdup(const char *s1);

static tunnelfs_handle_t *tunnelfs_handles = NULL;
static int size = 0;
static int last = -1;
static int chunk_size = 10;

int tunnelfs_file_id = 0;

static pthread_mutex_t tunnelfs_srv_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Getting index of file handle within global array of handles.
 * @param file_id tunnelfs id to reference this handle
 * @return index for the entry of file_id within handle array
 */
static int tunnelfs_srv_file_get_idx(int file_id);
static int tunnelfs_srv_file_get_idx(int file_id)
{
    int i = 0;

    /* INFO: Locking has to be done in calling function! */

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    if (last == -1)
        return last;

    assert(last >= 0);

    while ((i <= last) && (tunnelfs_handles[i].file_id != file_id))
        i++;

    if (i > last)
        return -1;
    else
        return i;
}

/**
 * Creating a tunnelfs handle object.
 * @param file_id tunnelfs id to reference this handle
 * @param comm_id tunnelfs id to reference the communicator associated with
 *        this handle
 * @return Pointer to MPI file handle entry within the created tunnelfs handle
 */
MPI_File *tunnelfs_srv_file_create_handle(int file_id, int comm_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(comm_id > 0);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    /* assure handle can be created */
    if ((tunnelfs_handles == NULL) || (size == 0) || (size == last + 1))
    {
        size += chunk_size;
        tunnelfs_handles = realloc(tunnelfs_handles,
                                   size * sizeof(tunnelfs_handle_t));
    }

    assert(tunnelfs_handles != NULL);

    /* making sure handle is not already allocated */
    idx = tunnelfs_srv_file_get_idx(file_id);
    assert(idx == -1);

    last++;

    /* set values */
    tunnelfs_handles[last].fh = MPI_FILE_NULL;
    tunnelfs_handles[last].filename = NULL;
    tunnelfs_handles[last].file_id = file_id;
    tunnelfs_handles[last].comm_id = comm_id;
    tunnelfs_handles[last].client_cookie = TUNNELFS_NEXT_COLL_ID;
    tunnelfs_handles[last].main_client = -1;
    tunnelfs_handles[last].main_server = -1;
    tunnelfs_handles[last].mutual = 0;
    tunnelfs_handles[last].file_system = 0;
    tunnelfs_handles[last].def_filetype_access = 0;
    tunnelfs_handles[last].def_server_behaviour = 0;
    tunnelfs_handles[last].def_dist_type = 0;
    tunnelfs_handles[last].cur_filetype_access = TUNNELFS_FILETYPE_JOINT;
    tunnelfs_handles[last].cur_server_behaviour = TUNNELFS_DIRECT;
    tunnelfs_handles[last].cur_dist_type = TUNNELFS_DIST_NONE;
    tunnelfs_handles[last].distribution = NULL;
    tunnelfs_handles[last].distribution_size = 0;
    tunnelfs_handles[last].opened = 0;
    tunnelfs_handles[last].is_clone = 0;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);

    /* return file handle */
    return &(tunnelfs_handles[last].fh);
}

/**
 * Deleting a tunnelfs handle object
 * @param file_id tunnelfs id to reference this handle
 */
void tunnelfs_srv_file_free_handle(int file_id)
{
    int idx = -1;

    assert((tunnelfs_handles != 0) && (size > 0));
    assert(last >= 0);
    assert(file_id > 0);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    if ((idx = tunnelfs_srv_file_get_idx(file_id)) != -1)
    {
        if (idx < last)
        {
            memcpy(&(tunnelfs_handles[idx]),
                   &(tunnelfs_handles[last]), sizeof(tunnelfs_handle_t));
        }

        tunnelfs_handles[last].fh = MPI_FILE_NULL;
        FREE(tunnelfs_handles[last].filename);
        tunnelfs_handles[last].file_id = -1;
        tunnelfs_handles[last].comm_id = -1;
        tunnelfs_handles[last].client_cookie = -1;
        tunnelfs_handles[last].main_client = -1;
        tunnelfs_handles[last].main_server = -1;
        tunnelfs_handles[last].mutual = -1;
        tunnelfs_handles[last].file_system = 0;
        tunnelfs_handles[last].def_filetype_access = 0;
        tunnelfs_handles[last].def_server_behaviour = 0;
        tunnelfs_handles[last].def_dist_type = 0;
        tunnelfs_handles[last].cur_filetype_access = TUNNELFS_FILETYPE_JOINT;
        tunnelfs_handles[last].cur_server_behaviour = TUNNELFS_DIRECT;
        tunnelfs_handles[last].cur_dist_type = TUNNELFS_DIST_NONE;
        FREE(tunnelfs_handles[last].distribution);
        tunnelfs_handles[last].distribution_size = 0;
        tunnelfs_handles[last].opened = 0;
        tunnelfs_handles[last].is_clone = 0;

        last--;

        pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    }
}

/**
 * Check if a file handle exists for a file id
 * @param file_id ID of file handle
 * @return
 *      - 1: file handle exists
 *      - 0: file handle does not exist
 */
int tunnelfs_srv_file_handle_exists(int file_id)
{
    int idx = -1;

    assert(file_id > 0);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    if ((idx = tunnelfs_srv_file_get_idx(file_id)) != -1)
    {
        pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
        return 1;
    }
    else
    {
        pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
        return 0;
    }
}

/**
 * Getting MPI handle for tunnelfs file handle id
 * @param file_id tunnelfs id to reference the file handle
 * @return Pointer to MPI file handle entry within the created tunnelfs handle
 */
MPI_File *tunnelfs_srv_file_get_handle(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx != -1);

    /* FIXME: 
     * it is not wise to return pointers in a multithreaded environment! */
    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return &(tunnelfs_handles[idx].fh);
}

/**
 * Getting tunnelfs id of the communicator associated with file_id
 * @param file_id tunnelfs id to reference the file handle
 * @return communicator id within tunnelfs
 */
int tunnelfs_srv_file_get_comm_id(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].comm_id;
}

/**
 * Define filetype access for later querying and setting
 * @param file_id file id this info is associated with
 * @param access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
void tunnelfs_srv_file_def_filetype_access(int file_id, int access)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].def_filetype_access = access;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Set filetype access on file handle
 * @param file_id file id this info is associated with
 * @param access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
void tunnelfs_srv_file_set_filetype_access(int file_id, int access)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].cur_filetype_access = access;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get filetype access of file handle
 * @param file_id file id this info is associated with
 * @return access access type defined in the info object. Valid values are:
 *      - joint
 *      - semijoint
 *      - disjoint
 */
int tunnelfs_srv_file_get_filetype_access(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].cur_filetype_access;
}

/**
 * Set mutual status on a file
 * @param file_id ID of file handle
 * @param status status of file handle
 */
void tunnelfs_srv_file_set_mutual(int file_id, int status)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].mutual = status;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Check if all servers have this handle
 * @param file_id ID of file handle
 * @return
 *      - 0: file is on local server only
 *      - 1: file is on all servers
 */
int tunnelfs_srv_file_is_mutual(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].mutual;
}

/**
 * Mark/unmark a file handle as a clone
 * Cloned file handles are used for parallel direct access, a la memfs
 * @param file_id ID of file handle
 * @param status Clone status of file handle
 */
void tunnelfs_srv_file_set_clone(int file_id, int status)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].is_clone = status;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Query clone status of a given file handle
 * @param file_id ID of file handle
 * @return Clone status of file handle
 */
int tunnelfs_srv_file_is_clone(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].is_clone;
}

/**
 * Mark/unmark a file handle as a opened
 * @param file_id ID of file handle
 * @param status Open/Close status of file handle
 */
void tunnelfs_srv_file_set_opened(int file_id, int status)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].opened = status;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Query Open/Close status of a given file handle
 * @param file_id ID of file handle
 * @return Open/Close status of file handle
 */
int tunnelfs_srv_file_is_opened(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].opened;
}

/**
 * Set client that gets reply for collective requests
 * @param file_id ID of file handle
 * @param client_id Rank of client
 */
void tunnelfs_srv_file_set_main_client(int file_id, int client_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(client_id >= 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].main_client = client_id;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get client that gets reply for collective requests
 * @param file_id ID of file handle
 * @return Rank of client
 */
int tunnelfs_srv_file_get_main_client(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].main_client;
}

/**
 * Set server that is responsible for a file handle
 * @param file_id ID of file handle
 * @param server_id Rank of main server
 */
void tunnelfs_srv_file_set_main_server(int file_id, int server_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(server_id >= 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].main_server = server_id;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get server that is responsible for a file handle
 * @param file_id ID of file handle
 * @return Rank of main server
 */
int tunnelfs_srv_file_get_main_server(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].main_server;
}

/**
 * Associate a file distribution with the given file handle
 * @param file_id ID of file handle
 * @param list Reference of the distribution list
 * @param list_size Size of the distribution list
 */
void tunnelfs_srv_file_set_distribution(int file_id, dist_list_t *list,
                                        int list_size)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);
    assert(list != NULL);
    assert(list_size > 0);
    assert(size > 0);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx != -1);

    ALLOC(tunnelfs_handles[idx].distribution,
          list_size * sizeof(dist_list_t));
    memcpy(tunnelfs_handles[idx].distribution, list,
           list_size * sizeof(dist_list_t));
    tunnelfs_handles[idx].distribution_size = list_size;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get the file distribution of the given file handle
 * @param file_id ID of file handle
 * @param list Reference of the distribution list
 * @param list_size Size of the distribution list
 */
void tunnelfs_srv_file_get_distribution(int file_id, dist_list_t **list,
                                        int *list_size)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx != -1);
    assert(tunnelfs_handles[idx].distribution_size > 0);
    assert(tunnelfs_handles[idx].distribution != NULL);

    *list_size = tunnelfs_handles[idx].distribution_size;
    ALLOC(*list, *list_size * sizeof(dist_list_t));
    memcpy(*list, tunnelfs_handles[idx].distribution, *list_size *
           sizeof(dist_list_t));

    assert(*list != NULL);
    assert(*list_size > 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get a list of all servers sharing a file handle
 * @param file_id ID of file handle
 * @param servers Reference on server list
 * @param server_size Reference on size of server list
 */
void tunnelfs_srv_file_get_serverlist(int file_id, int **servers,
                                      int *server_size)
{
    int idx = -1;
    int *tmp_list = NULL;
    int last = -1;
    int i, j, found;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx != -1);
    assert(tunnelfs_handles[idx].distribution_size > 0);

    ALLOC(tmp_list, tunnelfs_handles[idx].distribution_size * sizeof(int));

    tmp_list[0] = tunnelfs_handles[idx].distribution[0].fileserver;

    last = 0;
    for (i = 1; i < tunnelfs_handles[idx].distribution_size; i++)
    {
        found = 0;
        for (j = 0;
             (j <= last) && (j < tunnelfs_handles[idx].distribution_size);
             j++)
        {
            if (tunnelfs_handles[idx].distribution[i].fileserver ==
                tmp_list[j])
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            tmp_list[++last] =
                tunnelfs_handles[idx].distribution[i].fileserver;
        }
    }

    ALLOC(*servers, (last + 1) * sizeof(int));
    *server_size = last + 1;
    memcpy(*servers, tmp_list, *server_size * sizeof(int));
    FREE(tmp_list);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get server for a specific client
 * @param file_id ID of file handle
 * @param client Rank of client
 * @return Rank of corresponding server
 */
int tunnelfs_srv_file_get_server(int file_id, int client)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);
    assert(tunnelfs_handles[idx].distribution != NULL);

    int i = 0;
    while ((i < tunnelfs_handles[idx].distribution_size) &&
           (tunnelfs_handles[idx].distribution[i].client != client))
        i++;

    assert(i < tunnelfs_handles[idx].distribution_size);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].distribution[i].fileserver;
}

/**
 * Get magic cookie of client operation
 * @param file_id ID of file handle
 * @return magic cookie of client operation
 */
int tunnelfs_srv_file_get_client_cookie(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].client_cookie;
}

/**
 * Set filename on internal structures
 * @param file_id ID of file handle
 * @param filename Name of the file
 */
void tunnelfs_srv_file_set_name(int file_id, char *filename)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);
    assert(filename != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].filename = strdup(filename);
    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get filename of a file handle
 * @param file_id ID of file handle
 * @return Name of the file
 */
char *tunnelfs_srv_file_get_name(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].filename;
}

/**
 * Create a name for the tunnelfs cache file used in cached io
 * @param prefix Prefix for filename
 * @param rank Client rank the cache file is for
 * @param width width for implicit leading_zero call
 * @return filename of cache file
 */
char *tunnelfs_srv_file_create_cache_name(char *prefix, int rank, int width)
{
    const char *cache_str = ".TUNNELFS_CACHE.";
    char *string = NULL;
    int length = 0;
    int allocated = 0;

    if (prefix == NULL)
    {
        ALLOC(prefix, 1);
        allocated = 1;
        prefix[0] = '\0';
    }

    length = strnlen(prefix, 200) + strlen(cache_str) + 7;

    ALLOC(string, length);

    snprintf(string, length, "%s%s%s", prefix, cache_str, leadzero(6, rank));

    if (allocated)
        FREE(prefix);

    return string;
}

/**
 * Define a future server behaviour for file handling
 * @param file_id ID of file handle
 * @param behaviour Server behaviour for requests
 */
void tunnelfs_srv_file_def_behaviour(int file_id, int behaviour)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].def_server_behaviour = behaviour;
    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Set the future server behaviour for file handling
 * @param file_id ID of file handle
 * @param behaviour Server behaviour for requests
 */
void tunnelfs_srv_file_set_behaviour(int file_id, int behaviour)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].cur_server_behaviour = behaviour;
    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get the current server behaviour for file handling
 * @param file_id ID of file handle
 * @return Server behaviour for requests
 */
int tunnelfs_srv_file_get_behaviour(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
    return tunnelfs_handles[idx].cur_server_behaviour;
}

/**
 * Define distribution type for a file handle
 * @param file_id ID for file handle
 * @param dist_type Type of distribution
 */
void tunnelfs_srv_file_def_dist_type(int file_id, int dist_type)
{
    int idx = -1;

    assert(file_id > 0);
    assert(dist_type >= 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].def_dist_type = dist_type;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Set distribution type for a file handle
 * @param file_id ID for file handle
 * @param dist_type Type of distribution
 */
void tunnelfs_srv_file_set_dist_type(int file_id, int dist_type)
{
    int idx = -1;

    assert(file_id > 0);
    assert(dist_type >= 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    tunnelfs_handles[idx].cur_dist_type = dist_type;
    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);
}

/**
 * Get distribution type for a file handle
 * @param file_id ID for file handle
 * @return Type of distribution
 */
int tunnelfs_srv_file_get_dist_type(int file_id)
{
    int idx = -1;

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);

    return tunnelfs_handles[idx].cur_dist_type;
}

/**
 * Calculate distribution of a file handle
 * @param file_id ID for file handle
 * @param clnt_msg_id Message id of the client request needed for reply
 */
void tunnelfs_srv_file_distribute(int file_id, int clnt_msg_id)
{
    int idx = -1;
    tunnelfs_comminfo_t ci;
    int clone_handle = 0;
    int behaviour;
    int type;
    int systemtype;
    int set = 0;
    int needs_reconfig = 0;
    dist_list_t *dist_list = NULL;

    /* choose distribution on set hints */

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    pthread_mutex_lock(&tunnelfs_srv_file_mutex);

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);

    type = tunnelfs_handles[idx].def_dist_type;
    behaviour = tunnelfs_handles[idx].def_server_behaviour;
    systemtype = tunnelfs_handles[idx].fh->file_system;

    if (type != tunnelfs_handles[idx].cur_dist_type)
        needs_reconfig = 1;
    if (behaviour != tunnelfs_handles[idx].cur_server_behaviour)
        needs_reconfig = 1;

    pthread_mutex_unlock(&tunnelfs_srv_file_mutex);

    tunnelfs_srv_comminfo_create(tunnelfs_handles[idx].comm_id, &ci);

    if ((type == TUNNELFS_DIST_MEMFS) || (systemtype == ADIO_MEMFS))
    {
        /* get distribution from memfs layer and ignore filetype_access and
         * distribution hints */

        /*
           if (needs_reconfig)
           {
         */
        LOG("Using memfs distribution and direct access");
        tunnelfs_srv_file_set_behaviour(file_id, TUNNELFS_DIRECT);

        /* create distribution */
        ALLOC(dist_list, ci.size * sizeof(dist_list_t));
        memfs_get_fileserver_distribution(ci.ranks, ci.size, dist_list);

        tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_MEMFS);

        if (!tunnelfs_srv_file_is_mutual(file_id))
        {
            clone_handle = 1;
            tunnelfs_srv_file_set_mutual(file_id, 1);
        }
        /*
           }
           else
           {
           LOG("Reusing existing distribution of memfs layer");
           tunnelfs_srv_file_get_distribution(file_id, &dist_list, &ci.size);
           }
         */
        set = 1;
    }
    else if (type == TUNNELFS_DIST_BALANCED)
    {
        int *servers = NULL;
        int servers_size = 0;
        /* this would be the case if all fileservers are located on a single
         * metahost */
        LOG("Using balanced distribution and distributed access");

        tunnelfs_srv_get_serverlist(&servers, &servers_size);

        tunnelfs_srv_schedule_balanced_distribution(ci.ranks, ci.size,
                                                    servers, servers_size,
                                                    &dist_list);

        if (!tunnelfs_srv_file_is_mutual(file_id))
        {
            clone_handle = 1;
            tunnelfs_srv_file_set_mutual(file_id, 1);
        }
        set = 1;
    }
    else if (type == TUNNELFS_DIST_DOMAIN)
    {
        int *servers = NULL;
        int servers_size = 0;
        /* this would be the case if all fileservers are located on a single
         * metahost */
        LOG("Using balanced distribution and distributed access");

        tunnelfs_srv_get_serverlist(&servers, &servers_size);

        tunnelfs_srv_schedule_domain_distribution(ci.ranks, ci.size,
                                                  &dist_list);

        if (!tunnelfs_srv_file_is_mutual(file_id))
        {
            clone_handle = 1;
            tunnelfs_srv_file_set_mutual(file_id, 1);
        }
        set = 1;
    }

    if ((behaviour == TUNNELFS_ROUTE) || (behaviour == TUNNELFS_CACHE))
    {
        if (needs_reconfig)
        {
            /* this would be the case if fileservers are scattered over the
             * metahosts */
            LOG("Using filesystem domain distribution and distributed access");

            tunnelfs_srv_schedule_domain_distribution(ci.ranks, ci.size,
                                                      &dist_list);

            if (!tunnelfs_srv_file_is_mutual(file_id))
            {
                clone_handle = 1;
                tunnelfs_srv_file_set_mutual(file_id, 1);
            }
            set = 1;
        }
        else
        {
            LOG("Reusing existing distribution");
            tunnelfs_srv_file_get_distribution(file_id, &dist_list, &ci.size);
        }
    }
    else if (!set)
    {
        int my_rank;

        LOCK_MPI();
        MPI_Comm_rank(TUNNELFS_COMM_WORLD, &my_rank);
        UNLOCK_MPI();

        /* distribution local to the server */
        LOG("Using no distribution and direct, local access");
        tunnelfs_srv_schedule_balanced_distribution(ci.ranks, ci.size,
                                                    &my_rank, 1, &dist_list);
    }

    tunnelfs_srv_file_set_distribution(file_id, dist_list, ci.size);

    tunnelfs_srv_schedule_print_distribution(dist_list, ci.size);

    if (clone_handle)
        tunnelfs_srv_file_clone_handle(file_id, clnt_msg_id);

    if (needs_reconfig && (tunnelfs_srv_file_is_mutual(file_id)))
        tunnelfs_srv_file_clone_distribution(file_id);
}

/**
 * Clone handle on all servers
 * @param file_id ID for file handle
 * @param clnt_msg_id Message id of the client request needed for reply
 */
void tunnelfs_srv_file_clone_handle(int file_id, int clnt_msg_id)
{
    /* clone file handle on all servers */
    int i;
    int *servers = NULL;
    int servers_size = 0;
    int server_cookie = TUNNELFS_NEXT_COLL_ID;
    void *send_buf = NULL;
    int send_buf_size = 0;
    int pack_size = 0;
    int temp_size = 0;
    int position = 0;
    int main_client = -1;
    int my_rank = -1;
    tunnelfs_fileinfo_t fi;

    LOG("Cloning file handle");

    assert(file_id > 0);

    LOCK_MPI();
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &my_rank);
    UNLOCK_MPI();

    tunnelfs_srv_get_serverlist(&servers, &servers_size);

    assert(servers != NULL);
    assert(servers_size > 0);

    /* if we have only one server, no collective op is needed! */
    if (servers_size == 1)
        return;

    tunnelfs_srv_collop_start(file_id, server_cookie, servers_size - 1,
                              clnt_msg_id);

    LOCK_MPI();
    MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
    UNLOCK_MPI();

    tunnelfs_srv_fileinfo_create(file_id, &fi);
    tunnelfs_srv_fileinfo_pack_size(&fi, &temp_size);
    pack_size += temp_size;

    tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

    main_client = tunnelfs_srv_file_get_main_client(file_id);

    position = 0;
    LOCK_MPI();
    /*
       MPI_Pack(&clnt_msg_id, 1, MPI_INT, send_buf, send_buf_size, &position,
       TUNNELFS_COMM_WORLD);
     */
    MPI_Pack(&main_client, 1, MPI_INT, send_buf, send_buf_size,
             &position, TUNNELFS_COMM_WORLD);
    MPI_Pack(&server_cookie, 1, MPI_INT, send_buf, send_buf_size, &position,
             TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    tunnelfs_srv_fileinfo_pack(send_buf, send_buf_size, &position, &fi);

    for (i = 0; i < servers_size; i++)
    {
        if (servers[i] != my_rank)
            ptMPI_Send(send_buf, position,
                       MPI_PACKED, servers[i],
                       TUNNELFS_SERVER_OPEN_CLONE, TUNNELFS_COMM_WORLD);
    }

    FREE(servers);
    FREE(send_buf);
}

/**
 * Clone distribution on all servers
 * @param file_id ID for file handle
 */
void tunnelfs_srv_file_clone_distribution(int file_id)
{
    int idx = -1;
    int *servers = NULL;
    int servers_size = 0;
    void *buf = NULL;
    int buf_size = 0;
    int pack_size = 0;
    int position = 0;
    int var_id = 0;
    int my_rank = -1;
    int setup_id = TUNNELFS_NEXT_MSG_ID;
    int i;
    tunnelfs_comminfo_t ci;

    LOG("Cloning distribution");

    assert(file_id > 0);
    assert(tunnelfs_handles != NULL);

    tunnelfs_srv_get_serverlist(&servers, &servers_size);
    assert(servers != NULL);
    assert(servers_size > 0);

    /* if we only have one server we do not need to distribute! */
    if (servers_size == 1)
        return;

    idx = tunnelfs_srv_file_get_idx(file_id);

    assert(idx >= 0);
    assert(tunnelfs_handles[idx].distribution != NULL);

    LOCK_MPI();
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &my_rank);
    UNLOCK_MPI();

    tunnelfs_srv_comminfo_create(tunnelfs_handles[idx].comm_id, &ci);

    /* TODO: do we need a collective operation here? */

    /* send distribution to other servers */
    LOCK_MPI();
    MPI_Pack_size(4 + (2 * ci.size), MPI_INT, TUNNELFS_COMM_WORLD,
                  &pack_size);
    UNLOCK_MPI();

    tunnelfs_adjust_buffer(&buf, &buf_size, pack_size);

    LOCK_MPI();
    position = 0;
    MPI_Pack(&setup_id, 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    var_id = TUNNELFS_VAR_DISTLIST;
    MPI_Pack(&var_id, 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&file_id, 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&(ci.size), 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(tunnelfs_handles[idx].distribution, 2 * ci.size, MPI_INT,
             buf, buf_size, &position, TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    for (i = 0; i < servers_size; i++)
        if (my_rank != servers[i])
            ptMPI_Send(buf, position,
                       MPI_PACKED, servers[i],
                       TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);

    FREE(servers);
    FREE(buf);
}
