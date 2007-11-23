/* 
 *   $Id: ad_ntfs_wrcoll.c 24 2000-04-13 13:45:34Z karsten $    
 *
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_WriteStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
