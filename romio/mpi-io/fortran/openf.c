/* 
 *   $Id: openf.c 3732 2005-07-14 14:11:52Z tobias $    
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
#define mpi_file_open_ PMPI_FILE_OPEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_open_ pmpi_file_open__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_open_ pmpi_file_open
#else
#define mpi_file_open_ pmpi_file_open_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_OPEN = PMPI_FILE_OPEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_open__ = pmpi_file_open__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_open = pmpi_file_open
#else
#pragma weak mpi_file_open_ = pmpi_file_open_
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
void FORTRAN_API MPI_FILE_OPEN (MPI_Comm *, char *, int *, MPI_Fint *, MPI_Fint *, int *, int) __attribute__ ((weak, alias ("PMPI_FILE_OPEN")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
void FORTRAN_API mpi_file_open__ (MPI_Comm *, char *, int *, MPI_Fint *, MPI_Fint *, int *, int) __attribute__ ((weak, alias ("pmpi_file_open__")));
#elif !defined(FORTRANUNDERSCORE)
void FORTRAN_API mpi_file_open (MPI_Comm *, char *, int *, MPI_Fint *, MPI_Fint *, int *, int) __attribute__ ((weak, alias ("pmpi_file_open")));
#else
void FORTRAN_API mpi_file_open_ (MPI_Comm *, char *, int *, MPI_Fint *, MPI_Fint *, int *, int) __attribute__ ((weak, alias ("pmpi_file_open_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_OPEN MPI_FILE_OPEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_open__ mpi_file_open__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_open mpi_file_open
#else
#pragma _HP_SECONDARY_DEF pmpi_file_open_ mpi_file_open_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_OPEN as PMPI_FILE_OPEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_open__ as pmpi_file_open__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_open as pmpi_file_open
#else
#pragma _CRI duplicate mpi_file_open_ as pmpi_file_open_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_open_ MPI_FILE_OPEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_open_ mpi_file_open__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_open_ mpi_file_open
#else
#endif
#endif

#if defined(MPIHP) || defined(MPILAM)
void mpi_file_open_(MPI_Fint *comm,char *filename,int *amode,
                  MPI_Fint *info, MPI_Fint *fh, int *ierr, int str_len )
{
    char *newfname;
    MPI_File fh_c;
    int real_len, i;
    MPI_Comm comm_c;
    MPI_Info info_c;

    comm_c = MPI_Comm_f2c(*comm);
    info_c = MPI_Info_f2c(*info);

    /* strip trailing blanks */
    if (filename <= (char *) 0) {
        FPRINTF(stderr, "MPI_File_open: filename is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (i=str_len-1; i>=0; i--) if (filename[i] != ' ') break;
    if (i < 0) {
	FPRINTF(stderr, "MPI_File_open: filename is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    real_len = i + 1;

    newfname = (char *) ADIOI_Malloc((real_len+1)*sizeof(char));
    strncpy(newfname, filename, real_len);
    newfname[real_len] = '\0';

    *ierr = MPI_File_open(comm_c, newfname, *amode, info_c, &fh_c);

    *fh = MPI_File_c2f(fh_c);
    ADIOI_Free(newfname);
}

#else

#if _UNICOS
void mpi_file_open_(MPI_Comm *comm,_fcd filename_fcd,int *amode,
                  MPI_Fint *info, MPI_Fint *fh, int *ierr)
{
    char *filename = _fcdtocp(filename_fcd);
    int str_len = _fcdlen(filename_fcd);
#elif defined(VISUAL_FORTRAN)
void FORTRAN_API mpi_file_open_(MPI_Comm *comm,char *filename,int str_len,int *amode,
                  MPI_Fint *info, MPI_Fint *fh, int *ierr)
{
#else
void FORTRAN_API mpi_file_open_(MPI_Comm *comm,char *filename,int *amode,
                  MPI_Fint *info, MPI_Fint *fh, int *ierr, int str_len )
{
#endif
    char *newfname;
    MPI_File fh_c;
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

    *ierr = MPI_File_open(*comm, newfname, *amode, info_c, &fh_c);

    *fh = MPI_File_c2f(fh_c);
    ADIOI_Free(newfname);
}
#endif
