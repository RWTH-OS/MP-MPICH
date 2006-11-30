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
#pragma weak MPI_INFO_GET_NKEYS = PMPI_INFO_GET_NKEYS
EXPORT_MPI_API void MPI_INFO_GET_NKEYS (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_get_nkeys__ = pmpi_info_get_nkeys__
EXPORT_MPI_API void mpi_info_get_nkeys__ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_get_nkeys = pmpi_info_get_nkeys
EXPORT_MPI_API void mpi_info_get_nkeys (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_info_get_nkeys_ = pmpi_info_get_nkeys_
EXPORT_MPI_API void mpi_info_get_nkeys_ (MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_GET_NKEYS (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_INFO_GET_NKEYS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_nkeys__ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_get_nkeys__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_nkeys (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_get_nkeys ")));
#else
EXPORT_MPI_API void mpi_info_get_nkeys_ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_info_get_nkeys_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_GET_NKEYS  MPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_nkeys__  mpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_nkeys  mpi_info_get_nkeys
#else
#pragma _HP_SECONDARY_DEF pmpi_info_get_nkeys_  mpi_info_get_nkeys_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_GET_NKEYS as PMPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_nkeys__ as pmpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_nkeys as pmpi_info_get_nkeys
#else
#pragma _CRI duplicate mpi_info_get_nkeys_ as pmpi_info_get_nkeys_
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
#define mpi_info_get_nkeys_ PMPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys
#else
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_get_nkeys_ MPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nkeys_ mpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nkeys_ mpi_info_get_nkeys
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_nkeys_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_nkeys_(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *__ierr )
{
    MPI_Info info_c;
    int l_nkeys;
    
    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_nkeys(info_c, &l_nkeys);
    *nkeys = l_nkeys;
}
