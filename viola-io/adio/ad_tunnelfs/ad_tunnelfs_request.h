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
* File:         ad_tunnelfs_request.h                                     * 
* Description:  Request handling between IO and COMM Requests             *
*               (in MPICH IO-Requests and COMM-Requests for send/recv     *
*               actually differ!)                                         *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_REQUEST_H
#define AD_TUNNELFS_REQUEST_H

#include "mpi.h"

int tunnelfs_request_create(MPIO_Request *request);
void tunnelfs_request_delete(MPIO_Request *request);
void tunnelfs_request_attach(int idx, MPI_Request *request);
void tunnelfs_request_detach(int idx, MPI_Request *request);

int tunnelfs_request_get_idx(MPIO_Request *request);
void *tunnelfs_request_get_buf(int idx);
int tunnelfs_request_get_num_blocks(int idx);
int tunnelfs_request_get_pending(int idx);
int tunnelfs_request_get_source(int idx);
int tunnelfs_request_get_tag(int idx);
MPI_Comm tunnelfs_request_get_comm(int idx);

void tunnelfs_request_set_buf(int idx, void *buf);
void tunnelfs_request_set_num_blocks(int idx, int num);
void tunnelfs_request_set_source(int idx, int source);
void tunnelfs_request_set_tag(int idx, int tag);
void tunnelfs_request_set_comm(int idx, MPI_Comm comm);

#endif
