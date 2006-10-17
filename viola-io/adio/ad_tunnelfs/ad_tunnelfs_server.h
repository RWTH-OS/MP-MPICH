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
* File:         ad_tunnelfs_server.h                                      * 
* Description:  Server control for tunnelfs client side                   * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_SERVER_H
#define AD_TUNNELFS_SERVER_H

/**
 * determine the global master io server for a new request 
 * @return rank of global master tunnelfs server
 */
int tunnelfs_server_get_global_master();

/** 
 * determine the master io server for a given file_id 
 * @return rank of master tunnelfs server for the given file
 */
int tunnelfs_server_get_file_master(int file_id);

/**
 * set new file master io server for a client process 
 * @param file_id internal file id
 * @param master rank of master fileserver for this file
 */
void tunnelfs_server_set_file_master(int file_id, int master);

/**
 * Unset new file master io server for a client process 
 * @param file_id internal file id
 */
void tunnelfs_server_unset_file_master(int file_id);

#endif
