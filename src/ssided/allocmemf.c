/* $Id$ */

/* universal fortran binding */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ALLOC_MEM = PMPI_ALLOC_MEM
EXPORT_MPI_API void MPI_ALLOC_MEM ( MPI_Fint *, MPI_Fint *, void *, MPI_Fint *);
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_alloc_mem__ = pmpi_alloc_mem__
EXPORT_MPI_API void mpi_alloc_mem__ ( MPI_Fint *, MPI_Fint *, void *, MPI_Fint *);
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_alloc_mem = pmpi_alloc_mem
EXPORT_MPI_API void mpi_alloc_mem ( MPI_Fint *, MPI_Fint *, void *, MPI_Fint *);
#else
#pragma weak mpi_alloc_mem_ = pmpi_alloc_mem_
EXPORT_MPI_API void mpi_alloc_mem_ ( MPI_Fint *, MPI_Fint *, void *, MPI_Fint *);
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ALLOC_MEM ( MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *) __attribute__ ((weak, alias ("PMPI_ALLOC_MEM")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_alloc_mem__ ( MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_alloc_mem__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_alloc_mem ( MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_alloc_mem")));
#else
EXPORT_MPI_API void mpi_alloc_mem_ ( MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *) __attribute__ ((weak, alias ("pmpi_alloc_mem_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ALLOC_MEM  MPI_ALLOC_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_alloc_mem__  mpi_alloc_mem__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_alloc_mem  mpi_alloc_mem
#else
#pragma _HP_SECONDARY_DEF pmpi_alloc_mem_  mpi_alloc_mem_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ALLOC_MEM as PMPI_ALLOC_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_alloc_mem__ as pmpi_alloc_mem__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_alloc_mem as pmpi_alloc_mem
#else
#pragma _CRI duplicate mpi_alloc_mem_ as pmpi_alloc_mem_
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
#define mpi_alloc_mem_ PMPI_ALLOC_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alloc_mem_ pmpi_alloc_mem__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alloc_mem_ pmpi_alloc_mem
#else
#define mpi_alloc_mem_ pmpi_alloc_mem_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_alloc_mem_ MPI_ALLOC_MEM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alloc_mem_ mpi_alloc_mem__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alloc_mem_ mpi_alloc_mem
#endif
#endif

EXPORT_MPI_API
void MPI_Alloc_mem (MPI_Fint *size, MPI_Fint *info, void *baseptr, MPI_Fint *__ierr)
{
	*__ierr = MPI_Alloc_mem((size_t)*size, (MPI_Info)*info, baseptr);
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
