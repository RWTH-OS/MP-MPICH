/* special declarations needed by various machine or OS environments */

/* For POSIX standard versions of Unix */
#if defined(HAVE_UNISTD_H)
#    include <unistd.h>
#endif

/* HP-Convex spp */



/* At the moment these are machine independent, since they are used only to
 * keep locks apart, but this might change, hence they are in this file.
 */
#ifndef MPID_CACHE_LINE_SIZE
#    define MPID_CACHE_LINE_SIZE 128
#    define MPID_CACHE_LINE_LOG_SIZE 7
#endif

