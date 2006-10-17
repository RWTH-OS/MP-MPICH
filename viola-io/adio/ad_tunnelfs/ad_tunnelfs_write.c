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
* File:         ad_tunnelfs_write.c                                       * 
* Description:  Blocking contiguous and strided write requests            * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <unistd.h>
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Writing contigous elements to a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteContig(ADIO_File fd, void *buf, int count,
                                MPI_Datatype datatype, int file_ptr_type,
                                ADIO_Offset offset, ADIO_Status *status, int
                                *error_code)
{
    void *inbuf = NULL;
    int inbuf_size = 0;
    int datatype_size;
    int datatype_id;
    int send_id = 0;
    int reply_id = 0;
    int rcode = 0;
    int recvd = 0;
    int position = 0;
#ifdef DEBUG_TUNNELFS
    int rank = 0;
#endif

    int num_msgs = 0;
    int i = 0;
    long buf_offset = 0;
    int block_count = 0;

    int io_server_rank = -1;

    MPI_Request *send_request = NULL;
    MPI_Status *send_status = NULL;

    MPI_Type_size(datatype, &datatype_size);

    tunnelfs_datatype_get_id(datatype, &datatype_id);

    /* sending io request 
     * - function will determine number of messages and hand it as 
     *   return value */

    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);

    /* check if datatype is in sync with server */
    tunnelfs_datatype_sync(datatype, io_server_rank);


#ifdef DEBUG_TUNNELFS
    MPI_Comm_rank(fd->comm, &rank);
    fprintf(stderr, "[%i] file master for %i is %i\n", rank, fd->fd_sys,
            io_server_rank);
#endif

    num_msgs =
        tunnelfs_ioreq(TUNNELFS_WRITE, fd, buf, count, datatype,
                       file_ptr_type, offset, status, error_code, &send_id);

    /* calculating block size for first num_msgs-1 blocks */
    if (num_msgs > 1)
        block_count = TUNNELFS_MAX_MSG_SIZE / datatype_size;
    else
        block_count = count;

    if (num_msgs > 0)
    {
        ALLOC(send_request, num_msgs * sizeof(MPI_Request));
        ALLOC(send_status, num_msgs * sizeof(MPI_Status));
    }

    if ((num_msgs <= 0) || (send_request == NULL) || (send_status == NULL))
    {
        fprintf(stderr, "Memory allocation error for immediate sends.  Aborting!\n");
        exit(-1);
    }

    /* sending io data */
    for (i = 0; i < num_msgs; i++)
    {
        if (i == num_msgs - 1)
            block_count = count - (buf_offset / datatype_size);
        MPI_Isend(buf + buf_offset, block_count, datatype,
                 io_server_rank, TUNNELFS_IODATA, TUNNELFS_COMM_WORLD,
                 &(send_request[i]));
        buf_offset += block_count * datatype_size;
    }
    MPI_Waitall(num_msgs, send_request, send_status);

    FREE(send_request);
    FREE(send_status);

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%i] io data sent ... now waiting for reply\n", rank);
#endif
    /* waiting for reply */
    tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                           io_server_rank, send_id);

    position = 0;
    MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    TUNNELFS_CHECK_MSG_ID(send_id, reply_id);

    MPI_Unpack(inbuf, inbuf_size, &position, &rcode, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    tunnelfs_globals_set_active_fd(&fd);
    tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

    FREE(inbuf);

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, datatype, datatype_size * count);
#endif
    *error_code = rcode;
}

