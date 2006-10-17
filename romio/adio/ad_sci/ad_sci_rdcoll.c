/* 
 *   $Id: ad_sci_rdcoll.c,v 1.1 2001/01/03 17:34:53 joachim Exp $    
 *
 *   
 *   
 */

#include "ad_sci.h"

void ADIOI_SCI_ReadStridedColl(ADIO_File fd, void *buf, int count,
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Status *status, 
			       int *error_code)
{
    ADIOI_GEN_ReadStridedColl(fd, buf, count, datatype, file_ptr_type,
			      offset, status, error_code);
}
