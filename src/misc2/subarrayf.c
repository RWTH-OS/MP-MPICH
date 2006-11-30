/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_CREATE_SUBARRAY = PMPI_TYPE_CREATE_SUBARRAY
EXPORT_MPI_API void MPI_TYPE_CREATE_SUBARRAY (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_create_subarray__ = pmpi_type_create_subarray__
EXPORT_MPI_API void mpi_type_create_subarray__ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_create_subarray = pmpi_type_create_subarray
EXPORT_MPI_API void mpi_type_create_subarray (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_type_create_subarray_ = pmpi_type_create_subarray_
EXPORT_MPI_API void mpi_type_create_subarray_ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_CREATE_SUBARRAY (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_TYPE_CREATE_SUBARRAY")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_create_subarray__ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_subarray__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_create_subarray (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_subarray")));
#else
EXPORT_MPI_API void mpi_type_create_subarray_ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_subarray_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_CREATE_SUBARRAY  MPI_TYPE_CREATE_SUBARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_create_subarray__  mpi_type_create_subarray__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_create_subarray  mpi_type_create_subarray
#else
#pragma _HP_SECONDARY_DEF pmpi_type_create_subarray_  mpi_type_create_subarray_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_CREATE_SUBARRAY as PMPI_TYPE_CREATE_SUBARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_create_subarray__ as pmpi_type_create_subarray__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_create_subarray as pmpi_type_create_subarray
#else
#pragma _CRI duplicate mpi_type_create_subarray_ as pmpi_type_create_subarray_
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
#define mpi_type_create_subarray_ PMPI_TYPE_CREATE_SUBARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_subarray_ pmpi_type_create_subarray__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_subarray_ pmpi_type_create_subarray
#else
#define mpi_type_create_subarray_ pmpi_type_create_subarray_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_create_subarray_ MPI_TYPE_CREATE_SUBARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_subarray_ mpi_type_create_subarray__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_subarray_ mpi_type_create_subarray
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_create_subarray_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *,
					  MPI_Fint *, MPI_Fint *, MPI_Fint *,
					  MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_type_create_subarray_(MPI_Fint *ndims, MPI_Fint *array_of_sizes,
                               MPI_Fint *array_of_subsizes,
			       MPI_Fint *array_of_starts, MPI_Fint *order,
			       MPI_Fint *oldtype, MPI_Fint *newtype, 
			       MPI_Fint *__ierr )
{
    int i;
    int *l_array_of_sizes;
    int local_l_array_of_sizes[MPIR_USE_LOCAL_ARRAY];
    int *l_array_of_subsizes;
    int local_l_array_of_subsizes[MPIR_USE_LOCAL_ARRAY];
    int *l_array_of_starts;
    int local_l_array_of_starts[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype oldtype_c, newtype_c;

    oldtype_c = MPI_Type_f2c(*oldtype);

    if ((int)*ndims > 0) {
	if ((int)*ndims > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(l_array_of_sizes,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_SUBARRAY" );

	    MPIR_FALLOC(l_array_of_subsizes,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_SUBARRAY" );

	    MPIR_FALLOC(l_array_of_starts,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_SUBARRAY" );

	}
	else {
	    l_array_of_sizes = local_l_array_of_sizes;
	    l_array_of_subsizes = local_l_array_of_subsizes;
	    l_array_of_starts = local_l_array_of_starts;
	}

	for (i=0; i<(int)*ndims; i++) {
	    l_array_of_sizes[i] = (int)array_of_sizes[i];
	    l_array_of_subsizes[i] = (int)array_of_subsizes[i];
	    l_array_of_starts[i] = (int)array_of_starts[i];
	}
    }

    *__ierr = MPI_Type_create_subarray((int)*ndims, l_array_of_sizes,
				       l_array_of_subsizes, l_array_of_starts,
				       (int)*order, oldtype_c, &newtype_c);

    if ((int)*ndims > MPIR_USE_LOCAL_ARRAY) {
	FREE( l_array_of_sizes );
	FREE( l_array_of_subsizes );
	FREE( l_array_of_starts );
    }

    *newtype = MPI_Type_c2f(newtype_c);
}

