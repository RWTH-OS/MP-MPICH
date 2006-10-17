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
* File:         ad_tunnelfs_open.c                                        * 
* Description:                                                            *
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "ad_tunnelfs_globals.h"
#include "adioi.h"

/**
 * Opening a tunnelfs controled file
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Open(ADIO_File fd, int *error_code)
{
    /* 
     * To optain collective character of this call, together with minimum
     * ammount of cross-cluster messages, only one participating process
     * issues the open call, and broadcasts the result to the remaining
     * processes.
     */

    int filename_len = 0;
    int msg_id = 0;
    int comm_id = 0;
    int etype = 0;
    int ftype = 0;
    int hints = 0;

    void *inbuf = NULL;
    int inbuf_size = 0;
    void *outbuf = NULL;
    int outbuf_size = 0;
    int position = 0;

    int recv_id = 0;
    int rcode = 0;
    int file_id = -1;
    int tmp_size = 0;
    int pack_size = 0;
    int recvd = 0;

    int comm_rank = -1;
    int world_rank = -1;
    int ret_values[5];
    dist_list_t *list = NULL;
    int list_size = 0;

    int io_server_rank = -1;
    int i;

    /* TODO:
     * to  ensure a tighter synchronization, we place a barrier here. While
     * optimizing the communication scheme, we might have to rethink this */
    MPI_Barrier(fd->comm);

    /* what is the process' rank within the communicator? */
    MPI_Comm_rank(fd->comm, &comm_rank);
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &world_rank);
#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%2i] Entering Open on Client\n", world_rank);
#endif
    if (comm_rank == 0)
    {
        int num_ints = 11;
        int num_offsets = 1;
        int num_char = 0;
        /* 
         * allocate outbuf
         */
        num_char += strlen(fd->filename);
        if ((fd->info != MPI_INFO_NULL))
        {
            char info_key[MPI_MAX_INFO_KEY + 1];

            MPI_Info_get_nkeys(fd->info, &hints);
            for (i = 0; i < hints; i++)
            {
                int val_len = 0;
                int flag = 0;

                MPI_Info_get_nthkey(fd->info, i, info_key);
                MPI_Info_get_valuelen(fd->info, info_key, &val_len, &flag);
                num_ints += 2;
                num_char += strlen(info_key) + val_len + 2;
            }
        }

        MPI_Pack_size(num_ints, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
        pack_size += tmp_size;
        MPI_Pack_size(num_offsets, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD,
                      &tmp_size);
        pack_size += tmp_size;
        MPI_Pack_size(num_char, MPI_CHAR, TUNNELFS_COMM_WORLD, &tmp_size);
        pack_size += tmp_size;

        tunnelfs_adjust_buffer(&outbuf, &outbuf_size, pack_size);

        position = 0;
        /* create message id */
        msg_id = TUNNELFS_NEXT_MSG_ID;
        MPI_Pack(&msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* get internal comm identifier */
        comm_id = tunnelfs_comm_get_id(fd->comm);

        io_server_rank = tunnelfs_server_get_global_master();

        /* check if communicator needs to be sent */
        if (comm_id == -1)
        {
            void *setup_msg = NULL;
            int setup_msg_size = 0;
            int recvd = 0;
            int position = 0;
            int reply_id = 0;
            int rcode = 0;
            int send_id = 0;

            tunnelfs_comm_serialize_by_comm(fd->comm, &setup_msg,
                                            &setup_msg_size, &position,
                                            &send_id);

            MPI_Send(setup_msg, position, MPI_PACKED, io_server_rank,
                     TUNNELFS_SETUP, TUNNELFS_COMM_WORLD);

            /* waiting for reply */
            position = 0;
            tunnelfs_msg_get_reply(&setup_msg, &setup_msg_size, &recvd,
                                   io_server_rank, send_id);

            MPI_Unpack(setup_msg, setup_msg_size, &position, &reply_id, 1,
                       MPI_INT, TUNNELFS_COMM_WORLD);

            TUNNELFS_CHECK_MSG_ID(send_id, reply_id);

            MPI_Unpack(setup_msg, setup_msg_size, &position, &rcode, 1,
                       MPI_INT, TUNNELFS_COMM_WORLD);

            tunnelfs_globals_set_active_fd(&fd);
            tunnelfs_msg_get_variables(setup_msg, setup_msg_size, &position,
                                       recvd);

            /* retrieve comm_id from registered communicator */
            comm_id = tunnelfs_comm_get_id(fd->comm);

            FREE(setup_msg);
        }

        file_id = -1;
        MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&comm_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* filename */
        filename_len = strlen(fd->filename);
        MPI_Pack(&filename_len, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(fd->filename, filename_len, MPI_CHAR, outbuf, outbuf_size,
                 &position, TUNNELFS_COMM_WORLD);

        /* access mode */
        MPI_Pack(&(fd->access_mode), 1, MPI_INT, outbuf, outbuf_size,
                 &position, TUNNELFS_COMM_WORLD);

        /* displacement */
        MPI_Pack(&(fd->disp), 1, TUNNELFS_OFFSET, outbuf, outbuf_size,
                 &position, TUNNELFS_COMM_WORLD);

        /* elementary type */
        etype = 0;
        tunnelfs_datatype_get_id(fd->etype, &etype);
        if (etype == TUNNELFS_FAILURE)
        {
            /* TODO:
             * send datatype message !
             */
            fprintf(stderr, "Oops, we need a datatype!\n");
        }
        MPI_Pack(&etype, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* file type */
        ftype = 0;
        tunnelfs_datatype_get_id(fd->filetype, &ftype);
        if (ftype == TUNNELFS_FAILURE)
        {
            /* TODO:
             * send datatype message !
             */
            fprintf(stderr, "Oops, we need a datatype!\n");
        }
        MPI_Pack(&ftype, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* iomode */
        MPI_Pack(&(fd->iomode), 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* permissions */
        MPI_Pack(&(fd->perm), 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        if ((fd->info == MPI_INFO_NULL))
        {
            MPI_Pack(&hints, 1, MPI_INT, outbuf, outbuf_size, &position,
                     TUNNELFS_COMM_WORLD);
        }
        else
        {
            char info_key[MPI_MAX_INFO_KEY + 1];

            MPI_Info_get_nkeys(fd->info, &hints);
            MPI_Pack(&hints, 1, MPI_INT, outbuf, outbuf_size, &position,
                     TUNNELFS_COMM_WORLD);
            for (i = 0; i < hints; i++)
            {
                int key_len = 0;
                int val_len = 0;
                int flag = 0;
                char *info_val = NULL;

                MPI_Info_get_nthkey(fd->info, i, info_key);
                key_len = strlen(info_key);
                MPI_Pack(&key_len, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);
                MPI_Pack(&info_key, key_len, MPI_CHAR, outbuf, outbuf_size,
                         &position, TUNNELFS_COMM_WORLD);

                MPI_Info_get_valuelen(fd->info, info_key, &val_len, &flag);
                if (!flag)
                    ERR(TUNNELFS_ERR_NOT_FOUND);
                ALLOC(info_val, val_len + 1);
                MPI_Info_get(fd->info, info_key, val_len, info_val, &flag);
                if (!flag)
                    ERR(TUNNELFS_ERR_NOT_FOUND);
                MPI_Pack(&val_len, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);
                if (val_len > 0)
                    MPI_Pack(info_val, val_len, MPI_CHAR, outbuf, outbuf_size,
                             &position, TUNNELFS_COMM_WORLD);
            }
        }

        MPI_Send(outbuf, position, MPI_PACKED, io_server_rank, TUNNELFS_OPEN,
                 TUNNELFS_COMM_WORLD);

        /* waiting for reply */
        /* INFO: The MPI_ANY_SOURCE is here to allow transparent request
         * forwarding */
        tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd, MPI_ANY_SOURCE,
                               msg_id);

        position = 0;
        rcode = -1;
        MPI_Unpack(inbuf, inbuf_size, &position, &recv_id, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);
        MPI_Unpack(inbuf, inbuf_size, &position, &rcode, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);

        tunnelfs_globals_set_active_fd(&fd);
        tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

        /* retrieve file_id from updated file descriptor */
        file_id = fd->fd_sys;
        io_server_rank = tunnelfs_server_get_file_master(file_id);

        tunnelfs_globals_get_distlist(&list, &list_size);

        comm_id = tunnelfs_comm_get_id(fd->comm);

        /* initialize ret_values for broadcast */
        ret_values[0] = rcode;
        ret_values[1] = file_id;
        ret_values[2] = comm_id;
        ret_values[3] = io_server_rank;
        ret_values[4] = list_size;
    }

    /*
     * after rank 0 has dealt with open call to io server the other
     * processes need to get the new information
     */
    if (fd->comm != MPI_COMM_TUNNELFS_SELF)
    {
        /* settings need to be broadcasted to other clients */
        MPI_Bcast(ret_values, 5, MPI_INT, 0, fd->comm);

        /* allocate memory on other clients */
        if ((list == NULL) || (list_size == 0))
        {
            list_size = ret_values[4];
            ALLOC(list, list_size * sizeof(dist_list_t));
        }

        /* distribute filelist */
        MPI_Bcast(list, list_size * 2, MPI_INT, 0, fd->comm);

        /* all other clients have to set their values accordingly */
        /* save file_id */
        fd->fd_sys = ret_values[1];

        /* check if special fileserver was given */
        for (i = 0; i < list_size; i++)
            if (world_rank == list[i].client)
                tunnelfs_server_set_file_master(fd->fd_sys,
                                                list[i].fileserver);

        /* register comm if unknown */
        comm_id = tunnelfs_comm_get_id(fd->comm);
        if (comm_id == -1)
            tunnelfs_comm_register(ret_values[2], fd->comm);
    }

#ifdef DEBUG_TUNNELFS
    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);
    fprintf(stderr, "[%2i] Client: filemaster for %i is %i\n", world_rank,
            fd->fd_sys, io_server_rank);
#endif

    /* open resets file pointers in descriptor */
    fd->fp_ind = 0;
    fd->fp_sys_posn = 0;

    *error_code = ret_values[0];
}
