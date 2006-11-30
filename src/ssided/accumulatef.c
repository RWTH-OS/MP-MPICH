/* $Id$ */


/* universal fortran binding */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ACCUMULATE = PMPI_ACCUMULATE
EXPORT_MPI_API void MPI_ACCUMULATE ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									 MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_accumulate__ = pmpi_accumulate__
EXPORT_MPI_API void mpi_accumulate__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									   MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_accumulate = pmpi_accumulate
EXPORT_MPI_API void mpi_accumulate ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									 MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#else
#pragma weak mpi_accumulate_ = pmpi_accumulate_
EXPORT_MPI_API void mpi_accumulate_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									  MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);
#endif /* FORTRANCAPS */

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ACCUMULATE ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									 MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("PMPI_ACCUMULATE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_accumulate__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									   MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_accumulate__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_accumulate ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									 MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_accumulate")));
#else
EXPORT_MPI_API void mpi_accumulate_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
									  MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *) __attribute__ ((weak, alias ("pmpi_accumulate_")));
#endif /* FORTRANCAPS */

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ACCUMULATE  MPI_ACCUMULATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_accumulate__  mpi_accumulate__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_accumulate  mpi_accumulate
#else
#pragma _HP_SECONDARY_DEF pmpi_accumulate_  mpi_accumulate_
#endif /* FORTRANCAPS */

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ACCUMULATE as PMPI_ACCUMULATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_accumulate__ as pmpi_accumulate__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_accumulate as pmpi_accumulate
#else
#pragma _CRI duplicate mpi_accumulate_ as pmpi_accumulate_
#endif /* FORTRANCAPS */

/* end of weak pragmas */
#endif  /* HAVE_PRAGMA_WEAK */

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif /* HAVE_WEAK_SYMBOLS */

#ifdef FORTRANCAPS
#define mpi_accumulate_ PMPI_ACCUMULATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_accumulate_ pmpi_accumulate__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_accumulate_ pmpi_accumulate
#else
#define mpi_accumulate_ pmpi_accumulate_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_accumulate_ MPI_ACCUMULATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_accumulate_ mpi_accumulate__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_accumulate_ mpi_accumulate
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_accumulate_( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
												 MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);
EXPORT_MPI_API void FORTRAN_API 
mpi_accumulate_ (void *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, 
				 MPI_Fint *target_rank, MPI_Fint *target_disp, MPI_Fint *target_count, 
				 MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *op, MPI_Fint *__ierr)
{
	*__ierr = MPI_Accumulate (MPIR_F_PTR(origin_addr), (int)*count, MPI_Type_f2c(*origin_datatype),
							  (int)*target_rank, (int)*target_disp, (int)*target_count, 
							  MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win), 
							  MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
}









/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
