/* $Id: freemem.c,v 1.7 2003/06/13 16:20:42 joachim Exp $
 *
 * MPI_Free_mem		frees a memory prior allocated by MPI_Alloc_mem
 *
 * input parameters:
 *	base		pointer to memory te be freed
 *
 * output parameters:
 *	<none>
 */

#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Free_mem = PMPI_Free_mem
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Free_mem(void *baseptr) __attribute__ ((weak, alias ("PMPI_Free_mem")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Free_mem  MPI_Free_mem
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Free_mem as PMPI_Free_mem
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


EXPORT_MPI_API int MPI_Free_mem (void *base)
{
	if (!base) {
		return MPI_ERR_BASE;
	}

	return MPID_Free_mem (base) ? MPI_SUCCESS : MPI_ERR_BASE;
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
