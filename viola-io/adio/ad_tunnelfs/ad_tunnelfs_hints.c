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
 * Correlating MPI Info object with MPI file
 * @param fd MPI File descriptor
 * @param users_info User provided MPI info object
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_SetInfo(ADIO_File fd, MPI_Info users_info,
                            int *error_code)
{
    int rank = 0;
    int io_server_rank = 0;

    /* first see, if middleware wants to set something, too */
    ADIOI_GEN_SetInfo(fd, users_info, error_code);

    MPI_Comm_rank(fd->comm, &rank);
    if (((rank == 0) && (fd->is_open == 1) && (fd->ind_info_change == 1)))
    {
        int i;
        char info_key[MPI_MAX_INFO_KEY + 1];
        int num_keys = 0;
        int num_ints = 4;
        int num_char = 0;

        void *buf = NULL;
        int buf_size = 0;
        int tmp_size = 0;
        int pack_size = 0;
        int position = 0;
        char *info_val = NULL;
        int info_val_size = 0;

        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int var_id = TUNNELFS_REQ_SET_INFO;

        /* TODO: */
        /* file is already opened at remote site. We need to transfer the
         * information provided. */

        io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);
        MPI_Info_get_nkeys(fd->info, &num_keys);

        for (i = 0; i < num_keys; i++)
        {
            int val_len = 0;
            int flag = 0;

            MPI_Info_get_nthkey(fd->info, i, info_key);
            MPI_Info_get_valuelen(fd->info, info_key, &val_len, &flag);
            if (flag)
            {
                num_ints += 2;
                num_char += val_len + strlen(info_key) + 2;
            }
        }

        MPI_Pack_size(num_ints, MPI_INT, TUNNELFS_COMM_WORLD, &tmp_size);
        pack_size += tmp_size;
        MPI_Pack_size(num_char, MPI_CHAR, TUNNELFS_COMM_WORLD, &tmp_size);
        pack_size += tmp_size;

        tunnelfs_adjust_buffer(&buf, &buf_size, pack_size);

        MPI_Pack(&msg_id, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&var_id, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&(fd->fd_sys), 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&num_keys, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        for (i = 0; i < num_keys; i++)
        {
            int val_len = 0;
            int key_len = 0;
            int flag = 0;

            MPI_Info_get_nthkey(fd->info, i, info_key);
            MPI_Info_get_valuelen(fd->info, info_key, &val_len, &flag);
            if (flag)
            {
                if ((val_len + 1) > info_val_size)
                {
                    FREE(info_val);
                    ALLOC(info_val, val_len + 1);
                    info_val_size = val_len + 1;
                }

                MPI_Info_get(fd->info, info_key, val_len, info_val, &flag);
                if (flag)
                {
                    key_len = strlen(info_key);
                    MPI_Pack(&key_len, 1, MPI_INT, buf, buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(info_key, key_len, MPI_CHAR, buf, buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(&val_len, 1, MPI_INT, buf, buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                    MPI_Pack(info_val, val_len, MPI_CHAR, buf, buf_size,
                             &position, TUNNELFS_COMM_WORLD);
                }
            }
        }

        MPI_Send(buf, position, MPI_PACKED, io_server_rank, TUNNELFS_SETUP,
                 TUNNELFS_COMM_WORLD);
    }
}
