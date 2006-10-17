/* 
 *   $Id: ad_svm_seek.c,v 1.1 2000/04/19 17:42:08 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm.h"

ADIO_Offset ADIOI_SVM_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
