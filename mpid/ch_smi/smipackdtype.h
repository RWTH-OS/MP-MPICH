#ifndef MPID_SMI_PACKDTYPE_H
#define MPID_SMI_PACKDTYPE_H

#include "adi3types.h"
#include "packdtype.h"

void MPID_SMI_Init_pack_dtype_stubs (void);
MPID_Datatype * MPID_SMI_Unpack_dtype (void * buf, int rank);
int MPID_SMI_Pack_dtype (	MPID_Datatype * dtype, int rank, 
							void ** buf, size_t * size);
MPID_Datatype * MPID_SMI_Get_known_dtype (int id, int rank);














#endif	/* MPID_SMI_PACKDTYPE_H */


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
