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
* File:         ad_tunnelfs_io.c                                          * 
* Description:  I/O request handling                                      * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"

/**
 * Return file id for a given MPI file handle
 * @param fd MPI file handle
 * @return internal file id
 */
int tunnelfs_file_get_id(ADIO_File fd)
{
    return fd->fd_sys;
}

/**
 * Create an io request
 * @param tunnelfs_req_type Type of request (read/write)
 * @param fd MPI file handle
 * @param buf MPI message buffer
 * @param count number of elements
 * @param datatype MPI Datatype describing the elements
 * @param file_ptr_type MPI File pointer type
 * @param offset Offset for io operation
 * @param status Reference to MPI Status structure
 * @param error_code Reference to MPI error_code
 * @param msg_id Message id to use for request
 * @return Number of io blocks for request
 */
int tunnelfs_ioreq(int tunnelfs_req_type, ADIO_File fd, void *buf, int count,
                   MPI_Datatype datatype, int file_ptr_type,
                   ADIO_Offset offset, ADIO_Status *status, int *error_code,
                   int *msg_id)
{
    int file_id = 0;
    int dtype = 0;

    void *outbuf = NULL;
    int outbuf_size = 0;
    int position = 0;
    int num_blocks = 0;
    int datatype_size = 0;

    int pack_size = 0;
    int temp_size = 0;

    int io_server_rank = 0;

    MPI_Pack_size(6, MPI_INT, TUNNELFS_COMM_WORLD, &temp_size);
    pack_size += temp_size;
    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &temp_size);
    pack_size += temp_size;

    tunnelfs_adjust_buffer(&outbuf, &outbuf_size, pack_size);

    /* create message id */
    *msg_id = TUNNELFS_NEXT_MSG_ID;
    MPI_Pack(msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* get file id */
    file_id = tunnelfs_file_get_id(fd);
    MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* pack number of elements */
    MPI_Pack(&count, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* get datatype */
    tunnelfs_datatype_get_id(datatype, &dtype);
    MPI_Pack(&dtype, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* pack pointer type */
    MPI_Pack(&file_ptr_type, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* pack file offset */
    MPI_Pack(&offset, 1, TUNNELFS_OFFSET, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* pack number of io blocks */
    MPI_Type_size(datatype, &datatype_size);
    num_blocks = (int) ((count * datatype_size) / TUNNELFS_MAX_MSG_SIZE);
    if ((count * datatype_size) % TUNNELFS_MAX_MSG_SIZE)
        num_blocks++;

    MPI_Pack(&num_blocks, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* determine io server for this file */
    io_server_rank = tunnelfs_server_get_file_master(file_id);

    MPI_Send(outbuf, position, MPI_PACKED, io_server_rank,
             tunnelfs_req_type, TUNNELFS_COMM_WORLD);

    FREE(outbuf);

    return num_blocks;
}
