/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_CREATE = PMPI_INFO_CREATE
EXPORT_MPI_API void MPI_INFO_CREATE (MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_create__ = pmpi_info_create__
EXPORT_MPI_API void mpi_info_create__ (MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_create = pmpi_info_create
EXPORT_MPI_API void mpi_info_create (MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_info_create_ = pmpi_info_create_
EXPORT_MPI_API void mpi_info_create_ (MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_CREATE (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_INFO_CREATE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_create__ (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_create__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_create (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_create")));
#else
EXPORT_MPI_API void mpi_info_create_ (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_create_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_CREATE  MPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_create__  mpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_create  mpi_info_create
#else
#pragma _HP_SECONDARY_DEF pmpi_info_create_  mpi_info_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_CREATE as PMPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_create__ as pmpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_create as pmpi_info_create
#else
#pragma _CRI duplicate mpi_info_create_ as pmpi_info_create_
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
#define mpi_info_create_ PMPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_create_ pmpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_create_ pmpi_info_create
#else
#define mpi_info_create_ pmpi_info_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_create_ MPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_create_ mpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_create_ mpi_info_create
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_create_ ANSI_ARGS((MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_create_(MPI_Fint *info, MPI_Fint *__ierr )
{
    MPI_Info info_c;

    *__ierr = MPI_Info_create(&info_c);
    *info = MPI_Info_c2f(info_c);
}
