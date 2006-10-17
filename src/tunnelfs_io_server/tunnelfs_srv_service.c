/**************************************************************************
* TunnelFS IO Server                                                      * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
***************************************************************************
***************************************************************************
* File:         tunnelfs_srv_main.c                                       * 
* Description:  Tunnelfs main server thread handling client<->server      *
*               communication                                             *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "mpi.h"
#include "mpio.h"
#include "adio.h"
#include "ad_memfs.h"
#include "ad_tunnelfs.h"
#include "pario_threads.h"
#include "pario_probe.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Thread Management Variables
 */
pthread_mutex_t tunnelfs_service_sync = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_for_tunnelfs_service = PTHREAD_COND_INITIALIZER;
MPI_Status tunnelfs_service_status;
int tunnelfs_service_status_is_empty = 1;

void *tunnelfs_srv_service(void *args)
{
    int last_msg;
    int msg_size;
    int msg_id;
    int position = 0;

    int recv_buf_size = 0;
    void *recv_buf = NULL;
    int send_buf_size = 0;
    void *send_buf = NULL;
    /*
       int iodata_buf_size = 0;
       void *iodata_buf = NULL;
     */

    int *stop_server = NULL;
    int num_servers = 0;
    int num_clients = 0;
    int my_rank = 0;
    int world_size = 0;

    int rcode = 0;

    MPI_Status msg_status;

    num_servers = ((tunnelfs_thread_args_t *) args)->num_servers;
    num_clients = ((tunnelfs_thread_args_t *) args)->num_clients;
    my_rank = ((tunnelfs_thread_args_t *) args)->mpi_rank;
    stop_server = ((tunnelfs_thread_args_t *) args)->stop_server;

    LOCK_MPI();
    MPI_Comm_size(TUNNELFS_COMM_WORLD, &world_size);
    UNLOCK_MPI();

    /* Allocate initial message buffers */
    send_buf_size = (TUNNELFS_MAX_MSG_SIZE / 4);
    ALLOC(send_buf, send_buf_size);
    recv_buf_size = TUNNELFS_MAX_MSG_SIZE;
    ALLOC(recv_buf, recv_buf_size);

    /* initially locking mutex */
    pthread_mutex_lock(&tunnelfs_service_sync);

    /*while (!(*stop_server))*/
    while (!pario_shutdown)
    {
#if 0
        int msg_received = 0;
        while ((!msg_received) && (!(*stop_server)))
        {
            int flag = 0;
            /* lock mpi mutex */
            LOCK_MPI();
            /* Probing for incomming message */
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, TUNNELFS_COMM_WORLD,
                       &flag, &msg_status);

            /* check if message has client<->server tag */
            if ((flag) &&
                (msg_status.MPI_TAG >= 0x1000) &&
                (msg_status.MPI_TAG < 0x2000))
            {
                /* Receive message into recv_buf 
                 * - io data is received directly in case statement! */
                if ((msg_status.MPI_TAG < TUNNELFS_SERVER_IODATA_BASE) ||
                    (msg_status.MPI_TAG >= (TUNNELFS_SERVER_IODATA_BASE +
                                            world_size)))
                {
                    /* Retrieving size of next message */
                    MPI_Get_count(&msg_status, MPI_PACKED, &msg_size);

                    /* checking bounds of receive buffer */
                    tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size,
                                           msg_size);

                    MPI_Recv(recv_buf, recv_buf_size, MPI_PACKED,
                             msg_status.MPI_SOURCE, msg_status.MPI_TAG,
                             TUNNELFS_COMM_WORLD, &msg_status);
                }
                msg_received = 1;
            }
            /* unlock mpi mutex */
            UNLOCK_MPI();
        }

        /* if flag is set, break from message handling */
        if ((!msg_received) && (*stop_server))
            break;
#else
        /* clearing the status field */
        memset(&tunnelfs_service_status, 0, sizeof(MPI_Status));
        tunnelfs_service_status_is_empty = 1;

        LOG("Waiting for signal");
        /* wait for message signal */
        while ((tunnelfs_service_status_is_empty) && !pario_shutdown)
            pthread_cond_wait(&message_for_tunnelfs_service,
                              &tunnelfs_service_sync);

        /*if (*stop_server)*/
        if (pario_shutdown)
            break;

        LOG("Received signal");

        if ((tunnelfs_service_status.MPI_TAG < TUNNELFS_SERVER_IODATA_BASE) ||
            (tunnelfs_service_status.MPI_TAG >=
             (TUNNELFS_SERVER_IODATA_BASE + world_size)))
        {
            LOCK_MPI();
            MPI_Get_count(&tunnelfs_service_status, MPI_PACKED, &msg_size);

            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, msg_size);

            MPI_Recv(recv_buf, msg_size, MPI_PACKED,
                     tunnelfs_service_status.MPI_SOURCE,
                     tunnelfs_service_status.MPI_TAG, TUNNELFS_COMM_WORLD,
                     &msg_status);
            UNLOCK_MPI();

            /* Message is retrieved. Let's signal this to probe thread. */
            pthread_mutex_lock(&pario_probe_sync);
            message_is_processing = 0;
            pthread_cond_signal(&message_retrieved_from_queue);
            pthread_mutex_unlock(&pario_probe_sync);
        }
        else
        {
            memcpy(&msg_status, &tunnelfs_service_status, sizeof(MPI_Status));
        }

        /* INFO: IO Data is received in default case statement */
