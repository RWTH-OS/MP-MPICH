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
* File:                                                                   * 
* Description:                                                            * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Send flush request to tunnelfs server
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Flush(ADIO_File fd, int *error_code)
{
    void *inbuf = NULL;
    int inbuf_size = 0;
    void *outbuf = NULL;
    int outbuf_size = 0;

    int pack_size = 0;
    int tmp_size = 0;
    int position = 0;
    int recvd = 0;

    int msg_id = TUNNELFS_NEXT_MSG_ID;
    int file_id = 0;

    int my_rank = 0;
    int io_server_rank = -1;

    MPI_Comm_rank(fd->comm, &my_rank);

    if (my_rank == 0)
    {

        MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
        pack_size += tmp_size;

        tunnelfs_adjust_buffer(&outbuf, &outbuf_size, pack_size);

        MPI_Pack(&msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* determine io server to communicate with */
        file_id = fd->fd_sys;
        io_server_rank = tunnelfs_server_get_file_master(file_id);

        MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Send(outbuf, position, MPI_PACKED, io_server_rank, TUNNELFS_FLUSH,
                 TUNNELFS_COMM_WORLD);

        position = 0;
        tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                               io_server_rank, msg_id);

        MPI_Unpack(inbuf, inbuf_size, &position, &msg_id, 1,
                   MPI_INT, TUNNELFS_COMM_WORLD);
        MPI_Unpack(inbuf, inbuf_size, &position, error_code, 1,
                   MPI_INT, TUNNELFS_COMM_WORLD);

        tunnelfs_globals_set_active_fd(&fd);
        tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);
    }

    MPI_Barrier(fd->comm);
    MPI_Bcast(error_code, 1, MPI_INT, 0, fd->comm);
}
