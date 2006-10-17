/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: iowaitany.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $    
 *
 *   Copyright (C) 2003 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPIO_Waitany = PMPIO_Waitany
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPIO_Waitany MPIO_Waitany
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPIO_Waitany as PMPIO_Waitany
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

int MPIO_Waitany(int count, MPIO_Request requests[], int *index, 
		 MPI_Status *status)
{
    int i, flag, err; 

    if (count == 1) {
	err = MPIO_Wait( requests, status );
	if (!err) *index = 0;
	return err;
    }

    /* Check for no active requests */
    for (i=0; i<count; i++) {
	if (requests[i] != MPIO_REQUEST_NULL) {
	    break;
	}
    }
    if (i == count) {
	*index = MPI_UNDEFINED;
#ifdef MPICH2
	/* need to set empty status */
	if (status != MPI_STATUS_IGNORE) {
	    status->MPI_SOURCE = MPI_ANY_SOURCE;
	    status->MPI_TAG    = MPI_ANY_TAG;
	    status->count      = 0;
	    status->cancelled  = 0;
	}
#endif
	return MPI_SUCCESS;
    }

    err = MPI_SUCCESS;
    do {
	flag = 0;
	for (i=0; i<count; i++) {
	    if (requests[i] != MPIO_REQUEST_NULL) {
		err = MPIO_Test( &requests[i], &flag, status );
		if (flag) {
		    if (!err) *index = i;
		    break;
		}
	    }
	}
    } while (flag == 0);

    return err;
}
