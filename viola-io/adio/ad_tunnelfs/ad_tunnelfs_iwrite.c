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
 * Writing contigous elements to a file with MPI immediate return
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param request Pointer to MPI request structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_IwriteContig(ADIO_File fd, void *buf, int count,
                                 MPI_Datatype datatype, int file_ptr_type,
                                 ADIO_Offset offset, ADIO_Request *request,
                                 int *error_code)
{
    /* asynchronous write is not supported yet, so let's call the synchronous
     * write call */

    int typesize;
    ADIO_Status status;

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_WRITE;
    (*request)->fd = fd;
    (*request)->datatype = datatype;

    MPI_Type_size(datatype, &typesize);


    ADIOI_TUNNELFS_WriteContig(fd, buf, count, datatype, file_ptr_type,
                               offset, &status, error_code);
    (*request)->queued = 0;

#ifdef HAVE_STATUS_SET_BYTES
    if (*error_code == MPI_SUCCESS)
    {
        MPI_Get_elements(&status, MPI_BYTE, &len);
        (*request)->nbytes = len;
    }
#endif

    fd->fp_sys_posn = -1;       /* set it to null. */
    fd->async_count++;
}

/**
 * Writing non-contigous elements split-collectively to a file, like in
 *  - MPI_File_write_coll_start/end
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param request Pointer to MPI request structure for further request handling
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_IwriteStrided(ADIO_File fd, void *buf, int count,
                                  MPI_Datatype datatype, int file_ptr_type,
                                  ADIO_Offset offset, ADIO_Request *request,
                                  int *error_code)
{
    ADIOI_TUNNELFS_IwriteContig(fd, buf, count, datatype, file_ptr_type,
                                offset, request, error_code);
}
