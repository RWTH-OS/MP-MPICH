/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_main.h                                           * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 

#ifndef AD_MEMFS_MAIN_H
#define AD_MEMFS_MAIN_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ad_memfs.h"
#include "ad_memfs_service.h"
#include "adio_extern.h"
#include "ad_memfs_functions.h"

/* Include for memory debugger DMALLOC */
#if 0
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#endif


typedef struct
{
  int client;
  int fileserver;
} dist_list_t;


void memfs_get_server_for_client(int mode, int file_id, int client_rank, MPI_Offset offset, int count, int *server_rank);
void memfs_get_fileserver_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list);
int memfs_globalmaster_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list);
int memfs_roundrobin_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list);
int wait_for_reply();
void *memfs_main_loop();
int memfs_init_threads();
int memfs_destroy_threads();
int get_reply(MPI_Status *msg_status, int *msg_size, int tag);

#endif 





