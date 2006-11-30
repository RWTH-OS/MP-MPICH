/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Get_processor_name = PMPI_Get_processor_name
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Get_processor_name(char *name,
				int *resultlen) __attribute__ ((weak, alias ("PMPI_Get_processor_name")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Get_processor_name  MPI_Get_processor_name
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Get_processor_name as PMPI_Get_processor_name
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

/*@C
  MPI_Get_processor_name - Gets the name of the processor

Output Parameters:
+ name - A unique specifier for the actual (as opposed to virtual) node. 
- resultlen - Length (in characters) of the name 

Notes:
The name returned should identify a particular piece of hardware; 
the exact format is implementation defined.  This name may or may not
be the same as might be returned by 'gethostname', 'uname', or 'sysinfo'.

.N fortran
@*/
EXPORT_MPI_API int MPI_Get_processor_name( 
	char *name,
	int *resultlen)
{
    MPID_Node_name( name, MPI_MAX_PROCESSOR_NAME );
    *resultlen = strlen(name);
    return MPI_SUCCESS;
}


#ifdef META
/* 
 * This function returns the name of the metahost. It is an addition to the MPI-standard
 * and should therefore not be used if possible
 */
#include "../routing/router_config.h"
extern MPIR_MetaConfig MPIR_meta_cfg;

EXPORT_MPI_API int MPI_META_Get_metahost_name(char *name, int *resultlen) 
{
  strcpy(name,MPIR_meta_cfg.my_metahostname);
  *resultlen=strlen(MPIR_meta_cfg.my_metahostname);
  return MPI_SUCCESS;
}

#endif
