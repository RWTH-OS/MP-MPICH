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
* File:         ad_tunnelfs.h                                             * 
* Description:                                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_H
#define AD_TUNNELFS_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "../include/adio.h"
#include "ad_tunnelfs_comm.h"
#include "ad_tunnelfs_common.h"
#include "ad_tunnelfs_datatype.h"
#include "ad_tunnelfs_err.h"
#include "ad_tunnelfs_globals.h"
#include "ad_tunnelfs_io.h"
#include "ad_tunnelfs_msg.h"
#include "ad_tunnelfs_request.h"
#include "ad_tunnelfs_server.h"

/* Definition of tunnelfs specific variables and macros */

/**
 * Opening a tunnelfs controled file
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Open(ADIO_File fd, int *error_code);
/**
 * Closing a tunnelfs controled file
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Close(ADIO_File fd, int *error_code);
/**
 * Reading contigous elements from a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_ReadContig(ADIO_File fd, void *buf, int count,
                               MPI_Datatype datatype, int file_ptr_type,
                               ADIO_Offset offset, ADIO_Status *status,
                               int *error_code);
/**
 * Writing contigous elements to a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteContig(ADIO_File fd, void *buf, int count,
                                MPI_Datatype datatype, int file_ptr_type,
                                ADIO_Offset offset, ADIO_Status *status, int
                                *error_code);
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
                                 int *error_code);
/**
 * Reading contigous elements from a file using MPI immediate return
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
void ADIOI_TUNNELFS_IreadContig(ADIO_File fd, void *buf, int count,
                                MPI_Datatype datatype, int file_ptr_type,
                                ADIO_Offset offset, ADIO_Request *request,
                                int *error_code);
/**
 * Test for read request completion
 * @param request Pointer to MPI request structure of request to be tested
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 * @return 
 *      - 1 if request is finished
 *      - 0 if request is not yet finished
 */
int ADIOI_TUNNELFS_ReadDone(ADIO_Request *request, ADIO_Status *status,
                            int *error_code);
/**
 * Test for write request completion
 * @param request Pointer to MPI request structure of request to be tested
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 * @return 
 *      - 1 if request is finished
 *      - 0 if request is not yet finished
 */
int ADIOI_TUNNELFS_WriteDone(ADIO_Request *request, ADIO_Status *status,
                             int *error_code);
/**
 * Wait for completion of read request
 * @param request Pointer to MPI request structure of request to be completed
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_ReadComplete(ADIO_Request *request, ADIO_Status *status,
                                 int *error_code);
/**
 * Wait for completion of write request
 * @param request Pointer to MPI request structure of request to be completed
 * @param status Pointer to MPI status structure for results
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteComplete(ADIO_Request *request,
                                  ADIO_Status *status, int *error_code);
/**
 * Function to execute several smaller client functions like
 *      - Preallocating a file
 *      - Resizing a file
 *      - Setting atomicity flag on a file
 *      - Setting io mode on a file
 * @param fd MPI file descriptor
 * @param flag flag indicating operation to be performed
 * @param fcntl_struct ADIO internal structure for in- and output of parameters
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct,
                          int *error_code);
/**
 * Writing non-contigous elements to a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteStrided(ADIO_File fd, void *buf, int count,
                                 MPI_Datatype datatype, int file_ptr_type,
                                 ADIO_Offset offset, ADIO_Status *status,
                                 int *error_code);
/**
 * Reading non-contigous elements from a file 
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_ReadStrided(ADIO_File fd, void *buf, int count,
                                MPI_Datatype datatype, int file_ptr_type,
                                ADIO_Offset offset, ADIO_Status *status,
                                int *error_code);
/**
 * Writing non-contigous elements collectively to a file, like in
 *  - MPI_File_write_coll
 *  - MPI_File_write_ordered
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                                     MPI_Datatype datatype, int file_ptr_type,
                                     ADIO_Offset offset, ADIO_Status *status,
                                     int *error_code);
/**
 * Reading non-contigous elements collectivly from a file, like in
 *  - MPI_File_write_coll
 *  - MPI_File_write_ordered
 * @param fd MPI File descriptor
 * @param buf pointer to read buffer
 * @param count number of elements to be read
 * @param datatype MPI datatype for placement in memory
 * @param file_ptr_type Type of access
 *          - ADIO_INDIVIDUAL, individual file pointer
 *          - ADIO_SHARED, shared file pointer
 *          - ADIO_EXPLICIT_OFFSET, explicit offset not using file pointers
 * @param offset MPI offset in elements of etype (as defined in file view)
 * @param status Pointer to MPI status structure
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
                                    MPI_Datatype datatype, int file_ptr_type,
                                    ADIO_Offset offset, ADIO_Status *status,
                                    int *error_code);
/**
 * Reading non-contigous elements split-collectivly from a file, like in
 *  - MPI_File_read_coll_start/end
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
void ADIOI_TUNNELFS_IreadStrided(ADIO_File fd, void *buf, int count,
                                 MPI_Datatype datatype, int file_ptr_type,
                                 ADIO_Offset offset, ADIO_Request *request,
                                 int *error_code);
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
                                  int *error_code);
/**
 * Send flush request to tunnelfs server
 * @param fd MPI File descriptor
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Flush(ADIO_File fd, int *error_code);
/**
 * Send Resize request to tunnelfs server
 * @param fd MPI File descriptor
 * @param size new file size
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code);
/**
 * Correlating MPI Info object with MPI file
 * @param fd MPI File descriptor
 * @param users_info User provided MPI info object
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_SetInfo(ADIO_File fd, MPI_Info users_info,
                            int *error_code);
/**
 * Performing seek with shared file pointer
 * @param fd MPI File descriptor
 * @param size Dummy value not used with tunnelfs
 * @param shared_fp Pointer to variable to hold the filepointer position
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Get_shared_fp(ADIO_File fd, int size,
                                  ADIO_Offset *shared_fp, int *error_code);
/**
 * Setting shared file pointer on tunnelfs server
 * @param fd MPI File descriptor
 * @param offset Seek offset 
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset,
                                  int *error_code);
/**
 * Deleting a tunnelfs controlled file
 * @param filename string identifier for filename
 * @param error_code pointer to variable to return MPI error code
 */
void ADIOI_TUNNELFS_Delete(char *filename, int *error_code);

#endif
