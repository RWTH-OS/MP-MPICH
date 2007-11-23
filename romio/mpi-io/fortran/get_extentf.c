/* 
 *   $Id: get_extentf.c 3744 2005-07-18 10:55:08Z georg $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"


#if defined(MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_get_type_extent_ PMPI_FILE_GET_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_type_extent_ pmpi_file_get_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_get_type_extent pmpi_file_get_type_extent_
#endif
#define mpi_file_get_type_extent_ pmpi_file_get_type_extent
#else
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_get_type_extent_ pmpi_file_get_type_extent
#endif
#define mpi_file_get_type_extent_ pmpi_file_get_type_extent_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_GET_TYPE_EXTENT = PMPI_FILE_GET_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_get_type_extent__ = pmpi_file_get_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_get_type_extent = pmpi_file_get_type_extent
#else
#pragma weak mpi_file_get_type_extent_ = pmpi_file_get_type_extent_
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
void FORTRAN_API MPI_FILE_GET_TYPE_EXTENT (MPI_Fint *fh,MPI_Fint *datatype,
                             MPI_Fint *extent, int *ierr ) __attribute__ ((weak, alias ("PMPI_FILE_GET_TYPE_EXTENT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
void FORTRAN_API mpi_file_get_type_extent__ (MPI_Fint *fh,MPI_Fint *datatype,
                             MPI_Fint *extent, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_type_extent__")));
#elif !defined(FORTRANUNDERSCORE)
void FORTRAN_API mpi_file_get_type_extent (MPI_Fint *fh,MPI_Fint *datatype,
                             MPI_Fint *extent, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_type_extent")));
#else
void FORTRAN_API mpi_file_get_type_extent_ (MPI_Fint *fh,MPI_Fint *datatype,
                             MPI_Fint *extent, int *ierr ) __attribute__ ((weak, alias ("pmpi_file_get_type_extent_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_GET_TYPE_EXTENT MPI_FILE_GET_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_type_extent__ mpi_file_get_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_get_type_extent mpi_file_get_type_extent
#else
#pragma _HP_SECONDARY_DEF pmpi_file_get_type_extent_ mpi_file_get_type_extent_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_GET_TYPE_EXTENT as PMPI_FILE_GET_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_type_extent__ as pmpi_file_get_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_get_type_extent as pmpi_file_get_type_extent
#else
#pragma _CRI duplicate mpi_file_get_type_extent_ as pmpi_file_get_type_extent_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_get_type_extent_ MPI_FILE_GET_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_get_type_extent_ mpi_file_get_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_get_type_extent mpi_file_get_type_extent_
#endif
#define mpi_file_get_type_extent_ mpi_file_get_type_extent
#else
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_get_type_extent_ mpi_file_get_type_extent
#endif
#endif
#endif

#if defined(MPIHP) || defined(MPILAM)
void mpi_file_get_type_extent_(MPI_Fint *fh,MPI_Fint *datatype,
                             MPI_Fint *extent, int *ierr )
{
    MPI_File fh_c;
    MPI_Datatype datatype_c;
    MPI_Aint extent_c;
    
    fh_c = MPI_File_f2c(*fh);
    datatype_c = MPI_Type_f2c(*datatype);

    *ierr = MPI_File_get_type_extent(fh_c,datatype_c, &extent_c);
    *extent = (MPI_Fint) extent_c;
}

#else

void FORTRAN_API mpi_file_get_type_extent_(MPI_Fint *fh,MPI_Datatype *datatype,
                             MPI_Fint *extent, int *ierr )
{
    MPI_File fh_c;
    MPI_Aint extent_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_get_type_extent(fh_c,*datatype, &extent_c);
    *extent = (MPI_Fint) extent_c;
}
#endif
