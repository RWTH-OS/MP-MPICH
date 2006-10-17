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
* File:                                                                   * 
* Description:                                                            * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"
#include "adioi.h"

/**
 * Wait for completion of read request
 * @param request Pointer to MPI request structure of request to be completed
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_ReadComplete(ADIO_Request *request, ADIO_Status *status,
                                 int *error_code)
{
    if (*request == ADIO_REQUEST_NULL)
    {
        *error_code = MPI_SUCCESS;
        return;
    }

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, (*request)->datatype, (*request)->nbytes);
#endif
    (*request)->fd->async_count--;
    ADIOI_Free_request((ADIOI_Req_node *) (*request));
    *request = ADIO_REQUEST_NULL;
    *error_code = MPI_SUCCESS;
}

/**
 * Wait for completion of write request
 * @param request Pointer to MPI request structure of request to be completed
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteComplete(ADIO_Request *request,
                                  ADIO_Status *status, int *error_code)
{
    if (*request == ADIO_REQUEST_NULL)
    {
        *error_code = MPI_SUCCESS;
        return;
    }

#ifdef HAVE_STATUS_SET_BYTES
    MPIR_Status_set_bytes(status, (*request)->datatype, (*request)->nbytes);
#endif
    (*request)->fd->async_count--;
    ADIOI_Free_request((ADIOI_Req_node *) (*request));
    *request = ADIO_REQUEST_NULL;
    *error_code = MPI_SUCCESS;
}
