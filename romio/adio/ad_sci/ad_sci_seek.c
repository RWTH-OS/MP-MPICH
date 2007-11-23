/* 
 *   $Id: ad_sci_seek.c 538 2001-01-03 17:34:54Z joachim $    
 *
 *   
 *   
 */

#include "ad_sci.h"

ADIO_Offset ADIOI_SCI_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
