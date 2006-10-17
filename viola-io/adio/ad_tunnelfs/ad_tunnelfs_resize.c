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
 * Send Resize request to tunnelfs server
 * @param fd MPI File descriptor
 * @param size new file size
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
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
    int var_id = 0;
    int file_id = 0;
    int rcode = -1;
    int myrank_filecomm;
    int io_server_rank = -1;

    /* This call is collective! */
    MPI_Comm_rank(fd->comm, &myrank_filecomm);

    MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
    pack_size += tmp_size;
    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &tmp_size);
    pack_size += tmp_size;

    tunnelfs_adjust_buffer(&outbuf, &outbuf_size, pack_size);

    MPI_Pack(&msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    if (myrank_filecomm == 0)
    {
        var_id = TUNNELFS_REQ_RESIZE;
        MPI_Pack(&var_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        file_id = fd->fd_sys;
        MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&size, 1, TUNNELFS_OFFSET, outbuf, outbuf_size, &position,
                 TUNNELFS_COMM_WORLD);

        io_server_rank = tunnelfs_server_get_file_master(file_id);
        MPI_Send(outbuf, position, MPI_PACKED, io_server_rank,
                 TUNNELFS_SETUP, TUNNELFS_COMM_WORLD);

        position = 0;
        tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                               io_server_rank, msg_id);

        MPI_Unpack(inbuf, inbuf_size, &position, &msg_id, 1,
                   MPI_INT, TUNNELFS_COMM_WORLD);
        MPI_Unpack(inbuf, inbuf_size, &position, &rcode, 1,
                   MPI_INT, TUNNELFS_COMM_WORLD);

        tunnelfs_globals_set_active_fd(&fd);
        tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);
    }
    /* distribute rcode */
    MPI_Bcast(&rcode, 1, MPI_INT, 0, fd->comm);

    *error_code = rcode;
}
