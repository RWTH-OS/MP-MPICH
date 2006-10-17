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
                                    int *error_code)
{
    ADIO_Offset sfp_off = 0;
    int i;
    int size;
    int my_rank;

    if (file_ptr_type == ADIO_SHARED)
    {
        /* get comm size and rank */
        MPI_Comm_size(fd->comm, &size);
        MPI_Comm_rank(fd->comm, &my_rank);

#ifdef OLD_ORDERED_READ_HANDLING
        /* wait for others */
        MPI_Barrier(fd->comm);

        for (i = 0; i < size; i++)
        {
            if (my_rank == i)
                ADIOI_TUNNELFS_ReadStrided(fd, buf, count, datatype,
                                           file_ptr_type, offset, status,
                                           error_code);
            MPI_Barrier(fd->comm);
        }
#else
        if (my_rank == 0)
            MPI_File_get_position_shared(fd, &sfp_off);

        /* Broadcast is not sychronized, thus worst case we have to wait for
         * proc 0 only and not for all procs to be in sync */
        MPI_Bcast(&offset, 1, TUNNELFS_OFFSET, 0, fd->comm);

        /* calculate individual offset */
        sfp_off += my_rank * count;

        ADIOI_TUNNELFS_ReadStrided(fd, buf, count, datatype,
                                   ADIO_EXPLICIT_OFFSET, sfp_off, status,
                                   error_code);

#endif
    }
    else
        ADIOI_TUNNELFS_ReadStrided(fd, buf, count, datatype, file_ptr_type,
                                   offset, status, error_code);
}
