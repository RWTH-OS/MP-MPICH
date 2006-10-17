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
* File:         ad_tunnelfs_close.c                                       * 
* Description:                                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Closing a tunnelfs controled file
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Close(ADIO_File fd, int *error_code)
{
    int file_id = 0;
    int send_id = 0;
    int reply_id = 0;

    void *inbuf = NULL;
    int inbuf_size = 0;
    void *outbuf = NULL;
    int outbuf_size = 0;
    int position = 0;
    int recvd = 0;

    int ret_value = -1;
    int comm_rank = -1;
    int comm_id = -1;

    int io_server_rank = -1;

    /* TODO:
     * to  ensure a tighter synchronization, we place a barrier here. While
     * optimizing the communication scheme, we might have to rethink this */
    MPI_Barrier(fd->comm);

    MPI_Comm_rank(fd->comm, &comm_rank);

/*    MPI_File_sync(fd);*/

    if (comm_rank == 0)
    {
        /* 
         * allocate outbuf
         * - hints are not taken into account!
         */
        outbuf_size = 2 * sizeof(int);
        ALLOC(outbuf, outbuf_size);

        /* create message id */
        send_id = TUNNELFS_NEXT_MSG_ID;
        MPI_Pack(&send_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* get file id */
        file_id = tunnelfs_file_get_id(fd);
        MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* determine io server */
        io_server_rank = tunnelfs_server_get_file_master(file_id);
        MPI_Send(outbuf, position, MPI_PACKED, io_server_rank,
                 TUNNELFS_CLOSE, TUNNELFS_COMM_WORLD);

        /* TODO: 
         * Check for messages from any source to take request forwarding into
         * account.
         */
        tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                               MPI_ANY_SOURCE, send_id);

        position = 0;
        MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);
        MPI_Unpack(inbuf, inbuf_size, &position, &ret_value, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);

        tunnelfs_globals_set_active_fd(&fd);
        tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);
    }

    MPI_Bcast(&ret_value, 1, MPI_INT, 0, fd->comm);

    comm_id = tunnelfs_comm_get_id(fd->comm);
    tunnelfs_comm_unregister(comm_id);

    tunnelfs_server_unset_file_master(fd->fd_sys);

    fd->fd_sys = -1;            /* TUNNELFS File Descriptor */
    fd->fp_ind = 0;
    fd->fp_sys_posn = 0;
    *error_code = ret_value;

    FREE(inbuf);
    FREE(outbuf);
}
