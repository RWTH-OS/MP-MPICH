/* $Id$ */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"
#include "adi3types.h"
#include "mpid.h"
#include "sbcnst2.h"

#include "smidev.h"
#include "dev_smi.h"
#include "ssidesetup.h"
#include "smimem.h"
#include "uniqtag.h"
#include "smiostypes.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int initialized = 0;

#define INIT_OSTA_ENTRIES 100
#define INCR_OSTA_ENTRIES 50

MPID_SBHeader MPID_SMI_os_ta_allocator;


MPID_Sside_protocol *MPID_SMI_Sside_setup (void)
{
	MPID_Sside_protocol * sside;

	if (MPID_SMI_cfg.SSIDED_ENABLED == 0)
		return NULL;

	sside = (MPID_Sside_protocol *) MALLOC (sizeof (MPID_Sside_protocol));
	if (!sside) 
		return NULL;
	
	sside->Win_create 	= MPID_SMI_Win_create;
	sside->Win_free 	= MPID_SMI_Win_free;
	sside->Win_incr 	= MPID_SMI_Win_incr;
	sside->Win_lock 	= MPID_SMI_Win_lock;
	sside->Win_unlock 	= MPID_SMI_Win_unlock;
	sside->Win_sync 	= MPID_SMI_Win_sync;
	sside->Put_sametype = MPID_SMI_Put_sametype;
	sside->Get_sametype = MPID_SMI_Get_sametype;
	sside->Put_contig 	= MPID_SMI_Put_contig;
	sside->Get_contig 	= MPID_SMI_Get_contig;
	sside->Rhcv 		= MPID_SMI_Rhcv;

	if (!initialized) {
		/* call other init routines */
		if (!MPID_SMI_Uniq_tag_init ()) {
			FREE (sside);
			return NULL;
		}
		MPID_SMI_Init_sendrecv_stubs();
		MPID_SMI_Init_pack_dtype_stubs();

		MPID_SMI_os_ta_allocator = MPID_SBinit (sizeof(MPID_SMI_Delayed_ta_t), 
												INIT_OSTA_ENTRIES, INCR_OSTA_ENTRIES);

		initialized = 1;
	}

	return sside;
}


void MPID_SMI_Sside_destroy (MPID_Sside_protocol *sside)
{
	if (!initialized || MPID_SMI_cfg.SSIDED_ENABLED == 0)
		return;

	MPID_SMI_Uniq_tag_destroy ();
	MPID_SBdestroy (MPID_SMI_os_ta_allocator);

	FREE(sside);
	initialized = 0;

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
