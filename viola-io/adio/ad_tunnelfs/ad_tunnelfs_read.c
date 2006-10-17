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
* File:         ad_tunnelfs_read.c                                        * 
* Description:  Blocking contiguous and strided read requests             * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <string.h>
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Reading contigous elements from a file 
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
void ADIOI_TUNNELFS_ReadContig(ADIO_File fd, void *buf, int count,
                               MPI_Datatype datatype, int file_ptr_type,
                               ADIO_Offset offset, ADIO_Status *status, int
                               *error_code)
{
    int datatype_size = 0;
    int etype_size = 0;
    int inbuf_size = 0;
    int num_blocks = 0;
    int position = 0;
    int rcode = 0;
    int reply_id = 0;
    int send_id = 0;
    int recvd = 0;
    int io_server_rank = -1;
    void *inbuf = NULL;

#ifdef DEBUG_TUNNELFS
    int rank;
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank);
    fprintf(stderr, "[%i] Entering Read\n", rank);
#endif

    MPI_Type_size(datatype, &datatype_size);
    MPI_Type_size(fd->etype, &etype_size);

    tunnelfs_ioreq(TUNNELFS_READ, fd, buf, count, datatype, file_ptr_type,
                   offset, status, error_code, &send_id);

    /* determine io server */
    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);
    tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                           io_server_rank, send_id);

    position = 0;
    MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    TUNNELFS_CHECK_MSG_ID(send_id, reply_id);

    MPI_Unpack(inbuf, inbuf_size, &position, error_code, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    tunnelfs_globals_set_active_fd(&fd);
    tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

    num_blocks = tunnelfs_globals_get_io_blocks();

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%i] Number of blocks to receive: %i\n", rank,
            num_blocks);
#endif

    if (*error_code == MPI_SUCCESS)
    {
        int i = 0;
        int block_count = 0;
        ADIO_Offset buf_offset = 0;

        for (i = 0; i < num_blocks; i++)
        {
            /* get next io package */
            MPI_Probe(io_server_rank, TUNNELFS_IODATA, TUNNELFS_COMM_WORLD,
                      status);
            /* calculate buffer size in etypes */
            /*MPI_Get_count(status, fd->etype, &block_count); */
            MPI_Get_count(status, datatype, &block_count);
            /* adjust buffer */
            /*
               tunnelfs_adjust_buffer(&inbuf, &inbuf_size, block_count);
             */
            MPI_Recv((buf + buf_offset), block_count, datatype,
                     status->MPI_SOURCE, status->MPI_TAG, TUNNELFS_COMM_WORLD,
                     status);

            buf_offset += block_count * etype_size;
        }
    }

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%i] Done with read\n", rank);
#endif

    FREE(inbuf);

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, datatype, datatype_size * count);
#endif
    *error_code = rcode;
}

/**
 * Reading non-contigous elements from a file 
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
void ADIOI_TUNNELFS_ReadStrided(ADIO_File fd, void *buf, int count,
                                MPI_Datatype datatype, int file_ptr_type,
                                ADIO_Offset offset, ADIO_Status *status, int
                                *error_code)
{
    int datatype_size = 0;
    int etype_size = 0;
    int inbuf_size = 0;
    int num_blocks = 0;
    int position = 0;
    int rcode = 0;
    int reply_id = 0;
    int send_id = 0;
    int recvd = 0;
    int io_server_rank = -1;
    void *inbuf = NULL;
    void *pack_buf = NULL;
    int pack_buf_size = 0;
    int pack_size = 0;

    MPI_Type_size(datatype, &datatype_size);
    MPI_Type_size(fd->etype, &etype_size);

    tunnelfs_ioreq(TUNNELFS_READ, fd, buf, count, datatype, file_ptr_type,
                   offset, status, error_code, &send_id);

    /* determine io server */
    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);
    tunnelfs_msg_get_reply(&inbuf, &inbuf_size, &recvd,
                           io_server_rank, send_id);

    position = 0;
    MPI_Unpack(inbuf, inbuf_size, &position, &reply_id, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    TUNNELFS_CHECK_MSG_ID(send_id, reply_id);

    MPI_Unpack(inbuf, inbuf_size, &position, error_code, 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    tunnelfs_globals_set_active_fd(&fd);
    tunnelfs_msg_get_variables(inbuf, inbuf_size, &position, recvd);

    num_blocks = tunnelfs_globals_get_io_blocks();

    pack_buf_size = count * datatype_size;


    MPI_Pack_size(count * datatype_size, fd->etype, TUNNELFS_COMM_WORLD,
                  &pack_size);
#if 0
    tunnelfs_adjust_buffer(&pack_buf, &pack_buf_size, pack_size);
#endif

    position = 0;
    if (*error_code == MPI_SUCCESS)
    {
        int i = 0;
        int block_count = 0;
        ADIO_Offset buf_offset = 0;

        for (i = 0; i < num_blocks; i++)
        {
            /* get next io package */
            MPI_Probe(io_server_rank, TUNNELFS_IODATA, TUNNELFS_COMM_WORLD,
                      status);
            /* calcualte buffer size in etypes */
/*            MPI_Get_count(status, fd->etype, &block_count);*/
            MPI_Get_count(status, datatype, &block_count);

#if 0
            /* adjust buffer */
            tunnelfs_adjust_buffer(&inbuf, &inbuf_size,
                                   block_count * etype_size);
            /* receive into temporaray buffer */
            MPI_Recv(inbuf, block_count, fd->etype, status->MPI_SOURCE,
                     status->MPI_TAG, TUNNELFS_COMM_WORLD, status);

            /* pack into second temporary buffer */
            MPI_Pack(inbuf, block_count, fd->etype, pack_buf, pack_buf_size,
                     &position, TUNNELFS_COMM_WORLD);
#else
            MPI_Recv((buf + buf_offset), block_count, datatype,
                     status->MPI_SOURCE, status->MPI_TAG, TUNNELFS_COMM_WORLD,
                     status);
            buf_offset += block_count * datatype_size;
#endif
        }

#if 0
        /* unpack from second temporary buffer to user provided buffer */
        position = 0;
        MPI_Unpack(pack_buf, pack_buf_size, &position, buf, count, datatype,
                   TUNNELFS_COMM_WORLD);
#endif
    }

    FREE(inbuf);
    FREE(pack_buf);

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, datatype, datatype_size * count);
#endif
    *error_code = rcode;
}
