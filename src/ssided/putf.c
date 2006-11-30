/* $Id$ */

/* universal fortran binding */

#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_PUT = PMPI_PUT
EXPORT_MPI_API void MPI_PUT ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, M
			      PI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_put__ = pmpi_put__
EXPORT_MPI_API void mpi_put__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_put = pmpi_put
EXPORT_MPI_API void mpi_put ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			      MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_put_ = pmpi_put_
EXPORT_MPI_API void mpi_put_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			       MPI_Fint*, MPI_Fint *, MPI_Fint * , MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_PUT ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, M
			      PI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_PUT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_put__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_put__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_put ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			      MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_put")));
#else
EXPORT_MPI_API void mpi_put_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
			       MPI_Fint*, MPI_Fint *, MPI_Fint * , MPI_Fint *) __attribute__ ((weak, alias ("pmpi_put_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_PUT  MPI_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_put__  mpi_put__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_put  mpi_put
#else
#pragma _HP_SECONDARY_DEF pmpi_put_  mpi_put_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_PUT as PMPI_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_put__ as pmpi_put__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_put as pmpi_put
#else
#pragma _CRI duplicate mpi_put_ as pmpi_put_
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
#define mpi_put_ PMPI_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_put_ pmpi_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_put_ pmpi_put
#else
#define mpi_put_ pmpi_put_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_put_ MPI_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_put_ mpi_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_put_ mpi_put
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_put_( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint *, MPI_Fint*, MPI_Fint *, MPI_Fint *, MPI_Fint *);

EXPORT_MPI_API void FORTRAN_API mpi_put_( void *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, 
					  MPI_Fint *target_rank, MPI_Fint *target_disp, MPI_Fint *target_count, 
					  MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *__ierr )
{
    *__ierr = MPI_Put(MPIR_F_PTR(origin_addr), (int)*count, MPI_Type_f2c(*origin_datatype),
		      (int)*target_rank, (int)*target_disp, (int)*target_count, 
  		      MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win), MPI_Comm_f2c(*comm));
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