#endif

        last_msg = msg_status.MPI_TAG;
        position = 0;

        LOG("------------------------------------------------------------");

        switch (msg_status.MPI_TAG)
        {
        case TUNNELFS_SERVER_SHUTDOWN:
            {
                /* receiving command to shutdown from master */
                LOG("Received shutdown request from io master %i",
                    msg_status.MPI_SOURCE);

                LOG("Waiting for pending requests to finish");
                tunnelfs_srv_shutdown_close_pending();

                /**stop_server = 1;*/
                pario_shutdown = 1;

                /* Signal other threads that are possibly waiting. */
                pthread_cond_signal(&message_for_tunnelfs_main);
                /*
                   pthread_cond_signal(&message_for_memfs_main);
                   pthread_cond_signal(&message_for_memfs_service);
                 */
                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_SETUP:
            {
                int var_id = 0;
                int pack_size = 0;
                int tmp_size = 0;
                int comm_id = -1;

                LOG("Negotiating parameters with server %i",
                    msg_status.MPI_SOURCE);
                /* get message id  */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &msg_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* retrieving variables */
                while (position < msg_size)
                {
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, &var_id, 1,
                               MPI_INT, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    switch (var_id)
                    {
                    case TUNNELFS_VAR_COMMUNICATOR:
                        {
                            int num_ranks = 0;
                            void *ranks = NULL;
                            int ranks_size = 0;
#ifdef LOGGING
                            int i = 0;
                            char tmp[10];
                            char *ranks_str = NULL;
                            int str_len = 0;
#endif
                            LOG("Deserializing communicator");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &comm_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &num_ranks, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);

                            tunnelfs_adjust_buffer(&ranks, &ranks_size,
                                                   num_ranks * sizeof(int));

                            MPI_Unpack(recv_buf, msg_size, &position, ranks,
                                       num_ranks, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

#ifdef LOGGING
                            /* printing some communicator info */
                            str_len = num_ranks * 10;
                            ALLOC(ranks_str, str_len + 1);
                            /* initially ranks_str is empty */
                            ranks_str[0] = '\0';

                            for (i = 0; i < num_ranks; i++)
                            {
                                sprintf(tmp, "%i ", ((int *) ranks)[i]);
                                ranks_str = strcat(ranks_str, tmp);
                            }

                            LOG("Received communicator %i from %i with ranks: %s", comm_id, msg_status.MPI_SOURCE, ranks_str);

                            FREE(ranks_str);
#endif
                            if (comm_id == -1)
                                comm_id = NEXT_SRV_COMM_ID;

                            /* save communicator information */
                            tunnelfs_srv_comm_register(comm_id, num_ranks,
                                                       ranks);
                            LOG("Registered communicator with id: %i",
                                comm_id);

                            /* calculating size for reply package */
                            LOCK_MPI();
                            MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            UNLOCK_MPI();
                            pack_size += tmp_size;

                            break;
                        }
                    case TUNNELFS_VAR_DATATYPE:
                        {
                            int client_id = -1;
                            int num_int;
                            int *intvec = NULL;
                            int num_addr;
                            MPI_Aint *addrvec = NULL;
                            int num_types;
                            int *typevec = NULL;
                            MPI_Datatype *datatypevec = NULL;
                            int combiner = -1;
                            MPI_Datatype *new_type = NULL;
                            int type_id = -1;
                            int i;

                            /* get client id  */
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &client_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);

                            LOG("Server-side datatype definition for client %i", client_id);

                            /* get client specific type id */
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &type_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            LOG("Type id: %i", type_id);

                            /* prepare new datatype creation */
                            new_type =
                                tunnelfs_srv_datatype_prepare(client_id,
                                                              type_id);

                            /* get integer vector */
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &num_int, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            if (num_int > 0)
                            {
                                ALLOC(intvec, num_int * sizeof(int));
                                LOCK_MPI();
                                MPI_Unpack(recv_buf, msg_size, &position,
                                           intvec, num_int, MPI_INT,
                                           TUNNELFS_COMM_WORLD);
                                UNLOCK_MPI();
                            }
                            for (i = 0; i < num_int; i++)
                            {
                                LOG("Int[%i]: %i", i, intvec[i]);
                            }

                            /* get address vector */
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &num_addr, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            if (num_addr > 0)
                            {
                                ALLOC(addrvec, num_addr * sizeof(MPI_Aint));
                                LOCK_MPI();
                                MPI_Unpack(recv_buf, msg_size, &position,
                                           addrvec, num_addr, TUNNELFS_AINT,
                                           TUNNELFS_COMM_WORLD);
                                UNLOCK_MPI();
                            }
                            for (i = 0; i < num_addr; i++)
                            {
                                LOG("Address[%i]: %Lx", i, addrvec[i]);
                            }

                            /* get datatype vector */
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &num_types, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            if (num_types > 0)
                            {
                                ALLOC(typevec, num_types * sizeof(int));
                                ALLOC(datatypevec, num_types * sizeof(int));

                                LOCK_MPI();
                                MPI_Unpack(recv_buf, msg_size, &position,
                                           typevec, num_types, MPI_INT,
                                           TUNNELFS_COMM_WORLD);
                                UNLOCK_MPI();

                                for (i = 0; i < num_types; i++)
                                {
                                    tunnelfs_srv_datatype_get_type(client_id,
                                                                   typevec[i],
                                                                   &
                                                                   (datatypevec
                                                                    [i]));
                                    if (typevec[i] != datatypevec[i])
                                    {
                                        LOG("Translation from type id %i to mpi datatype %i", typevec[i], datatypevec[i]);
                                    }
                                }
                            }
                            for (i = 0; i < num_types; i++)
                            {
                                LOG("datatype[%i]: %i", i, datatypevec[i]);
                            }

                            /* get combiner */
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &combiner, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            if (msg_size != position)
                                fprintf(stderr,
                                        "WARNING: Message seems not to be unpacked completely.\n");

                            /* create new datatype */
                            /* TODO:
                             * actually there are more combiner functions defined in MPI
                             * but we can handle only those defined in mpi.h */
                            LOCK_MPI();
                            switch (combiner)
                            {
                            case MPI_COMBINER_CONTIGUOUS:
                                {
                                    MPI_Type_contiguous(intvec[0],
                                                        datatypevec[0],
                                                        new_type);
                                    break;
                                }
                            case MPI_COMBINER_VECTOR:
                                {
                                    MPI_Type_vector(intvec[0], intvec[1],
                                                    intvec[2], datatypevec[0],
                                                    new_type);
                                    break;
                                }
                            case MPI_COMBINER_HVECTOR:
                                {
                                    MPI_Type_hvector(intvec[0], intvec[1],
                                                     addrvec[0],
                                                     datatypevec[0],
                                                     new_type);
                                    break;
                                }
                            case MPI_COMBINER_INDEXED:
                                {
                                    MPI_Type_indexed(intvec[0], &(intvec[1]),
                                                     &(intvec[intvec[0]]),
                                                     datatypevec[0],
                                                     new_type);
                                    break;
                                }
                            case MPI_COMBINER_HINDEXED:
                                {
                                    MPI_Type_indexed(intvec[0], &(intvec[1]),
                                                     &(intvec[intvec[0]]),
                                                     datatypevec[0],
                                                     new_type);
                                    break;
                                }
                            case MPI_COMBINER_STRUCT:
                                {
                                    MPI_Type_struct(intvec[0], &(intvec[1]),
                                                    addrvec, datatypevec,
                                                    new_type);
                                    break;
                                }
                            default:
                                {
                                    fprintf(stderr,
                                            "Unrecognized combiner function: %i!  Unable to recreate datatype on server side!\n",
                                            combiner);
                                    exit(-1);
                                }
                            }
                            rcode = MPI_Type_commit(new_type);
                            UNLOCK_MPI();

                            LOG("Datatype %i committed on serverside.",
                                *new_type);

                            tunnelfs_srv_datatype_calc_param(*new_type);

                            LOG("Datatype parameters cached");

                            /* new type is commited and can be retrieved via
                             * tunnelfs_srv_datatype_get_type() */

                            /* free memory */
                            FREE(intvec);
                            FREE(addrvec);
                            FREE(typevec);
                            FREE(datatypevec);

                            break;
                        }
                    case TUNNELFS_VAR_CLIENTFSDOM:
                        {
                            int client_id;
                            int domain_len;
                            char *domain = NULL;

                            LOG("Registering Client Filesystem Domain");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &client_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &domain_len, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            if (domain_len > 0)
                            {
                                ALLOC(domain, domain_len + 1);
                                MPI_Unpack(recv_buf, msg_size, &position,
                                           domain, domain_len, MPI_CHAR,
                                           TUNNELFS_COMM_WORLD);
                                UNLOCK_MPI();
                                domain[domain_len] = '\0';

                                tunnelfs_srv_client_fs_domain_insert
                                    (client_id, domain);

                                LOG("Client filesystem domain %s registered for %i", tunnelfs_srv_client_fs_domain(client_id), client_id);

                                FREE(domain);
                            }
                            break;
                        }
                    case TUNNELFS_VAR_FILEHANDLE:
                        {
                            int file_id;
                            int comm_id;
                            int main_client;
                            int main_server;
                            int *cachenodes = NULL;
                            int cachenodes_size = 0;
                            int cached = 0;

                            LOG("Server-side file handle duplication");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &comm_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &main_client, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &main_server, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position, &cached,
                                       1, MPI_INT, TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &cachenodes_size, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            if (cachenodes_size > 0)
                            {
                                ALLOC(cachenodes, cachenodes_size *
                                      sizeof(int));
                                MPI_Unpack(recv_buf, msg_size, &position,
                                           cachenodes, cachenodes_size,
                                           MPI_INT, TUNNELFS_COMM_WORLD);
                            }
                            UNLOCK_MPI();

                            break;
                        }
                    case TUNNELFS_VAR_FILEVIEW:
                        {
                            int client_id;
                            int file_id;
                            MPI_Offset disp;
                            int etype_id;
                            int ftype_id;
                            MPI_Datatype etype;
                            MPI_Datatype ftype;

                            LOG("Server-side client file view specification");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &client_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &disp, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &etype_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &ftype_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            tunnelfs_srv_datatype_get_type(client_id,
                                                           etype_id, &etype);
                            tunnelfs_srv_datatype_get_type(client_id,
                                                           ftype_id, &ftype);

                            /* saving file view */
                            LOG("Saving file view for %i on file %i: disp=%Li etype=%i ftype=%i", client_id, file_id, disp, etype, ftype);
                            tunnelfs_srv_fileview_set(client_id, file_id,
                                                      disp, etype, ftype);

                            LOG("done");
                            break;
                        }
                    case TUNNELFS_VAR_DISTLIST:
                        {
                            int file_id;
                            int list_size;
                            dist_list_t *list = NULL;

                            LOG("Server-side file distribution list");
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &list_size, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            ALLOC(list, list_size * sizeof(dist_list_t));

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position, list,
                                       2 * list_size, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            tunnelfs_srv_file_set_distribution(file_id, list,
                                                               list_size);
                            break;
                        }
                    case TUNNELFS_REQ_SHARED_PTR:
                        {
                            int file_id = 0;
                            MPI_Offset offset = 0;

                            LOG("Server-side shared file pointer request");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &offset, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            offset = tunnelfs_srv_fptr_get_shared(file_id);

                            LOG("SFP value: 0x%Lx", offset);

                            ptMPI_Send(&offset, 1, TUNNELFS_OFFSET,
                                       msg_status.MPI_SOURCE, TUNNELFS_REPLY,
                                       TUNNELFS_COMM_WORLD);

                            break;
                        }
                    case TUNNELFS_REQ_SET_SHARED_PTR:
                        {
                            int file_id = 0;
                            MPI_Offset offset = 0;

                            LOG("Server-side shared file pointer set request");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &offset, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            tunnelfs_srv_fptr_getset_shared_atomic(file_id,
                                                                   offset);

                            LOG("New shared file pointer value: 0x%Lx",
                                offset);

                            break;
                        }
                    case TUNNELFS_REQ_GETSET_SHARED_PTR:
                        {
                            int file_id = 0;
                            MPI_Offset offset = 0;

                            LOG("Server-side atomic shared file pointer get/set request");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &offset, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            offset =
                                tunnelfs_srv_fptr_getset_shared_atomic
                                (file_id, offset);

                            LOG("SFP value: 0x%Lx", offset);

                            ptMPI_Send(&offset, 1, TUNNELFS_OFFSET,
                                       msg_status.MPI_SOURCE, TUNNELFS_REPLY,
                                       TUNNELFS_COMM_WORLD);

                            break;
                        }
                    default:
                        {
                            LOG("WARNING: Skipping rest of the message, because of unknown variable id %i", var_id);
                            position = msg_size;
                            break;
                        }

                    }
                }
                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_OPEN_PART:
            {
                MPI_File *fh_ptr;
                char *str = NULL;
                int client_id;
                int clnt_msg_id = 0;
                int pack_size;
                int ready = 0;
                tunnelfs_fileinfo_t fi;

                position = 0;

                /* Unpacking message */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Opening cache file for %i", client_id);

                tunnelfs_srv_fileinfo_unpack(recv_buf, msg_size, &position,
                                             &fi);

                str =
                    tunnelfs_srv_file_create_cache_name(fi.filename,
                                                        client_id, 0);
                LOG("Filename is %s", str);

                LOG("Creating handle for cache file");
                tunnelfs_srv_parts_create(fi.file_id, client_id,
                                          msg_status.MPI_SOURCE);
                fh_ptr = tunnelfs_srv_parts_get_handle(fi.file_id, client_id);

                LOG("Creating contiguous view for cache file (%i,%i)",
                    client_id, fi.file_id);
                tunnelfs_srv_fileview_set(client_id, fi.file_id, 0, MPI_BYTE,
                                          MPI_BYTE);

                LOCK_MPI();
                ready =
                    MPI_File_open(MPI_COMM_TUNNELFS_SELF, str, fi.accessmode,
                                  fi.info, fh_ptr);
                UNLOCK_MPI();

                /* send completion signal to issuing server */
                LOCK_MPI();
#if 1
                MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                position = 0;
                rcode = 0;

                MPI_Pack(&clnt_msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&client_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("client msg id: %i file id: %i client id: %i",
                    clnt_msg_id, fi.file_id, client_id);
                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_SERVER_OPEN_PART_DONE,
                           TUNNELFS_COMM_WORLD);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_OPEN_PART_DONE:
            {
                int file_id;
                int rcode;
                int clnt_msg_id;
                int client_id;
                int pack_size;
                int var_id;

                LOG("Received completion signal for partial open from %i",
                    msg_status.MPI_SOURCE);

                position = 0;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("client msg id: %i file id: %i client id: %i",
                    clnt_msg_id, file_id, client_id);

                /* send reply to client */
                position = 0;
                rcode = 0;

                LOCK_MPI();
#if 1
                MPI_Pack_size(5, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);

                var_id = TUNNELFS_REQ_SET_FILESERVER;
                MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT, send_buf,
                         send_buf_size, &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Sending reply to client %i", client_id);
                ptMPI_Send(send_buf, position, MPI_PACKED, client_id,
                           TUNNELFS_REPLY_BASE + clnt_msg_id,
                           TUNNELFS_COMM_WORLD);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_OPEN_CLONE:
            {
                MPI_File *fh_ptr;
                int client_id;
                int pack_size;
                int rcode = 0;
                int server_cookie;
                int num_clients;
                int *clients;

                tunnelfs_fileinfo_t fi;

                position = 0;

                LOG("Cloning request for file handle");

                /* Unpacking message */
                LOCK_MPI();

                /* rank of main client */
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);

                /* server cookie for collective op */
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                tunnelfs_srv_fileinfo_unpack(recv_buf, msg_size, &position,
                                             &fi);

                fh_ptr =
                    tunnelfs_srv_file_create_handle(fi.file_id, fi.comm_id);

                LOG("File handle created");

                LOG("Setting main client to %i", client_id);
                tunnelfs_srv_file_set_main_client(fi.file_id, client_id);
                LOG("Setting main server to %i", msg_status.MPI_SOURCE);
                tunnelfs_srv_file_set_main_server(fi.file_id,
                                                  msg_status.MPI_SOURCE);

                tunnelfs_srv_file_set_mutual(fi.file_id, 1);

                tunnelfs_srv_comm_get_ranks(fi.comm_id, &num_clients,
                                            &clients);

                /* save mpi filename: somehow the name in the ADIO handle is
                 * mutilated */
                tunnelfs_srv_file_set_name(fi.file_id, fi.mpi_filename);

                LOG("Marking this handle as a clone");
                /* set this handle to be a clone, so it is not distributed */
                tunnelfs_srv_file_set_clone(fi.file_id, 1);

                /* set local tunnelfs hints from info */
                tunnelfs_srv_info_eval(fi.info, fi.file_id);

                if ((tunnelfs_srv_file_get_behaviour(fi.file_id) ==
                     TUNNELFS_DIRECT) ||
                    (tunnelfs_srv_file_get_main_server(fi.file_id) ==
                     my_rank))
                {
                    LOG("Calling MPI_File_open");

                    LOCK_MPI();
                    rcode =
                        MPI_File_open(MPI_COMM_TUNNELFS_SELF, fi.mpi_filename,
                                      fi.accessmode, fi.info, fh_ptr);
                    tunnelfs_srv_file_set_opened(fi.file_id, 1);
                    UNLOCK_MPI();
                }

                /* send completion signal to issuing server */
                LOCK_MPI();
#if 1
                MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                position = 0;
                rcode = 0;

                MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&client_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&server_cookie, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("file id: %i client id: %i", fi.file_id, client_id);

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_SERVER_OPEN_CLONE_DONE,
                           TUNNELFS_COMM_WORLD);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_OPEN_CLONE_DONE:
            {
                int file_id;
                int rcode;
                int clnt_msg_id;
                int client_id;
                int pack_size;
                int var_id;
                int server_cookie;

                LOG("Received completion signal for cloned open from %i",
                    msg_status.MPI_SOURCE);

                position = 0;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("client msg id: %i file id: %i client id: %i server cookie: %i", clnt_msg_id, file_id, client_id, server_cookie);

                tunnelfs_srv_collop_done(file_id, server_cookie);

                if (tunnelfs_srv_collop_num_pending(file_id, server_cookie) ==
                    0)
                {
                    dist_list_t *dist_list = NULL;
                    int num_clients = 0;

                    clnt_msg_id =
                        tunnelfs_srv_collop_get_clnt_msg_id(file_id,
                                                            server_cookie);
                    tunnelfs_srv_collop_free(file_id, server_cookie);

                    /* send reply to client */
                    position = 0;
                    rcode = 0;

                    tunnelfs_srv_file_get_distribution(file_id, &dist_list,
                                                       &num_clients);

                    LOCK_MPI();
                    MPI_Pack_size(7 + (2 * num_clients), MPI_INT,
                                  TUNNELFS_COMM_WORLD, &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);

                    MPI_Pack(&clnt_msg_id, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_VAR_FILE_ID;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_VAR_DISTLIST;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&num_clients, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(dist_list, 2 * num_clients, MPI_INT,
                             send_buf, send_buf_size, &position,
                             TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending reply to client %i", client_id);
                    ptMPI_Send(send_buf, position, MPI_PACKED, client_id,
                               TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                }
                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_CLOSE_MUTUAL:
            {
                int file_id;
                int client_id;
                int main_client;
                int clnt_msg_id;
                int server_cookie;
                int handle_type;
                int pack_size;

                MPI_File *fh_ptr;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &main_client, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Closing mutual file for client file %i (%i)", file_id,
                    server_cookie);

                if (tunnelfs_srv_file_handle_exists(file_id))
                {
                    handle_type = 1;
                    fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                    LOCK_MPI();
                    MPI_File_close(fh_ptr);
                    UNLOCK_MPI();

                    tunnelfs_srv_file_free_handle(file_id);
                }
                else
                {
                    while ((client_id =
                            tunnelfs_srv_parts_find_any_client(file_id)) !=
                           -1)
                    {
                        handle_type = 2;
                        fh_ptr =
                            tunnelfs_srv_parts_get_handle(file_id, client_id);

                        LOCK_MPI();
                        MPI_File_close(fh_ptr);
                        UNLOCK_MPI();

                        tunnelfs_srv_parts_destroy(file_id, client_id);
                    }
                }

                /* TODO: check if part is clean and flush if necessary */

                /* report end of operation to main server */
                LOCK_MPI();
#if 1
                MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                position = 0;

                MPI_Pack(&clnt_msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&server_cookie, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&main_client, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_SERVER_CLOSE_MUTUAL_DONE,
                           TUNNELFS_COMM_WORLD);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_CLOSE_MUTUAL_DONE:
            {
                MPI_File *fh_ptr;
                int file_id;
                int client_id;
                int server_cookie;
                int pack_size;
                int clnt_msg_id;
                int rcode;
                int main_client;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Received end signal for closing mutual file %i from %i (%i)", file_id, msg_status.MPI_SOURCE, server_cookie);

                tunnelfs_srv_collop_done(file_id, server_cookie);

                if (tunnelfs_srv_collop_num_pending(file_id, server_cookie) ==
                    0)
                {
                    clnt_msg_id =
                        tunnelfs_srv_collop_get_clnt_msg_id(file_id,
                                                            server_cookie);
                    tunnelfs_srv_collop_free(file_id, server_cookie);

                    LOG("Closing file");

                    fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                    LOCK_MPI();
                    rcode = MPI_File_close(fh_ptr);
                    UNLOCK_MPI();

                    /* send reply to client */
                    position = 0;

                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    main_client = tunnelfs_srv_file_get_main_client(file_id);

                    LOG("Replying to client %i with message id %i",
                        main_client, clnt_msg_id);

                    ptMPI_Send(send_buf, position, MPI_PACKED, main_client,
                               TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                break;
            }
        case TUNNELFS_SERVER_FLUSH_PART:
            {
                int file_id;
                int client_id;
                MPI_File *fh_ptr;
                int server_cookie;
                int pack_size;
                int clnt_msg_id;
                void *cachebuf = NULL;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Flushing cache file for client %i for file %i",
                    client_id, file_id);

                if (tunnelfs_srv_file_handle_exists(file_id))
                {
                    fh_ptr = tunnelfs_srv_file_get_handle(file_id);
                }
                else
                {
                    fh_ptr =
                        tunnelfs_srv_parts_get_handle(file_id, client_id);
                }
                LOCK_MPI();
                MPI_File_sync(*fh_ptr);
                UNLOCK_MPI();

                /* TODO: send IODATA packets */

                /* report end of operation to main server */
                LOCK_MPI();
#if 1
                MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                position = 0;

                MPI_Pack(&clnt_msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&server_cookie, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&client_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_SERVER_FLUSH_PART_DONE,
                           TUNNELFS_COMM_WORLD);

                FREE(cachebuf);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_FLUSH_PART_DONE:
            {
                MPI_File *fh_ptr;
                int file_id;
                int client_id;
                int server_cookie;
                int pack_size;
                int clnt_msg_id;
                int rcode;
                int main_client;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &server_cookie, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                tunnelfs_srv_collop_done(file_id, server_cookie);
                LOG("Received end signal for collective operation %i for file %i on %i", server_cookie, file_id, msg_status.MPI_SOURCE);

                if (tunnelfs_srv_collop_num_pending(file_id, server_cookie) ==
                    0)
                {
                    clnt_msg_id =
                        tunnelfs_srv_collop_get_clnt_msg_id(file_id,
                                                            server_cookie);
                    tunnelfs_srv_collop_free(file_id, server_cookie);

                    LOG("Synchronizing file");

                    fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                    LOCK_MPI();
                    rcode = MPI_File_sync(*fh_ptr);
                    UNLOCK_MPI();

                    /* send reply to client */
                    position = 0;

                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    main_client = tunnelfs_srv_file_get_main_client(file_id);

                    ptMPI_Send(send_buf, position,
                               MPI_PACKED,
                               main_client,
                               TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_DELEGATED_OPEN:
            {
                tunnelfs_fileinfo_t fi;
                dist_list_t *distribution_list = NULL;
                MPI_File *fh_ptr;
                int client_id;
                int file_id;
                int pack_size = 0;
                int var_id;
                int clnt_msg_id = 0;
                int postpone_reply = 0;

                LOG("Forwarded Open Request");

                position = 0;

                /* Unpacking message */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* Unpacking relevant file information */
                tunnelfs_srv_fileinfo_unpack(recv_buf, msg_size, &position,
                                             &fi);

                fh_ptr =
                    tunnelfs_srv_file_create_handle(fi.file_id, fi.comm_id);

                LOG("File handle created %i.", fi.file_id);

                LOG("Setting main client to %i", client_id);
                tunnelfs_srv_file_set_main_client(fi.file_id, client_id);
                LOG("Setting main server to %i", my_rank);
                tunnelfs_srv_file_set_main_server(fi.file_id, my_rank);

                LOCK_MPI();
                rcode = MPI_File_open(MPI_COMM_TUNNELFS_SELF, fi.mpi_filename,
                                      fi.accessmode, fi.info, fh_ptr);
                UNLOCK_MPI();

                tunnelfs_srv_file_set_opened(fi.file_id, 1);

                tunnelfs_srv_file_set_name(fi.file_id, fi.mpi_filename);

                if (rcode == MPI_SUCCESS)
                {
                    int num_clients = 0;
                    int *clients = NULL;
                    int i = 0;

                    tunnelfs_srv_comm_get_ranks(fi.comm_id, &num_clients,
                                                &clients);
                    LOG("Registering shared file pointer");
                    tunnelfs_srv_fptr_register(fi.file_id);

                    LOG("Retrieving communicator %i with %i mpi processes",
                        fi.comm_id, num_clients);

                    /* creating default file view for all clients */
                    for (i = 0; i < num_clients; i++)
                    {
                        tunnelfs_srv_fileview_set(clients[i], fi.file_id,
                                                  (MPI_Offset) 0,
                                                  MPI_BYTE, MPI_BYTE);
                        LOG("Creating default file view for %i on file %i",
                            clients[i], fi.file_id);
                    }

                    tunnelfs_srv_info_eval(fi.info, fi.file_id);

                    tunnelfs_srv_file_distribute(fi.file_id, clnt_msg_id);

                    if (tunnelfs_srv_file_is_mutual(fi.file_id))
                        postpone_reply = 1;

                }
                else
                {
                    LOG("ERROR: could not open file!");

                    position = 0;
                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    msg_id = TUNNELFS_NEXT_MSG_ID;

                    MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending reply to client: %i", client_id);

                    ptMPI_Send(send_buf, position, MPI_PACKED,
                               client_id, TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                if (!postpone_reply)
                {
                    int dist_size = 0;

                    /* retrieve distribution */
                    tunnelfs_srv_file_get_distribution(fi.file_id,
                                                       &distribution_list,
                                                       &dist_size);

                    /* forward request result to client */
                    position = 0;
                    LOCK_MPI();
                    MPI_Pack_size(8 + 2 * dist_size, MPI_INT,
                                  TUNNELFS_COMM_WORLD, &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);

                    msg_id = TUNNELFS_NEXT_MSG_ID;

                    MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_VAR_FILE_ID;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_VAR_COMM_ID;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(fi.comm_id), 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_REQ_SET_FILESERVER;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&my_rank, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                    var_id = TUNNELFS_VAR_DISTLIST;
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&dist_size, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(distribution_list, 2 * dist_size, MPI_INT,
                             send_buf, send_buf_size, &position,
                             TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending reply to client: %i", client_id);

                    ptMPI_Send(send_buf, position, MPI_PACKED,
                               client_id, TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_DELEGATED_DELETE:
            {
                int client_id;
                int filename_len = 0;
                char *filename = NULL;
                int pack_size = 0;
                int position = 0;
                int retries = 10;

                LOG("Forwarded Delete Request");

                /* get message id  */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &filename_len,
                           1, MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                if (filename_len > 0)
                    ALLOC(filename, filename_len + 1);

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, filename,
                           filename_len, MPI_CHAR, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();
                /* make sure char array is closed properly */
                filename[filename_len] = '\0';

                do
                {
                    LOCK_MPI();
                    rcode = MPI_File_delete(filename, MPI_INFO_NULL);
                    UNLOCK_MPI();

                    if (rcode == 0)
                        retries = 0;
                    else
                    {
                        LOG("Return code of delete for %s was: %i", filename,
                            rcode);
                        sleep(1);
                    }
                }
                while (--retries > 0);

                /* send reply */
                position = 0;
                LOCK_MPI();
#if 1
                MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED, client_id,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                FREE(filename);
                
                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_DELEGATED_CLOSE:
            {
                int file_id = 0;
                int comm_id = 0;
                int pack_size = 0;
                int send_reply = 0;
                int main_client = 0;
                int clnt_msg_id = 0;
                MPI_File *fh_ptr;

                LOG("Delegated close request");
                /* get message id  */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size,
                           &position, &clnt_msg_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size,
                           &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("File id: %i", file_id);

                if (tunnelfs_srv_file_is_mutual(file_id))
                {
                    int *servers = NULL;
                    int size;
                    int i;
                    int server_cookie = TUNNELFS_NEXT_COLL_ID;

                    LOG("Have to remove mutual handles first");

                    tunnelfs_srv_file_get_serverlist(file_id, &servers,
                                                     &size);

                    LOG("Got list of servers");

#if 1
                    LOCK_MPI();
                    MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);
                    UNLOCK_MPI();

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    tunnelfs_srv_collop_start(file_id, server_cookie, size,
                                              clnt_msg_id);

                    for (i = 0; i < size; i++)
                    {
                        if (servers[i] != my_rank)
                        {
                            int main_client;

                            LOG("Closing mutual file handle on %i",
                                servers[i]);

                            main_client =
                                tunnelfs_srv_file_get_main_client(file_id);

                            position = 0;
                            LOCK_MPI();
                            MPI_Pack(&clnt_msg_id, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&file_id, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&server_cookie, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&main_client, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            ptMPI_Send(send_buf, position,
                                       MPI_PACKED, servers[i],
                                       TUNNELFS_SERVER_CLOSE_MUTUAL,
                                       TUNNELFS_COMM_WORLD);
                        }
                        else
                        {
                            LOG("Removing local part of collective operation");
                            tunnelfs_srv_collop_done(file_id, server_cookie);
                        }
                    }
                    FREE(servers);

                    if (!tunnelfs_srv_collop_num_pending(file_id,
                                                         server_cookie))
                        send_reply = 1;
                }
                else
                    send_reply = 1;

                if (send_reply)
                {
                    fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                    LOCK_MPI();
                    rcode = MPI_File_close(fh_ptr);
                    UNLOCK_MPI();

                    comm_id = tunnelfs_srv_file_get_comm_id(file_id);
                    /*tunnelfs_srv_comm_unregister(comm_id); */
                    tunnelfs_srv_fptr_unregister(file_id);
                    tunnelfs_srv_file_free_handle(file_id);

                    /* send reply */
                    position = 0;
                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    main_client = tunnelfs_srv_file_get_main_client(file_id);
                    ptMPI_Send(send_buf, position, MPI_PACKED, main_client,
                               TUNNELFS_REPLY_BASE + clnt_msg_id,
                               TUNNELFS_COMM_WORLD);
                    LOG("Reply sent.");
                }

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_WRITE:
            {
                int client_id;
                int file_id;
                int count;
                MPI_Offset offset;
                MPI_Offset fv_disp;
                int ptr_type;
                int fv_etype;
                int fv_ftype;
                int fv_etype_size;
                int num_pending;
                /*void *iobuf = NULL;*/

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &count, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &ptr_type, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &offset, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &num_pending, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Delegated write from %i for client %i on file %i",
                    msg_status.MPI_SOURCE, client_id, file_id);

#if 0
                LOG("client=%i file_id=%i count=%i ptr_type=%i offset=0x%Lx",
                    client_id, file_id, count, ptr_type, offset);
#endif
                /* getting client fileview */
                tunnelfs_srv_fileview_get(client_id, file_id, &fv_disp,
                                          &fv_etype, &fv_ftype);

                tunnelfs_srv_datatype_get_size(fv_etype, &fv_etype_size);

                /*
                   if (count > 0)
                   ALLOC(iobuf, count * fv_etype_size);
                 */

                /* starting io for client, data will be received via
                 * TAG: TUNNELFS_SERVER_IODATA_BASE + client_id */
                LOG("Setting pending io for client %i on file %i", client_id,
                    file_id);
                tunnelfs_srv_pending_io_set(client_id, file_id, num_pending,
                                            count, ptr_type, offset, msg_id);

                LOG("done");
                break;
            }
        case TUNNELFS_SERVER_READ:
            {
                MPI_Offset offset = 0;
                int buf_offset = 0;
                MPI_Offset fv_disp = 0;
                MPI_File *fh_ptr = NULL;
                void *iobuf = NULL;
                int client_id = -1;
                int file_id = -1;
                int count;
                int ptr_type;
                int fv_etype;
                int fv_etype_size;
                int fv_ftype;
                int num_blocks;
                int i;
                int block_count;
                MPI_Status status;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &client_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &count, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &ptr_type, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &offset, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &num_blocks, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Delegated read from %i for client %i on file %i",
                    msg_status.MPI_SOURCE, client_id, file_id);

#if 0
                LOG("client=%i file_id=%i count=%i ptr_type=%i offset=0x%Lx blocks=%i", client_id, file_id, count, ptr_type, offset, num_blocks);
#endif
                /* getting client fileview */
                tunnelfs_srv_fileview_get(client_id, file_id, &fv_disp,
                                          &fv_etype, &fv_ftype);

                tunnelfs_srv_datatype_get_size(fv_etype, &fv_etype_size);

                if (count > 0)
                    ALLOC(iobuf, count * fv_etype_size);

                fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                LOCK_MPI();
                if (ptr_type == ADIO_EXPLICIT_OFFSET)
                {
                    MPI_File_set_view(*fh_ptr, 0, fv_etype, fv_ftype,
                                      "native", MPI_INFO_NULL);
                }
                else
                {
                    MPI_File_set_view(*fh_ptr, fv_disp, fv_etype, fv_ftype,
                                      "native", MPI_INFO_NULL);
                }
                UNLOCK_MPI();

                /* calculating block size for first num_msgs-1 blocks */
                if (num_blocks > 1)
                    block_count = TUNNELFS_MAX_MSG_SIZE / fv_etype_size;
                else
                    block_count = count;

                LOG("Using view (%Li, %i, %i)", fv_disp, fv_etype, fv_ftype);

                buf_offset = 0;
                for (i = 0; i < num_blocks; i++)
                {
                    if (i == num_blocks - 1)
                        block_count = count - buf_offset;

                    LOG("Reading at offset 0x%Lx", offset + buf_offset);

                    LOCK_MPI();
                    rcode =
                        MPI_File_read_at(*fh_ptr, offset + buf_offset,
                                         iobuf, block_count, fv_etype,
                                         &status);
                    UNLOCK_MPI();

                    LOG("Sending %i blocks of type %i", block_count,
                        fv_etype);

                    ptMPI_Send(iobuf, block_count, fv_etype,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_SERVER_IODATA_BASE +
                               client_id, TUNNELFS_COMM_WORLD);

                    buf_offset += block_count;
                }

                FREE(iobuf);

                LOG("done");
                break;
            }
        default:
            {
                int client_id =
                    msg_status.MPI_TAG - TUNNELFS_SERVER_IODATA_BASE;
                int world_size = 0;

                /* sanity check on message tag */
                LOCK_MPI();
                MPI_Comm_size(TUNNELFS_COMM_WORLD, &world_size);
                UNLOCK_MPI();

                if ((client_id >= 0) && (client_id < world_size))
                {
                    int file_id;
                    int count = 0;
                    int ptr_type;
                    MPI_Offset offset = 0;
                    void *iobuf = NULL;
                    int iobuf_offset = 0;
                    int msg_id;

                    MPI_Offset fv_disp = 0;
                    int fv_etype = 0;
                    int fv_etype_size = 0;
                    int fv_ftype = 0;
                    int blocks = 0;

                    MPI_File *fh_ptr = NULL;
                    /*MPI_Status status;*/

                    LOG("Received delegated io data from server %i for client %i", msg_status.MPI_SOURCE, client_id);

                    /* get parameters of pending io for the client */
                    LOG("Retrieving parameters for pending io");
                    tunnelfs_srv_pending_io_get(client_id, &file_id, &count,
                                                &ptr_type, &offset,
                                                &iobuf_offset, &msg_id);

                    LOG("Retrieving file view for client %i on file %i",
                        client_id, file_id);
                    tunnelfs_srv_fileview_get(client_id, file_id, &fv_disp,
                                              &fv_etype, &fv_ftype);

                    tunnelfs_srv_datatype_get_size(fv_etype, &fv_etype_size);

                    LOCK_MPI();
                    MPI_Get_count(&msg_status, fv_etype, &blocks);
                    UNLOCK_MPI();

                    LOG("Aquiring buffer");
                    iobuf = tunnelfs_srv_buffer_get_block(client_id);
                    LOG("Buffer block at address %x", iobuf);

                    LOG("Receiving buffer");
                    ptMPI_Recv(iobuf, blocks, fv_etype,
                               tunnelfs_service_status.MPI_SOURCE,
                               tunnelfs_service_status.MPI_TAG,
                               TUNNELFS_COMM_WORLD, &msg_status);

                    /* Message is retrieved. Let's signal this to probe thread. */
                    pthread_mutex_lock(&pario_probe_sync);
                    message_is_processing = 0;
                    pthread_cond_signal(&message_retrieved_from_queue);
                    pthread_mutex_unlock(&pario_probe_sync);

                    if ((tunnelfs_srv_file_get_behaviour(file_id) ==
                         TUNNELFS_DIRECT) ||
                        (tunnelfs_srv_file_get_main_server(file_id) ==
                         my_rank))
                    {
                        LOG("Data target is local");

                        fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                        /* Explicit offset is counted from the beginning of
                         * the file */
                        /*
                           if (ptr_type == ADIO_EXPLICIT_OFFSET)
                           fv_disp = 0;
                         */

                        /*
                           LOCK_MPI();
                           MPI_File_set_view(*fh_ptr, fv_disp, fv_etype,
                           fv_ftype, "native", MPI_INFO_NULL);

                           LOG("Using view (%Li, %i, %i)", fv_disp, fv_etype,
                           fv_ftype);
                           LOG("Writing at offset 0x%Lx", offset);

                           rcode =
                           MPI_File_write_at(*fh_ptr, offset, recv_buf,
                           blocks, fv_etype, &status);
                           UNLOCK_MPI();
                         */
                        tunnelfs_srv_buffer_write(iobuf, file_id, client_id,
                                                  offset, blocks, fv_etype);

                        tunnelfs_srv_pending_io_inc_offset(client_id, blocks);
                        tunnelfs_srv_pending_io_dec(client_id);
                    }
                    else if (tunnelfs_srv_file_get_behaviour(file_id)
                             == TUNNELFS_ROUTE)
                    {
                        LOG("Data will be forwarded to client %i", client_id);

                        ptMPI_Send(iobuf, blocks, fv_etype, client_id,
                                   TUNNELFS_IODATA, TUNNELFS_COMM_WORLD);

                        tunnelfs_srv_pending_io_dec(client_id);
                        tunnelfs_srv_buffer_release_block(iobuf);
                    }
                    else
                        ERR(TUNNELFS_ERR_NOT_FOUND);

                    if (tunnelfs_srv_pending_io_is_done(client_id))
                    {
                        tunnelfs_srv_buffer_flush_queue(client_id);

                        /*
                           FREE(iobuf);
                         */
                    }

                }
                else
                {
                    LOG("Received message of unknown type %i from %i\n",
                        msg_status.MPI_TAG, msg_status.MPI_SOURCE);
                }

                break;
            }                   /* case */
        }                       /* switch */
    }                           /* while */

    FREE(send_buf);
    FREE(recv_buf);

    LOG("At end of thread service");

    pthread_mutex_unlock(&tunnelfs_service_sync);

    pthread_exit(NULL);
    return NULL;
}
