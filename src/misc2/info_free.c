/* 
 *   $Id: info_free.c,v 1.3 2003/06/05 11:50:35 rainer Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Info_free = PMPI_Info_free
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Info_free(MPI_Info *info) __attribute__ ((weak, alias ("PMPI_Info_free")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Info_free  MPI_Info_free
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Info_free as PMPI_Info_free
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif
#include "mpimem.h"

/*@
    MPI_Info_free - Frees an info object

Input Parameters:
. info - info object (handle)

.N fortran
@*/
EXPORT_MPI_API int MPI_Info_free(MPI_Info *info)
{
    MPI_Info curr, next;
    int mpi_errno;
    static char myname[] = "MPI_INFO_FREE";

    if ((!info) || (*info <= (MPI_Info) 0) || ((*info)->cookie != MPIR_INFO_COOKIE)) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO, MPIR_ERR_DEFAULT, myname, 
				     (char *)0, (char *)0 );
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
    }

    curr = (*info)->next;
    FREE(*info);
    *info = MPI_INFO_NULL;

    while (curr) {
	next = curr->next;
#ifdef free
/* By default, we define free as an illegal expression when doing memory
   checking; we need to undefine it to handle the fact that strdup does
   a naked malloc.
 */
#undef free
#endif
	free(curr->key);
	free(curr->value);
	FREE(curr);
	curr = next;
    }

    return MPI_SUCCESS;
}
