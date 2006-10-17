/* iprobe.c */
/* Custom Fortran interface file  */
#include "mpiimpl.h"
#include "mpifort.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_IPROBE = PMPI_IPROBE
EXPORT_MPI_API void MPI_IPROBE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_iprobe__ = pmpi_iprobe__
EXPORT_MPI_API void mpi_iprobe__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_iprobe = pmpi_iprobe
EXPORT_MPI_API void mpi_iprobe ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_iprobe_ = pmpi_iprobe_
EXPORT_MPI_API void mpi_iprobe_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_IPROBE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_IPROBE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_iprobe__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_iprobe__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_iprobe ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_iprobe")));
#else
EXPORT_MPI_API void mpi_iprobe_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_iprobe_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_IPROBE  MPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_iprobe__  mpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_iprobe  mpi_iprobe
#else
#pragma _HP_SECONDARY_DEF pmpi_iprobe_  mpi_iprobe_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_IPROBE as PMPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_iprobe__ as pmpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_iprobe as pmpi_iprobe
#else
#pragma _CRI duplicate mpi_iprobe_ as pmpi_iprobe_
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
#define mpi_iprobe_ PMPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe
#else
#define mpi_iprobe_ pmpi_iprobe_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_iprobe_ MPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_iprobe_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                             MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_iprobe_( MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *__ierr )
{
    int lflag;
    MPI_Status c_status;

    *__ierr = MPI_Iprobe((int)*source,(int)*tag,MPI_Comm_f2c(*comm),
                         &lflag,&c_status);
    *flag = MPIR_TO_FLOG(lflag);
    MPI_Status_c2f(&c_status, status);
}
