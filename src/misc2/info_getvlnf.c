/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#ifdef MP_MPICH
extern char *  MPIR_strncpy(char *, const char *, unsigned int);
#define strncpy MPIR_strncpy
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INFO_GET_VALUELEN = PMPI_INFO_GET_VALUELEN
EXPORT_MPI_API void MPI_INFO_GET_VALUELEN (MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_info_get_valuelen__ = pmpi_info_get_valuelen__
EXPORT_MPI_API void mpi_info_get_valuelen__ (MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_info_get_valuelen = pmpi_info_get_valuelen
EXPORT_MPI_API void mpi_info_get_valuelen (MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint);
#else
#pragma weak mpi_info_get_valuelen_ = pmpi_info_get_valuelen_
EXPORT_MPI_API void mpi_info_get_valuelen_ (MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INFO_GET_VALUELEN (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("PMPI_INFO_GET_VALUELEN")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_valuelen__ (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_valuelen__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_info_get_valuelen (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_valuelen")));
#else
EXPORT_MPI_API void mpi_info_get_valuelen_ (MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint) __attribute__ ((weak, alias ("pmpi_info_get_valuelen_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INFO_GET_VALUELEN  MPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_valuelen__  mpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_info_get_valuelen  mpi_info_get_valuelen
#else
#pragma _HP_SECONDARY_DEF pmpi_info_get_valuelen_  mpi_info_get_valuelen_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INFO_GET_VALUELEN as PMPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_valuelen__ as pmpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_info_get_valuelen as pmpi_info_get_valuelen
#else
#pragma _CRI duplicate mpi_info_get_valuelen_ as pmpi_info_get_valuelen_
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
#define mpi_info_get_valuelen_ PMPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen
#else
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_info_get_valuelen_ MPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_valuelen_ mpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_valuelen_ mpi_info_get_valuelen
#endif
#endif

#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_info_get_valuelen_(MPI_Fint *info, char *key, MPI_Fint keylen, 
						       MPI_Fint *valuelen,
						       MPI_Fint *flag, MPI_Fint *__ierr )
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_valuelen_ ANSI_ARGS((MPI_Fint *, char *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *, MPI_Fint));
/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_info_get_valuelen_(MPI_Fint *info, char *key, MPI_Fint *valuelen,
			    MPI_Fint *flag, MPI_Fint *__ierr, MPI_Fint keylen )
#endif
{
    MPI_Info info_c;
    char *newkey;
    int new_keylen, lead_blanks, i;
    int lvaluelen, lflag;
    int mpi_errno;
    static char myname[] = "MPI_INFO_GET_VALUELEN";

    if (!key) {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_INFO_KEY, MPIR_ERR_DEFAULT, 
				     myname, (char *)0, (char *)0);
	*__ierr =  MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
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

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_valuelen(info_c, newkey, &lvaluelen, &lflag);
    *valuelen = lvaluelen;
    *flag = MPIR_TO_FLOG(lflag);
    FREE(newkey);
}

