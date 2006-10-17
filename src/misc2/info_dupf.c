/* 
 *   $Id: info_dupf.c,v 1.3 2003/06/13 09:39:37 rainer Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_DUP = PMPI_INFO_DUP
EXPORT_MPI_API void MPI_INFO_DUP (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_dup__ = pmpi_info_dup__
EXPORT_MPI_API void mpi_info_dup__ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_dup = pmpi_info_dup
EXPORT_MPI_API void mpi_info_dup (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_info_dup_ = pmpi_info_dup_
EXPORT_MPI_API void mpi_info_dup_ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_DUP (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_INFO_DUP")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_dup__ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_dup__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_dup (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_dup")));
#else
EXPORT_MPI_API void mpi_info_dup_ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_dup_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_DUP  MPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_dup__  mpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_dup  mpi_info_dup
#else
#pragma _HP_SECONDARY_DEF pmpi_info_dup_  mpi_info_dup_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_DUP as PMPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_dup__ as pmpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_dup as pmpi_info_dup
#else
#pragma _CRI duplicate mpi_info_dup_ as pmpi_info_dup_
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
#define mpi_info_dup_ PMPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_dup_ pmpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_dup_ pmpi_info_dup
#else
#define mpi_info_dup_ pmpi_info_dup_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_dup_ MPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_dup_ mpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_dup_ mpi_info_dup
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_dup_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_dup_(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *__ierr )
{
    MPI_Info info_c, newinfo_c;

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_dup(info_c, &newinfo_c);
    *newinfo = MPI_Info_c2f(newinfo_c);
}
