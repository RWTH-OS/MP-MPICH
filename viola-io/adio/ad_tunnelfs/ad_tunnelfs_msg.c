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
* File:         ad_tunnelfs_msg.c                                         * 
* Description:  Provide standard message handling routines                * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "ad_tunnelfs_globals.h"
#include "metaconfig.h"

/**
 * Tunnelfs message id
 */
int tunnelfs_msg_id = 0;

/**
 * Reference to global Meta-Config Structure
 */
extern MPIR_MetaConfig MPIR_meta_cfg;

/**
 * Reallocate a buffer only if necessary
 * @param buf Reference to buffer pointer
 * @param buf_size Reference to size of buffer
 * @param min Required size of buffer
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_adjust_buffer_x(void **buf, int *buf_size, int min, char *file,
                              int line)
{
    if (min <= 0)
        fprintf(stderr, "%s, Line %i: WARNING: Size parameter is zero\n",
                file, line);

    if ((*buf_size >= min) && (*buf != NULL))
        return;
    else
    {
        /* allocate new buffer */
        if (*buf != NULL)
            free(*buf);
        *buf = calloc(1, min);
        if (*buf == NULL)
            errmsg(TUNNELFS_ERR_ALLOC, file, line, 1);
        *buf_size = min;
    }
}

/**
 * Receive reply to a specific message
 * @param reply_buf Buffer for receive operation
 * @param reply_buf_size Size of buffer
 * @param rcvd Number of bytes received
 * @param source Source of receive operation
 * @param msg_id Tunnelfs message id
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_get_reply_x(void **reply_buf, int *reply_buf_size,
                              int *rcvd, int source, int msg_id,
                              char *file, int line)
{
    MPI_Status status;
    int count = 0;
    int rank = 0;

    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &rank);

#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "[%i] Waiting for reply from %i with tag %i (%s, %i)\n",
            rank, source, TUNNELFS_REPLY_BASE + msg_id, file, line);
#endif

    /* reply tag is TUNNELFS_REPLY_BASE plus initial message id */
    MPI_Probe(source, TUNNELFS_REPLY_BASE + msg_id, TUNNELFS_COMM_WORLD,
              &status);
    MPI_Get_count(&status, MPI_PACKED, &count);

    tunnelfs_adjust_buffer_x(reply_buf, reply_buf_size, count, file, line);

    MPI_Recv(*reply_buf, *reply_buf_size, MPI_PACKED, status.MPI_SOURCE,
             status.MPI_TAG, TUNNELFS_COMM_WORLD, &status);
    *rcvd = count;

    switch (status.MPI_TAG - msg_id)
    {
    case TUNNELFS_REPLY_BASE:
    case TUNNELFS_REPLY:
        {
            /* correct tag - no warning */
            break;
        }
    case TUNNELFS_SETUP:
        {
            fprintf(stderr,
                    "%s, Line %i: Old style protocol used! Server should not issue TUNNELFS_SETUP messages ! \n",
                    file, line);
            break;
        }
    default:
        {
            fprintf(stderr,
                    "%s, Line %i: Received unexpected message tag %i\n",
                    file, line, status.MPI_TAG);
            exit(-1);
            break;
        }
    }
}

