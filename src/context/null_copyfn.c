/*
 *  $Id: null_copyfn.c,v 1.1 2000/04/12 16:22:11 joachim Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*D
  
MPI_NULL_COPY_FN - A function to not copy attributes  

D*/
int MPIR_null_copy_fn ( 
	MPI_Comm comm, 
	int keyval, 
	void *extra_state, 
	void *attr_in, 
	void *attr_out, 
	int *flag )
{
  /* No error checking at present */

  /* Set attr_out, the flag and return success */
  (*flag) = 0;
  return (MPI_SUCCESS);
}
