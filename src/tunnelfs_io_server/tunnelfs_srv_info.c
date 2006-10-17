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
* File:         tunnelfs_srv_info.c                                       * 
* Description:  Info handling                                             * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include "mpi.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include <assert.h>

/**
 * Save string comparision
 */
#define STR_EQ(str1, str2) (strncmp( ((str1 != NULL) ? str1 : "\0"), \
                                     ((str2 != NULL) ? str2 : "\0"), \
                                     (str2 != NULL) ? strlen(str2) : 0 ) == 0)

/**
 * Create an MPI_Info object from a message buffer
 * @param buffer message buffer
 * @param buffer_size size of message buffer
 * @param position Reference to position indicator
 * @param count Number of info entries
 * @param info Reference to MPI_Info handle
 */
void tunnelfs_srv_info_create(void *buffer, int buffer_size, int *position,
                              int count, MPI_Info *info)
{
    assert(count >= 0);
    if (count == 0)
    {
        /* no info object in buffer, leave everything untouched an return a
         * null handle */
        LOG("Empty info object");
        *info = MPI_INFO_NULL;
        return;
    }
    else
    {
        int i = 0;
        char info_key[MPI_MAX_INFO_KEY + 1];
        int key_len = 0;
        char *info_val = NULL;
        int val_len = 0;

        LOCK_MPI();

        LOG("Creating MPI_Info object");
        MPI_Info_create(info);

        for (i = 0; i < count; i++)
        {
            key_len = 0;
            val_len = 0;

            MPI_Unpack(buffer, buffer_size, position, &key_len, 1, MPI_INT,
                       TUNNELFS_COMM_WORLD);

            assert(key_len <= MPI_MAX_INFO_KEY);

            MPI_Unpack(buffer, buffer_size, position, info_key, key_len,
                       MPI_CHAR, TUNNELFS_COMM_WORLD);

            /* null terminate string */
            info_key[key_len] = 0;

            MPI_Unpack(buffer, buffer_size, position, &val_len, 1, MPI_INT,
                       TUNNELFS_COMM_WORLD);

            assert(val_len <= MPI_MAX_INFO_VAL);

            ALLOC(info_val, val_len + 1);
            if (val_len > 0)
            {
                MPI_Unpack(buffer, buffer_size, position, info_val, val_len,
                           MPI_CHAR, TUNNELFS_COMM_WORLD);
                info_val[val_len] = '\0';
            }
            else
            {
                info_val[0] = '\0';
            }

            LOG("Setting Info: %s=%s", info_key, info_val);

            MPI_Info_set(*info, info_key, info_val);
        }

        UNLOCK_MPI();

        FREE(info_val);
    }
}

/**
 * Pack MPI_Info object into a message buffer
 * @param buffer message buffer
 * @param buffer_size size of message buffer
 * @param position Reference to position indicator
 * @param count Number of info entries
 * @param info MPI_Info handle
 */
