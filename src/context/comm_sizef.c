/* comm_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

  #if defined(HAVE_WEAK_SYMBOLS)
    #if defined(HAVE_PRAGMA_WEAK)
      #if defined(FORTRANCAPS)
        #pragma weak MPI_COMM_SIZE = PMPI_COMM_SIZE
        EXPORT_MPI_API void MPI_COMM_SIZE ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma weak mpi_comm_size__ = pmpi_comm_size__
        EXPORT_MPI_API void mpi_comm_size__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
      #elif !defined(FORTRANUNDERSCORE)
        #pragma weak mpi_comm_size = pmpi_comm_size
        EXPORT_MPI_API void mpi_comm_size ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
      #else
        #pragma weak mpi_comm_size_ = pmpi_comm_size_
        EXPORT_MPI_API void mpi_comm_size_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
      #endif

    #elif defined(HAVE_ATTRIBUTE_WEAK)
      #if defined(FORTRANCAPS)
        EXPORT_MPI_API void MPI_COMM_SIZE ( MPI_Fint *, MPI_Fint *,
			MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_SIZE")));
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        EXPORT_MPI_API void mpi_comm_size__ ( MPI_Fint *, MPI_Fint *,
			MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_size__")));
      #elif !defined(FORTRANUNDERSCORE)
        EXPORT_MPI_API void mpi_comm_size ( MPI_Fint *, MPI_Fint *,
			MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_size")));
      #else
        EXPORT_MPI_API void mpi_comm_size_ ( MPI_Fint *, MPI_Fint *,
			MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_size_")));
      #endif

    #elif defined(HAVE_PRAGMA_HP_SEC_DEF)
      #if defined(FORTRANCAPS)
        #pragma _HP_SECONDARY_DEF PMPI_COMM_SIZE  MPI_COMM_SIZE
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma _HP_SECONDARY_DEF pmpi_comm_size__  mpi_comm_size__
      #elif !defined(FORTRANUNDERSCORE)
        #pragma _HP_SECONDARY_DEF pmpi_comm_size  mpi_comm_size
      #else
        #pragma _HP_SECONDARY_DEF pmpi_comm_size_  mpi_comm_size_
      #endif

    #elif defined(HAVE_PRAGMA_CRI_DUP)
      #if defined(FORTRANCAPS)
        #pragma _CRI duplicate MPI_COMM_SIZE as PMPI_COMM_SIZE
      #elif defined(FORTRANDOUBLEUNDERSCORE)
        #pragma _CRI duplicate mpi_comm_size__ as pmpi_comm_size__
      #elif !defined(FORTRANUNDERSCORE)
        #pragma _CRI duplicate mpi_comm_size as pmpi_comm_size
      #else
        #pragma _CRI duplicate mpi_comm_size_ as pmpi_comm_size_
      #endif

/* end of weak pragmas */
    #endif /* #if defined(HAVE_PRAGMA_WEAK)*/

/* Include mapping from MPI->PMPI */
    #include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
    #undef __MPI_BINDINGS
    #include "binding.h"
  #endif /*#if defined(HAVE_WEAK_SYMBOLS)*/

  #ifdef FORTRANCAPS
    #define mpi_comm_size_ PMPI_COMM_SIZE
  #elif defined(FORTRANDOUBLEUNDERSCORE)
    #define mpi_comm_size_ pmpi_comm_size__
  #elif !defined(FORTRANUNDERSCORE)
    #define mpi_comm_size_ pmpi_comm_size
  #else
    #define mpi_comm_size_ pmpi_comm_size_
  #endif

#else  /*#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)*/

  #ifdef FORTRANCAPS
    #define mpi_comm_size_ MPI_COMM_SIZE
  #elif defined(FORTRANDOUBLEUNDERSCORE)
    #define mpi_comm_size_ mpi_comm_size__
  #elif !defined(FORTRANUNDERSCORE)
    #define mpi_comm_size_ mpi_comm_size
  #endif
#endif /*#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)*/


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_size_ ( MPI_Fint *comm, MPI_Fint *size, MPI_Fint *__ierr )
{
    int l_size;

    *__ierr = MPI_Comm_size( MPI_Comm_f2c(*comm), &l_size );
    *size = (MPI_Fint)l_size;
}

