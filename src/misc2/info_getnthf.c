/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MP_MPICH
extern char *  MPIR_strncpy(char *, const char *, unsigned int);
#define strncpy MPIR_strncpy
#else
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif
#endif

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_GET_NTHKEY = PMPI_INFO_GET_NTHKEY
EXPORT_MPI_API void MPI_INFO_GET_NTHKEY (MPI_Fint *, MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_get_nthkey__ = pmpi_info_get_nthkey__
EXPORT_MPI_API void mpi_info_get_nthkey__ (MPI_Fint *, MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_get_nthkey = pmpi_info_get_nthkey
EXPORT_MPI_API void mpi_info_get_nthkey (MPI_Fint *, MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#else
#pragma weak mpi_info_get_nthkey_ = pmpi_info_get_nthkey_
EXPORT_MPI_API void mpi_info_get_nthkey_ (MPI_Fint *, MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_GET_NTHKEY (MPI_Fint *, MPI_Fint *, char *,
	MPI_Fint *, MPI_Fint) __attribute__ ((weak, alias ("PMPI_INFO_GET_NTHKEY")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_nthkey__ (MPI_Fint *, MPI_Fint *, char *,
	MPI_Fint *, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_nthkey__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_nthkey (MPI_Fint *, MPI_Fint *, char *,
	MPI_Fint *, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_nthkey")));
#else
EXPORT_MPI_API void mpi_info_get_nthkey_ (MPI_Fint *, MPI_Fint *, char *,
	MPI_Fint *, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_nthkey_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_GET_NTHKEY  MPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_nthkey__  mpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_nthkey  mpi_info_get_nthkey
#else
#pragma _HP_SECONDARY_DEF pmpi_info_get_nthkey_  mpi_info_get_nthkey_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_GET_NTHKEY as PMPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_nthkey__ as pmpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_nthkey as pmpi_info_get_nthkey
#else
#pragma _CRI duplicate mpi_info_get_nthkey_ as pmpi_info_get_nthkey_
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
#define mpi_info_get_nthkey_ PMPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey
#else
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_get_nthkey_ MPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nthkey_ mpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nthkey_ mpi_info_get_nthkey
#endif
#endif

#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_info_get_nthkey_(MPI_Fint *info, MPI_Fint *n, 
						     char *key, MPI_Fint keylen, 
 						     MPI_Fint *__ierr)
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_nthkey_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, char *, 
				     MPI_Fint *, MPI_Fint));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_nthkey_(MPI_Fint *info, MPI_Fint *n, char *key, 
 			  MPI_Fint *__ierr, MPI_Fint keylen)
#endif
{
    MPI_Info info_c;
    int i, tmpkeylen;
    char *tmpkey;
    int    mpi_errno;
    static char myname[] = "MPI_INFO_GET_NTHKEY";

    if (!key) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_KEY, MPIR_ERR_DEFAULT, 
				     myname, (char *)0, (char *)0);
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }

    tmpkey = (char *) MALLOC((MPI_MAX_INFO_KEY+1) * sizeof(char));
    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_nthkey(info_c, (int)*n, tmpkey);

    tmpkeylen = strlen(tmpkey);

    if (tmpkeylen <= (int)keylen) {
	strncpy(key, tmpkey, tmpkeylen);

	/* blank pad the remaining space */
	for (i=tmpkeylen; i<(int)keylen; i++) key[i] = ' ';
    }
    else {
	/* not enough space */
	strncpy(key, tmpkey, (int)keylen);
	/* this should be flagged as an error. */
	*__ierr = MPI_ERR_UNKNOWN;
    }

    FREE(tmpkey);
}


