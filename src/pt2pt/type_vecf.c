/* type_vec.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_VECTOR = PMPI_TYPE_VECTOR
EXPORT_MPI_API void MPI_TYPE_VECTOR ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_vector__ = pmpi_type_vector__
EXPORT_MPI_API void mpi_type_vector__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_vector = pmpi_type_vector
EXPORT_MPI_API void mpi_type_vector ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_vector_ = pmpi_type_vector_
EXPORT_MPI_API void mpi_type_vector_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_VECTOR ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_VECTOR")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_vector__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_vector__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_vector ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_vector")));
#else
EXPORT_MPI_API void mpi_type_vector_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_vector_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_VECTOR  MPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_vector__  mpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_vector  mpi_type_vector
#else
#pragma _HP_SECONDARY_DEF pmpi_type_vector_  mpi_type_vector_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_VECTOR as PMPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_vector__ as pmpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_vector as pmpi_type_vector
#else
#pragma _CRI duplicate mpi_type_vector_ as pmpi_type_vector_
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
#define mpi_type_vector_ PMPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector
#else
#define mpi_type_vector_ pmpi_type_vector_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_vector_ MPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_vector_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_vector_( MPI_Fint *count, MPI_Fint *blocklen, MPI_Fint *stride, MPI_Fint *old_type, MPI_Fint *newtype, MPI_Fint *__ierr )
{
    MPI_Datatype l_datatype;

    *__ierr = MPI_Type_vector((int)*count, (int)*blocklen, (int)*stride,
                              MPI_Type_f2c(*old_type), 
                              &l_datatype);
    *newtype = MPI_Type_c2f(l_datatype);
}
