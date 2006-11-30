/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
/* Update log
 *
 * Jun 18 1997 jcownie@dolphinics.com: They changed the calling convention when I wasn't
 *             looking ! Do what the Forum says...
 * Nov 28 1996 jcownie@dolphinics.com: Implement MPI-2 communicator naming function.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*+

MPI_Comm_get_name - return the print name from the communicator

+*/
EXPORT_MPI_API int MPI_Comm_get_name( MPI_Comm comm, char *namep, int *reslen )
{
  struct MPIR_COMMUNICATOR *comm_ptr = MPIR_GET_COMM_PTR(comm);
  static char myname[] = "MPI_COMM_GET_NAME";
  int mpi_errno;
  char *nm;

  TR_PUSH(myname);

  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if (comm_ptr->comm_name)
    nm =  comm_ptr->comm_name;
  else
    nm = "";		/* The standard says null string... */

  /* The user better have allocated the result big enough ! */
  strncpy (namep, nm, MPI_MAX_NAME_STRING);
  *reslen = strlen (nm);

  TR_POP;
  return MPI_SUCCESS;
}

