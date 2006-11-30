/* 
 *   $Id$    
 *
 * 
 * 
 */

#include "ad_sci.h"

void ADIOI_SCI_WriteStridedColl(ADIO_File fd, void *buf, int count,
				MPI_Datatype datatype, int file_ptr_type,
				ADIO_Offset offset, ADIO_Status *status, 
				int *error_code)
{
    ADIOI_GEN_WriteStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
