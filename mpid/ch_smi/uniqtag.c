/* $Id: uniqtag.c,v 1.12.8.1 2004/12/15 13:45:21 boris Exp $ */

#include <stdlib.h>

#include "mpiimpl.h"
#include "smidev.h"
#include "smi.h"

#include "uniqtag.h"
#include "mmu.h"

static int			mutex;
static volatile int	*lasttag = NULL;



int MPID_SMI_Uniq_tag_init (void)
{
	SMIcall (SMI_Mutex_init (&mutex));
	if (MPID_SMI_myid == 0) {
		lasttag = (int *)&(MPID_SMI_Int_info_exp[0]->Sside_tag_cnt);
		*lasttag = MPID_SMI_INTERNAL_START_TAG;
	} else {
		lasttag = (int *)&(MPID_SMI_Int_info_imp[0]->Sside_tag_cnt);
	}
		
	return 1;
}


void MPID_SMI_Uniq_tag_destroy (void)
{
	if (!lasttag) 
		return;
	SMIcall (SMI_Mutex_destroy (mutex));
	lasttag = NULL;
}
	

int MPID_SMI_Get_uniq_tag (void)
{
	int	tag;

	SMIcall (SMI_Mutex_lock (mutex));

	do {
		SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
		tag = *lasttag + 1;
	} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
	/* wrap around */
	if (tag >= MPID_SMI_INTERNAL_TAG_UB)
		tag = MPID_SMI_INTERNAL_START_TAG;
	do {
		*lasttag = tag;
#if 0
		SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));
#endif
	} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);

	SMIcall (SMI_Mutex_unlock (mutex));

	return tag;
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
