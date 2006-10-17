#ifndef MPID_SMI_SSIDE_SETUP_H
#define MPID_SMI_SSIDE_SETUP_H

#include "mpid.h"
#include "smidev.h"
#include "dev_smi.h"
#include "adi3types.h"
#include "sside_protocol.h"





MPID_Sside_protocol * MPID_SMI_Sside_setup (void);

void MPID_SMI_Sside_destroy (MPID_Sside_protocol *sside);




















#endif 	/* MPID_SMI_SSIDE_SETUP_H */



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
