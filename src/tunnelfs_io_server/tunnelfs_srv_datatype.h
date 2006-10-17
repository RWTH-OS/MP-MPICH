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
* File:         tunnelfs_datatype.h                                       * 
* Description:  Handling of client side datatypes                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef TUNNELFS_SRV_DATATYPE_H
#define TUNNELFS_SRV_DATATYPE_H

#include "mpi.h"

/** 
 * Create entry in datatype structure and return reference to MPI datatype
 * handle for subsequent MPI_Type_create call
 * @param client Rank of client that define the type
 * @param type_id Internal id for datatype
 * @return Reference to MPI Datatype handle. 
 */
MPI_Datatype *tunnelfs_srv_datatype_prepare(int client, int type_id);

/** 
 * Calculate parameters to avoid MPI locking for obtaining them later
 * @param dtype Handle of committed MPI datatype
 */
void tunnelfs_srv_datatype_calc_param(MPI_Datatype dtype);

/** 
 * Get MPI datatype for a given client and type id
 * @param client Rank of client that define the type
 * @param type_id Internal id for datatype
 * @param mpi_type Reference to MPI Datatype handle. 
 */
void tunnelfs_srv_datatype_get_type(int client, int type_id,
                                    MPI_Datatype *mpi_type);

/**
 * Get size of a defined datatype
 * @param dtype Handle of committed MPI datatype
 * @param type_size Reference to size variable. 
 */
void tunnelfs_srv_datatype_get_size(MPI_Datatype dtype, int *type_size);
#endif
