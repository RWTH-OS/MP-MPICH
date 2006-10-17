/* comm_group.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_GROUP = PMPI_COMM_GROUP
EXPORT_MPI_API void MPI_COMM_GROUP ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_group__ = pmpi_comm_group__
EXPORT_MPI_API void mpi_comm_group__ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_group = pmpi_comm_group
EXPORT_MPI_API void mpi_comm_group ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_comm_group_ = pmpi_comm_group_
EXPORT_MPI_API void mpi_comm_group_ ( MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_GROUP ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_COMM_GROUP")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_group__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_group__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_group ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_group")));
#else
EXPORT_MPI_API void mpi_comm_group_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_comm_group_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_GROUP  MPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_group__  mpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_group  mpi_comm_group
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_group_  mpi_comm_group_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_GROUP as PMPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_group__ as pmpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_group as pmpi_comm_group
#else
#pragma _CRI duplicate mpi_comm_group_ as pmpi_comm_group_
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
#define mpi_comm_group_ PMPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_group_ pmpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_group_ pmpi_comm_group
#else
#define mpi_comm_group_ pmpi_comm_group_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_group_ MPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_group_ mpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_group_ mpi_comm_group
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_group_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_group_ ( MPI_Fint *comm, MPI_Fint *group, MPI_Fint *__ierr )
{
    MPI_Group l_group;

    *__ierr = MPI_Comm_group( MPI_Comm_f2c(*comm), &l_group);
    *group = MPI_Group_c2f(l_group);
}