/**
 * Unpack variable from buffer
 * @param buf Message buffer
 * @param buf_size Size of buffer
 * @param position Reference to position indicator
 * @param recvd Number of bytes received
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_get_variables_x(void *buf, int buf_size, int
                                  *position, int recvd, char *file, int line)
{
    int var_id = 0;
#ifdef DEBUG_TUNNELFS
    int my_rank = -1;

    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &my_rank);

    fprintf(stderr, "################## GETTING VARIABLES ##############\n");
#endif
    while ((*position < buf_size) && (*position < recvd))
    {
        MPI_Unpack(buf, buf_size, position, &var_id, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);
#ifdef DEBUG_TUNNELFS
        fprintf(stderr, "Var id: %i\n", var_id);
#endif
        switch (var_id)
        {
        case TUNNELFS_VAR_COMM_ID:
            {
                int comm_id = -1;
                ADIO_File fh;
                MPI_Unpack(buf, buf_size, position, &comm_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                fh = tunnelfs_globals_get_active_fd();
                tunnelfs_comm_register(comm_id, fh->comm);
                tunnelfs_comm_set_transfered(comm_id);
                break;
            }
        case TUNNELFS_VAR_FILE_ID:
            {
                int file_id = -1;
                ADIO_File fh;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                fh = tunnelfs_globals_get_active_fd();
                fh->fd_sys = file_id;
                break;
            }
        case TUNNELFS_VAR_SHARED_PTR:
            {
                int file_id = -1;
                ADIO_Offset value;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &value, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                tunnelfs_globals_set_shared_fp(value);
                break;
            }
        case TUNNELFS_REQ_NUM_IO_BLOCKS:
            {
                int num_blocks;
                MPI_Unpack(buf, buf_size, position, &num_blocks, 1,
                           MPI_INT, TUNNELFS_COMM_WORLD);
                tunnelfs_globals_set_io_blocks(num_blocks);
                break;
            }
        case TUNNELFS_VAR_FILESIZE:
            {
                int file_id;
                ADIO_Offset filesize;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &filesize, 1,
                           TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);
                tunnelfs_globals_set_filesize(filesize);
                break;
            }
        case TUNNELFS_VAR_ATOMICITY:
            {
                int file_id;
                MPI_File fd;
                int atomicity;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &atomicity, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                fd = tunnelfs_globals_get_active_fd();
                if ((atomicity == 0) || (atomicity == 1))
                    fd->atomicity = atomicity;
                break;
            }
        case TUNNELFS_REQ_SET_FILESERVER:
            {
                int file_id;
                int io_server;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &io_server, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                tunnelfs_server_set_file_master(file_id, io_server);

#ifdef DEBUG_TUNNELFS
                fprintf(stderr, "[%2i] Set file master for %i to %i\n",
                        my_rank, file_id, io_server);
#endif
                break;
            }
        case TUNNELFS_VAR_DISTLIST:
            {
                int file_id;
                int num_clients;
                dist_list_t *list = NULL;
#ifdef DEBUG_TUNNELFS
                int i;
                char *str = NULL;
                char tmp[8];
#endif

                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &num_clients, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                ALLOC(list, num_clients * sizeof(dist_list_t));

                MPI_Unpack(buf, buf_size, position, list, 2 * num_clients,
                           MPI_INT, TUNNELFS_COMM_WORLD);

                tunnelfs_globals_set_distlist(list, num_clients);

#ifdef DEBUG_TUNNELFS
                ALLOC(str, num_clients * 7 + 1);

                for (i = 0; i < num_clients; i++)
                {
                    snprintf(tmp, 8, "%i->%i ", list[i].client,
                             list[i].fileserver);
                    str = strcat(str, tmp);
                }

                fprintf(stderr, "[%2i] Server distribution: %s\n", my_rank,
                        str);
#endif
                break;
            }
        case TUNNELFS_VAR_INFO:
            {
                int file_id;
                MPI_File *fh_ptr = NULL;
                int num_hints = 0;
                MPI_Info info = MPI_INFO_NULL;
                char info_key[MPI_MAX_INFO_KEY + 1];
                char *info_val = NULL;
                MPI_Unpack(buf, buf_size, position, &file_id, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                MPI_Unpack(buf, buf_size, position, &num_hints, 1, MPI_INT,
                           TUNNELFS_COMM_WORLD);
                if (num_hints > 0)
                {
                    int i = 0;
                    MPI_Info_create(&info);
                    for (i = 0; i < num_hints; i++)
                    {
                        int key_len = 0;
                        int val_len = 0;
                        MPI_Unpack(buf, buf_size, position,
                                   &key_len, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                        if (key_len > MPI_MAX_INFO_KEY)
                            ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);
                        MPI_Unpack(buf, buf_size, position,
                                   info_key, key_len, MPI_CHAR,
                                   TUNNELFS_COMM_WORLD);
                        /* null terminate string */
                        info_key[key_len] = 0;
                        MPI_Unpack(buf, buf_size, position,
                                   &val_len, 1, MPI_INT, TUNNELFS_COMM_WORLD);
                        if (val_len > MPI_MAX_INFO_VAL)
                            ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);
                        ALLOC(info_val, val_len + 1);
                        if (val_len > 0)
                        {
                            MPI_Unpack(buf, buf_size, position, info_val,
                                       val_len, MPI_CHAR,
                                       TUNNELFS_COMM_WORLD);
                            info_val[val_len] = 0;
                        }
                        else
                        {
                            info_val[0] = 0;
                        }
                        MPI_Info_set(info, info_key, info_val);
                    }

                    *fh_ptr = tunnelfs_globals_get_active_fd();
                    MPI_File_set_info(*fh_ptr, info);
                    MPI_Info_free(&info);
                }
                break;
            }
        default:
            {
                fprintf(stderr,
                        "Unknown variable type %i in reply at %s, %i.\n",
                        var_id, file, line);
                break;
            }
        }
    }
