/**************************************************************************
* TunnelFS Server                                                         * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         tunnelfs_srv_fileinfo.c                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  encapsulate file information                              *
*                                                                         *
**************************************************************************/
#include <assert.h>
#include "mpi.h"
#include "adio.h"
#include "pario_threads.h"
#include "ad_tunnelfs.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * Fill fileinfo structure with values of a given file handle
 * @param file_id Internal file id
 * @param fi Reference on fileinfo structure
 */
void tunnelfs_srv_fileinfo_create(int file_id, tunnelfs_fileinfo_t *fi)
{
    MPI_File *fh = NULL;

    assert(file_id > 0);
    assert(fi != NULL);

    fh = tunnelfs_srv_file_get_handle(file_id);
    assert(fh != NULL);

    fi->file_id = file_id;
    fi->comm_id = tunnelfs_srv_file_get_comm_id(file_id);
    fi->filename = tunnelfs_srv_file_get_name(file_id);
    fi->mpi_filename = tunnelfs_srv_file_get_name(file_id);

    fi->accessmode = (*fh)->access_mode;

    fi->disp = 0;
    fi->etype_id = MPI_BYTE;
    fi->ftype_id = MPI_BYTE;
    fi->iomode = (*fh)->iomode;
    fi->perm = (*fh)->perm;

    LOCK_MPI();
    MPI_Info_get_nkeys((*fh)->info, &(fi->info_size));
    UNLOCK_MPI();

    fi->info = (*fh)->info;
}

/**
 * Calculate size of fileinfo structure in an mpi message buffer
 * @param fi Reference to fileinfo structure
 * @param size Reference to size variable
 */
void tunnelfs_srv_fileinfo_pack_size(tunnelfs_fileinfo_t *fi, int *size)
{
    int num_ints = 9;
    int num_char = 0;
    int temp_size = 0;
    int filename_len = 0;

    assert(fi != NULL);

    *size = 0;

    LOCK_MPI();
    if (fi->info_size > 0)
    {
        int i;
        char info_key[MPI_MAX_INFO_KEY + 1];
        for (i = 0; i < fi->info_size; i++)
        {
            int val_len = 0;
            int flag = 0;
            MPI_Info_get_nthkey(fi->info, i, info_key);
            MPI_Info_get_valuelen(fi->info, info_key, &val_len, &flag);
            num_ints += 2;
            num_char += strlen(info_key) + val_len + 2;
        }
    }

    MPI_Pack_size(num_ints, MPI_INT, TUNNELFS_COMM_WORLD, &temp_size);
    *size += temp_size;
    filename_len = strlen(fi->mpi_filename);
    MPI_Pack_size(num_char +
                  filename_len, MPI_CHAR, TUNNELFS_COMM_WORLD, &temp_size);
    *size += temp_size;
    MPI_Pack_size(1, TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD, &temp_size);
    *size += temp_size;
    UNLOCK_MPI();
}

/**
 * Pack fileinfo structure into an mpi message buffer
 * @param buf Reference of mpi message buffer
 * @param size Size of message buffer
 * @param position Reference on position indicator
 * @param fi Reference to fileinfo structure
 */
void tunnelfs_srv_fileinfo_pack(void *buf, int size, int *position,
                                tunnelfs_fileinfo_t *fi)
{
    int filename_len = 0;

    assert(buf != NULL);
    assert(size > 0);
    assert(position >= 0);
    assert(fi != NULL);

    assert(fi->mpi_filename != NULL);
    filename_len = strlen(fi->mpi_filename);

    LOCK_MPI();
    /* file id */
    MPI_Pack(&(fi->file_id), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* comm id */
    MPI_Pack(&(fi->comm_id), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* filename */
    MPI_Pack(&filename_len, 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(fi->mpi_filename, filename_len, MPI_CHAR, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* access mode */
    MPI_Pack(&(fi->accessmode), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* displacement */
    MPI_Pack(&(fi->disp), 1, TUNNELFS_OFFSET, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* etype and filetype */
    MPI_Pack(&(fi->etype_id), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    MPI_Pack(&(fi->ftype_id), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* iomode */
    MPI_Pack(&(fi->iomode), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    /* permissions */
    MPI_Pack(&(fi->perm), 1, MPI_INT, buf, size, position,
             TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    tunnelfs_srv_info_pack(buf, size, position, fi->info_size, fi->info);
}

/**
 * Unpack fileinfo structure from an mpi message buffer
 * @param buf Reference of mpi message buffer
 * @param size Size of message buffer
 * @param position Reference on position indicator
 * @param fi Reference to fileinfo structure
 */
void tunnelfs_srv_fileinfo_unpack(void *buf, int size, int *position,
                                  tunnelfs_fileinfo_t *fi)
{
    char *str_tmp = NULL;
    int filename_len = 0;

    assert(buf != NULL);
    assert(size > 0);
    assert(position >= 0);
    assert(fi != NULL);

    LOCK_MPI();

    MPI_Unpack(buf, size, position, &(fi->file_id), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);
    LOG("File = %i", fi->file_id);

    MPI_Unpack(buf, size, position, &(fi->comm_id), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);
    LOG("Communicator = %i", fi->comm_id);

    MPI_Unpack(buf, size, position, &filename_len, 1,
               MPI_INT, TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    assert(filename_len > 0);
    ALLOC(str_tmp, filename_len + 1);

    LOCK_MPI();
    MPI_Unpack(buf, size, position, str_tmp,
               filename_len, MPI_CHAR, TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();
    /* be sure to place a \0 at the end */
    str_tmp[filename_len] = '\0';
    
    LOG("transfered filename: %s with length %i", str_tmp, filename_len);

    fi->filesystem = NULL;
    fi->fs_domain = NULL;
    fi->filename = NULL;
    tunnelfs_srv_tokenize_filename(str_tmp, &(fi->filesystem),
                                   &(fi->fs_domain), &(fi->filename));

    assert(fi->filesystem != NULL);
    assert(fi->fs_domain != NULL);
    assert(fi->filename != NULL);

    LOG("Filesystem: %s", fi->filesystem);
    LOG("Filesystem domain: %s", fi->fs_domain);
    LOG("Filename: %s", fi->filename);

    fi->mpi_filename = NULL;
    /* reconstruct original filename without filesystemdomain */
    if ((strncmp(fi->filesystem, "default", 7) != 0) &&
        (strlen(fi->filename) > 0))
    {
        LOG("Filesystem: %s", fi->filesystem);
        ALLOC(fi->mpi_filename,
              strlen(fi->filename) + strlen(fi->filesystem) + 2);
        sprintf(fi->mpi_filename, "%s:%s", fi->filesystem, fi->filename);
    }
    else
    {
        ALLOC(fi->mpi_filename, strlen(fi->filename) + 1);
        STRNCPY(fi->mpi_filename, fi->filename, strlen(fi->filename));
    }

    LOG("MPI Filename: %s", fi->mpi_filename);

    LOCK_MPI();
    MPI_Unpack(buf, size, position, &(fi->accessmode), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->disp), 1,
               TUNNELFS_OFFSET, TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->etype_id), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->ftype_id), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->iomode), 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->perm), 1, MPI_INT,
               TUNNELFS_COMM_WORLD);

    MPI_Unpack(buf, size, position, &(fi->info_size), 1,
               MPI_INT, TUNNELFS_COMM_WORLD);
    UNLOCK_MPI();

    tunnelfs_srv_info_create(buf, size, position, fi->info_size, &(fi->info));
}
