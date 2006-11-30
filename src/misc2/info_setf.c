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
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_SET = PMPI_INFO_SET
EXPORT_MPI_API void MPI_INFO_SET (MPI_Fint *, char *, char *, MPI_Fint *, MPI_Fint, MPI_Fint);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_set__ = pmpi_info_set__
EXPORT_MPI_API void mpi_info_set__ (MPI_Fint *, char *, char *, MPI_Fint *, MPI_Fint, MPI_Fint);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_set = pmpi_info_set
EXPORT_MPI_API void mpi_info_set (MPI_Fint *, char *, char *, MPI_Fint *, MPI_Fint, MPI_Fint);
#else
#pragma weak mpi_info_set_ = pmpi_info_set_
EXPORT_MPI_API void mpi_info_set_ (MPI_Fint *, char *, char *, MPI_Fint *, MPI_Fint, MPI_Fint);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_SET (MPI_Fint *, char *, char *, MPI_Fint *,
	MPI_Fint, MPI_Fint) __attribute__ ((weak, alias ("PMPI_INFO_SET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_set__ (MPI_Fint *, char *, char *, MPI_Fint *,
	MPI_Fint, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_set__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_set (MPI_Fint *, char *, char *, MPI_Fint *,
	MPI_Fint, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_set")));
#else
EXPORT_MPI_API void mpi_info_set_ (MPI_Fint *, char *, char *, MPI_Fint *,
	MPI_Fint, MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_set_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_SET  MPI_INFO_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_set__  mpi_info_set__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_set  mpi_info_set
#else
#pragma _HP_SECONDARY_DEF pmpi_info_set_  mpi_info_set_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_SET as PMPI_INFO_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_set__ as pmpi_info_set__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_set as pmpi_info_set
#else
#pragma _CRI duplicate mpi_info_set_ as pmpi_info_set_
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
#define mpi_info_set_ PMPI_INFO_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_set_ pmpi_info_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_set_ pmpi_info_set
#else
#define mpi_info_set_ pmpi_info_set_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_set_ MPI_INFO_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_set_ mpi_info_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_set_ mpi_info_set
#endif
#endif

#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_info_set_(MPI_Fint *info, char *key, MPI_Fint keylen,
					      char *value, MPI_Fint vallen, MPI_Fint *__ierr)
#else

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_set_ ANSI_ARGS((MPI_Fint *, char *, char *, MPI_Fint *,
			      MPI_Fint, MPI_Fint));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_set_(MPI_Fint *info, char *key, char *value, MPI_Fint *__ierr, 
                   MPI_Fint keylen, MPI_Fint vallen)
#endif
{
    MPI_Info info_c;
    char *newkey, *newvalue;
    int new_keylen, new_vallen, lead_blanks, i;
    static char myname[] = "MPI_INFO_SET";
    int mpi_errno;

    if (!key) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_KEY, MPIR_ERR_DEFAULT, 
				     myname, (char *)0, (char *)0);
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }
    if (!value) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_VALUE, MPIR_ERR_DEFAULT,
				     myname, (char *)0, (char *)0 );
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
    strncpy(newkey, key, new_keylen);
    newkey[new_keylen] = '\0';


    /* strip leading and trailing blanks in value */
    lead_blanks = 0;
    for (i=0; i<(int)vallen; i++) 
	if (value[i] == ' ') lead_blanks++;
	else break;

    for (i=(int)vallen-1; i>=0; i--) if (value[i] != ' ') break;
    if (i < 0) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_VALUE, 
				     MPIR_ERR_DEFAULT,
				     myname, (char *)0, (char *)0 );
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }
    new_vallen = i + 1 - lead_blanks;
    value += lead_blanks;

    newvalue = (char *) MALLOC((new_vallen+1)*sizeof(char));
    strncpy(newvalue, value, new_vallen);
    newvalue[new_vallen] = '\0';

 
    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_set(info_c, newkey, newvalue);
    FREE(newkey);
    FREE(newvalue);
}