/**
 * Writing non-contigous elements to a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteStrided(ADIO_File fd, void *buf, int count,
                                 MPI_Datatype datatype, int file_ptr_type,
                                 ADIO_Offset offset, ADIO_Status *status,
                                 int *error_code)
{
    void *inbuf = NULL;
    int inbuf_size = 0;
    int datatype_size;
    int etype_size;
    int datatype_id;
    int send_id = 0;
    int reply_id = 0;
    int rcode = 0;
    int recvd = 0;
    int position = 0;

    int num_msgs = 0;
    int i = 0;
    long buf_offset = 0;
    int block_count = 0;

    int io_server_rank = -1;
    void *pack_buf = NULL;
    int pack_buf_size = 0;
    int pack_size = 0;
    void *temp_buf = NULL;

    MPI_Request *send_request = NULL;
    MPI_Status *send_status = NULL;

    MPI_Type_size(datatype, &datatype_size);
    MPI_Type_size(fd->etype, &etype_size);

    tunnelfs_datatype_get_id(datatype, &datatype_id);

    /* sending io request 
     * - function will determine number of messages and hand it as 
     *   return value */

    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);

    num_msgs =
        tunnelfs_ioreq(TUNNELFS_WRITE, fd, buf, count, datatype,
                       file_ptr_type, offset, status, error_code, &send_id);

    if (datatype_size > TUNNELFS_MAX_MSG_SIZE)
    {
        int blocks;

        /* we cannot fit one datatype into a single send buffer, therefore we
         * have to do some rearranging */

        MPI_Pack_size(count, datatype, TUNNELFS_COMM_WORLD, &pack_size);
        tunnelfs_adjust_buffer(&pack_buf, &pack_buf_size, pack_size);

        MPI_Pack(buf, count, datatype, pack_buf, pack_buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        /* calculating block size for first num_msgs-1 blocks */
        blocks = count * datatype_size / etype_size;
        if (num_msgs > 1)
            block_count = TUNNELFS_MAX_MSG_SIZE / etype_size;
        else
            block_count = blocks;

        ALLOC(temp_buf, block_count * etype_size);

        if (num_msgs > 0)
        {
            ALLOC(send_request, num_msgs * sizeof(MPI_Request));
            ALLOC(send_status, num_msgs * sizeof(MPI_Status));
        }

        if ((num_msgs <= 0) || (send_request == NULL) || (send_status == NULL))
        {
            fprintf(stderr, "Memory allocation error for immediate sends.  Aborting!\n");
            exit(-1);
        }

        position = 0;
        /* sending io data */
        for (i = 0; i < num_msgs; i++)
        {
            if (i == num_msgs - 1)
                block_count = blocks - (buf_offset / etype_size);

            MPI_Unpack(pack_buf, pack_buf_size, &position, temp_buf,
                       block_count, fd->etype, TUNNELFS_COMM_WORLD);
            MPI_Isend(temp_buf, block_count, fd->etype,
                      io_server_rank, TUNNELFS_IODATA, TUNNELFS_COMM_WORLD,
                      &(send_request[i]));
            buf_offset += block_count * etype_size;
        }

        MPI_Waitall(num_msgs, send_request, send_status);

        FREE(send_request);
        FREE(send_status);

        FREE(pack_buf);
        FREE(temp_buf);
    }
    else
    {
        /* calculating block size for first num_msgs-1 blocks */
        if (num_msgs > 1)
            block_count = TUNNELFS_MAX_MSG_SIZE / datatype_size;
        else
            block_count = count;

        if (num_msgs > 0)
        {
            ALLOC(send_request, num_msgs * sizeof(MPI_Request));
            ALLOC(send_status, num_msgs * sizeof(MPI_Status));
        }

        if ((num_msgs <= 0) || (send_request == NULL) || (send_status == NULL))
        {
            fprintf(stderr, "Memory allocation error for immediate sends.  Aborting!\n");
            exit(-1);
        }

        /* sending io data */
        for (i = 0; i < num_msgs; i++)
        {
            if (i == num_msgs - 1)
                block_count = count - (buf_offset / datatype_size);
            MPI_Isend(buf + buf_offset, block_count, datatype,
                      io_server_rank, TUNNELFS_IODATA, TUNNELFS_COMM_WORLD,
                      &(send_request[i]));
            buf_offset += block_count * datatype_size;
        }
        MPI_Waitall(num_msgs, send_request, send_status);

        FREE(send_request);
        FREE(send_status);
    }

    /* waiting for reply */
    tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                           io_server_rank, send_id);

    position = 0;
    MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    TUNNELFS_CHECK_MSG_ID(send_id, reply_id);

    MPI_Unpack(inbuf, inbuf_size, &position, &rcode, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    tunnelfs_globals_set_active_fd(&fd);
    tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

    FREE(inbuf);

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, datatype, datatype_size * count);
#endif
    *error_code = rcode;
}