#ifdef DEBUG_TUNNELFS
    fprintf(stderr, "###################################################\n");
#endif
}

/**
 * Send initial message to main server
 * @param buf Reference to message buffer pointer
 * @param buf_size Reference to size of buffer
 * @param argc Number of arguments in argument vector
 * @param argv Vector of character arrays holding arguments 
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_send_init_x(void **buf, int *buf_size,
                              int argc, char **argv, char *file, int line)
{
    int i = 0;
    int msg_id = TUNNELFS_CURR_MSG_ID;
    int position = 0;
    int size = 0;               /* size of standard init without arguments */
    int pack_size = 0;
    int io_server_rank = -1;
    char *domain = NULL;
    int domain_len = 0;
    tunnelfs_version_t tver;
    tver.maj = TUNNELFS_VERSION_MAJ;
    tver.min = TUNNELFS_VERSION_MIN;

    MPI_Pack_size(5, MPI_INT, TUNNELFS_COMM_WORLD, &size);

    /* retrieve my filesystem domain */
    ALLOC(domain, TUNNELFS_MAX_FS_DOMAINLEN + 1);
    strncpy(domain, MPIR_meta_cfg.my_metahostname, TUNNELFS_MAX_FS_DOMAINLEN);
    for (i = strlen(domain) - 1; i >= 0; i--)
        if (domain[i] != '_')
        {
            domain[i] = '\0';
            break;
        }
    if (i > 0)
        domain[i - 1] = '\0';

    domain_len = strlen(domain);
    MPI_Pack_size(domain_len, MPI_CHAR, TUNNELFS_COMM_WORLD, &pack_size);
    size += pack_size;

    if (argc > 0)
    {
        int i;
        for (i = 0; i < argc; i++)
        {
            MPI_Pack_size(1, MPI_INT, TUNNELFS_COMM_WORLD, &pack_size);
            size += pack_size;
            MPI_Pack_size(strlen(argv[i]), MPI_CHAR, TUNNELFS_COMM_WORLD,
                          &pack_size);
            size += pack_size;
        }
    }

    tunnelfs_adjust_buffer_x(buf, buf_size, size, file, line);
    MPI_Pack(&msg_id, 1, MPI_INT, *buf, *buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&tver, 2, MPI_INT, *buf, *buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&domain_len, 1, MPI_INT, *buf, *buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(domain, domain_len, MPI_CHAR, *buf, *buf_size, &position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&argc, 1, MPI_INT, *buf, *buf_size, &position,
             TUNNELFS_COMM_WORLD);
    if (argc > 0)
    {
        int arg_len = 0;
        int i;
        for (i = 0; i < argc; i++)
        {
            arg_len = strlen(argv[i]);
            if (arg_len > 0)
            {
                MPI_Pack(&arg_len, 1, MPI_INT, *buf, *buf_size, &position,
                         TUNNELFS_COMM_WORLD);
                MPI_Pack(argv[i], arg_len, MPI_CHAR, *buf, *buf_size,
                         &position, TUNNELFS_COMM_WORLD);
            }
            else
            {
                fprintf(stderr,
                        "%s, Line %i: WARNING: malformed commandline argument ignored.\n",
                        file, line);
            }
        }
    }

    io_server_rank = tunnelfs_server_get_global_master();
    MPI_Send(*buf, position, MPI_PACKED, io_server_rank,
             TUNNELFS_INIT, TUNNELFS_COMM_WORLD);

    FREE(domain);
}


/**
 * Send final message to main server
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_send_end_x(char *file, int line)
{
    int msg_id = TUNNELFS_CURR_MSG_ID;
    int error_code = 0;
    int io_server_rank = -1;
    io_server_rank = tunnelfs_server_get_global_master();
    error_code =
        MPI_Send(&msg_id, 1, MPI_INT, io_server_rank, TUNNELFS_END,
                 TUNNELFS_COMM_WORLD);
    if (error_code != MPI_SUCCESS)
    {
        fprintf(stderr,
                "%s, Line %i: ERROR: Tunnelfs connection could not be exited cleanly.\n",
                file, line);
    }
}
