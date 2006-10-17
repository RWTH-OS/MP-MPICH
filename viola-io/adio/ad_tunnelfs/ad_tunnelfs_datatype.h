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
* File:         ad_tunnelfs_datatype.h                                    * 
* Description:                                                            *
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_DATATYPE_H
#define AD_TUNNELFS_DATATYPE_H

/**
 * Register an MPI datatype
 * @param dtype MPI Datatype handle
 * @return internal datatype handle
 */
int tunnelfs_datatype_register(MPI_Datatype dtype);

/**
 * Get internal datatype reference for a given MPI datatype
 * @param dtype MPI datatype handle
 * @param dtype_id Reference to internal datatype id
 */
void tunnelfs_datatype_get_id(MPI_Datatype dtype, int *dtype_id);

/**
 * Get MPI datatype for a given internal datatype id
 * @param dtype_id Internal datatype id
 * @param dtype Reference to MPI datatype handle
 */
void tunnelfs_datatype_get_type(int dtype_id, MPI_Datatype *dtype);

/**
 * Synchronize datatype with a tunnelfs server
 * @param type MPI datatype handle
 * @param server Rank of tunnelfs server
 * @return boolean inticating successful synchronization
 */
int tunnelfs_datatype_sync(MPI_Datatype type, int server);

#endif
