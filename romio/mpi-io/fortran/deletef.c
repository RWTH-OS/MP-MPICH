/* 
 *   $Id: deletef.c 3732 2005-07-14 14:11:52Z tobias $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#if _UNICOS
#include <fortran.h>
#endif
#include "mpio.h"
#include "adio.h"

#ifdef MP_MPICH
#include "mpi_error.h"
#include "mpiimpl.h"
extern char *  MPIR_strncpy(char *, const char *, unsigned int);
#define strncpy MPIR_strncpy
#endif

#if defined(MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_delete_ PMPI_FILE_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_delete_ pmpi_file_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_delete_ pmpi_file_delete
#else
#define mpi_file_delete_ pmpi_file_delete_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_DELETE = PMPI_FILE_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_delete__ = pmpi_file_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_delete = pmpi_file_delete
#else
#pragma weak mpi_file_delete_ = pmpi_file_delete_
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
void FORTRAN_API MPI_FILE_DELETE (char *filename, MPI_Fint *info, int *ierr, int str_len) __attribute__ ((weak, alias ("PMPI_FILE_DELETE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
void FORTRAN_API mpi_file_delete__ (char *filename, MPI_Fint *info, int *ierr, int str_len) __attribute__ ((weak, alias ("pmpi_file_delete__")));
#elif !defined(FORTRANUNDERSCORE)
void FORTRAN_API mpi_file_delete (char *filename, MPI_Fint *info, int *ierr, int str_len) __attribute__ ((weak, alias ("pmpi_file_delete")));
#else
void FORTRAN_API mpi_file_delete_ (char *filename, MPI_Fint *info, int *ierr, int str_len) __attribute__ ((weak, alias ("pmpi_file_delete_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_DELETE MPI_FILE_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_delete__ mpi_file_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_delete mpi_file_delete
#else
#pragma _HP_SECONDARY_DEF pmpi_file_delete_ mpi_file_delete_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_DELETE as PMPI_FILE_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_delete__ as pmpi_file_delete__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_delete as pmpi_file_delete
#else
#pragma _CRI duplicate mpi_file_delete_ as pmpi_file_delete_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_delete_ MPI_FILE_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_delete_ mpi_file_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_delete_ mpi_file_delete
#endif
#endif

#if _UNICOS
void mpi_file_delete_(_fcd filename_fcd, MPI_Fint *info, int *ierr)
{
    char *filename = _fcdtocp(filename_fcd);
    int str_len = _fcdlen(filename_fcd);
#elif defined(VISUAL_FORTRAN)
void FORTRAN_API mpi_file_delete_(char *filename, int str_len, MPI_Fint *info, int *ierr) {
#else
void FORTRAN_API mpi_file_delete_(char *filename, MPI_Fint *info, int *ierr, int str_len)
{
#endif
    char *newfname;
    int real_len, i;
    MPI_Info info_c;

    info_c = MPI_Info_f2c(*info);

    /* strip trailing blanks */
    if (filename <= (char *) 0) {
#ifdef MP_MPICH
	*ierr = MPI_ERR_ARG;
	MPIR_ERROR(MPIR_COMM_WORLD,MPI_ERR_OTHER,"MPI_File_delete: filename is an invalid address");
	return;
#else
	FPRINTF(stderr, "MPI_File_delete: filename is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#endif
    }
    for (i=str_len-1; i>=0; i--) if (filename[i] != ' ') break;
    if (i < 0) {
#ifdef MP_MPICH
	*ierr = MPI_ERR_ARG;
	MPIR_ERROR(MPIR_COMM_WORLD,MPI_ERR_OTHER,"MPI_File_delete: filename is a blank string");
	return;
#else
        FPRINTF(stderr, "MPI_File_delete: filename is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#endif
    }
    real_len = i + 1;

    newfname = (char *) ADIOI_Malloc((real_len+1)*sizeof(char));
    strncpy(newfname, filename, real_len);
    newfname[real_len] = '\0';

    *ierr = MPI_File_delete(newfname, info_c);

    ADIOI_Free(newfname);
}
