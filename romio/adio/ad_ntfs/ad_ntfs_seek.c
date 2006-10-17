/* 
 *   $Id: ad_ntfs_seek.c,v 1.2 2000/04/13 13:45:32 karsten Exp $    
 *
 */

#include "ad_ntfs.h"

ADIO_Offset ADIOI_NTFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
