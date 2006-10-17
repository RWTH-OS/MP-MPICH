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
* File:         ad_tunnelfs_datatype.c                                    * 
* Description:                                                            *
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdlib.h>
#include "mpi.h"
#include "ad_tunnelfs.h"

/**
 * Internal list of datatypes
 */
static MPI_Datatype *tunnelfs_datatypes = NULL;

static int tunnelfs_datatypes_size = 0;
static int tunnelfs_datatypes_chunk_size = 10;
static int tunnelfs_datatype_last = -1;

/**
 * Register an MPI datatype
 * @param dtype MPI Datatype handle
 * @return internal datatype handle
 */
int tunnelfs_datatype_register(MPI_Datatype dtype)
{
    int dtype_id = -1;
#ifdef DEBUG_TUNNELFS
    int rank = 0;

    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank);
#endif
    if (dtype == 0)
        return -1;

    tunnelfs_datatype_get_id(dtype, &dtype_id);

    if (dtype_id != TUNNELFS_FAILURE)
    {
        return dtype_id;
    }
    else if ((tunnelfs_datatypes == NULL) ||
             (tunnelfs_datatypes_size == 0) ||
             (tunnelfs_datatype_last + 1 == tunnelfs_datatypes_size))
    {
        tunnelfs_datatypes_size += tunnelfs_datatypes_chunk_size;
        tunnelfs_datatypes = realloc(tunnelfs_datatypes,
                                     tunnelfs_datatypes_size *
                                     sizeof(MPI_Datatype));

        if (tunnelfs_datatypes == NULL)
            ERR(TUNNELFS_ERR_ALLOC);
    }

    /* point to free field */
    tunnelfs_datatype_last++;

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%i] Datatype registered locally: id=%i mpi handle=%i\n",
            rank, tunnelfs_datatype_last, dtype);

#endif
    tunnelfs_datatypes[tunnelfs_datatype_last] = dtype;
    return tunnelfs_datatype_last;
}

/**
 * Delete an MPI Datatype reference from list
 * @param dtype MPI datatype handle
 * @return boolean indicating successful deletion
 */
int tunnelfs_datatype_deregister(MPI_Datatype dtype)
{
    /* this might seem wise to prevent polution with non-existing types in the
     * array, which can cost time to go through */
    return 1;
}

/**
 * Initialise intern datatype structures
 * @return boolean indicating successful initialization
 */
int tunnelfs_datatype_init()
{
    int i = 0;

    /* sanity check */
    if (TUNNELFS_NUM_NAMED_TYPES <= 0)
        ERR(TUNNELFS_ERR_ALLOC);

    tunnelfs_datatypes =
        (MPI_Datatype *) calloc(TUNNELFS_NUM_NAMED_TYPES,
                                sizeof(MPI_Datatype));
    if (tunnelfs_datatypes == NULL)
        return TUNNELFS_FAILURE;

    for (i = 0; i < TUNNELFS_NUM_NAMED_TYPES; i++)
    {
        tunnelfs_datatypes[i] = i;
    }

    tunnelfs_datatypes_size = TUNNELFS_NUM_NAMED_TYPES;
    /* defined in ad_tunnelfs_common.h */
    tunnelfs_datatype_last = TUNNELFS_NUM_NAMED_TYPES - 1;

    return TUNNELFS_SUCCESS;
}

/**
 * Get internal datatype reference for a given MPI datatype
 * @param dtype MPI datatype handle
 * @param dtype_id Reference to internal datatype id
 */
void tunnelfs_datatype_get_id(MPI_Datatype dtype, int *dtype_id)
{
    int i = 0;

    if ((tunnelfs_datatypes == NULL) &&
        (tunnelfs_datatype_init() == TUNNELFS_FAILURE))
    {
        *dtype_id = TUNNELFS_FAILURE;
        return;
    }

    while ((i <= tunnelfs_datatype_last) && (tunnelfs_datatypes[i] != dtype))
        i++;

    if (i > tunnelfs_datatype_last)
        *dtype_id = TUNNELFS_FAILURE;
    else
        *dtype_id = i;
}

/**
 * Get MPI datatype for a given internal datatype id
 * @param dtype_id Internal datatype id
 * @param dtype Reference to MPI datatype handle
 */
void tunnelfs_datatype_get_type(int dtype_id, MPI_Datatype *dtype)
{
    if ((tunnelfs_datatypes == NULL) &&
        (tunnelfs_datatype_init() == TUNNELFS_FAILURE))
        *dtype = MPI_DATATYPE_NULL;

    if (dtype_id > tunnelfs_datatype_last)
        *dtype = MPI_DATATYPE_NULL;
    else
        *dtype = tunnelfs_datatypes[dtype_id];
}

/**
 * Synchronize datatype with a tunnelfs server
 * @param type MPI datatype handle
 * @param server Rank of tunnelfs server
 * @return boolean inticating successful synchronization
 */
int tunnelfs_datatype_sync(MPI_Datatype type, int server)
{
    int type_id = -1;
#ifdef DEBUG_TUNNELFS
    int rank = 0;

    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank);
