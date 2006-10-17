/* $Id: sside_protocol.h,v 1.3 2002/03/28 16:30:58 joachim Exp $ */

/* Protocol interface for one-sided (aka single-sided) communication. */

#ifndef MPID_SSIDE_PROTOCOL
#define MPID_SSIDE_PROTOCOL

#include "adi3types.h"


typedef struct _MPID_Sside_protocol {
	MPID_Win * (*Win_create) (void *, size_t, int, MPID_Info *, MPID_Comm *);
	int (*Win_free) (MPID_Win *);
	int (*Win_incr) (MPID_Win *, int);
	int (*Win_lock) (MPID_Win *, int, int);
	int (*Win_unlock) (MPID_Win *, int);
	int (*Win_sync) (int, MPID_Win *, MPID_Group *, int);

	int (*Put_sametype) (	void *, int, MPID_Datatype *, MPI_Aint, int, 
							MPID_Win *, volatile int *, MPI_Aint);
	int (*Get_sametype) (	void *, int, MPID_Datatype *, MPI_Aint, int, 
							MPID_Win *, volatile int *, MPI_Aint);
	int (*Put_contig) (	void *, int, MPI_Aint, int, MPID_Win *, 
						volatile int *, MPI_Aint);
	int (*Get_contig) (	void *, int, MPI_Aint, int, MPID_Win *, 
						volatile int *, MPI_Aint);
    int (*Rhcv) (	int, MPID_Win *, MPID_Handler_id, 
					const struct iovec *, int, int *);
} MPID_Sside_protocol;



















#endif	/* MPID_SSIDE_PROTOCOL */

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
