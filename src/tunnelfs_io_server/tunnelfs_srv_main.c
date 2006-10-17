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
#include "mpi.h"
#include "mpio.h"
#include "adio.h"
#include "ad_memfs.h"
#include "ad_tunnelfs_common.h"
#include "ad_tunnelfs_msg.h"
#include "ad_tunnelfs_err.h"
#include "ad_tunnelfs_datatype.h"
#include "ad_tunnelfs_server.h"
#include "ad_tunnelfs_globals.h"
#include "pario_threads.h"
#include "pario_probe.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Thread Management Variables
 */
pthread_mutex_t tunnelfs_main_sync = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_for_tunnelfs_main = PTHREAD_COND_INITIALIZER;
MPI_Status tunnelfs_main_status;
int tunnelfs_main_status_is_empty = 1;

void *tunnelfs_srv_main(void *args)
{
    tunnelfs_version_t tver;

    int last_msg;
    int msg_size;
    int msg_id;
    int position = 0;

    int recv_buf_size = 0;
    void *recv_buf = NULL;
    int send_buf_size = 0;
    void *send_buf = NULL;
    int iodata_buf_size = 0;
    void *iodata_buf = NULL;

    int *stop_server = NULL;
    int num_servers = 0;
    int num_clients = 0;
    int my_rank = 0;

    int rcode = 0;

    MPI_Status msg_status;

    num_servers = ((tunnelfs_thread_args_t *) args)->num_servers;
    num_clients = ((tunnelfs_thread_args_t *) args)->num_clients;
    my_rank = ((tunnelfs_thread_args_t *) args)->mpi_rank;
    stop_server = ((tunnelfs_thread_args_t *) args)->stop_server;

    /* Allocate initial message buffers */
    send_buf_size = (TUNNELFS_MAX_MSG_SIZE / 4);
    ALLOC(send_buf, send_buf_size);
    recv_buf_size = (TUNNELFS_MAX_MSG_SIZE / 4);
    ALLOC(recv_buf, recv_buf_size);
    iodata_buf_size = TUNNELFS_MAX_MSG_SIZE;
    ALLOC(iodata_buf, iodata_buf_size);

    /* initially locking mutex */
    pthread_mutex_lock(&tunnelfs_main_sync);

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
                (msg_status.MPI_TAG >= 0x0000) &&
                (msg_status.MPI_TAG < 0x1000))
            {
                /* Receive message into recv_buf 
                 * - io data is received directly in case statement! */
                if (msg_status.MPI_TAG != TUNNELFS_IODATA)
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
        memset(&tunnelfs_main_status, 0, sizeof(MPI_Status));
        tunnelfs_main_status_is_empty = 1;

        LOG("Waiting for signal");
        /* wait for signal from probe thread */
        while ((tunnelfs_main_status_is_empty) && !pario_shutdown)
            pthread_cond_wait(&message_for_tunnelfs_main, &tunnelfs_main_sync);

        /*if (*stop_server)*/
        if (pario_shutdown)
            break;

        LOG("Received Signal on message from %i (0x%x)",
            tunnelfs_main_status.MPI_SOURCE, tunnelfs_main_status.MPI_TAG);

        if (tunnelfs_main_status.MPI_TAG != TUNNELFS_IODATA)
        {
            LOCK_MPI();
            MPI_Get_count(&tunnelfs_main_status, MPI_PACKED, &msg_size);

            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, msg_size);

            MPI_Recv(recv_buf, msg_size, MPI_PACKED,
                     tunnelfs_main_status.MPI_SOURCE,
                     tunnelfs_main_status.MPI_TAG, TUNNELFS_COMM_WORLD,
                     &msg_status);
            UNLOCK_MPI();

            /* Message is retrieved. Let's signal this to probe thread.  */
            pthread_mutex_lock(&pario_probe_sync);
            message_is_processing = 0;
            pthread_cond_signal(&message_retrieved_from_queue);
            pthread_mutex_unlock(&pario_probe_sync);

        }

        /* INFO: IO Data is received in case statement TUNNELFS_IODATA */
#endif

        last_msg = msg_status.MPI_TAG;
        position = 0;

        LOG("------------------------------------------------------------");

        if (msg_status.MPI_TAG != TUNNELFS_IODATA)
        {
            position = 0;

            LOCK_MPI();
            MPI_Unpack(recv_buf, msg_size, &position, &msg_id, 1, MPI_INT,
                       TUNNELFS_COMM_WORLD);
            UNLOCK_MPI();

            LOG("Received message from %i with id %i", msg_status.MPI_SOURCE,
                msg_id);
        }

#if 0
        switch (msg_status.MPI_TAG)
#else
        switch (tunnelfs_main_status.MPI_TAG)
