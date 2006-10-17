


#include <stdlib.h>
#include <mpi.h>
#include <smi.h>
#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#ifdef MPID_USE_DEVTHREADS
#  ifndef WIN32
#    include <pthread.h>
#  endif
#endif
#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"


















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
