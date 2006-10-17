/* dims_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_DIMS_CREATE = PMPI_DIMS_CREATE
EXPORT_MPI_API void MPI_DIMS_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_dims_create__ = pmpi_dims_create__
EXPORT_MPI_API void mpi_dims_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_dims_create = pmpi_dims_create
EXPORT_MPI_API void mpi_dims_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_dims_create_ = pmpi_dims_create_
EXPORT_MPI_API void mpi_dims_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_DIMS_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_DIMS_CREATE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_dims_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dims_create__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_dims_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dims_create")));
#else
EXPORT_MPI_API void mpi_dims_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dims_create_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_DIMS_CREATE  MPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_dims_create__  mpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_dims_create  mpi_dims_create
#else
#pragma _HP_SECONDARY_DEF pmpi_dims_create_  mpi_dims_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_DIMS_CREATE as PMPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_dims_create__ as pmpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_dims_create as pmpi_dims_create
#else
#pragma _CRI duplicate mpi_dims_create_ as pmpi_dims_create_
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
#define mpi_dims_create_ PMPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create
#else
#define mpi_dims_create_ pmpi_dims_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_dims_create_ MPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_dims_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_dims_create_(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint *dims, MPI_Fint *__ierr )
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Dims_create(*nnodes,*ndims,dims);
    else {
        int *ldims;
        int i;

	MPIR_FALLOC(ldims,(int*)MALLOC(sizeof(int)* (int)*ndims),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Dims_create");

        for (i=0; i<(int)*ndims; i++)
	    ldims[i] = (int)dims[i];

        *__ierr = MPI_Dims_create((int)*nnodes, (int)*ndims, ldims);

        for (i=0; i<(int)*ndims; i++)
	    dims[i] = (MPI_Fint)ldims[i];
	    
        FREE( ldims );
    }

}
