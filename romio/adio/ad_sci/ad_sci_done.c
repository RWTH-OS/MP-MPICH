/* 
 *   $Id: ad_sci_done.c,v 1.1 2001/01/03 17:34:53 joachim Exp $    
 */

#include "ad_sci.h"

int ADIOI_SCI_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
#ifndef NO_AIO
    int done=0;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_READDONE";
#endif
#ifdef AIO_SUN 
    aio_result_t *result=0, *tmp;
#else
    int err;
#endif
#ifdef AIO_HANDLE_IN_AIOCB
    struct aiocb *tmp1;
#endif
#endif

    if (*request == ADIO_REQUEST_NULL) {
	*error_code = MPI_SUCCESS;
	return 1;
    }

}


int ADIOI_SCI_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    return ADIOI_SCI_ReadDone(request, status, error_code);
} 
