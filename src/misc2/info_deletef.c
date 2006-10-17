/* 
 *   $Id: info_deletef.c,v 1.5 2003/06/13 09:39:37 rainer Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MP_MPICH
extern char *  MPIR_strncpy(char *, const char *, unsigned int);
#define strncpy MPIR_strncpy
#endif

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_DELETE = PMPI_INFO_DELETE
EXPORT_MPI_API void MPI_INFO_DELETE (MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_delete__ = pmpi_info_delete__
EXPORT_MPI_API void mpi_info_delete__ (MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_delete = pmpi_info_delete
EXPORT_MPI_API void mpi_info_delete (MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#else
#pragma weak mpi_info_delete_ = pmpi_info_delete_
EXPORT_MPI_API void mpi_info_delete_ (MPI_Fint *, char *, MPI_Fint *, MPI_Fint);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_DELETE (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("PMPI_INFO_DELETE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_delete__ (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_delete__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_delete (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_delete")));
#else
EXPORT_MPI_API void mpi_info_delete_ (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_delete_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_DELETE  MPI_INFO_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_delete__  mpi_info_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_delete  mpi_info_delete
#else
#pragma _HP_SECONDARY_DEF pmpi_info_delete_  mpi_info_delete_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_DELETE as PMPI_INFO_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_delete__ as pmpi_info_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_delete as pmpi_info_delete
#else
#pragma _CRI duplicate mpi_info_delete_ as pmpi_info_delete_
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
#define mpi_info_delete_ PMPI_INFO_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_delete_ pmpi_info_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_delete_ pmpi_info_delete
#else
#define mpi_info_delete_ pmpi_info_delete_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_delete_ MPI_INFO_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_delete_ mpi_info_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_delete_ mpi_info_delete
#endif
#endif


#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_info_delete_(MPI_Fint *info,MPI_Fint keylen, char *key, MPI_Fint *__ierr)
#else
/* Prototype to suppress warning about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_delete_ ANSI_ARGS((MPI_Fint *, char *, MPI_Fint *, MPI_Fint));

/* Definitions of Fortran Wrapper routines */ 
EXPORT_MPI_API void FORTRAN_API mpi_info_delete_(MPI_Fint *info, char *key, MPI_Fint *__ierr, 
		      MPI_Fint keylen)
#endif
{
    MPI_Info info_c;
    char *newkey;
    int new_keylen, lead_blanks, i;
    static char myname[] = "MPI_INFO_DELETE";
    int mpi_errno;

    if (!key) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_KEY, MPIR_ERR_DEFAULT, 
				     myname, (char *)0, (char *)0);
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }

    /* strip leading and trailing blanks in key */
    lead_blanks = 0;
    for (i=0; i<(int)keylen; i++) 
        if (key[i] == ' ') lead_blanks++;
        else break;

    for (i=(int)keylen-1; i>=0; i--) if (key[i] != ' ') break;
    if (i < 0) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_KEY, MPIR_ERR_KEY_EMPTY,
				     myname, (char *)0, (char *)0 );
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }
    new_keylen = i + 1 - lead_blanks;
    key += lead_blanks;

    newkey = (char *) MALLOC((new_keylen+1)*sizeof(char));
    if (!newkey) {
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, myname );
	return;
    }
    strncpy(newkey, key, new_keylen);
    newkey[new_keylen] = '\0';

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_delete(info_c, newkey);
    FREE(newkey);
}

