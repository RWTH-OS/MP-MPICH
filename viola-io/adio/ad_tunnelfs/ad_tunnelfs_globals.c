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
* File:         ad_tunnelfs_globals.c                                     * 
* Description:  Let global variables point to needed varibales to keep    *
*               the prototype of variable handling functions steady       *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "ad_tunnelfs_globals.h"
#include "adio.h"

/**
 * Structure for storing intermediary values
 */
typedef struct
{
    ADIO_File *fd;              /**< active file descriptor */
    int io_blocks;              /**< number of io blocks */
    ADIO_Offset shared_fp;      /**< value of shared file pointer */
    ADIO_Offset filesize;       /**< filesize */
    dist_list_t *fileserver;    /**< Reference to distribution */
    int fileserver_size;        /**< Size of distribution list */
}
tunnelfs_globals_struct;

/**
 * Global structure for tunnelfs intermediary values
 */
tunnelfs_globals_struct tunnelfs_globals;

/**
 * Set active file descriptor
 * @param fd ADIO file descriptor
 */
void tunnelfs_globals_set_active_fd(ADIO_File *fd)
{
    tunnelfs_globals.fd = fd;
}

/**
 * Get active file descriptor
 * @return active ADIO file descriptor
 */
ADIO_File tunnelfs_globals_get_active_fd()
{
    return *tunnelfs_globals.fd;
}

/**
 * Set number of io blocks
 * @param n number of io blocks
 */
void tunnelfs_globals_set_io_blocks(int n)
{
    tunnelfs_globals.io_blocks = n;
}

/**
 * Get number of io blocks
 * @return number of io blocks
 */
int tunnelfs_globals_get_io_blocks()
{
    return tunnelfs_globals.io_blocks;
}

/**
 * Set value of shared file pointer
 * @param off value of shared file pointer
 */
void tunnelfs_globals_set_shared_fp(ADIO_Offset off)
{
    tunnelfs_globals.shared_fp = off;
}

/**
 * Get value of shared file pointer
 * @return off value of shared file pointer
 */
ADIO_Offset tunnelfs_globals_get_shared_fp()
{
    return tunnelfs_globals.shared_fp;
}

/**
 * Set intermediary filesize
 * @param filesize Filesize
 */
void tunnelfs_globals_set_filesize(ADIO_Offset filesize)
{
    tunnelfs_globals.filesize = filesize;
}

/**
 * Get intermediary filesize
 * @return Filesize
 */
ADIO_Offset tunnelfs_globals_get_filesize()
{
    return tunnelfs_globals.filesize;
}

/**
 * Set distribution list
 * @param list Reference to distribution list
 * @param size Size of distribution list
 */
void tunnelfs_globals_set_distlist(dist_list_t *list, int size)
{
    tunnelfs_globals.fileserver = list;
    tunnelfs_globals.fileserver_size = size;
}

/**
 * Get distribution list
 * @param list Reference to distribution list pointer
 * @param size Reference to size variable
 */
void tunnelfs_globals_get_distlist(dist_list_t **list, int *size)
{
    *list = tunnelfs_globals.fileserver;
    *size = tunnelfs_globals.fileserver_size;
}
