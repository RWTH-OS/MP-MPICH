/* $Id$ */

#ifndef _MPI_TYPE_FF_H
#define _MPI_TYPE_FF_H

#include "mpiimpl.h"
#include "mpipt2pt.h"

/* Creation and Deletion of direct_ff datatype representation. */

/* Build the FF stack for a given datatype */
int MPIR_build_ff (struct MPIR_DATATYPE *type, int *leaves);

/* Remove the FF stack from the datatype */
int MPIR_free_ff (struct MPIR_DATATYPE *type);

/* Get to know the depth of a datatype. */
int MPIR_get_dt_depth (struct MPIR_DATATYPE *dtype_ptr);

#endif

