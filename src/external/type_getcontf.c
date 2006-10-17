/* type_get_cont.c */
/* Fortran interface file */

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
#pragma weak MPI_TYPE_GET_CONTENTS = PMPI_TYPE_GET_CONTENTS
EXPORT_MPI_API void MPI_TYPE_GET_CONTENTS (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_get_contents__ = pmpi_type_get_contents__
EXPORT_MPI_API void mpi_type_get_contents__ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_get_contents = pmpi_type_get_contents
EXPORT_MPI_API void mpi_type_get_contents (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_type_get_contents_ = pmpi_type_get_contents_
EXPORT_MPI_API void mpi_type_get_contents_ (MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_GET_CONTENTS (MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_TYPE_GET_CONTENTS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_get_contents__ (MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_get_contents__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_get_contents (MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_get_contents")));
#else
EXPORT_MPI_API void mpi_type_get_contents_ (MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_type_get_contents_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_GET_CONTENTS  MPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_get_contents__  mpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_get_contents  mpi_type_get_contents
#else
#pragma _HP_SECONDARY_DEF pmpi_type_get_contents_  mpi_type_get_contents_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_GET_CONTENTS as PMPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_get_contents__ as pmpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_get_contents as pmpi_type_get_contents
#else
#pragma _CRI duplicate mpi_type_get_contents_ as pmpi_type_get_contents_
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
#define mpi_type_get_contents_ PMPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_contents_ pmpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_contents_ pmpi_type_get_contents
#else
#define mpi_type_get_contents_ pmpi_type_get_contents_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_get_contents_ MPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_contents_ mpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_contents_ mpi_type_get_contents
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_get_contents_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
EXPORT_MPI_API void FORTRAN_API mpi_type_get_contents_(MPI_Fint *datatype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, 
										   MPI_Fint *array_of_integers, MPI_Fint *array_of_addresses, MPI_Fint *array_of_datatypes, MPI_Fint *__ierr )
{
    int i;
    int *l_array_of_integers;
    MPI_Aint *l_array_of_addresses;
    MPI_Datatype *l_array_of_datatypes;
    
    MPIR_FALLOC(l_array_of_integers, (int*)MALLOC(sizeof(int)* (int)*
		max_integers), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		"MPI_TYPE_GET_CONTENTS");

    MPIR_FALLOC(l_array_of_addresses, (MPI_Aint *)MALLOC(sizeof(MPI_Aint) *
		(int)*max_addresses), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		"MPI_TYPE_GET_CONTENTS");

    MPIR_FALLOC(l_array_of_datatypes, (MPI_Datatype *)MALLOC(sizeof
               (MPI_Datatype) * (int)*max_datatypes), MPIR_COMM_WORLD, 
		MPI_ERR_EXHAUSTED, "MPI_TYPE_GET_CONTENTS");
    
    *__ierr = MPI_Type_get_contents( MPI_Type_f2c(*datatype), 
				     (int)*max_integers, 
				     (int)*max_addresses,
				     (int)*max_datatypes, 
				     l_array_of_integers,
				     l_array_of_addresses,
				     l_array_of_datatypes );

    for (i=0; i<(int)*max_integers; i++)
	array_of_integers[i] = (MPI_Fint)l_array_of_integers[i];
    for (i=0; i<(int)*max_addresses; i++)
	array_of_addresses[i] = (MPI_Aint)l_array_of_addresses[i];
    for (i=0; i<(int)*max_datatypes; i++)
	array_of_datatypes[i] = MPI_Type_c2f(l_array_of_datatypes[i]);

    FREE( l_array_of_integers );
    FREE( l_array_of_addresses );
    FREE( l_array_of_datatypes );
}

