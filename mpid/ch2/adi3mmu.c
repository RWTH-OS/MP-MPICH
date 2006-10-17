/* $Id$

   Memory allocation via the device. */

#include <stdio.h>
#include <stdlib.h>
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpidmpi.h"
#include "sside_protocol.h"
#include "adi3types.h"




void *MPID_Alloc_mem (size, info)
	size_t 		size;
	MPID_Info 	* info;
{
	MPID_Device	* dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->alloc_mem != NULL) {
		return MPID_Device_call_alloc_mem (size, info, dev);
	} else {
		return malloc(size);
	}
}


int MPID_Free_mem (ptr)
	void * ptr;
{
	MPID_Device	* dev = MPID_devset->dev[MPID_MyWorldRank];

	if (dev->alloc_mem != NULL) {
		return MPID_Device_call_free_mem (ptr, dev);
	} else {
		free(ptr);
		return 0;
	}
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
