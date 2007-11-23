/* 
 *   $Id: get_atomf.c 3732 2005-07-14 14:11:52Z tobias $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"


#if defined(MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_get_atomicity_ PMPI_FILE_GET_ATOMICITY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_atomicity_ pmpi_file_get_atomicity__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_get_atomicity_ pmpi_file_get_atomicity
#else
#define mpi_file_get_atomicity_ pmpi_file_get_atomicity_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_GET_ATOMICITY = PMPI_FILE_GET_ATOMICITY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_get_atomicity__ = pmpi_file_get_atomicity__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_get_atomicity = pmpi_file_get_atomicity
#else
#pragma weak mpi_file_get_atomicity_ = pmpi_file_get_atomicity_
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
void FORTRAN_API MPI_FILE_GET_ATOMICITY (MPI_Fint *fh,int *flag, int *ierr ) __attribute__ ((weak, alias ("PMPI_FILE_GET_ATOMICITY")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
void FORTRAN_API mpi_file_get_atomicity__ (MPI_Fint *fh,int *flag, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_atomicity__")));
#elif !defined(FORTRANUNDERSCORE)
void FORTRAN_API mpi_file_get_atomicity (MPI_Fint *fh,int *flag, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_atomicity")));
#else
void FORTRAN_API mpi_file_get_atomicity_ (MPI_Fint *fh,int *flag, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_atomicity_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_GET_ATOMICITY MPI_FILE_GET_ATOMICITY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_atomicity__ mpi_file_get_atomicity__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_atomicity mpi_file_get_atomicity
#else
#pragma _HP_SECONDARY_DEF pmpi_file_get_atomicity_ mpi_file_get_atomicity_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_GET_ATOMICITY as PMPI_FILE_GET_ATOMICITY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_atomicity__ as pmpi_file_get_atomicity__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_atomicity as pmpi_file_get_atomicity
#else
#pragma _CRI duplicate mpi_file_get_atomicity_ as pmpi_file_get_atomicity_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_get_atomicity_ MPI_FILE_GET_ATOMICITY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_atomicity_ mpi_file_get_atomicity__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_get_atomicity_ mpi_file_get_atomicity
#endif
#endif

void FORTRAN_API mpi_file_get_atomicity_(MPI_Fint *fh,int *flag, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_get_atomicity(fh_c, flag);
}

