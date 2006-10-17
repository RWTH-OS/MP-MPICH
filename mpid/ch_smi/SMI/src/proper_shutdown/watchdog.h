/* $Id$ */

#ifndef _SMI_WATCHDOG_H_
#define _SMI_WATCHDOG_H_

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include "env/general_definitions.h"

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#define WD_RUNNING     1
#define WD_ABORTED     2
#define WD_SIGERROR    4   /* for abnormal, internal termination (SEGV, BUS, FPE) */
#define WD_SIGINT      8
#define WD_DEFUNCT     16
#define WD_FINALIZING  32
#define WD_SHUTDOWN    64

#define WD_TICK_TIMESPAN      2000000  /* timespan of a watchdog tick in us - the watchdog will check
					  other processes each WD_TICK_TIMESPAN us */
#define WD_TICKS_TIMEOUT_DEF  5         /* default delay for recognition of a crashed process (in ticks) */
#define WD_TRIGGER_BADCNCT    3         /* even if the state of another process can not be determined
					  due to a dysfunctional SCI connection, abort after
					  WD_TRIGGER_BADCNCT*WD_TICKS_TIMEOUT_DEF*WD_TICK_TIMESPAN us 
					  without heartbea-update of a process */

#define WD_SHUTDOWN_SIGNAL SIGUSR1
/* watchdog control */
#ifdef WIN32
#define SIGINT  2
#define SIGUSR1 16
#endif

/* Decide how to realize the watchdog via a thread or via signals. Usually, a thread
   is the better approach because signals can be blocked if the process waits for an SCI
   interrupt (which leads to false watchdog alarms). However, Linux can still not dump 
   core when running threads; if this is needed for debugging, the thread can be disabled
   by defining WD_SIGNAL in Makefile.common */
#define USE_WATCHDOG            1

#ifdef HAVE_CONFIG_H
/* WIN32 does not have config.h - see below */
#include "smiconfig.h"
#elif defined WIN32
#include "smiconfig_win32.h"
#else
#error No configuration file found! (smiconfig.h)
#endif

#ifdef WD_TYPE_SIGNAL
#define USE_THREAD_FOR_WATCHDOG 0
#elif defined WD_TYPE_NONE
#define USE_THREAD_FOR_WATCHDOG 0
#undef USE_WATCHDOG
#define USE_WATCHDOG            0
#elif defined WD_TYPE_THREAD
/* There is no easy translation of signals to WIN32, this means
   always use a watchdog thread there (or no watchdog at all). */
#define USE_THREAD_FOR_WATCHDOG 1 
#elif defined WD_TYPE_CALLBACK
/* #error callback watchdog not yet implemented. */
#define USE_THREAD_FOR_WATCHDOG 1
#endif

#define SMI_WATCHDOG_DISABLE  0
#define SMI_WATCHDOG_OFF     -1

#define ALL_ALIVE -1
#define DEFUNCT_DETECTED -2

#define USE_IMPLICIT_LOCALSEG_CALLBACK 1
#define USE_IMPLICIT_REMOTESEG_CALLBACK 0


/* public Calls */
smi_error_t SMI_Watchdog(int iWatchdogTimeout);
void SMI_Abort(int error_code);
smi_error_t SMI_Watchdog_callback( void (*callback_function)(void) );

#ifdef WD_TYPE_CALLBACK
#ifdef WIN32
#error callback does not work under win32
#endif
typedef struct smi_sci_seg_t_ {
    union {
	sci_local_segment_t* local;
	sci_remote_segment_t* remote;	
    } pSegment;
    enum {
	localseg, 
	remoteseg
    } type;
    pthread_t mainthread;
} smi_sci_seg_t;
#endif

/* internal calls */

#ifdef WD_TYPE_CALLBACK
void _smi_init_watchdog_cb(int my_rank, int numprocs, int volatile *wd_addr, smi_sci_seg_t* wd_segment);
#endif
void _smi_init_watchdog(int my_rank, int numprocs, int volatile *wd_addr);
void _smi_finalize_watchdog(void);
void _smi_init_signal_handler(void);
int _smi_wd_set_timeout(int timeout);
void _smi_wd_set_rank(int new_wd_rank);
void _smi_wd_enable(int timeout);
void _smi_wd_disable(void);
int _smi_wd_shutdown_in_progress(void);
void _smi_wd_request_shutdown(int errorcode);

#endif 