#endif
        {
        case TUNNELFS_INIT:
            {
                int arg_count = 0;
                char **arg_list = NULL;
                int i;
                int min_size = 0;
                int temp_size = 0;
                int domain_len = 0;
                char *domain = NULL;
                int var_id = 0;
                int spos = 0;
                int *servers = NULL;
                int servers_size = 0;

                LOG("Initializing tunnelfs communication");

                /* unpacking recv_buffer */
                LOCK_MPI();

                MPI_Unpack(recv_buf, msg_size, &position, &tver, 2, MPI_INT,
                           TUNNELFS_COMM_WORLD);

                MPI_Unpack(recv_buf, msg_size, &position, &domain_len, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);

                if (domain_len > 0)
                {
                    ALLOC(domain, domain_len + 1);
                    MPI_Unpack(recv_buf, msg_size, &position, domain,
                               domain_len, MPI_CHAR, TUNNELFS_COMM_WORLD);
                    domain[domain_len] = '\0';
                    tunnelfs_srv_client_fs_domain_insert(msg_status.
                                                         MPI_SOURCE, domain);
                }

                LOG("Client filesystem domain '%s' registered", domain);

                /* synchronizing client fs domain with slaves */
                MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD, &temp_size);
                min_size += temp_size;
                MPI_Pack_size(domain_len, MPI_CHAR, TUNNELFS_COMM_WORLD,
                              &temp_size);
                min_size += temp_size;

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

                spos = 0;

                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &spos, TUNNELFS_COMM_WORLD);
                var_id = TUNNELFS_VAR_CLIENTFSDOM;
                MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                         &spos, TUNNELFS_COMM_WORLD);
                MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT, send_buf,
                         send_buf_size, &spos, TUNNELFS_COMM_WORLD);
                MPI_Pack(&domain_len, 1, MPI_INT, send_buf, send_buf_size,
                         &spos, TUNNELFS_COMM_WORLD);
                MPI_Pack(domain, domain_len, MPI_CHAR, send_buf,
                         send_buf_size, &spos, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                tunnelfs_srv_get_serverlist(&servers, &servers_size);

                for (i = 0; i < servers_size; i++)
                {
                    if (my_rank != servers[i])
                        ptMPI_Send(send_buf, spos, MPI_PACKED,
                                   servers[i],
                                   TUNNELFS_SERVER_SETUP,
                                   TUNNELFS_COMM_WORLD);
                }
                /* starting command line argument handling */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &arg_count, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* 
                 * if command line arguments are given, allocate memory for 
                 * an array to hold all pointers to them
                 */
                if (arg_count > 0)
                    ALLOC(arg_list, arg_count * sizeof(char *));

                for (i = 0; i < arg_count; i++)
                {
                    int tmp;
                    /* get i-th argument length */
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, &tmp, 1,
                               MPI_INT, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    /* allocate memory for argument */
                    arg_list[i] = NULL;
                    ALLOC(arg_list[i], tmp + 1);

                    /* unpack argument from buffer */
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, arg_list[i],
                               tmp, MPI_CHAR, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }

                /* TODO: 
                 * process command line arguments of clients
                 */

                /* send reply */
                position = 0;
                rcode = 0;

                LOCK_MPI();
#if 1
                MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &min_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
#endif
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                for (i = 0; i < arg_count; i++)
                {
                    FREE(arg_list[i]);
                }
                FREE(arg_list);
                FREE(servers);
                FREE(domain);

                LOG("done");
                break;
            }
        case TUNNELFS_END:
            {
                int min_size = 0;

                LOG("Finalizing tunnelfs communication for client %i",
                    msg_status.MPI_SOURCE);

                if (msg_size != sizeof(int))
                {
                    LOG("Received malformed connections shutdown request");
                }

                /* send reply */
                position = 0;
                rcode = 0;

                LOCK_MPI();
#if 1
                MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &min_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
#endif
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                num_clients--;
                if (num_clients == 0)
                {
                    /* send shutdown request to slaves */
                    tunnelfs_srv_shutdown_slaves();
                    /* stop local service */
                    pario_shutdown = 1;
                    /* *stop_server = 1; */

                    /* Signal other threads that are possibly waiting. */
                    pthread_cond_signal(&message_for_tunnelfs_service);
                    /*
                       pthread_cond_signal(&message_for_memfs_main);
                       pthread_cond_signal(&message_for_memfs_service);
                     */
                }

                LOG("done");
                break;
            }
        case TUNNELFS_SETUP:
            {
                int var_id = 0;
                int pack_size = 0;
                int tmp_size = 0;
                int comm_id = -1;
                int file_id = -1;
                int atom = -1;
                ADIO_Offset shared_ptr = -1;
                ADIO_Offset filesize = -1;

                int send_comm = 0;
                int send_shfptr = 0;
                int send_size = 0;
                int send_atom = 0;

                LOG("Negotiating parameters with client");

                /* get message id  */
                LOCK_MPI();

                /* calculating size for reply package */
                MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
                pack_size += tmp_size;
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
                            int i = 0;
#ifdef LOGGING
                            char tmp[10];
                            char *ranks_str = NULL;
                            int str_len = 0;
#endif
                            int *servers = NULL;
                            int servers_size = 0;

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
                            /* initially rank_str is empty */
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

                            /* sending communicator to all other io servers */
                            tunnelfs_srv_get_serverlist(&servers,
                                                        &servers_size);
                            for (i = 0; i < servers_size; i++)
                            {
                                if (my_rank != servers[i])
                                    tunnelfs_srv_comminfo_send(comm_id,
                                                               servers[i]);
                            }

                            /* calculating size for reply package */

                            LOCK_MPI();
                            MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            UNLOCK_MPI();

                            send_comm = 1;
                            break;
                        }
                    case TUNNELFS_REQ_SHARED_PTR:
                        {
                            LOG("Handling shared file pointer request (client read)");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            shared_ptr =
                                tunnelfs_srv_fptr_get_shared(file_id);

                            LOG("Shared file pointer of file %i has offset of 0x%Lx", file_id, shared_ptr);

                            /* calculating size for reply package */
                            LOCK_MPI();
                            MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            pack_size += tmp_size;
                            /* offsets are system dependend */
                            MPI_Pack_size(1, TUNNELFS_OFFSET,
                                          TUNNELFS_COMM_WORLD, &tmp_size);
                            UNLOCK_MPI();
                            pack_size += tmp_size;

                            send_shfptr = 1;
                            break;
                        }
                    case TUNNELFS_REQ_SET_SHARED_PTR:
                        {

                            LOG("Handling shared file pointer request (client set)");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &shared_ptr, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            assert(shared_ptr >= 0);

                            tunnelfs_srv_fptr_set_shared(file_id, shared_ptr);

#if 0
                            LOG("Set shared file pointer of file %i to %Li",
                                file_id, shared_ptr);
#endif

                            LOCK_MPI();
                            /* calculating size for reply package */
                            MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            pack_size += tmp_size;
                            /* offsets are of type long long int */
                            MPI_Pack_size(1, TUNNELFS_OFFSET,
                                          TUNNELFS_COMM_WORLD, &tmp_size);
                            pack_size += tmp_size;
                            UNLOCK_MPI();

                            send_shfptr = 1;
                            break;
                        }
                    case TUNNELFS_REQ_PREALLOCATE:
                        {
                            MPI_File *fh_ptr;
                            int rcode = 0;

                            LOG("Preallocating file");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &filesize, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                            LOCK_MPI();
                            rcode = MPI_File_preallocate(*fh_ptr, filesize);
                            UNLOCK_MPI();

                            if (rcode != MPI_SUCCESS)
                            {
                                LOG("ERROR: Unable to preallocate requested filesize.");
                            }

                            /* check wether it was changed */
                            LOCK_MPI();
                            MPI_File_get_size(*fh_ptr, &filesize);

                            MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            pack_size += tmp_size;
                            MPI_Pack_size(1, TUNNELFS_OFFSET,
                                          TUNNELFS_COMM_WORLD, &tmp_size);
                            pack_size += tmp_size;
                            UNLOCK_MPI();

                            send_size = 1;
                            break;
                        }
                    case TUNNELFS_REQ_RESIZE:
                        {
                            MPI_File *fh_ptr;
                            int rcode = 0;

                            LOG("Resizing file");
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &filesize, 1, TUNNELFS_OFFSET,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                            LOCK_MPI();
                            rcode = MPI_File_set_size(*fh_ptr, filesize);
                            UNLOCK_MPI();

                            if (rcode != MPI_SUCCESS)
                            {
                                LOG("ERROR: Unable to resize requested file.");
                            }

                            /* check wether it was changed */
                            LOCK_MPI();
                            MPI_File_get_size(*fh_ptr, &filesize);

                            MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            pack_size += tmp_size;
                            MPI_Pack_size(1, TUNNELFS_OFFSET,
                                          TUNNELFS_COMM_WORLD, &tmp_size);
                            UNLOCK_MPI();
                            pack_size += tmp_size;

                            send_size = 1;
                            break;
                        }
                    case TUNNELFS_REQ_SET_ATOMICITY:
                        {
                            MPI_File *fh_ptr;

                            LOG("Setting atomicity");
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            MPI_Unpack(recv_buf, msg_size, &position, &atom,
                                       1, MPI_INT, TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                            LOCK_MPI();
                            rcode = MPI_File_set_atomicity(*fh_ptr, atom);
                            UNLOCK_MPI();

                            if (rcode != MPI_SUCCESS)
                            {
                                LOG("ERROR: Unable to set atomicity on file.");
                            }

                            LOCK_MPI();
                            MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            UNLOCK_MPI();
                            pack_size += tmp_size;

                            send_atom = 1;
                            break;
                        }
                    case TUNNELFS_REQ_SET_INFO:
                        {
                            int num_hints = 0;
                            MPI_Info info = MPI_INFO_NULL;

                            LOG("Setting remote info object");

                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);

                            LOG("File id: %i", file_id);
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &num_hints, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();
#if 1
                            tunnelfs_srv_info_create(recv_buf, msg_size,
                                                     &position, num_hints,
                                                     &info);
                            tunnelfs_srv_info_eval(info, file_id);

                            /*TODO: save info to file handle */
#endif
                            break;
                        }
                    case TUNNELFS_REQ_FILESIZE:
                        {
                            MPI_File *fh_ptr;

                            LOG("Getting file size");
                            LOCK_MPI();
                            MPI_Unpack(recv_buf, msg_size, &position,
                                       &file_id, 1, MPI_INT,
                                       TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                            LOCK_MPI();
                            MPI_File_get_size(*fh_ptr, &filesize);

                            MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD,
                                          &tmp_size);
                            pack_size += tmp_size;
                            MPI_Pack_size(1, TUNNELFS_OFFSET,
                                          TUNNELFS_COMM_WORLD, &tmp_size);
                            pack_size += tmp_size;
                            UNLOCK_MPI();

                            send_size = 1;
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

                position = 0;
                rcode = 0;
                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

                LOCK_MPI();
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* pack comm_id */
                if (send_comm)
                {
                    var_id = TUNNELFS_VAR_COMM_ID;
                    LOCK_MPI();
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&comm_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }

                if (send_shfptr)
                {
                    var_id = TUNNELFS_VAR_SHARED_PTR;
                    LOCK_MPI();
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&shared_ptr, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
                if (send_size)
                {
                    var_id = TUNNELFS_VAR_FILESIZE;
                    LOCK_MPI();
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&filesize, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
                if (send_atom)
                {
                    var_id = TUNNELFS_VAR_ATOMICITY;
                    LOCK_MPI();
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&atom, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
#if 0
                if (send_server)
                {
                    var_id = TUNNELFS_REQ_SET_FILESERVER;
                    LOCK_MPI();
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&fileserver, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
#endif
                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                LOG("done");
                break;
            }
        case TUNNELFS_SET_VIEW:
            {
                MPI_Offset disp = 0;
                int etype = 0;
                int ftype = 0;
                MPI_Datatype elemtype = 0;
                MPI_Datatype filetype = 0;
                int file_id;
                int fileserver = -1;
                int pack_size = 0;

                int num_hints = 0;
                MPI_Info info = MPI_INFO_NULL;

                int var_id;
                int main_server;

                LOG("File View Set Request for %i", msg_status.MPI_SOURCE);

                fileserver = my_rank;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position,
                           &file_id, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position,
                           &disp, 1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position,
                           &etype, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position,
                           &ftype, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position,
                           &num_hints, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* rebuild info object from buffer */
                tunnelfs_srv_info_create(recv_buf, msg_size, &position,
                                         num_hints, &info);

                tunnelfs_srv_datatype_get_type(msg_status.
                                               MPI_SOURCE, etype, &elemtype);
                LOG("Datatype ID=%i -> MPI Datatype=%i", etype, elemtype);

                tunnelfs_srv_datatype_get_type(msg_status.
                                               MPI_SOURCE, ftype, &filetype);
                LOG("Datatype ID=%i -> MPI Datatype=%i", ftype, filetype);

                /* saving file view */
                LOG("Saving file view for %i on file %i: disp=%Li etype=%i ftype=%i", msg_status.MPI_SOURCE, file_id, disp, elemtype, filetype);
                tunnelfs_srv_fileview_set(msg_status.MPI_SOURCE,
                                          file_id, disp, elemtype, filetype);

                if ((main_server =
                     tunnelfs_srv_file_get_main_server(file_id)) != my_rank)
                {
                    int temp_size;

                    /* send file view data to forward-server */
                    LOCK_MPI();
#if 1
                    MPI_Pack_size(7, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);
                    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    position = 0;
                    var_id = TUNNELFS_VAR_FILEVIEW;

                    MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&disp, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&etype, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&ftype, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending new file view to main server");
                    ptMPI_Send(send_buf, position, MPI_PACKED, main_server,
                               TUNNELFS_SERVER_SETUP, TUNNELFS_COMM_WORLD);
                }
                else
                {
                    /* let only main server figure out the distribution and
                     * propagate the new distribution to the other servers */

                    /* set hints from info object */
                    tunnelfs_srv_info_eval(info, file_id);

                    /* decide distribution after hints are set */
                    tunnelfs_srv_file_distribute(file_id, msg_id);
                }

                if (fileserver == my_rank)
                {
                    /* reply to client */
                    position = 0;
                    rcode = 0;

                    LOCK_MPI();
#if 1
                    MPI_Pack_size(5, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
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
                    MPI_Pack(&fileserver, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    ptMPI_Send(send_buf, position, MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE + msg_id,
                               TUNNELFS_COMM_WORLD);

                }
                LOG("done");
                break;
            }
        case TUNNELFS_REPLY:
            {
                LOG("Got reply");
                LOG("done");
                break;
            }
        case TUNNELFS_DATATYPE:
            {
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
                int pack_size = 0;
                int temp_size = 0;

                int *servers = NULL;
                int servers_size = 0;
                int var_id = 0;

                LOG("Remote datatype definition");

                /* get message id  */
                LOCK_MPI();

                /* get client specific type id */
                MPI_Unpack(recv_buf, msg_size, &position, &type_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Type id: %i", type_id);

                /* prepare new datatype creation */
                new_type =
                    tunnelfs_srv_datatype_prepare(msg_status.MPI_SOURCE,
                                                  type_id);

                /* get integer vector */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &num_int, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                if (num_int > 0)
                {
                    ALLOC(intvec, num_int * sizeof(int));
                    if (intvec == NULL)
                        ERR(TUNNELFS_ERR_ALLOC);
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, intvec, num_int,
                               MPI_INT, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
                for (i = 0; i < num_int; i++)
                {
                    LOG("Int[%i]: %i", i, intvec[i]);
                }

                /* get address vector */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &num_addr, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                if (num_addr > 0)
                {
                    ALLOC(addrvec, num_addr * sizeof(MPI_Aint));
                    if (addrvec == NULL)
                        ERR(TUNNELFS_ERR_ALLOC);
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, addrvec,
                               num_addr, TUNNELFS_AINT, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                }
                for (i = 0; i < num_addr; i++)
                {
                    LOG("Address[%i]: %li", i, addrvec[i]);
                }

                /* get datatype vector */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &num_types, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                if (num_types > 0)
                {
                    ALLOC(typevec, num_types * sizeof(int));
                    if (typevec == NULL)
                        ERR(TUNNELFS_ERR_ALLOC);

                    ALLOC(datatypevec, num_types * sizeof(MPI_Datatype));
                    if (datatypevec == NULL)
                        ERR(TUNNELFS_ERR_ALLOC);
                    LOCK_MPI();
                    MPI_Unpack(recv_buf, msg_size, &position, typevec,
                               num_types, MPI_INT, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    for (i = 0; i < num_types; i++)
                    {
                        tunnelfs_srv_datatype_get_type(msg_status.MPI_SOURCE,
                                                       typevec[i],
                                                       &(datatypevec[i]));
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
                MPI_Unpack(recv_buf, msg_size, &position, &combiner, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
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
                        MPI_Type_contiguous(intvec[0], datatypevec[0],
                                            new_type);
                        break;
                    }
                case MPI_COMBINER_VECTOR:
                    {
                        MPI_Type_vector(intvec[0], intvec[1], intvec[2],
                                        datatypevec[0], new_type);
                        break;
                    }
                case MPI_COMBINER_HVECTOR:
                    {
                        MPI_Type_hvector(intvec[0], intvec[1], addrvec[0],
                                         datatypevec[0], new_type);
                        break;
                    }
                case MPI_COMBINER_INDEXED:
                    {
                        MPI_Type_indexed(intvec[0], &(intvec[1]),
                                         &(intvec[intvec[0]]), datatypevec[0],
                                         new_type);
                        break;
                    }
                case MPI_COMBINER_HINDEXED:
                    {
                        MPI_Type_indexed(intvec[0], &(intvec[1]),
                                         &(intvec[intvec[0]]), datatypevec[0],
                                         new_type);
                        break;
                    }
                case MPI_COMBINER_STRUCT:
                    {
                        MPI_Type_struct(intvec[0], &(intvec[1]), addrvec,
                                        datatypevec, new_type);
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

                LOG("Datatype %i committed on serverside.", *new_type);

                tunnelfs_srv_datatype_calc_param(*new_type);

                LOG("Datatype parameters cached");

                /* new type is commited and can be retrieved via
                 * tunnelfs_srv_datatype_get_type() */

                /* synchronize with other servers before replying to client */
                LOG("synchronizing datatype with other servers.");

                LOCK_MPI();
                pack_size = 0;
                /* pack message and send it */
                MPI_Pack_size(6 + num_int + num_types, MPI_INT,
                              TUNNELFS_COMM_WORLD, &temp_size);
                pack_size += temp_size;

                MPI_Pack_size(num_addr, TUNNELFS_AINT, TUNNELFS_COMM_WORLD,
                              &temp_size);
                pack_size += temp_size;

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);

                position = 0;

                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);

                var_id = TUNNELFS_VAR_DATATYPE;
                MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT, send_buf,
                         send_buf_size, &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&type_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&num_int, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                if (num_int > 0)
                    MPI_Pack(intvec, num_int, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&num_addr, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                if (num_addr > 0)
                    MPI_Pack(addrvec, num_addr, TUNNELFS_AINT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&num_types, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                if (num_types > 0)
                    MPI_Pack(typevec, num_types, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);

                MPI_Pack(&combiner, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                tunnelfs_srv_get_serverlist(&servers, &servers_size);
                for (i = 0; i < servers_size; i++)
                    if (my_rank != servers[i])
                        ptMPI_Send(send_buf, position, MPI_PACKED, servers[i],
                                   TUNNELFS_SERVER_SETUP,
                                   TUNNELFS_COMM_WORLD);

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

                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                FREE(servers);
                FREE(intvec);
                FREE(addrvec);
                FREE(typevec);
                FREE(datatypevec);

                LOG("done");
                break;
            }
        case TUNNELFS_OPEN:
            {
                tunnelfs_fileinfo_t fi;
                MPI_File *fh_ptr;
                int pack_size = 0;
                int temp_size = 0;
                int var_id;

                int next_server = TUNNELFS_GLOBAL_MASTER;
                int num_clients = 0;
                int *clients = NULL;

                int postpone_reply = 0;

                dist_list_t *distribution_list = NULL;

                LOG("Open Request");

                /* unpack file information from buffer */
                tunnelfs_srv_fileinfo_unpack(recv_buf, msg_size, &position,
                                             &fi);

                /* creating global file id */
                fi.file_id = TUNNELFS_NEXT_FILE_ID;

                /* get next responsible io server */
                next_server = tunnelfs_srv_schedule_current(fi.fs_domain);

                assert(next_server != -1);

                LOG("Next server in schedule: %i", next_server);

                if (next_server == my_rank)
                {
                    LOG("Client request handled locally");

                    fh_ptr =
                        tunnelfs_srv_file_create_handle(fi.file_id,
                                                        fi.comm_id);

                    LOG("File handle created.");

                    LOG("Filename used: %s", fi.mpi_filename);

                    tunnelfs_srv_file_set_main_client(fi.file_id,
                                                      msg_status.MPI_SOURCE);
                    tunnelfs_srv_file_set_main_server(fi.file_id, my_rank);

                    LOCK_MPI();
                    rcode =
                        MPI_File_open(MPI_COMM_TUNNELFS_SELF,
                                      fi.mpi_filename,
                                      fi.accessmode, fi.info, fh_ptr);
                    UNLOCK_MPI();

                    tunnelfs_srv_file_set_opened(fi.file_id, 1);

                    tunnelfs_srv_file_set_name(fi.file_id, fi.mpi_filename);

                    if (rcode == MPI_SUCCESS)
                    {
                        int ft_access;

                        ft_access =
                            tunnelfs_srv_file_get_filetype_access(fi.file_id);

                        int i = 0;
                        tunnelfs_srv_comm_get_ranks(fi.comm_id, &num_clients,
                                                    &clients);
                        tunnelfs_srv_fptr_register(fi.file_id);

                        LOG("Retrieving communicator %i with %i mpi processes", fi.comm_id, num_clients);

                        /* creating default file view for all clients */
                        for (i = 0; i < num_clients; i++)
                        {
                            tunnelfs_srv_fileview_set(clients[i], fi.file_id,
                                                      (MPI_Offset) 0,
                                                      MPI_BYTE, MPI_BYTE);
                            LOG("Creating default file view for %i on file %i", clients[i], fi.file_id);
                        }

                        tunnelfs_srv_info_eval(fi.info, fi.file_id);

                        tunnelfs_srv_file_distribute(fi.file_id, msg_id);

                        if (tunnelfs_srv_file_is_mutual(fi.file_id))
                            postpone_reply = 1;

                        if (num_servers == 1)
                            postpone_reply = 0;
                    }
                    else
                    {
                        LOG("ERROR: could not open file!");
                        /* Return error code to client. */
                        position = 0;
                        LOCK_MPI();
#if 1
                        MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                      &pack_size);

                        tunnelfs_adjust_buffer(&send_buf,
                                               &send_buf_size, pack_size);
#endif
                        MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        UNLOCK_MPI();

                        ptMPI_Send(send_buf, position, MPI_PACKED,
                                   msg_status.MPI_SOURCE,
                                   TUNNELFS_REPLY_BASE + msg_id,
                                   TUNNELFS_COMM_WORLD);
                    }

                    if (!postpone_reply && rcode == MPI_SUCCESS)
                    {
                        dist_list_t *distribution_list = NULL;
                        int list_size = 0;
                        tunnelfs_srv_file_get_distribution(fi.file_id,
                                                           &distribution_list,
                                                           &list_size);

                        /* send reply to client */
                        position = 0;
                        LOCK_MPI();
                        MPI_Pack_size(7 + (2 * num_clients), MPI_INT,
                                      TUNNELFS_COMM_WORLD, &pack_size);

                        tunnelfs_adjust_buffer(&send_buf,
                                               &send_buf_size, pack_size);

                        MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);

                        var_id = TUNNELFS_VAR_FILE_ID;
                        MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf,
                                 send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);

                        var_id = TUNNELFS_VAR_DISTLIST;
                        MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(&(fi.file_id), 1, MPI_INT, send_buf,
                                 send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);
                        MPI_Pack(&num_clients, 1, MPI_INT, send_buf,
                                 send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);
                        MPI_Pack(distribution_list, 2 * num_clients, MPI_INT,
                                 send_buf, send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);
                        UNLOCK_MPI();

                        ptMPI_Send(send_buf, position, MPI_PACKED,
                                   msg_status.MPI_SOURCE,
                                   TUNNELFS_REPLY_BASE + msg_id,
                                   TUNNELFS_COMM_WORLD);
                    }
                }
                else
                {
                    LOG("Forwarding request to io server %i", next_server);

                    /* send forward request to other server */
                    tunnelfs_srv_fileinfo_pack_size(&fi, &pack_size);

                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;

                    tunnelfs_adjust_buffer(&send_buf,
                                           &send_buf_size, pack_size);
#endif
                    position = 0;

                    /* message id */
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    /* client rank */
                    MPI_Pack(&(msg_status.MPI_SOURCE), 1,
                             MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    /* pack relevant file information */
                    tunnelfs_srv_fileinfo_pack(send_buf,
                                               send_buf_size, &position, &fi);

                    /* send message to forward-server */
                    ptMPI_Send(send_buf, position,
                               MPI_PACKED, next_server,
                               TUNNELFS_SERVER_DELEGATED_OPEN,
                               TUNNELFS_COMM_WORLD);
                }

                FREE(distribution_list);

                LOG("done");
                break;
            }
        case TUNNELFS_CLOSE:
            {
                int file_id = 0;
                int comm_id = 0;
                int pack_size = 0;
                int send_reply = 0;
                int main_server = 0;
                MPI_File *fh_ptr;

                LOG("Close Request");
                /* get message id  */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size,
                           &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("File id: %i", file_id);

                if ((main_server = tunnelfs_srv_file_get_main_server(file_id))
                    != my_rank)
                {
                    LOG("Delegating request to main server");

                    ptMPI_Send(recv_buf, msg_size, MPI_PACKED, main_server,
                               TUNNELFS_SERVER_DELEGATED_CLOSE,
                               TUNNELFS_COMM_WORLD);
                }
                else if (tunnelfs_srv_file_is_mutual(file_id))
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
                                              msg_id);

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
                            MPI_Pack(&msg_id, 1, MPI_INT,
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
                    ptMPI_Send(send_buf, position,
                               MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE + msg_id,
                               TUNNELFS_COMM_WORLD);
                    LOG("Reply sent.");
                }

                LOG("done");
                break;
            }
        case TUNNELFS_FCNTL:
            {
                LOG("Preallocating file");
                /* INFO:
                 * This message type is depricated as this is done with a setup
                 * message.
                 */
                LOG("done");
                break;
            }
        case TUNNELFS_SEEKIND:
            {
                LOG("Individual Seek");
                /* INFO:
                 * This message type is depricated as individual file pointers
                 * are handled on the client side only.
                 */
                LOG("done");
                break;
            }
        case TUNNELFS_DELETE:
            {
                int filename_len = 0;
                char *filename = NULL;
                char *fs_domain = NULL;
                char *filesystem = NULL;
                char *pure_filename = NULL;
                char *mpi_filename = NULL;
                int next_server = TUNNELFS_GLOBAL_MASTER;
                int rcode = -1;
                int pack_size = 0;

                LOG("Delete Request");
                /* get message id  */

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size,
                           &position, &filename_len,
                           1, MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                if (filename_len > 0)
                {
                    ALLOC(filename, filename_len + 1);
                    if (filename == NULL)
                        ERR(TUNNELFS_ERR_ALLOC);
                }

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size,
                           &position, filename,
                           filename_len, MPI_CHAR, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();
                /* make sure char array is closed properly */
                filename[filename_len] = '\0';

                LOG("Transfered filename = %s", filename);

                tunnelfs_srv_tokenize_filename
                    (filename, &filesystem, &fs_domain, &pure_filename);

                LOG("Filesystem: %s", filesystem);
                LOG("Filesystem domain: %s", fs_domain);
                LOG("Filename: %s", pure_filename);

                ALLOC(mpi_filename, strlen(filesystem) +
                      strlen(pure_filename) + 2);
                if (strncmp(filesystem, "default", 7) == 0)
                    strcpy(mpi_filename, pure_filename);
                else
                    sprintf(mpi_filename, "%s:%s", filesystem, pure_filename);

                LOG("MPI filename: %s", mpi_filename);

                if (strcmp(fs_domain, tunnelfs_srv_local_fs_domain()) == 0)
                    next_server = my_rank;
                else
                    next_server = tunnelfs_srv_schedule_current(fs_domain);

                LOG("Next server in schedule: %i", next_server);
                if (next_server == my_rank)
                {
                    int retries = 10;
                    do
                    {
                        LOCK_MPI();
                        rcode = MPI_File_delete(mpi_filename, MPI_INFO_NULL);
                        UNLOCK_MPI();

                        if (rcode == 0)
                            retries = 0;
                        else
                        {
                            LOG("Return code of delete for %s was: %i",
                                mpi_filename, rcode);
                            sleep(1);
                        }
                    }
                    while (--retries > 0);
                    /* send reply */
                    position = 0;
                    LOCK_MPI();
#if 1
                    MPI_Pack_size(2, MPI_INT,
                                  TUNNELFS_COMM_WORLD, &pack_size);

                    tunnelfs_adjust_buffer(&send_buf,
                                           &send_buf_size, pack_size);
#endif
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();
                    ptMPI_Send(send_buf, position,
                               MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE +
                               msg_id, TUNNELFS_COMM_WORLD);
                }
                else
                {
                    if (next_server != -1)
                    {
                        int part_size = 0;
                        LOG("Forwarding request to io server %i",
                            next_server);
                        pack_size = 0;
                        LOCK_MPI();
                        MPI_Pack_size(3, MPI_INT,
                                      TUNNELFS_COMM_WORLD, &part_size);
                        pack_size += part_size;
                        /* reconstruct filename */
                        if ((strcmp(filesystem, "default") !=
                             0) && (strlen(pure_filename) > 0))
                            sprintf(filename,
                                    "%s:%s", filesystem, pure_filename);
                        else
                            sprintf(filename, "%s", pure_filename);
                        filename_len = strlen(filename);

                        MPI_Pack_size(filename_len, MPI_CHAR,
                                      TUNNELFS_COMM_WORLD, &part_size);
                        pack_size += part_size;

                        tunnelfs_adjust_buffer(&send_buf,
                                               &send_buf_size, pack_size);

                        position = 0;
                        MPI_Pack(&msg_id, 1, MPI_INT,
                                 send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(&msg_status.MPI_SOURCE, 1,
                                 MPI_INT, send_buf,
                                 send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);
                        MPI_Pack(&filename_len, 1, MPI_INT,
                                 send_buf, send_buf_size,
                                 &position, TUNNELFS_COMM_WORLD);
                        MPI_Pack(filename, filename_len,
                                 MPI_CHAR, send_buf,
                                 send_buf_size, &position,
                                 TUNNELFS_COMM_WORLD);
                        UNLOCK_MPI();
                        ptMPI_Send(send_buf, position,
                                   MPI_PACKED, next_server,
                                   TUNNELFS_SERVER_DELEGATED_DELETE,
                                   TUNNELFS_COMM_WORLD);
                    }
                    else
                    {
                        LOG("WARNING: No server for filesystem domain");
                    }

                }

                FREE(filename);
                FREE(pure_filename);
                FREE(mpi_filename);
                FREE(filesystem);
                FREE(fs_domain);
                LOG("done");
                break;
            }
        case TUNNELFS_RESIZE:
            {
                LOG("Resize Request");
                /* INFO:
                 * This message type is depricated as this is done with a setup
                 * message.
                 */
                LOG("done");
                break;
            }
        case TUNNELFS_FLUSH:
            {
                int pack_size;
                int file_id;
                MPI_File *fh_ptr;
                int send_reply = 0;
                int main_server = -1;

                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size,
                           &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                LOG("Flush Request on file %i", file_id);

                if ((main_server == my_rank) &&
                    (tunnelfs_srv_file_is_mutual(file_id)))
                {
                    /* File is cached, so we have to clean up the cache files
                     * and wait for the service thread to correctly end the
                     * file synchronization and send the buffer to the client
                     */
                    dist_list_t *distribution = NULL;
                    int size;
                    int i;
                    int server_cookie = TUNNELFS_NEXT_COLL_ID;

                    main_server = tunnelfs_srv_file_get_main_server(file_id);

                    /* passive flush on cloned handled */
                    tunnelfs_srv_file_get_distribution(file_id,
                                                       &distribution, &size);

                    tunnelfs_srv_collop_start(file_id, server_cookie, size,
                                              msg_id);

                    for (i = 0; i < size; i++)
                    {
#if 1
                        LOCK_MPI();
                        MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD,
                                      &pack_size);
                        UNLOCK_MPI();

                        tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                               pack_size);
#endif
                        if (distribution[i].fileserver != my_rank)
                        {
                            LOG("Flushing cache file for client %i on %i",
                                distribution[i].client,
                                distribution[i].fileserver);

                            position = 0;
                            LOCK_MPI();
                            MPI_Pack(&msg_id, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&file_id, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&server_cookie, 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            MPI_Pack(&(distribution[i].client), 1, MPI_INT,
                                     send_buf, send_buf_size,
                                     &position, TUNNELFS_COMM_WORLD);
                            UNLOCK_MPI();

                            ptMPI_Send(send_buf, position,
                                       MPI_PACKED,
                                       distribution[i].fileserver,
                                       TUNNELFS_SERVER_FLUSH_PART,
                                       TUNNELFS_COMM_WORLD);
                        }
                        else
                        {
                            LOG("Removing local part of collective operation");
                            tunnelfs_srv_collop_done(file_id, server_cookie);
                        }

                        if (!tunnelfs_srv_collop_num_pending
                            (file_id, server_cookie))
                            send_reply = 1;

                        /* file is not cached anywhere, so we can directly sync the
                         * file and send the reply to the client
                         */
                        fh_ptr = tunnelfs_srv_file_get_handle(file_id);

                        LOCK_MPI();
                        MPI_File_sync(*fh_ptr);
                        UNLOCK_MPI();

                        FREE(distribution);
                    }
                }
                else
                    send_reply = 1;

                if (send_reply)
                {
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

                    ptMPI_Send(send_buf, position,
                               MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE + msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                LOG("done");
                break;
            }
        case TUNNELFS_READ:
            {
                MPI_File *fh_ptr;
                MPI_Status status;
                MPI_Offset buf_offset = 0;
                int block_count = 0;
                int count = 0;
                int datatype_id = -1;
                int etype_size = 0;
                int blocks = 0;
                int file_id = 0;
                int i = 0;
                int num_blocks = 0;
                MPI_Offset offset = 0;
                int pack_size = 0;
                int ptr_type = 0;
                int var_id = 0;
                int server;

                MPI_Offset fv_disp = 0;
                MPI_Datatype fv_etype = 0;
                MPI_Datatype fv_ftype = 0;

                LOG("Read request from client %i", msg_status.MPI_SOURCE);
                /* unpacking message */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &count, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &datatype_id,
                           1, MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &ptr_type, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &offset, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &num_blocks, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                /* get client fileview and set access parameters accordingly */
                tunnelfs_srv_fileview_get(msg_status.MPI_SOURCE, file_id,
                                          &fv_disp, &fv_etype, &fv_ftype);

                tunnelfs_srv_datatype_get_size(fv_etype, &etype_size);

                fh_ptr = tunnelfs_srv_file_get_handle(file_id);
                blocks = count;
#if 0
                LOG("client=%i file_id=%i count=%i datatype=%i ptr_type=%i offset=%Lx, iobuf=%Lx", msg_status.MPI_SOURCE, file_id, count, datatype, ptr_type, offset, (ADIO_Offset) iodata_buf);
#endif
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

                /* calculating number of needed blocks */
                num_blocks =
                    (int) ((blocks * etype_size) / TUNNELFS_MAX_MSG_SIZE);
                if ((blocks * etype_size) % TUNNELFS_MAX_MSG_SIZE)
                    num_blocks++;

                /* calculating block size for first num_msgs-1 blocks */
                if (num_blocks > 1)
                    block_count = TUNNELFS_MAX_MSG_SIZE / etype_size;
                else
                    block_count = blocks;

                tunnelfs_adjust_buffer(&iodata_buf, &iodata_buf_size,
                                       TUNNELFS_MAX_MSG_SIZE);

                /* send iodata */
                buf_offset = 0;

                LOG("Sending transfer parameters to client");

                /* send reply */
                position = 0;
#if 1
                MPI_Pack_size(4, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);

                tunnelfs_adjust_buffer(&send_buf, &send_buf_size, pack_size);
#endif
                MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&rcode, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);

                var_id = TUNNELFS_REQ_NUM_IO_BLOCKS;

                MPI_Pack(&var_id, 1, MPI_INT, send_buf, send_buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                MPI_Pack(&num_blocks, 1, MPI_INT, send_buf,
                         send_buf_size, &position, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();
                ptMPI_Send(send_buf, position, MPI_PACKED,
                           msg_status.MPI_SOURCE,
                           TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD);

                if ((tunnelfs_srv_file_get_behaviour(file_id) ==
                     TUNNELFS_DIRECT) ||
                    ((server = tunnelfs_srv_file_get_main_server(file_id)) ==
                     my_rank))
                {
                    if (ptr_type == ADIO_SHARED)
                        offset =
                            tunnelfs_srv_fptr_getset_shared_atomic(file_id,
                                                                   count);

                    for (i = 0; i < num_blocks; i++)
                    {
                        if (i == num_blocks - 1)
                            block_count = blocks - buf_offset;

                        LOG("Using view (%Li, %i, %i)", fv_disp, fv_etype,
                            fv_ftype);
                        LOG("Reading at offset 0x%Lx", offset + buf_offset);

                        assert(block_count * etype_size <=
                               TUNNELFS_MAX_MSG_SIZE);

                        LOCK_MPI();
                        rcode =
                            MPI_File_read_at(*fh_ptr, offset + buf_offset,
                                             iodata_buf, block_count,
                                             fv_etype, &status);
                        UNLOCK_MPI();

                        LOG("Sending %i blocks of type %i", block_count,
                            fv_etype);

                        ptMPI_Send(iodata_buf, block_count, fv_etype,
                                   msg_status.MPI_SOURCE, TUNNELFS_IODATA,
                                   TUNNELFS_COMM_WORLD);

                        buf_offset += block_count;
                    }
                }
                else if (((server =
                           tunnelfs_srv_file_get_main_server(file_id)) !=
                          my_rank)
                         && (tunnelfs_srv_file_get_behaviour(file_id) ==
                             TUNNELFS_ROUTE))
                {
                    /* send request to main server */
                    int temp_size = 0;
                    int pack_size = 0;

                    LOG("Setting pending io for routed read of client %i on file %i", msg_status.MPI_SOURCE, file_id);

                    tunnelfs_srv_pending_io_set(msg_status.MPI_SOURCE,
                                                file_id, num_blocks, count,
                                                ptr_type, offset, msg_id);

                    LOCK_MPI();
#if 1
                    MPI_Pack_size(5, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);
                    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    position = 0;

                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&count, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&ptr_type, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&offset, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&num_blocks, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    ptMPI_Send(send_buf, position, fv_etype, server,
                               TUNNELFS_SERVER_READ, TUNNELFS_COMM_WORLD);
                }

                LOG("done");
                break;
            }
        case TUNNELFS_WRITE:
            {
                MPI_Datatype datatype;
                int count = 0;
                int datatype_id = 0;
                int datatype_size = 0;
                int file_id = 0;
                MPI_Offset offset = 0;
                int ptr_type = 0;
                int num_blocks = 0;
                int server;

                LOG("Write Request from client %i", msg_status.MPI_SOURCE);
                /* unpacking message */
                LOCK_MPI();
                MPI_Unpack(recv_buf, msg_size, &position, &file_id, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &count, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &datatype_id,
                           1, MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &ptr_type, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                MPI_Unpack(recv_buf, msg_size, &position, &offset, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                LOG("Offset 0x%Lx", offset);
                MPI_Unpack(recv_buf, msg_size, &position, &num_blocks, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                UNLOCK_MPI();

                tunnelfs_srv_datatype_get_type
                    (msg_status.MPI_SOURCE, datatype_id, &datatype);

                tunnelfs_srv_datatype_get_size(datatype, &datatype_size);

                if (ptr_type == ADIO_SHARED)
                {
                    int etype_size;
                    MPI_Datatype etype;

                    tunnelfs_srv_fileview_get_etype(msg_status.MPI_SOURCE,
                                                    file_id, &etype);

                    tunnelfs_srv_datatype_get_size(etype, &etype_size);

                    offset =
                        tunnelfs_srv_fptr_getset_shared_atomic(file_id,
                                                               count);
                    LOG("Shared Pointer at 0x%Lx", offset);

                    /* shared pointer value has been retrieved, access is now
                     * at explicit offset */
                    ptr_type = ADIO_EXPLICIT_OFFSET;
                }

                /* check if request needs to be delegated */
                if (((server = tunnelfs_srv_file_get_main_server(file_id))
                     != my_rank) &&
                    (tunnelfs_srv_file_get_behaviour(file_id) ==
                     TUNNELFS_ROUTE))
                {
                    void *send_buf = NULL;
                    int send_buf_size = 0;
                    int pack_size = 0;
                    int temp_size = 0;

                    /* get routing target */
                    server = tunnelfs_srv_file_get_main_server(file_id);

                    /* allocate request buffer */
                    LOCK_MPI();
#if 1
                    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD,
                                  &pack_size);
                    MPI_Pack_size(5, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);
#endif
                    /* pack request buffer */
                    position = 0;

                    MPI_Pack(&file_id, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT,
                             send_buf, send_buf_size, &position,
                             TUNNELFS_COMM_WORLD);
                    MPI_Pack(&count, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&ptr_type, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&offset, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&num_blocks, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    /* sending request */
                    ptMPI_Send(send_buf, position, MPI_PACKED, server,
                               TUNNELFS_SERVER_WRITE, TUNNELFS_COMM_WORLD);
                }

                tunnelfs_srv_pending_io_set(msg_status.MPI_SOURCE,
                                            file_id, num_blocks, count,
                                            ptr_type, offset, msg_id);
#if 0
                LOG("client=%i file_id=%i count=%i datatype=%i ptr_type=%i offset=%Lx, iobuf=%x", msg_status.MPI_SOURCE, file_id, count, datatype, ptr_type, offset, (int) iobuf);
#endif

                LOG("done");
                break;
            }
        case TUNNELFS_IODATA:
            {
                MPI_Datatype fv_etype;
                MPI_Datatype fv_ftype;
                MPI_Offset fv_disp = 0;
                MPI_Offset offset = 0;
                MPI_File *fh_ptr = NULL;

                int blocks;
                int count = 0;
                int etype_size;
                int file_id = 0;
                int iobuf_offset = 0;
                int pack_size = 0;
                int position = 0;
                int ptr_type = 0;
                int rcode = 0;
                void *iobuf = NULL;
                int server;
                int early_answer = 1;
                int pending_blocks = 0;

                LOG("IO data received from client rank %i",
                    tunnelfs_main_status.MPI_SOURCE);
                /* get parameters of pending io for the client */
                tunnelfs_srv_pending_io_get(tunnelfs_main_status.MPI_SOURCE,
                                            &file_id, &count, &ptr_type,
                                            &offset, &iobuf_offset, &msg_id);

                LOG("Got pending io parameters (offset = 0x%Lx)", offset);

                tunnelfs_srv_fileview_get(tunnelfs_main_status.MPI_SOURCE,
                                          file_id, &fv_disp, &fv_etype,
                                          &fv_ftype);

                LOG("Got fileview data");

                /* get a valid receive buffer */
                iobuf =
                    tunnelfs_srv_buffer_get_block(tunnelfs_main_status.
                                                  MPI_SOURCE);

                LOG("Memory block address retrieved");

                LOCK_MPI();
                MPI_Get_count(&tunnelfs_main_status, fv_etype, &blocks);
                tunnelfs_srv_datatype_get_size(fv_etype, &etype_size);

                /* obsolete as buffer of size TUNNELFS_MAX_MSG_SIZE is used  
                   tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size,
                   blocks * etype_size);
                 */

                /* Receiving message with correct datatypes */
                MPI_Recv(iobuf, blocks, fv_etype,
                         tunnelfs_main_status.MPI_SOURCE,
                         tunnelfs_main_status.MPI_TAG, TUNNELFS_COMM_WORLD,
                         &msg_status);
                UNLOCK_MPI();

                /* Message is retrieved. Let's signal this to probe thread. */
                pthread_mutex_lock(&pario_probe_sync);
                message_is_processing = 0;
                pthread_cond_signal(&message_retrieved_from_queue);
                pthread_mutex_unlock(&pario_probe_sync);

                /* decrease pending packets */
                tunnelfs_srv_pending_io_dec(msg_status.MPI_SOURCE);
                pending_blocks =
                    tunnelfs_srv_pending_io_num_blocks(msg_status.MPI_SOURCE);

                if (early_answer && !pending_blocks)
                {
                    /* send reply */
                    LOCK_MPI();

                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf,
                                           &send_buf_size, pack_size);

                    position = 0;
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending io completion message to client %i",
                        msg_status.MPI_SOURCE);

                    ptMPI_Send(send_buf, position, MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE + msg_id,
                               TUNNELFS_COMM_WORLD);
                }

#if 0
                /* search for filehandle in files and parts */
                if ((fh_ptr = tunnelfs_srv_file_get_handle(file_id)) != NULL)
                {
                    if ((tunnelfs_srv_file_get_behaviour(file_id) ==
                         TUNNELFS_DIRECT) ||
                        (tunnelfs_srv_file_get_main_server(file_id) ==
                         my_rank))
                    {
                        LOG("Writing data from rank %i to file %i",
                            msg_status.MPI_SOURCE, file_id);

                        LOCK_MPI();
                        MPI_File_set_view(*fh_ptr, fv_disp, fv_etype,
                                          fv_ftype, "native", MPI_INFO_NULL);

                        LOG("Using view (%Li, %i, %i)", fv_disp, fv_etype,
                            fv_ftype);
                        LOG("Writing at offset 0x%Lx", offset);

                        rcode =
                            MPI_File_write_at(*fh_ptr, offset, iobuf,
                                              blocks, fv_etype, &status);
                        UNLOCK_MPI();

                        tunnelfs_srv_pending_io_inc_offset(msg_status.
                                                           MPI_SOURCE,
                                                           blocks);
                    }
                    else if (((server =
                               tunnelfs_srv_file_get_main_server(file_id)) !=
                              my_rank)
                             && (tunnelfs_srv_file_get_behaviour(file_id) ==
                                 TUNNELFS_ROUTE))
                    {
                        LOG("Delegating io data to server %i", server);

                        ptMPI_Send(iobuf, blocks, fv_etype, server,
                                   TUNNELFS_SERVER_IODATA_BASE +
                                   msg_status.MPI_SOURCE,
                                   TUNNELFS_COMM_WORLD);
                    }
                }
                else if (tunnelfs_srv_parts_search
                         (file_id, msg_status.MPI_SOURCE) != -1)
                {
                    int temp_size = 0;
                    int server;

                    fh_ptr =
                        tunnelfs_srv_parts_get_handle(file_id,
                                                      msg_status.MPI_SOURCE);
                    /* server write request to main server */
                    server = tunnelfs_srv_parts_get_server(file_id,
                                                           msg_status.
                                                           MPI_SOURCE);

                    LOG("Delegating write request to server %i", server);
                    pack_size = 0;
                    LOCK_MPI();

                    MPI_Pack_size(6, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;
                    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD,
                                  &temp_size);
                    pack_size += temp_size;

                    tunnelfs_adjust_buffer(&send_buf, &send_buf_size,
                                           pack_size);

                    position = 0;

                    MPI_Pack(&file_id, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&(msg_status.MPI_SOURCE), 1, MPI_INT,
                             send_buf, send_buf_size, &position,
                             TUNNELFS_COMM_WORLD);
                    MPI_Pack(&count, 1, MPI_INT, send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&ptr_type, 1, MPI_INT, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&offset, 1, TUNNELFS_OFFSET, send_buf,
                             send_buf_size, &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending Header");
                    ptMPI_Send(send_buf, position, MPI_PACKED, server,
                               TUNNELFS_SERVER_WRITE, TUNNELFS_COMM_WORLD);
                    LOG("Sending Buffer");
                    ptMPI_Send(iobuf, blocks, fv_etype, server,
                               TUNNELFS_SERVER_IODATA, TUNNELFS_COMM_WORLD);
                }
                else
                    ERR(TUNNELFS_ERR_NOT_FOUND);
#else
                /* search for filehandle in files and parts */
                if ((fh_ptr = tunnelfs_srv_file_get_handle(file_id)) != NULL)
                {
                    if ((tunnelfs_srv_file_get_behaviour(file_id) ==
                         TUNNELFS_DIRECT) ||
                        (tunnelfs_srv_file_get_main_server(file_id) ==
                         my_rank))
                    {
                        LOG("Writing data from rank %i to file %i",
                            msg_status.MPI_SOURCE, file_id);

                        tunnelfs_srv_buffer_write(iobuf, file_id,
                                                  msg_status.MPI_SOURCE,
                                                  offset, blocks, fv_etype);
                        tunnelfs_srv_pending_io_inc_offset(msg_status.
                                                           MPI_SOURCE,
                                                           blocks);
                    }
                    else if (((server =
                               tunnelfs_srv_file_get_main_server(file_id)) !=
                              my_rank)
                             && (tunnelfs_srv_file_get_behaviour(file_id) ==
                                 TUNNELFS_ROUTE))
                    {
                        LOG("Delegating io data to server %i", server);

                        tunnelfs_srv_buffer_send(iobuf, server,
                                                 TUNNELFS_SERVER_IODATA_BASE +
                                                 msg_status.MPI_SOURCE,
                                                 TUNNELFS_COMM_WORLD, blocks,
                                                 fv_etype);
                    }
                }
#endif

                if (!pending_blocks)
                    tunnelfs_srv_buffer_flush_queue(msg_status.MPI_SOURCE);

                if (!early_answer && !pending_blocks)
                {
                    /* flush buffer for io transfer */

                    /* send reply */
                    LOCK_MPI();

                    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD,
                                  &pack_size);

                    tunnelfs_adjust_buffer(&send_buf,
                                           &send_buf_size, pack_size);

                    position = 0;
                    MPI_Pack(&msg_id, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&rcode, 1, MPI_INT,
                             send_buf, send_buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    UNLOCK_MPI();

                    LOG("Sending io completion message to client %i",
                        msg_status.MPI_SOURCE);

                    ptMPI_Send(send_buf, position, MPI_PACKED,
                               msg_status.MPI_SOURCE,
                               TUNNELFS_REPLY_BASE + msg_id,
                               TUNNELFS_COMM_WORLD);
                }

                if (!pending_blocks)
                    tunnelfs_srv_pending_io_is_done(msg_status.MPI_SOURCE);

                LOG("done");
                break;
            }
        default:
            {
                LOG("Received message of unknown type %i from %i\n",
                    msg_status.MPI_TAG, msg_status.MPI_SOURCE);
                break;
            }                   /* case */
        }                       /* switch */
    }                           /* while */

    FREE(iodata_buf);
    FREE(send_buf);
    FREE(recv_buf);

    LOG("At end of thread main");

    pthread_mutex_unlock(&tunnelfs_main_sync);

    pthread_exit(NULL);
    return NULL;
}
