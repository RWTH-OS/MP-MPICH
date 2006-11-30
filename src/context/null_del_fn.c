/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*D
  
MPI_NULL_DELETE_FN - A function to not delete attributes  

Input Parameters:
. comm - Communicator
. keyval - Key value
. attr   - attribute
. extra_state - User-defined state to give user functions

Notes:
See discussion of MPI_Keyval_create for the use of this function.

D*/
int MPIR_null_delete_fn ( 
	MPI_Comm comm, 
	int keyval, 
	void *attr, 
	void *extra_state )
{
  return (MPI_SUCCESS);
}
