/* 
 *   $Id: ad_ntfs_hints.c,v 1.2 2000/04/13 13:45:27 karsten Exp $    
 *
 */

#include "ad_ntfs.h"

void ADIOI_NTFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    ADIOI_GEN_SetInfo(fd, users_info, error_code); 
}
