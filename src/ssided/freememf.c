/* $Id$ */

/* universal fortran binding */

#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"
#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FREE_MEM = PMPI_FREE_MEM
EXPORT_MPI_API void MPI_FREE_MEM (void *base, MPI_Fint *__ierr);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_free_mem__ = pmpi_free_mem__
EXPORT_MPI_API void mpi_free_mem__ (void *base, MPI_Fint *__ierr);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_free_mem = pmpi_free_mem
EXPORT_MPI_API void mpi_free_mem (void *base, MPI_Fint *__ierr);
#else
#pragma weak mpi_free_mem_ = pmpi_free_mem_
EXPORT_MPI_API void mpi_free_mem_ (void *base, MPI_Fint *__ierr);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_FREE_MEM (void *base,
	MPI_Fint *__ierr) __attribute__ ((weak, alias ("PMPI_FREE_MEM")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_free_mem__ (void *base,
	MPI_Fint *__ierr) __attribute__ ((weak, alias ("pmpi_free_mem__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_free_mem (void *base,
	MPI_Fint *__ierr) __attribute__ ((weak, alias ("pmpi_free_mem")));
#else
EXPORT_MPI_API void mpi_free_mem_ (void *base,
	MPI_Fint *__ierr) __attribute__ ((weak, alias ("pmpi_free_mem_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FREE_MEM  MPI_FREE_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_free_mem__  mpi_free_mem__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_free_mem  mpi_free_mem
#else
#pragma _HP_SECONDARY_DEF pmpi_free_mem_  mpi_free_mem_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FREE_MEM as PMPI_FREE_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_free_mem__ as pmpi_free_mem__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_free_mem as pmpi_free_mem
#else
#pragma _CRI duplicate mpi_free_mem_ as pmpi_free_mem_
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
#define mpi_free_mem_ PMPI_FREE_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_free_mem_ pmpi_free_mem__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_free_mem_ pmpi_free_mem
#else
#define mpi_free_mem_ pmpi_free_mem_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_free_mem_ MPI_FREE_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_free_mem_ mpi_free_mem__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_free_mem_ mpi_free_mem
#endif
#endif

EXPORT_MPI_API void mpi_free_mem_ (void *base, MPI_Fint *__ierr);

EXPORT_MPI_API void mpi_free_mem_ (void *base, MPI_Fint *__ierr)
{
	if (!base) {
		*__ierr = MPI_ERR_BASE;
		return;
	}

	*__ierr = MPID_Free_mem (base) ? MPI_SUCCESS : MPI_ERR_BASE;
	return;
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