void tunnelfs_srv_info_pack(void *buffer, int buffer_size, int *position,
                            int count, MPI_Info info)
{
    assert(buffer != NULL);
    assert(buffer_size > 0);
    assert(buffer_size > *position);
    assert(count >= 0);

    LOG("Packing info object with %i value pairs", count);

    LOCK_MPI();
    MPI_Pack(&count, 1, MPI_INT, buffer, buffer_size, position,
             TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    if (count > 0)
    {
        int i;
        char info_key[MPI_MAX_INFO_KEY + 1];

        LOCK_MPI();
        for (i = 0; i < count; i++)
        {
            int key_len = 0;
            int val_len = 0;
            int flag = 0;
            char *info_val = NULL;

            MPI_Info_get_nthkey(info, i, info_key);
            key_len = strlen(info_key);
            MPI_Pack(&key_len, 1, MPI_INT, buffer, buffer_size, position,
                     TUNNELFS_COMM_WORLD);
            MPI_Pack(&info_key, key_len, MPI_CHAR, buffer, buffer_size,
                     position, TUNNELFS_COMM_WORLD);

            MPI_Info_get_valuelen(info, info_key, &val_len, &flag);
            assert(flag != 0);

            ALLOC(info_val, val_len + 1);
            MPI_Info_get(info, info_key, val_len, info_val, &flag);
            assert(flag != 0);

            MPI_Pack(&val_len, 1, MPI_INT, buffer, buffer_size, position,
                     TUNNELFS_COMM_WORLD);
            if (val_len > 0)
                MPI_Pack(info_val, val_len, MPI_CHAR, buffer, buffer_size,
                         position, TUNNELFS_COMM_WORLD);

            LOG("Packed %s (%i) -> %s (%i)", info_key, key_len, info_val,
                val_len);
            FREE(info_val);
        }
        UNLOCK_MPI();
    }
}

/**
 * Check if a key is set to a specific value
 * @param info MPI_Info handle
 * @param key String identifier for key
 * @param val String identifier for value
 * @return Boolean value
 *      - 1: Key is set to val in info object
 *      - 0: Key is not set to val in info object
 */
int tunnelfs_srv_info_is_set(MPI_Info info, char *key, char *val)
{
    int valuelen = 0;
    int flag = 0;
    char *value = NULL;
    int rc = 0;

    if (info == MPI_INFO_NULL)
        return 0;
    else
    {
        LOCK_MPI();
        MPI_Info_get_valuelen(info, key, &valuelen, &flag);
        if (flag)
        {
            ALLOC(value, valuelen + 1);
            MPI_Info_get(info, key, valuelen, value, &flag);
            if (strncmp(value, val, strlen(val)) == 0)
                rc = 1;
            else
                rc = 0;
            FREE(value);
        }
        else
            rc = 0;

        UNLOCK_MPI();
    }
    return rc;
}

/**
 * Get value of a specific key of the info object
 * @param info MPI_Info handle
 * @param key String identifier for key
 * @return String identifier for value
 */
char *tunnelfs_srv_info_get(MPI_Info info, char *key)
{
    int valuelen = 0;
    int flag = 0;
    char *value = NULL;

    if (info == MPI_INFO_NULL)
        return NULL;
    else
    {
        LOCK_MPI();
        MPI_Info_get_valuelen(info, key, &valuelen, &flag);
        if (flag)
        {
            ALLOC(value, valuelen + 1);
            MPI_Info_get(info, key, valuelen, value, &flag);
            assert(flag != 0);
            UNLOCK_MPI();
            FREE(value);
            return value;
        }
        else
        {
            UNLOCK_MPI();
            return NULL;
        }
    }
}

/**
 * Evaluate a given info object for relevant settings
 * @param info MPI_Info handle
 * @param file_id Internal file id
 */
void tunnelfs_srv_info_eval(MPI_Info info, int file_id)
{
    char *value = NULL;
    int is_clone = 0;

    assert(file_id >= 0);

    /* if no info is provided, we are done */
    if (info == MPI_INFO_NULL)
    {
        LOG("No info is provided, leaving distribution and access parameters untouched.");
        return;
    }

    is_clone = tunnelfs_srv_file_is_clone(file_id);

    value = tunnelfs_srv_info_get(info, "tunnelfs_server_behaviour");

    if (STR_EQ(value, "direct"))
    {
        LOG("Direct access defined");
        tunnelfs_srv_file_def_behaviour(file_id, TUNNELFS_DIRECT);
        if (is_clone)
            tunnelfs_srv_file_set_behaviour(file_id, TUNNELFS_DIRECT);
    }
    else if (STR_EQ(value, "routed io"))
    {
        LOG("Routed io defined");
        tunnelfs_srv_file_def_behaviour(file_id, TUNNELFS_ROUTE);
        if (is_clone)
            tunnelfs_srv_file_set_behaviour(file_id, TUNNELFS_ROUTE);
    }
    else if (STR_EQ(value, "cached io"))
    {
        LOG("Cached io not available yet");
        LOG("Routed io defined");

        tunnelfs_srv_file_def_behaviour(file_id, TUNNELFS_ROUTE);
        if (is_clone)
            tunnelfs_srv_file_set_behaviour(file_id, TUNNELFS_ROUTE);
    }
    else
    {
        LOG("No access method defined, assuming direct access");
        tunnelfs_srv_file_def_behaviour(file_id, TUNNELFS_DIRECT);
        if (is_clone)
            tunnelfs_srv_file_set_behaviour(file_id, TUNNELFS_DIRECT);
    }

    value = tunnelfs_srv_info_get(info, "tunnelfs_filetype_distribution");

    if (STR_EQ(value, "memfs"))
    {
        LOG("MEMFS distribution defined");
        tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_MEMFS);
        if (is_clone)
            tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_MEMFS);
    }
    else if (STR_EQ(value, "local"))
    {
        LOG("Local distribution defined");
        tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_LOCAL);
        if (is_clone)
            tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_LOCAL);
    }
    else if (STR_EQ(value, "balanced"))
    {
        LOG("Balanced distribution defined");
        tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_BALANCED);
        if (is_clone)
            tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_BALANCED);
    }
    else if (STR_EQ(value, "domain"))
    {
        LOG("Filesystem domain distribution defined");
        tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_DOMAIN);
        if (is_clone)
            tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_DOMAIN);
    }
    else
    {
        if (tunnelfs_srv_file_get_behaviour(file_id) == TUNNELFS_ROUTE)
        {
            LOG("No distribution defined, Routed IO implicating domain distribution");
            tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_DOMAIN);
            if (is_clone)
                tunnelfs_srv_file_set_dist_type(file_id,
                                                TUNNELFS_DIST_DOMAIN);
        }
        else
        {
            LOG("No distribution defined, assuming local");
            tunnelfs_srv_file_def_dist_type(file_id, TUNNELFS_DIST_LOCAL);
            if (is_clone)
                tunnelfs_srv_file_set_dist_type(file_id, TUNNELFS_DIST_LOCAL);
        }
    }

    value = tunnelfs_srv_info_get(info, "tunnelfs_filetype_access");

    if (STR_EQ(value, "joint"))
    {
        /* all processes have equal access to all parts of the file */
        LOG("Joint access defined");
        tunnelfs_srv_file_def_filetype_access(file_id,
                                              TUNNELFS_FILETYPE_JOINT);
    }
    else if (STR_EQ(value, "semijoint"))
    {
        /* some processes share access to some parts of the file */
        LOG("Semijoint access defined");
        tunnelfs_srv_file_def_filetype_access(file_id,
                                              TUNNELFS_FILETYPE_SEMIJOINT);
    }
    else if (STR_EQ(value, "disjoint"))
    {
        /* each process has only access to disjoint parts */
        LOG("Disjoint access defined");
        tunnelfs_srv_file_def_filetype_access(file_id,
                                              TUNNELFS_FILETYPE_DISJOINT);
    }
    else
    {
        /* all processes have equal access to all parts of the file */
        LOG("No filetype access defined, assuming joint");
        tunnelfs_srv_file_def_filetype_access(file_id,
                                              TUNNELFS_FILETYPE_JOINT);
    }
}
