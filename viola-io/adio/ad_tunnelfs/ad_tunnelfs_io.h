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
* File:         ad_tunnelfs_io.h                                          * 
* Description:  I/O Request handling                                      * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_IO_H
#define AD_TUNNELFS_IO_H

#include "ad_tunnelfs.h"

/**
 * Return file id for a given MPI file handle
 * @param fd MPI file handle
 * @return internal file id
 */
int tunnelfs_file_get_id(ADIO_File fd);

/**
 * Create an io request
 * @param tunnelfs_req_type Type of request (read/write)
 * @param fd MPI file handle
 * @param buf MPI message buffer
 * @param count number of elements
 * @param datatype MPI Datatype describing the elements
 * @param file_ptr_type MPI File pointer type
 * @param offset Offset for io operation
 * @param status Reference to MPI Status structure
 * @param error_code Reference to MPI error_code
 * @param msg_id Message id to use for request
 * @return Number of io blocks for request
 */
int tunnelfs_ioreq(int tunnelfs_req_type, ADIO_File fd, void *buf, int count,
                   MPI_Datatype datatype, int file_ptr_type,
                   ADIO_Offset offset, ADIO_Status *status, int *error_code,
                   int *msg_id);

#endif
