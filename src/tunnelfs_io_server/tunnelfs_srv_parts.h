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
* File:         tunnelfs_srv_parts.h                                      * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  handle distribution of partial files                      *
*                                                                         *
**************************************************************************/
#ifndef _TUNNELFS_SRV_PARTS_H
#define _TUNNELFS_SRV_PARTS_H

int tunnelfs_srv_parts_search(int file_id, int client);
int tunnelfs_srv_parts_find_any_client(int file_id);

void tunnelfs_srv_parts_create(int file_id, int client, int server);
void tunnelfs_srv_parts_destroy(int file_id, int client);

int tunnelfs_srv_parts_get_server(int file_id, int client);
int tunnelfs_srv_parts_is_modified(int file_id, int client);

MPI_File *tunnelfs_srv_parts_get_handle(int file_id, int client);

#endif
