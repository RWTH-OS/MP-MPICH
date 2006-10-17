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
 * Test for read request completion
 * @param request Pointer to MPI request structure of request to be tested
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 * @return 
 *      - 1 if request is finished
 *      - 0 if request is not yet finished
 */
int ADIOI_TUNNELFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int
                            *error_code)
{
    return 1;
}

/**
 * Test for write request completion
 * @param request Pointer to MPI request structure of request to be tested
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 * @return 
 *      - 1 if request is finished
 *      - 0 if request is not yet finished
 */
int ADIOI_TUNNELFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int
                             *error_code)
{
    return 1;
}
