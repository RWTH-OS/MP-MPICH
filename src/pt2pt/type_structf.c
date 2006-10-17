/* type_struct.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_STRUCT = PMPI_TYPE_STRUCT
EXPORT_MPI_API void MPI_TYPE_STRUCT ( MPI_Fint *, MPI_Fint [], MPI_Fint [], MPI_Fint [], MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_struct__ = pmpi_type_struct__
EXPORT_MPI_API void mpi_type_struct__ ( MPI_Fint *, MPI_Fint [], MPI_Fint [], MPI_Fint [], MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_struct = pmpi_type_struct
EXPORT_MPI_API void mpi_type_struct ( MPI_Fint *, MPI_Fint [], MPI_Fint [], MPI_Fint [], MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_struct_ = pmpi_type_struct_
EXPORT_MPI_API void mpi_type_struct_ ( MPI_Fint *, MPI_Fint [], MPI_Fint [], MPI_Fint [], MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_STRUCT ( MPI_Fint *, MPI_Fint [], MPI_Fint [],
	MPI_Fint [], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_STRUCT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_struct__ ( MPI_Fint *, MPI_Fint [], MPI_Fint [],
	MPI_Fint [], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_struct__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_struct ( MPI_Fint *, MPI_Fint [], MPI_Fint [],
	MPI_Fint [], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_struct")));
#else
EXPORT_MPI_API void mpi_type_struct_ ( MPI_Fint *, MPI_Fint [], MPI_Fint [],
	MPI_Fint [], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_struct_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_STRUCT  MPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_struct__  mpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_struct  mpi_type_struct
#else
#pragma _HP_SECONDARY_DEF pmpi_type_struct_  mpi_type_struct_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_STRUCT as PMPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_struct__ as pmpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_struct as pmpi_type_struct
#else
#pragma _CRI duplicate mpi_type_struct_ as pmpi_type_struct_
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
#define mpi_type_struct_ PMPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct
#else
#define mpi_type_struct_ pmpi_type_struct_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_struct_ MPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_struct_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint [], 
                                  MPI_Fint [], MPI_Fint *, 
                                  MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_struct_( MPI_Fint *count, MPI_Fint blocklens[], MPI_Fint indices[], MPI_Fint old_types[], MPI_Fint *newtype, MPI_Fint *__ierr )
{
    MPI_Aint     *c_indices;
    MPI_Aint     local_c_indices[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype *l_datatype;
    MPI_Datatype local_l_datatype[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype l_newtype;
    int          *l_blocklens;
    int          local_l_blocklens[MPIR_USE_LOCAL_ARRAY];
    int          i;
    int          mpi_errno;
    static char  myname[] = "MPI_TYPE_STRUCT";
    
    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	/* Since indices come from MPI_ADDRESS (the FORTRAN VERSION),
	   they are currently relative to MPIF_F_MPI_BOTTOM.  
	   Convert them back */
	    MPIR_FALLOC(c_indices,(MPI_Aint *) MALLOC( *count * sizeof(MPI_Aint) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, myname );

	    MPIR_FALLOC(l_blocklens,(int *) MALLOC( *count * sizeof(int) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, myname );

	    MPIR_FALLOC(l_datatype,(MPI_Datatype *) MALLOC( *count * sizeof(MPI_Datatype) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, myname );
	}
	else {
	    c_indices = local_c_indices;
	    l_blocklens = local_l_blocklens;
	    l_datatype = local_l_datatype;
	}

	for (i=0; i<(int)*count; i++) {
	    c_indices[i] = (MPI_Aint) indices[i]/* + (MPI_Aint)MPIR_F_MPI_BOTTOM*/;
            l_blocklens[i] = (int) blocklens[i];
            l_datatype[i] = MPI_Type_f2c(old_types[i]);
	}
	*__ierr = MPI_Type_struct((int)*count, l_blocklens, c_indices,
                                  l_datatype,
				  &l_newtype);

        if ((int)*count > MPIR_USE_LOCAL_ARRAY) {    
	    FREE( c_indices );
            FREE( l_blocklens );
            FREE( l_datatype );
	}
    }
    else if ((int)*count == 0) {
	*__ierr = MPI_SUCCESS;
	*newtype = 0;
    }
    else {
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_COUNT, MPIR_ERR_DEFAULT, myname,
				     (char *)0, (char *)0, (int)(*count) );
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
	return;
    }
    *newtype = MPI_Type_c2f(l_newtype);

}
