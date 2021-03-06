/* 
 *   $Id: get_posnf.c 3732 2005-07-14 14:11:52Z tobias $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"


#if defined(MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_get_position_ PMPI_FILE_GET_POSITION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_position_ pmpi_file_get_position__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_get_position_ pmpi_file_get_position
#else
#define mpi_file_get_position_ pmpi_file_get_position_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_GET_POSITION = PMPI_FILE_GET_POSITION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_get_position__ = pmpi_file_get_position__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_get_position = pmpi_file_get_position
#else
#pragma weak mpi_file_get_position_ = pmpi_file_get_position_
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
void FORTRAN_API MPI_FILE_GET_POSITION (MPI_Fint *fh, MPI_Offset *offset, int *ierr ) __attribute__ ((weak, alias ("PMPI_FILE_GET_POSITION")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
void FORTRAN_API mpi_file_get_position__ (MPI_Fint *fh, MPI_Offset *offset, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_position__")));
#elif !defined(FORTRANUNDERSCORE)
void FORTRAN_API mpi_file_get_position (MPI_Fint *fh, MPI_Offset *offset, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_position")));
#else
void FORTRAN_API mpi_file_get_position_ (MPI_Fint *fh, MPI_Offset *offset, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_position_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_GET_POSITION MPI_FILE_GET_POSITION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_position__ mpi_file_get_position__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_position mpi_file_get_position
#else
#pragma _HP_SECONDARY_DEF pmpi_file_get_position_ mpi_file_get_position_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_GET_POSITION as PMPI_FILE_GET_POSITION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_position__ as pmpi_file_get_position__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_position as pmpi_file_get_position
#else
#pragma _CRI duplicate mpi_file_get_position_ as pmpi_file_get_position_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_get_position_ MPI_FILE_GET_POSITION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_position_ mpi_file_get_position__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_file_get_position_ mpi_file_get_position
#endif
#endif

void FORTRAN_API mpi_file_get_position_(MPI_Fint *fh, MPI_Offset *offset, int *ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_get_position(fh_c, offset);
}
