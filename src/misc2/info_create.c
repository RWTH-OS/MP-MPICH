/* 
 *   $Id: info_create.c,v 1.2 2003/06/05 11:50:35 rainer Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Info_create = PMPI_Info_create
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Info_create(MPI_Info *info) __attribute__ ((weak, alias ("PMPI_Info_create")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Info_create  MPI_Info_create
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Info_create as PMPI_Info_create
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
    MPI_Info_create - Creates a new info object

Output Parameters:
. info - info object (handle)

.N fortran
@*/
EXPORT_MPI_API int MPI_Info_create(MPI_Info *info)
{
    *info	    = (MPI_Info) MALLOC(sizeof(struct MPIR_Info));
    (*info)->cookie = MPIR_INFO_COOKIE;
    (*info)->key    = 0;
    (*info)->value  = 0;
    (*info)->next   = 0;
    /* this is the first structure in this linked list. it is 
       always kept empty. new (key,value) pairs are added after it. */

    return MPI_SUCCESS;
}
