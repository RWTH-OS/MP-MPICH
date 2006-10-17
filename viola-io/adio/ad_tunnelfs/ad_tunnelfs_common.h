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
* File:         tunnelfs.h                                                * 
* Description:  Basic defines for client and server                       * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#ifndef AD_TUNNELFS_COMMON_H
#define AD_TUNNELFS_COMMON_H

/* Protocol Version */
#define TUNNELFS_VERSION_MAJ 1
#define TUNNELFS_VERSION_MIN 0

typedef struct
{
    int maj;
    int min;
}
tunnelfs_version_t;

#define TUNNELFS_SUCCESS             1
#define TUNNELFS_FAILURE            -1

/* Message Types */
#define TUNNELFS_INIT               0x0001
#define TUNNELFS_END                0x0002
#define TUNNELFS_SETUP              0x0003
#define TUNNELFS_REPLY              0x0004
#define TUNNELFS_REPLY_BASE         0xf000

#define TUNNELFS_DATATYPE           0x0101
#define TUNNELFS_OPEN               0x0102
#define TUNNELFS_CLOSE              0x0103
#define TUNNELFS_FCNTL              0x0104
#define TUNNELFS_SEEKIND            0x0105
#define TUNNELFS_DELETE             0x0106
#define TUNNELFS_RESIZE             0x0107
#define TUNNELFS_FLUSH              0x0108
#define TUNNELFS_SET_VIEW           0x0109

#define TUNNELFS_READ               0x0201
#define TUNNELFS_WRITE              0x0202
#define TUNNELFS_IODATA             0x0203

/* Server side communication tags */

#define TUNNELFS_SERVER_SHUTDOWN    0x1001
#define TUNNELFS_SERVER_SETUP       0x1003
#define TUNNELFS_SERVER_REPLY       0x1004

#define TUNNELFS_SERVER_OPEN_CLONE          0x1101
#define TUNNELFS_SERVER_OPEN_CLONE_DONE     0x1102
#define TUNNELFS_SERVER_OPEN_PART           0x1103
#define TUNNELFS_SERVER_DELEGATED_CLOSE     0x1104
#define TUNNELFS_SERVER_OPEN_PART_DONE      0x1105
#define TUNNELFS_SERVER_DELEGATED_DELETE    0x1106
#define TUNNELFS_SERVER_DELEGATED_OPEN      0x1107
#define TUNNELFS_SERVER_CLOSE_MUTUAL        0x1108
#define TUNNELFS_SERVER_CLOSE_MUTUAL_DONE   0x1109
#define TUNNELFS_SERVER_FLUSH_PART          0x110a
#define TUNNELFS_SERVER_FLUSH_PART_DONE     0x110b
#define TUNNELFS_SERVER_READ                0x110c
#define TUNNELFS_SERVER_WRITE               0x110d
#define TUNNELFS_SERVER_IODATA              0x110e

#define TUNNELFS_SERVER_IODATA_BASE         0x1200

/* Variable Types */
#define TUNNELFS_ARCH_ENDIAN        0x0001
#define TUNNELFS_ARCH_TYPE          0x0002

#define TUNNELFS_VAR_VERSION        0x0101
#define TUNNELFS_VAR_COMMUNICATOR   0x0102
#define TUNNELFS_VAR_IND_PTR        0x0103
#define TUNNELFS_VAR_SHARED_PTR     0x0104
#define TUNNELFS_VAR_FILE_ID        0x0105
#define TUNNELFS_VAR_COMM_ID        0x0106
#define TUNNELFS_VAR_FILESIZE       0x0107
#define TUNNELFS_VAR_ATOMICITY      0x0108
#define TUNNELFS_VAR_DATATYPE       0x0109
#define TUNNELFS_VAR_INFO           0x010a
#define TUNNELFS_VAR_CLIENTFSDOM    0x010b
#define TUNNELFS_VAR_DISTLIST       0x010c
#define TUNNELFS_VAR_FILEHANDLE     0x010d
#define TUNNELFS_VAR_FILEVIEW       0x010e
#define TUNNELFS_VAR_OPERATION_ID   0x010f

#define TUNNELFS_REQ_SET_FILESERVER 0x0201
#define TUNNELFS_REQ_SET_MASTER     0x0202
#define TUNNELFS_REQ_NUM_IO_BLOCKS  0x0203
#define TUNNELFS_REQ_SHARED_PTR     0x0204
#define TUNNELFS_REQ_RESIZE         0x0205
#define TUNNELFS_REQ_PREALLOCATE    0x0206
#define TUNNELFS_REQ_FILESIZE       0x0207
#define TUNNELFS_REQ_SET_ATOMICITY  0x0208
#define TUNNELFS_REQ_SET_VIEW       0x0209
#define TUNNELFS_REQ_SET_SHARED_PTR 0x020a
#define TUNNELFS_REQ_SET_INFO       0x020b
#define TUNNELFS_REQ_GETSET_SHARED_PTR 0x020c

/* global communicator */
extern MPI_Comm MPI_COMM_TUNNELFS_WORLD;
#define TUNNELFS_COMM_WORLD MPI_COMM_TUNNELFS_WORLD

/* offset sizes */
#define TUNNELFS_AINT               MPI_AINT
#define TUNNELFS_OFFSET             ADIO_OFFSET

/* global master io server - defined in ad_tunnelfs.c */
extern int TUNNELFS_GLOBAL_MASTER;

/* maximum message size =  P4_MAX_MSGLEN */
/* this is the maximum size the p4 device can 
 * handle, so we have to split larger buffers 
 * into smaller parts.
 */
/*#define TUNNELFS_MAX_MSG_SIZE       (1<<28)*/
/*#define TUNNELFS_MAX_MSG_SIZE       16777216*/
#define TUNNELFS_MAX_MSG_SIZE       4194304

#define TUNNELFS_NUM_NAMED_TYPES    33

#define TUNNELFS_MAX_FS_DOMAINLEN   256
#define TUNNELFS_MAX_STRLEN         256

/* possible filetype flavors */
#define TUNNELFS_FILETYPE_JOINT     0
#define TUNNELFS_FILETYPE_DISJOINT  1
#define TUNNELFS_FILETYPE_SEMIJOINT 2

/* server behaviour for file handles */
#define TUNNELFS_DIRECT             1
#define TUNNELFS_ROUTE              2
#define TUNNELFS_CACHE              3

#define TUNNELFS_DIST_NONE          0
#define TUNNELFS_DIST_MEMFS         1
#define TUNNELFS_DIST_LOCAL         2
#define TUNNELFS_DIST_BALANCED      3
#define TUNNELFS_DIST_DOMAIN        4

#endif
