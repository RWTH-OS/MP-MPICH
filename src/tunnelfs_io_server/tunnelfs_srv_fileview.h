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
* File:         tunnelfs_fileview.h                                       * 
* Description:  Handling the file view set by clients                     * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_FILEVIEW_H
#define TUNNELFS_SRV_FILEVIEW_H

#include "mpi.h"

/**
 * Structure conatining the MPI Fileview parameters
 */
typedef struct
{
    MPI_Offset disp;    /**< Displacement */
    MPI_Datatype etype; /**< Elementary Type */
    MPI_Datatype ftype; /**< Filetype */
} tunnelfs_fileview_t;

/**
 * Get index of file view in internal list
 * @param client Rank of client
 * @param file_id Internal file id
 */
void tunnelfs_srv_fileview_set(int client, int file_id, MPI_Offset disp,
                               MPI_Datatype etype, MPI_Datatype filetype);

/**
 * Save fileview for a client on a specific file
 * @param client Rank of client
 * @param file_id Internal file id
 * @param disp Displacement of MPI file view
 * @param etype Elementary type of MPI file view
 * @param filetype Filetype of MPI file view
 */
void tunnelfs_srv_fileview_get(int client, int file_id, MPI_Offset *disp,
                               MPI_Datatype *etype, MPI_Datatype *filetype);

/**
 * Get saved fileview for a client on a specific file
 * @param client Rank of client
 * @param file_id Internal file id
 * @param disp Reference on displacement of MPI file view
 * @param etype Reference on elementary type of MPI file view
 * @param filetype Reference on filetype of MPI file view
 */
void tunnelfs_srv_fileview_get_etype(int client, int file_id,
                                     MPI_Datatype *etype);

/**
 * Get elemantary type of mpi file view
 * @param client Rank of client
 * @param file_id Internal file id
 * @param filetype Reference on datatype variable
 */
void tunnelfs_srv_fileview_get_filetype(int client, int file_id,
                                        MPI_Datatype *filetype);
#endif
