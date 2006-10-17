/* $Id: packdtype.h,v 1.2 2002/05/17 16:17:53 joachim Exp $ */

#ifndef MPID_PACKDTYPE_H
#define MPID_PACKDTYPE_H

#include "adi3types.h"


int MPID_Pack_dtype (MPID_Datatype *, int, void **, size_t *);
MPID_Datatype * MPID_Unpack_dtype (void *, int);

MPID_Datatype * MPID_Get_known_dtype (int, int);
 
#define MPID_IS_KNOWN_DTYPE(dtype,rank) \
			((dtype) && ((dtype)->basic || \
			(dtype)->known && (dtype)->known[rank]))

#define MPID_GET_KNOWN_DTYPE_ID(dtype) (dtype ? dtype->self : 0)

#define MPID_DTYPE_CONTIG	-1
#define MPID_DTYPE_UNKNOWN	-2
#define MPID_DTYPE_KNOWN	-3











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
