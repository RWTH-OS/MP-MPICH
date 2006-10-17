/* dup_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#undef MPI_DUP_FN

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

  #if defined(HAVE_WEAK_SYMBOLS)

    #if defined(HAVE_PRAGMA_WEAK)
      #if defined(FORTRANCAPS)
        #pragma weak MPI_DUP_FN = PMPI_DUP_FN
        EXPORT_MPI_API void MPI_DUP_FN ( MPI_Fint, MPI_Fint *, void *, void **, void **, MPI_Fint * );
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma weak mpi_dup_fn__ = pmpi_dup_fn__
        EXPORT_MPI_API void mpi_dup_fn__ ( MPI_Fint, MPI_Fint *, void *, void **, void **, MPI_Fint * );
      #elif !defined(FORTRANUNDERSCORE)
        #pragma weak mpi_dup_fn = pmpi_dup_fn
        EXPORT_MPI_API void mpi_dup_fn ( MPI_Fint, MPI_Fint *, void *, void **, void **, MPI_Fint * );
      #else
        #pragma weak mpi_dup_fn_ = pmpi_dup_fn_
        EXPORT_MPI_API void mpi_dup_fn_ ( MPI_Fint, MPI_Fint *, void *, void **, void **, MPI_Fint * );
      #endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_DUP_FN ( MPI_Fint, MPI_Fint *, void *, void **, void **,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_DUP_FN")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_dup_fn__ ( MPI_Fint, MPI_Fint *, void *, void **,
	void **, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dup_fn__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_dup_fn ( MPI_Fint, MPI_Fint *, void *, void **,
	void **, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dup_fn")));
#else
EXPORT_MPI_API void mpi_dup_fn_ ( MPI_Fint, MPI_Fint *, void *, void **,
	void **, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_dup_fn_")));
#endif

    #elif defined(HAVE_PRAGMA_HP_SEC_DEF)
      #if defined(FORTRANCAPS)
        #pragma _HP_SECONDARY_DEF PMPI_DUP_FN  MPI_DUP_FN
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma _HP_SECONDARY_DEF pmpi_dup_fn__  mpi_dup_fn__
      #elif !defined(FORTRANUNDERSCORE)
        #pragma _HP_SECONDARY_DEF pmpi_dup_fn  mpi_dup_fn
      #else
        #pragma _HP_SECONDARY_DEF pmpi_dup_fn_  mpi_dup_fn_
      #endif

    #elif defined(HAVE_PRAGMA_CRI_DUP)
      #if defined(FORTRANCAPS)
        #pragma _CRI duplicate MPI_DUP_FN as PMPI_DUP_FN
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma _CRI duplicate mpi_dup_fn__ as pmpi_dup_fn__
      #elif !defined(FORTRANUNDERSCORE)
        #pragma _CRI duplicate mpi_dup_fn as pmpi_dup_fn
      #else
        #pragma _CRI duplicate mpi_dup_fn_ as pmpi_dup_fn_
    #endif

/* end of weak pragmas */
  #endif /*#if defined(HAVE_WEAK_SYMBOLS)*/

/* Include mapping from MPI->PMPI */
  #include "mpiprof.h"
  #endif


  #ifdef FORTRANCAPS
    #define mpi_dup_fn_ PMPI_DUP_FN
  #elif defined(FORTRANDOUBLEUNDERSCORE)
    #define mpi_dup_fn_ pmpi_dup_fn__
  #elif !defined(FORTRANUNDERSCORE)
    #define mpi_dup_fn_ pmpi_dup_fn
  #else
    #define mpi_dup_fn_ pmpi_dup_fn_
  #endif

#else /*#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)*/

  #ifdef FORTRANCAPS
    #define mpi_dup_fn_ MPI_DUP_FN
  #elif defined(FORTRANDOUBLEUNDERSCORE)
    #define mpi_dup_fn_ mpi_dup_fn__
  #elif !defined(FORTRANUNDERSCORE)
    #define mpi_dup_fn_ mpi_dup_fn
  #endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_dup_fn_ ( MPI_Fint, MPI_Fint *, void *, void **, void **, 
			     MPI_Fint * );

/* Fortran functions aren't quite the same */
EXPORT_MPI_API void FORTRAN_API mpi_dup_fn_ ( MPI_Fint comm, MPI_Fint *keyval, void *extra_state, void **attr_in, void **attr_out, MPI_Fint *flag )
{
    int l_flag;

    MPIR_dup_fn(MPI_Comm_f2c(comm), (int)*keyval, extra_state, *attr_in,
                attr_out, &l_flag);
    *flag = MPIR_TO_FLOG(l_flag);

}
