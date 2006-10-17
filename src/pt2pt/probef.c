/* probe.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_PROBE = PMPI_PROBE
EXPORT_MPI_API void MPI_PROBE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_probe__ = pmpi_probe__
EXPORT_MPI_API void mpi_probe__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_probe = pmpi_probe
EXPORT_MPI_API void mpi_probe ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_probe_ = pmpi_probe_
EXPORT_MPI_API void mpi_probe_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_PROBE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_PROBE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_probe__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_probe__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_probe ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_probe")));
#else
EXPORT_MPI_API void mpi_probe_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_probe_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_PROBE  MPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_probe__  mpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_probe  mpi_probe
#else
#pragma _HP_SECONDARY_DEF pmpi_probe_  mpi_probe_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_PROBE as PMPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_probe__ as pmpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_probe as pmpi_probe
#else
#pragma _CRI duplicate mpi_probe_ as pmpi_probe_
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
#define mpi_probe_ PMPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ pmpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ pmpi_probe
#else
#define mpi_probe_ pmpi_probe_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_probe_ MPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ mpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ mpi_probe
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_probe_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                            MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_probe_( MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;
   
    *__ierr = MPI_Probe((int)*source, (int)*tag, MPI_Comm_f2c(*comm),
                        &c_status);
    MPI_Status_c2f(&c_status, status);
    
}
