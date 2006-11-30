/* $Id$
 *
 * MPI_Alloc_mem	allocates memory to be used by communication windows
 *					the use of allocated memory is strongly recommanded
 *
 * input parameters:
 *	size		size of memory to be allocated (pos. integer)
 *	info		info argument (handle)
 *
 * output parameters:
 *	baseptr		a pointer to the allocated memory or NULL on error
 */


#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "adi3types.h"
#include "sside.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Alloc_mem = PMPI_Alloc_mem
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Alloc_mem(MPI_Aint size, MPI_Info info, void *baseptr) __attribute__ ((weak, alias ("PMPI_Alloc_mem")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Alloc_mem  MPI_Alloc_mem
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Alloc_mem as PMPI_Alloc_mem
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif


static MPI_Info malloc_info = MPI_INFO_NULL;
static char malloc_info_value[32];

/* malloc() wrapper function */
void *_mpi_malloc(size_t size)
{
	void *base;

	if (malloc_info == MPI_INFO_NULL) {
		MPI_Info_create (&malloc_info);
		MPI_Info_set (malloc_info, "type", "private");
		sprintf(malloc_info_value, "%d", MPI_MALLOC_ALIGNMENT);
		MPI_Info_set (malloc_info, "alignment", malloc_info_value);
	}
	
	MPI_Alloc_mem (size, malloc_info, &base);
	return base;
}


EXPORT_MPI_API int MPI_Alloc_mem (size, info, baseptr)
	MPI_Aint 	size;
	MPI_Info 	info;
	void 		* baseptr;		/* Keep in mind, that this is a void** 
									using a void** would cause casting
									problems, if the user says &a and a is  
									not a void* but an int*
								 */
{
	MPID_Info * info_ptr = MPID_GET_INFO_PTR (info);

	if (size < 0 || baseptr == NULL) {
		return MPI_ERR_ARG;
	}
	info_ptr = MPID_GET_INFO_PTR (info);
	if (MPID_TEST_INFO_NOTOK (info, info_ptr)) {
		return MPI_ERR_INFO;
	}

	*(void **)baseptr = MPID_Alloc_mem ((size_t) size, info_ptr);
	return *(void **)baseptr ? MPI_SUCCESS : MPI_ERR_NO_MEM;
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
