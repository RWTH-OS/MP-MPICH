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
* File:         ad_tunnelfs_globals.h                                     * 
* Description:  Let global variables point to needed varibales to keep    *
*               the prototype of variable handling functions steady       *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_GLOBALS_H
#define AD_TUNNELFS_GLOBALS_H

#include "../include/adio.h"
#include "ad_memfs_main.h"

/**
 * Set active file descriptor
 * @param fd ADIO file descriptor
 */
void tunnelfs_globals_set_active_fd(ADIO_File *fd);

/**
 * Get active file descriptor
 * @return active ADIO file descriptor
 */
ADIO_File tunnelfs_globals_get_active_fd();

/**
 * Set number of io blocks
 * @param n number of io blocks
 */
void tunnelfs_globals_set_io_blocks(int n);

/**
 * Get number of io blocks
 * @return number of io blocks
 */
int tunnelfs_globals_get_io_blocks();

/**
 * Set value of shared file pointer
 * @param off value of shared file pointer
 */
void tunnelfs_globals_set_shared_fp(ADIO_Offset off);

/**
 * Get value of shared file pointer
 * @return value of shared file pointer
 */
ADIO_Offset tunnelfs_globals_get_shared_fp();

/**
 * Set intermediary filesize
 * @param filesize Filesize
 */
void tunnelfs_globals_set_filesize(ADIO_Offset filesize);

/**
 * Get intermediary filesize
 * @return Filesize
 */
ADIO_Offset tunnelfs_globals_get_filesize();

/**
 * Set distribution list
 * @param list Reference to distribution list
 * @param size Size of distribution list
 */
void tunnelfs_globals_set_distlist(dist_list_t *list, int size);

/**
 * Get distribution list
 * @param list Reference to distribution list pointer
 * @param size Reference to size variable
 */
void tunnelfs_globals_get_distlist(dist_list_t **list, int *size);
#endif
