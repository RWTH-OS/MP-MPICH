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
 * Deleting a tunnelfs controlled file
 * @param filename string identifier for filename
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Delete(char *filename, int *error_code)
{
    void *buf = NULL;
    int buf_size = 0;
    int filename_len = 0;
    int pack_size = 0;
    int part_size = 0;
    int position = 0;
    int msg_id = TUNNELFS_NEXT_MSG_ID;
    int recv_id = 0;
    int recvd = 0;
    int rcode = 0;
    int io_server_rank = -1;

    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &part_size);
    pack_size += part_size;

    filename_len = strlen(filename);
    MPI_Pack_size(filename_len, MPI_CHAR, TUNNELFS_COMM_WORLD, &part_size);
    pack_size += part_size;

    tunnelfs_adjust_buffer(&buf, &buf_size, pack_size);

    msg_id = TUNNELFS_NEXT_MSG_ID;
    MPI_Pack(&msg_id, 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&filename_len, 1, MPI_INT, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(filename, filename_len, MPI_CHAR, buf, buf_size, &position,
             TUNNELFS_COMM_WORLD);

    io_server_rank = tunnelfs_server_get_global_master();
    MPI_Send(buf, position, MPI_PACKED, io_server_rank,
             TUNNELFS_DELETE, TUNNELFS_COMM_WORLD);

    /* INFO: reply can come from any other server of the filesystem domain */
    tunnelfs_msg_get_reply(&buf, &buf_size, &recvd, MPI_ANY_SOURCE, msg_id);

    position = 0;
    MPI_Unpack(buf, buf_size, &position, &recv_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);
    MPI_Unpack(buf, buf_size, &position, &rcode, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    if (position < recvd)
        tunnelfs_msg_get_variables(buf, buf_size, &position, recvd);

    *error_code = rcode;
}
