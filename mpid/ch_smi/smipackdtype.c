#include <stdlib.h>
#include "smicoll.h"
#include "packdtype.h"
#include "adi3types.h"


typedef MPID_Datatype * (unpack_dtype_t) (void *, int);
typedef int (pack_dtype_t) (MPID_Datatype *, int, void **, size_t *);
typedef MPID_Datatype * (get_known_dtype_t) (int, int);

static unpack_dtype_t		* unpack_dtype_ptr = NULL;
static pack_dtype_t			* pack_dtype_ptr = NULL;
static get_known_dtype_t	* get_known_dtype_ptr = NULL;


void
MPID_SMI_Init_pack_dtype_stubs (void)
{
#if defined MPI_SHARED_LIBS && !defined WIN32
	GET_DLL_FCTNPTR ("MPID_Unpack_dtype", unpack_dtype_ptr, unpack_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_Pack_dtype", pack_dtype_ptr, pack_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_Get_known_dtype", get_known_dtype_ptr, 
												get_known_dtype_t *);
#endif
}


MPID_Datatype *
MPID_SMI_Unpack_dtype (buf, rank)
	void	* buf;
	int		rank;
{
#if defined MPI_SHARED_LIBS && !defined WIN32
	return (*unpack_dtype_ptr) (buf, rank);
#else
	return MPID_Unpack_dtype (buf, rank);
#endif
}

int
MPID_SMI_Pack_dtype (dtype, rank, buf, size)
	MPID_Datatype	* dtype;
	int				rank;
	void			** buf;
	size_t			* size;
{
#if defined MPI_SHARED_LIBS && !defined WIN32
	return (*pack_dtype_ptr) (dtype, rank, buf, size);
#else
	return MPID_Pack_dtype (dtype, rank, buf, size);
#endif
}


MPID_Datatype *
MPID_SMI_Get_known_dtype (id, rank)
	int id, rank;
{
#if defined MPI_SHARED_LIBS && !defined WIN32
	return (*get_known_dtype_ptr) (id, rank);
#else
	return MPID_Get_known_dtype (id, rank);
#endif
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
