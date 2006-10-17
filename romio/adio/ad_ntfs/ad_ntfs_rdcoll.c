/* 
 *   $Id: ad_ntfs_rdcoll.c,v 1.2 2000/04/13 13:45:29 karsten Exp $    
 *
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_ReadStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
