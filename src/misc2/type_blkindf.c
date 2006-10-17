/* type_blkind.c */
/* Custom Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_CREATE_INDEXED_BLOCK = PMPI_TYPE_CREATE_INDEXED_BLOCK
EXPORT_MPI_API void MPI_TYPE_CREATE_INDEXED_BLOCK (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_create_indexed_block__ = pmpi_type_create_indexed_block__
EXPORT_MPI_API void mpi_type_create_indexed_block__ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_create_indexed_block = pmpi_type_create_indexed_block
EXPORT_MPI_API void mpi_type_create_indexed_block (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_type_create_indexed_block_ = pmpi_type_create_indexed_block_
EXPORT_MPI_API void mpi_type_create_indexed_block_ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_CREATE_INDEXED_BLOCK (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_TYPE_CREATE_INDEXED_BLOCK")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_create_indexed_block__ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_indexed_block__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_create_indexed_block (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_indexed_block")));
#else
EXPORT_MPI_API void mpi_type_create_indexed_block_ (MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_create_indexed_block_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_CREATE_INDEXED_BLOCK  MPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_create_indexed_block__  mpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_create_indexed_block  mpi_type_create_indexed_block
#else
#pragma _HP_SECONDARY_DEF pmpi_type_create_indexed_block_  mpi_type_create_indexed_block_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_CREATE_INDEXED_BLOCK as PMPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_create_indexed_block__ as pmpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_create_indexed_block as pmpi_type_create_indexed_block
#else
#pragma _CRI duplicate mpi_type_create_indexed_block_ as pmpi_type_create_indexed_block_
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
#define mpi_type_create_indexed_block_ PMPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block
#else
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_create_indexed_block_ MPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_indexed_block_ mpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_indexed_block_ mpi_type_create_indexed_block
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_create_indexed_block_ (MPI_Fint *, MPI_Fint *, 
					       MPI_Fint *, MPI_Fint *,
					       MPI_Fint *, MPI_Fint *);
/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_type_create_indexed_block_( MPI_Fint *count, MPI_Fint *blocklength, 
				     MPI_Fint array_of_displacements[], MPI_Fint *old_type, 
				     MPI_Fint *newtype, MPI_Fint *__ierr )
{

    int i;
    int *l_array_of_displacements;
    int local_l_array_of_displacements[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype lnewtype;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(l_array_of_displacements,(int *) MALLOC( *count * 
			sizeof(int) ), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_INDEXED_BLOCK");
	}
	else {
	    l_array_of_displacements = local_l_array_of_displacements;
	}

	for (i=0; i<(int)*count; i++)
	    l_array_of_displacements[i] = (int)(array_of_displacements[i]);
    }

    *__ierr = MPI_Type_create_indexed_block((int)*count, (int)*blocklength,
					l_array_of_displacements,
					MPI_Type_c2f( *old_type ),&lnewtype);

    if ((int)*count > MPIR_USE_LOCAL_ARRAY) 
	FREE( l_array_of_displacements );

    *newtype = MPI_Type_c2f( lnewtype );
}