#endif

    /* check if datatype is registered */
    tunnelfs_datatype_get_id(type, &type_id);

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "Reported type id for %i is %i\n", type, type_id);
#endif
    if (type_id == TUNNELFS_FAILURE)
    {
        int num_integers = 0;
        int *integers = NULL;
        int num_addresses = 0;
        MPI_Aint *addresses = NULL;
        int num_datatypes = 0;
        MPI_Datatype *datatypes = NULL;
        int *types = NULL;
        int combiner = -1;

        int pack_size = 0;
        int temp_size = 0;
        int position = 0;
        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int i = 0;
        void *buf = NULL;
        int buf_size = 0;
        int rcode = -1;
        int reply_id = 0;
        int recvd = 0;

        int io_server_rank = -1;

        /* create a local type_id, so the server can identify it with the
         * combination of client_rank and type_id */
        type_id = tunnelfs_datatype_register(type);

        /* decode datatype */
        MPI_Type_get_envelope(type, &num_integers, &num_addresses,
                              &num_datatypes, &combiner);

        if (num_integers > 0)
            ALLOC(integers, num_integers * sizeof(int));

        if (num_addresses > 0)
            ALLOC(addresses, num_addresses * sizeof(MPI_Aint));

        if (num_datatypes > 0)
        {
            ALLOC(datatypes, num_datatypes * sizeof(MPI_Datatype));
            ALLOC(types, num_datatypes * sizeof(int));
        }

        if (combiner != MPI_COMBINER_NAMED)
        {
            MPI_Type_get_contents(type, num_integers, num_addresses,
                                  num_datatypes, integers, addresses,
                                  datatypes);

            /* The datatypes have to be checked if they are all in sync with
             * the server */
            for (i = 0; i < num_datatypes; i++)
            {
#ifdef DEBUG_TUNNELFS
                fprintf(stderr, "[%i] checking datatype %i for sync\n",
                        rank, datatypes[i]);
#endif
                tunnelfs_datatype_sync(datatypes[i], server);
#ifdef DEBUG_TUNNELFS
                fprintf(stderr, "[%i] datatype %i in sync\n",
                        rank, datatypes[i]);
#endif
            }

            /* pack message and send it */
            MPI_Pack_size(6 + num_integers + num_datatypes, MPI_INT,
                          TUNNELFS_COMM_WORLD, &temp_size);
            pack_size += temp_size;

            MPI_Pack_size(num_addresses, TUNNELFS_AINT, TUNNELFS_COMM_WORLD,
                          &temp_size);
            pack_size += temp_size;

            tunnelfs_adjust_buffer(&buf, &buf_size, pack_size);

            MPI_Pack(&msg_id, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
#ifdef DEBUG_TUNNELFS
            fprintf(stderr, "[%i] Type id %i packed\n", rank, type_id);
#endif

            MPI_Pack(&type_id, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);

            MPI_Pack(&num_integers, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
            if (num_integers > 0)
                MPI_Pack(integers, num_integers, MPI_INT, buf, buf_size,
                         &position, TUNNELFS_COMM_WORLD);

            MPI_Pack(&num_addresses, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
            if (num_addresses > 0)
                MPI_Pack(addresses, num_addresses, TUNNELFS_AINT, buf,
                         buf_size, &position, TUNNELFS_COMM_WORLD);

            MPI_Pack(&num_datatypes, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
            if (num_datatypes > 0)
            {
                /* translate the mpi types into tunnelfs specific datatypes */
                for (i = 0; i < num_datatypes; i++)
                    tunnelfs_datatype_get_id(datatypes[i], &(types[i]));
                MPI_Pack(types, num_datatypes, MPI_INT, buf, buf_size,
                         &position, TUNNELFS_COMM_WORLD);
            }
            MPI_Pack(&combiner, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);

            /* determine io server the datatype has to be sent to */
            /* io_server_rank = tunnelfs_server_get_global_master(); */
            io_server_rank = server;

            /* sending datatype definition to server */
            MPI_Send(buf, position, MPI_PACKED, io_server_rank,
                     TUNNELFS_DATATYPE, TUNNELFS_COMM_WORLD);

            tunnelfs_msg_get_reply(&buf, &buf_size, &recvd, io_server_rank,
                                   msg_id);

            position = 0;
            MPI_Unpack(buf, buf_size, &position, &reply_id, 1, MPI_INT,
                       TUNNELFS_COMM_WORLD);
            MPI_Unpack(buf, buf_size, &position, &rcode, 1, MPI_INT,
                       TUNNELFS_COMM_WORLD);

            tunnelfs_msg_get_variables(buf, buf_size, &position, recvd);

            FREE(buf);

#ifdef DEBUG_TUNNELFS
            fprintf(stderr, "[%i] Datatype %i in sync\n", rank, type_id);
#endif

            return TUNNELFS_SUCCESS;
        }
        else
            return TUNNELFS_SUCCESS;

        if (num_integers > 0)
            FREE(integers);
        if (num_addresses > 0)
            FREE(addresses);
        if (num_datatypes > 0)
        {
            FREE(types);
            FREE(datatypes);
        }
    }
    else
        return TUNNELFS_SUCCESS;

    /* datatype is registered and should be in sync with the tunnelfs
     * server. Therefore no more actions are needed */
}
