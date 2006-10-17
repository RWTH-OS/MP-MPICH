/* pack_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_PACK_SIZE = PMPI_PACK_SIZE
EXPORT_MPI_API void MPI_PACK_SIZE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_pack_size__ = pmpi_pack_size__
EXPORT_MPI_API void mpi_pack_size__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_pack_size = pmpi_pack_size
EXPORT_MPI_API void mpi_pack_size ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_pack_size_ = pmpi_pack_size_
EXPORT_MPI_API void mpi_pack_size_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_PACK_SIZE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_PACK_SIZE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_pack_size__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pack_size__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_pack_size ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pack_size")));
#else
EXPORT_MPI_API void mpi_pack_size_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_pack_size_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_PACK_SIZE  MPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_pack_size__  mpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_pack_size  mpi_pack_size
#else
#pragma _HP_SECONDARY_DEF pmpi_pack_size_  mpi_pack_size_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_PACK_SIZE as PMPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_pack_size__ as pmpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_pack_size as pmpi_pack_size
#else
#pragma _CRI duplicate mpi_pack_size_ as pmpi_pack_size_
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
#define mpi_pack_size_ PMPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size
#else
#define mpi_pack_size_ pmpi_pack_size_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_pack_size_ MPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_pack_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_pack_size_ ( MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *__ierr )
{
    int lsize;

    *__ierr = MPI_Pack_size((int)*incount, MPI_Type_f2c(*datatype),
                            MPI_Comm_f2c(*comm), &lsize);
    *size = (MPI_Fint)lsize;
}
