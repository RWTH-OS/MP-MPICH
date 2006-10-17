/* $Id: getf.c,v 1.2 2003/06/13 17:24:24 rainer Exp $ */

/* universal fortran binding */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GET = PMPI_GET
EXPORT_MPI_API void MPI_GET ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, M
			      PI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_get__ = pmpi_get__
EXPORT_MPI_API void mpi_get__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_get = pmpi_get
EXPORT_MPI_API void mpi_get ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			      MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_get_ = pmpi_get_
EXPORT_MPI_API void mpi_get_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			       MPI_Fint*, MPI_Fint *, MPI_Fint * , MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GET ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, M
			      PI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_get__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_get ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			      MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get")));
#else
EXPORT_MPI_API void mpi_get_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			       MPI_Fint*, MPI_Fint *, MPI_Fint * , MPI_Fint *) __attribute__ ((weak, alias ("pmpi_get_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GET  MPI_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get__  mpi_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get  mpi_get
#else
#pragma _HP_SECONDARY_DEF pmpi_get_  mpi_get_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GET as PMPI_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_get__ as pmpi_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_get as pmpi_get
#else
#pragma _CRI duplicate mpi_get_ as pmpi_get_
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
#define mpi_get_ PMPI_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_ pmpi_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_ pmpi_get
#else
#define mpi_get_ pmpi_get_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_get_ MPI_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_ mpi_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_ mpi_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_get_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);

EXPORT_MPI_API void FORTRAN_API mpi_get_( void *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, 
					  MPI_Fint *target_rank, MPI_Fint *target_disp, MPI_Fint *target_count, 
					  MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *__ierr )
{
    *__ierr = MPI_Get(MPIR_F_PTR(origin_addr), (int)*count, MPI_Type_f2c(*origin_datatype),
		      (int)*target_rank, (int)*target_disp, (int)*target_count, 
  		      (int)*target_datatype, (MPI_Win)*win, MPI_Comm_f2c(*comm));
}
#endif
