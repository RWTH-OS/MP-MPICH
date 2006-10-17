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
* File:         ad_tunnelfs_fcntl.c                                       * 
* Description:  File control for tunnelfs                                 * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "adioi.h"
#include "adio_extern.h"

/**
 * Function to execute several smaller client functions like
 *      - Preallocating a file
 *      - Resizing a file
 *      - Setting atomicity flag on a file
 *      - Setting io mode on a file
 * @param fd MPI file descriptor
 * @param flag flag indicating operation to be performed
 * @param fcntl_struct ADIO internal structure for in- and output of parameters
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct,
                          int *error_code)
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

    int io_server_rank = -1;

    MPI_Pack_size(3, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
    pack_size += tmp_size;
    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &tmp_size);
    pack_size += tmp_size;

    tunnelfs_adjust_buffer(&outbuf, &outbuf_size, pack_size);

    MPI_Pack(&msg_id, 1, MPI_INT, outbuf, outbuf_size, &position,
             TUNNELFS_COMM_WORLD);

    /* determine io server to communicate with */
    file_id = fd->fd_sys;
    io_server_rank = tunnelfs_server_get_file_master(file_id);

    switch (flag)
    {
    case ADIO_FCNTL_SET_ATOMICITY:
        {
            int myrank_filecomm;

            /* This call is collective! */
            MPI_Comm_rank(fd->comm, &myrank_filecomm);

            if (myrank_filecomm == 0)
            {
                var_id = TUNNELFS_REQ_SET_ATOMICITY;
                MPI_Pack(&var_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);

                MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);

                MPI_Pack(&(fcntl_struct->atomicity), 1, MPI_INT, outbuf,
                         outbuf_size, &position, TUNNELFS_COMM_WORLD);

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
                tunnelfs_msg_get_variables(inbuf, inbuf_size, &position,
                                           recvd);
            }
            /* distribute rcode */
            MPI_Bcast(&rcode, 1, MPI_INT, 0, fd->comm);

            if (rcode == 0)
                fd->atomicity = fcntl_struct->atomicity;
            break;
        }
    case ADIO_FCNTL_SET_IOMODE:
        {
            fprintf(stderr,
                    "### TUNNELFS: Setting iomode not implemented yet\n");
            break;
        }
    case ADIO_FCNTL_SET_DISKSPACE:
        {
            int myrank_filecomm;

            /* This call is collective! */
            MPI_Comm_rank(fd->comm, &myrank_filecomm);

            if (myrank_filecomm == 0)
            {
                var_id = TUNNELFS_REQ_PREALLOCATE;
                MPI_Pack(&var_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);

                MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                         TUNNELFS_COMM_WORLD);

                MPI_Pack(&(fcntl_struct->diskspace), 1, TUNNELFS_OFFSET,
                         outbuf, outbuf_size, &position, TUNNELFS_COMM_WORLD);

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
                tunnelfs_msg_get_variables(inbuf, inbuf_size, &position,
                                           recvd);

                fcntl_struct->fsize = tunnelfs_globals_get_filesize();
            }
            /* distribute rcode */
            MPI_Bcast(&rcode, 1, MPI_INT, 0, fd->comm);
            break;
        }
    case ADIO_FCNTL_GET_FSIZE:
        {
            var_id = TUNNELFS_REQ_FILESIZE;
            MPI_Pack(&var_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                     TUNNELFS_COMM_WORLD);

            MPI_Pack(&file_id, 1, MPI_INT, outbuf, outbuf_size, &position,
                     TUNNELFS_COMM_WORLD);

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

            fcntl_struct->fsize = tunnelfs_globals_get_filesize();
            break;
        }
    default:
        {
            fprintf(stderr,
                    "### TUNNELFS: Calling File control with unknown flag: %i\n",
                    flag);
            break;
        }
    }
    *error_code = rcode;

    FREE(inbuf);
    FREE(outbuf);
}
