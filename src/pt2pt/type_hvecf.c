/* type_hvec.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_TYPE_HVECTOR = PMPI_TYPE_HVECTOR
EXPORT_MPI_API void MPI_TYPE_HVECTOR ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_type_hvector__ = pmpi_type_hvector__
EXPORT_MPI_API void mpi_type_hvector__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_type_hvector = pmpi_type_hvector
EXPORT_MPI_API void mpi_type_hvector ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_type_hvector_ = pmpi_type_hvector_
EXPORT_MPI_API void mpi_type_hvector_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_TYPE_HVECTOR ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_TYPE_HVECTOR")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_type_hvector__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_hvector__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_type_hvector ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_hvector")));
#else
EXPORT_MPI_API void mpi_type_hvector_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_type_hvector_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_TYPE_HVECTOR  MPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_hvector__  mpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_type_hvector  mpi_type_hvector
#else
#pragma _HP_SECONDARY_DEF pmpi_type_hvector_  mpi_type_hvector_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_TYPE_HVECTOR as PMPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_type_hvector__ as pmpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_type_hvector as pmpi_type_hvector
#else
#pragma _CRI duplicate mpi_type_hvector_ as pmpi_type_hvector_
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
#define mpi_type_hvector_ PMPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector
#else
#define mpi_type_hvector_ pmpi_type_hvector_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_type_hvector_ MPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_type_hvector_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_type_hvector_( MPI_Fint *count, MPI_Fint *blocklen, MPI_Fint *stride, MPI_Fint *old_type, MPI_Fint *newtype, MPI_Fint *__ierr )
{
    MPI_Aint     c_stride = (MPI_Aint)*stride;
    MPI_Datatype ldatatype;

    *__ierr = MPI_Type_hvector((int)*count, (int)*blocklen, c_stride,
                               MPI_Type_f2c(*old_type),
                               &ldatatype);
    *newtype = MPI_Type_c2f(ldatatype);
}
