#if defined(HAVE_MPICHCONF_H) && !defined(MPICHCONF_INC)
/* This includes the definitions found by configure, and can be found in
   the library directory (lib/$ARCH/$COMM) corresponding to this configuration
 */
#define MPICHCONF_INC
#include "mpichconf.h"
#endif
#include "mpid_time.h"

#ifndef MPID_CH_Wtime

#include <stdlib.h>

#if defined(HAVE_GETTIMEOFDAY) || defined(USE_WIERDGETTIMEOFDAY) || \
    defined(HAVE_BSDGETTIMEOFDAY)
#include <sys/types.h>
#include <sys/time.h>
#endif
 
#ifdef MPI_LINUX
#include <malloc.h>
#endif

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <wtypes.h>
#include <winbase.h>
static LARGE_INTEGER TimerFrequ;
static int TimeInitialized=0;
#endif

/* function ptr to timing-function in a device */
double (*MPID_dev_wtime)(void) = 0;

void MPID_CH_Wtime( double *seconds )
{
    if (MPID_dev_wtime != 0) {
	*seconds = MPID_dev_wtime();
	return ;
    }
#if defined(WIN32)
    {
    	LARGE_INTEGER t;
	if(!TimeInitialized) {
	    QueryPerformanceFrequency(&TimerFrequ);
	    TimeInitialized = 1;
	}
	QueryPerformanceCounter(&t);
	*seconds = (double)t.QuadPart/(double)TimerFrequ.QuadPart;
    }
#elif defined(USE_ALPHA_CYCLE_COUNTER)
/* Code from LinuxJournal #42 (Oct-97), p50; 
   thanks to Dave Covey dnc@gi.alaska.edu
   Untested; we don't have a Linux alpha
   Also 
 */
   {
    unsigned long cc
    asm volatile( "rpcc %0" : "=r"(cc) : : "memory" );
    /* Convert to time.  Scale cc by 1024 incase it would overflow a double;
       consider using long double as well */
    *seconds = 1024.0 * ((double)(cc/1024) / (double)CLOCK_FREQ_HZ);
   }
#elif defined(HAVE_BSDGETTIMEOFDAY)
    {
    struct timeval tp;
    struct timezone tzp;

    BSDgettimeofday(&tp,&tzp);
    *seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
    }
#elif defined(USE_WIERDGETTIMEOFDAY)
   {
    /* This is for Solaris, where they decided to change the CALLING
       SEQUENCE OF gettimeofday! (Solaris 2.3 and 2.4 only?) */
    struct timeval tp;

    gettimeofday(&tp);
    *seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
   }
#elif defined (MPI_solaris) || defined (MPI_solaris86)
    {
	hrtime_t hrtime;
	hrtime = gethrtime();
	/* hrtime returns nanoseconds */
	*seconds = ((double)hrtime)/1e+9;
    }
#elif defined(HAVE_GETTIMEOFDAY)
    {
	struct timeval tp;
	struct timezone tzp;

	gettimeofday(&tp,&tzp);
	*seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
    }
#else
   /* this should never happen! */
    *seconds = 0;
#endif
}
#endif

