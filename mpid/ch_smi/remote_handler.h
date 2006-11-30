/* $Id$ */

#ifndef MPID_SMI_SSIDE_REMOTE_HANDLER_H
#define MPID_SMI_SSIDE_REMOTE_HANDLER_H


#include "smipackets.h"
#include "mpiimpl.h"
#include "adi3types.h"




int MPID_SMI_Do_put_emulation (MPID_PKT_T * pkt, int from_drank);
int MPID_SMI_Do_get_emulation (MPID_PKT_T * pkt, int from_drank);
int MPID_SMI_Do_accumulate (MPID_PKT_T * pkt, int from_drank);

int MPID_SMI_Walk_thru_dtype_and_accu (	MPID_Datatype *, void *, 
										void *, MPI_Op);




















#endif	/* MPID_SMI_SSIDE_REMOTE_HANDLER_H */


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
