/*
 *  $Id: getversionf.c,v 1.3 2003/06/13 09:39:36 rainer Exp $
 *
 *  (C) 1997 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GET_VERSION = PMPI_GET_VERSION
EXPORT_MPI_API void MPI_GET_VERSION ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_get_version__ = pmpi_get_version__
EXPORT_MPI_API void mpi_get_version__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_get_version = pmpi_get_version
EXPORT_MPI_API void mpi_get_version ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_get_version_ = pmpi_get_version_
EXPORT_MPI_API void mpi_get_version_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GET_VERSION ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GET_VERSION")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_get_version__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_version__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_get_version ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_version")));
#else
EXPORT_MPI_API void mpi_get_version_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_version_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GET_VERSION  MPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_version__  mpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_version  mpi_get_version
#else
#pragma _HP_SECONDARY_DEF pmpi_get_version_  mpi_get_version_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GET_VERSION as PMPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_get_version__ as pmpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_get_version as pmpi_get_version
#else
#pragma _CRI duplicate mpi_get_version_ as pmpi_get_version_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#ifdef FORTRANCAPS
#define mpi_get_version_ PMPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_version_ pmpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_version_ pmpi_get_version
#else
#define mpi_get_version_ pmpi_get_version_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_get_version_ MPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_version_ mpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_version_ mpi_get_version
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_get_version_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_get_version_( MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr )
{
    *version    = MPI_VERSION;
    *subversion = MPI_SUBVERSION;
    *ierr       = MPI_SUCCESS;
}
