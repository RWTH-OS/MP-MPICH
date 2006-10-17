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
* File:         tunnelfs_srv_fileinfo.h                                   * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  encapsulate file information                              *
*                                                                         *
**************************************************************************/
#ifndef _TUNNELFS_SRV_FILEINFO_H
#define _TUNNELFS_SRV_FILEINFO_H

#include "mpi.h"

/**
 * File information structure
 */
typedef struct
{
    int file_id;        /**< ID for file handle */
    int comm_id;        /**< ID for associated communicator */
    char *filename;     /**< String identifier for the filename */
    char *fs_domain;    /**< Filesystem domain */
    char *filesystem;   /**< Filesystem */
    char *mpi_filename; /**< filename used for MPI open */
    int accessmode;     /**< MPI access mode */
    MPI_Offset disp;    /**< Displacement to beginning of the file */
    int etype_id;       /**< Elementary type id */
    int ftype_id;       /**< Filetype type id */
    int iomode;         /**< IO mode for open call */
    int perm;           /**< Permissions for open call */
    int info_size;      /**< Size of info object */
    MPI_Info info;      /**< MPI_Info object */
} tunnelfs_fileinfo_t;

/**
 * Fill fileinfo structure with values of a given file handle
 * @param file_id Internal file id
 * @param fi Reference on fileinfo structure
 */
void tunnelfs_srv_fileinfo_create(int file_id, tunnelfs_fileinfo_t *fi);

/**
 * Calculate size of fileinfo structure in an mpi message buffer
 * @param fi Reference to fileinfo structure
 * @param size Reference to size variable
 */
void tunnelfs_srv_fileinfo_pack_size(tunnelfs_fileinfo_t *fi, int *size);

/**
 * Pack fileinfo structure into an mpi message buffer
 * @param buf Reference of mpi message buffer
 * @param size Size of message buffer
 * @param position Reference on position indicator
 * @param fi Reference to fileinfo structure
 */
void tunnelfs_srv_fileinfo_pack(void *buf, int size, int *position,
                                tunnelfs_fileinfo_t *fi);

/**
 * Unpack fileinfo structure from an mpi message buffer
 * @param buf Reference of mpi message buffer
 * @param size Size of message buffer
 * @param position Reference on position indicator
 * @param fi Reference to fileinfo structure
 */
void tunnelfs_srv_fileinfo_unpack(void *buf, int size, int *position,
                                  tunnelfs_fileinfo_t *fi);

#endif
