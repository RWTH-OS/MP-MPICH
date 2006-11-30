/* $Id$
 * 
 * MPID_SMI_Win_incr	- manipulates the ref_count field in win
 *
 * input parameters:
 *	win		pointer to window structure to be manipulated
 *	incr	increment value (typically +1 or -1)
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	the new ref_count value
 */


#include <stdlib.h>
#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#ifdef MPID_USE_DEVTHREADS
#  ifndef WIN32
#    include <pthread.h>
#  endif
#endif



int MPID_SMI_Win_incr (win, incr)
	MPID_Win * win;
	int incr;
{
	int	val;

	MPID_SMI_LOCK (&win->mutex);
	win->ref_count += incr;
	val = win->ref_count;
	MPID_SMI_UNLOCK (&win->mutex);
	
	return val;
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
