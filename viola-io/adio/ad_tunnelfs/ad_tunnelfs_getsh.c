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
* File:         ad_tunnelfs_getsh.c                                       * 
* Description:  Retrieve the shared filepointer value                     * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include <stdlib.h>
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Performing seek with shared file pointer
 * @param fd MPI File descriptor
 * @param size Dummy value not used with tunnelfs
 * @param shared_fp Pointer to variable to hold the filepointer position
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Get_shared_fp(ADIO_File fd, int size,
                                  ADIO_Offset *shared_fp, int *error_code)
{
    /*
     * Shared file pointers are handled on server side. However the client has
     * the possibility to query the current value.
     */
    int inbuf_size = 0;
    int io_server_rank = -1;
    int msg_id = 0;
    int msg_size = 0;
    int outbuf_size = 0;
    int position = 0;
    int recvd = 0;
    int reply_id = 0;
    int var_id = 0;
    int var_value = 0;
    void *inbuf = NULL;
    void *outbuf = NULL;


    MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &msg_size);
    ALLOC(outbuf, msg_size);
    outbuf_size = msg_size;

    var_id = TUNNELFS_REQ_SHARED_PTR;
    var_value = fd->fd_sys;

    MPI_Pack(&msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&var_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&var_value, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    io_server_rank = tunnelfs_server_get_file_master(var_value);

    MPI_Send(outbuf, position, MPI_PACKED, io_server_rank,
             TUNNELFS_SETUP, TUNNELFS_COMM_WORLD);

    /* waiting for reply */
    tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                           io_server_rank, reply_id);

    position = 0;
    MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);
    MPI_Unpack(inbuf, inbuf_size, &position, error_code, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    tunnelfs_globals_set_active_fd(&fd);
    tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

    /* retrieving shared file pointer value */
    *shared_fp = tunnelfs_globals_get_shared_fp();

    FREE(outbuf);
    FREE(inbuf);
}
