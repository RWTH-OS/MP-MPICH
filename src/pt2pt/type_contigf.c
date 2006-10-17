/* type_contig.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_CONTIGUOUS = PMPI_TYPE_CONTIGUOUS
EXPORT_MPI_API void MPI_TYPE_CONTIGUOUS ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_contiguous__ = pmpi_type_contiguous__
EXPORT_MPI_API void mpi_type_contiguous__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_contiguous = pmpi_type_contiguous
EXPORT_MPI_API void mpi_type_contiguous ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_contiguous_ = pmpi_type_contiguous_
EXPORT_MPI_API void mpi_type_contiguous_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_CONTIGUOUS ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_CONTIGUOUS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_contiguous__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_contiguous__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_contiguous ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_contiguous")));
#else
EXPORT_MPI_API void mpi_type_contiguous_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_contiguous_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_CONTIGUOUS  MPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_contiguous__  mpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_contiguous  mpi_type_contiguous
#else
#pragma _HP_SECONDARY_DEF pmpi_type_contiguous_  mpi_type_contiguous_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_CONTIGUOUS as PMPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_contiguous__ as pmpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_contiguous as pmpi_type_contiguous
#else
#pragma _CRI duplicate mpi_type_contiguous_ as pmpi_type_contiguous_
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
#define mpi_type_contiguous_ PMPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous
#else
#define mpi_type_contiguous_ pmpi_type_contiguous_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_contiguous_ MPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_contiguous_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				      MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_contiguous_( MPI_Fint *count, MPI_Fint *old_type, MPI_Fint *newtype, MPI_Fint *__ierr )
{
    MPI_Datatype  ldatatype;

    *__ierr = MPI_Type_contiguous((int)*count, MPI_Type_f2c(*old_type),
                                  &ldatatype);
    *newtype = MPI_Type_c2f(ldatatype);
}
