/*
 *  $Id: pcontrol.c,v 1.2 2003/06/05 11:50:36 rainer Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Pcontrol = PMPI_Pcontrol
#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(__STDC__) || defined(HAVE_PROTOTYPES) || defined(USE_STDARG)
#ifdef HAVE_NO_C_CONST
EXPORT_MPI_API int MPI_Pcontrol( int level, ... ) __attribute__ ((weak, alias ("PMPI_Pcontrol")));
#else
int MPI_Pcontrol( const int level, ... ) __attribute__ ((weak, alias ("PMPI_Pcontrol")));
#endif
#else
int MPI_Pcontrol( int level ) __attribute__ ((weak, alias ("PMPI_Pcontrol")));
#endif
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Pcontrol  MPI_Pcontrol
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Pcontrol as PMPI_Pcontrol
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#if defined(__STDC__) || defined(HAVE_PROTOTYPES) || defined(USE_STDARG)
#ifdef HAVE_NO_C_CONST
EXPORT_MPI_API int MPI_Pcontrol( int level, ... )
#else
int MPI_Pcontrol( const int level, ... )
#endif
{
    return MPI_SUCCESS;
}
#else
/*@
  MPI_Pcontrol - Controls profiling

  Input Parameters:
. level - Profiling level 

  Notes:
  This routine provides a common interface for profiling control.  The
  interpretation of 'level' and any other arguments is left to the
  profiling library.

.N fortran
@*/
int MPI_Pcontrol( int level )
{
    return MPI_SUCCESS;
}
#endif
