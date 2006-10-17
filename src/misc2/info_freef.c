/* 
 *   $Id: info_freef.c,v 1.3 2003/06/13 09:39:37 rainer Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_FREE = PMPI_INFO_FREE
EXPORT_MPI_API void MPI_INFO_FREE (MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_free__ = pmpi_info_free__
EXPORT_MPI_API void mpi_info_free__ (MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_free = pmpi_info_free
EXPORT_MPI_API void mpi_info_free (MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_info_free_ = pmpi_info_free_
EXPORT_MPI_API void mpi_info_free_ (MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_FREE (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_INFO_FREE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_free__ (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_free__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_free (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_free")));
#else
EXPORT_MPI_API void mpi_info_free_ (MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_free_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_FREE  MPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_free__  mpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_free  mpi_info_free
#else
#pragma _HP_SECONDARY_DEF pmpi_info_free_  mpi_info_free_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_FREE as PMPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_free__ as pmpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_free as pmpi_info_free
#else
#pragma _CRI duplicate mpi_info_free_ as pmpi_info_free_
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
#define mpi_info_free_ PMPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_free_ pmpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_free_ pmpi_info_free
#else
#define mpi_info_free_ pmpi_info_free_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_free_ MPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_free_ mpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_free_ mpi_info_free
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_free_ ANSI_ARGS((MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_free_(MPI_Fint *info, MPI_Fint *__ierr )
{
    MPI_Info info_c;

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_free(&info_c);
    *info = MPI_Info_c2f(info_c);
}

