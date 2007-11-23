/* 
 *   $Id: status_setb.c 10 2000-04-12 19:09:10Z karsten $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifdef MPICH

#include "mpi.h"

int MPIR_Status_set_bytes(MPI_Status *status, MPI_Datatype datatype, 
			  int nbytes)
{
    status->count = nbytes;
    return MPI_SUCCESS;
}

#endif
