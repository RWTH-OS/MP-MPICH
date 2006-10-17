/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: iowaitsome.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $    
 *
 *   Copyright (C) 2003 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPIO_Waitsome = PMPIO_Waitsome
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPIO_Waitsome MPIO_Waitsome
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPIO_Waitsome as PMPIO_Waitsome
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/*
  This is a temporary function until we switch to using MPI-2's generalized
  requests.
*/

int MPIO_Waitsome(int count, MPIO_Request requests[], int *outcount,
		  int indices[], MPI_Status *statuses)
{
    int i, flag, err; 

    if (count == 1) {
	err = MPIO_Wait( requests, statuses );
	if (!err) {
	    *outcount = 1;
	    indices[0] = 0;
	}
	return err;
    }

    /* Check for no active requests */
    for (i=0; i<count; i++) {
	if (requests[i] != MPIO_REQUEST_NULL) {
	    break;
	}
    }
    if (i == count) {
	*outcount = MPI_UNDEFINED;
	return MPI_SUCCESS;
    }

    err = MPI_SUCCESS;
    *outcount = 0;
    do {
	for (i=0; i<count; i++) {
	    if (requests[i] != MPIO_REQUEST_NULL) {
		err = MPIO_Test( &requests[i], &flag, statuses );
		if (flag) {
		    if (!err) {
			indices[0] = i;
			indices++;
			statuses++;
			*outcount = *outcount + 1;
		    }
		}
	    }
	}
    } while (*outcount == 0);

    return err;
}
