/* ic_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INTERCOMM_CREATE = PMPI_INTERCOMM_CREATE
EXPORT_MPI_API void MPI_INTERCOMM_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_intercomm_create__ = pmpi_intercomm_create__
EXPORT_MPI_API void mpi_intercomm_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_intercomm_create = pmpi_intercomm_create
EXPORT_MPI_API void mpi_intercomm_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_intercomm_create_ = pmpi_intercomm_create_
EXPORT_MPI_API void mpi_intercomm_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_INTERCOMM_CREATE ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_INTERCOMM_CREATE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_intercomm_create__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_create__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_intercomm_create ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_create")));
#else
EXPORT_MPI_API void mpi_intercomm_create_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_intercomm_create_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INTERCOMM_CREATE  MPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_intercomm_create__  mpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_intercomm_create  mpi_intercomm_create
#else
#pragma _HP_SECONDARY_DEF pmpi_intercomm_create_  mpi_intercomm_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INTERCOMM_CREATE as PMPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_intercomm_create__ as pmpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_intercomm_create as pmpi_intercomm_create
#else
#pragma _CRI duplicate mpi_intercomm_create_ as pmpi_intercomm_create_
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
#define mpi_intercomm_create_ PMPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_create_ pmpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_create_ pmpi_intercomm_create
#else
#define mpi_intercomm_create_ pmpi_intercomm_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_intercomm_create_ MPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_create_ mpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_create_ mpi_intercomm_create
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_intercomm_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_intercomm_create_ ( MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *peer_comm, 
                           MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *comm_out, MPI_Fint *__ierr )
{
    MPI_Comm l_comm_out;
    *__ierr = MPI_Intercomm_create( MPI_Comm_f2c(*local_comm), 
                                    (int)*local_leader, 
                                    MPI_Comm_f2c(*peer_comm), 
                                    (int)*remote_leader, (int)*tag,
				    &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
