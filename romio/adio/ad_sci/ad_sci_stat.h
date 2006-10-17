/* $Id: ad_sci_stat.h,v 1.2 2001/05/10 18:46:45 joachim Exp $ */

#ifndef _AD_SCI_STAT_H
#define _AD_SCI_STAT_H

#include <sys/types.h>


/* 
 * types
 */
#if defined(MPI_LINUX)
typedef long long int longlong_t;
#define LLONG_MAX 9223372036854775807LL
#elif defined(_WIN32)
#include <wtypes.h>
typedef LONGLONG longlong_t;
#define LLONG_MAX 9223372036854775807
#endif

/* For now, always generate statistics. Later, make this a configure option. */
#define AD_SCI_STATISTICS

#if defined(AD_SCI_STATISTICS)
typedef enum {
    /*** GENERAL ***/
    /* internal */
    sci_check_seq,
    sci_create_sgmnt,
    stat_overhead,

    /*** EVENT COUNTERS ***/
    sci_transm_error,
    sci_create_sgmnt_fail,

    /*** stop marker ***/
    fun_dummy
} function_number_t;


typedef struct {
    char name[21];		             /* name of function */
    int number_calls;		             /* how many times called */
    longlong_t min_ticks;		     /* minimal ticks spent */
    longlong_t max_ticks;		     /* maximal ticks spent */
    longlong_t acc_ticks;		     /* accumulated ticks spent */
} ADSCI_statistics_t;


/* 
 * variables
 */
extern longlong_t _adsci_stat_t0, _adsci_stat_t1;
extern int ADSCI_do_statistics;	       /* do runtime statistics */
extern ADSCI_statistics_t ADSCI_statistics[fun_dummy + 1];

#endif

extern double processor_wait_time;


/* 
 * macros
 */
#define CAL_LOOPS 100
#define FRQ_BASE  1000000
#define FRQ_LOOPS (25*FRQ_BASE)

#if defined(ADSCI_STATISTICS)
/* measure a complete function */
#define ADSCI_STAT_ENTRY(fun) \
  longlong_t t0, t1; \
  if (ADSCI_do_statistics) \
    { \
      ++ADSCI_statistics[fun].number_calls; \
      _adsci_get_ticks(&t0); \
    }
#define ADSCI_STAT_EXIT(fun) \
  if (ADSCI_do_statistics) { \
      _adsci_get_ticks(&t1); \
      ADSCI_Update_ticks(t1 - t0, &ADSCI_statistics[fun]); \
    }

/* measure a function call - attention: uses global variables! */
#define ADSCI_STAT_CALL(fun) \
  if (ADSCI_do_statistics) { \
      ++ADSCI_statistics[fun].number_calls; \
      _adsci_get_ticks(&_adsci_stat_t0); \
    }

#define ADSCI_STAT_RETURN(fun) \
  if (ADSCI_do_statistics) { \
      _adsci_get_ticks(&_adsci_stat_t1); \
      ADSCI_Update_ticks(_adsci_stat_t1 - _adsci_stat_t0, &ADSCI_statistics[fun]); \
    }

/* special macros to stop times between different functions.
   _stat_t0 and _stat_t1 must be defined visible to both macro
   calls. Take care that you really know which timespan you are 
   measuring! */
#define ADSCI_STAT_START(timespan) \
  if (ADSCI_do_statistics) { \
      ++ADSCI_statistics[timespan].number_calls; \
      _adsci_get_ticks(&_adsci_stat_t0); \
    }

#define ADSCI_STAT_END(timespan) \
  if (ADSCI_do_statistics) { \
      _adsci_get_ticks(&_adsci_stat_t1); \
      ADSCI_Update_ticks(_adsci_stat_t1 - _adsci_stat_t0, &ADSCI_statistics[timespan]); \
    }

/* event counter */
#define ADSCI_STAT_COUNT(event) \
  if (ADSCI_do_statistics) \
    { \
      ++ADSCI_statistics[event].number_calls; \
    }
#else
/* dummy defines if compiling without statistics support */
#define ADSCI_STAT_ENTRY(fun) 
#define ADSCI_STAT_EXIT(fun) 
#define ADSCI_STAT_CALL(fun) 
#define ADSCI_STAT_RETURN(fun) 
#define ADSCI_STAT_START(timespan) 
#define ADSCI_STAT_END(timespan) 
#define ADSCI_STAT_COUNT(event) 
#endif

/* 
 * function prototypes  
 */

/* initialize statistics module */
extern void ADSCI_Statistics_init ( void );

#if defined(ADSCI_STATISTICS)
#if defined(INLINE)
#define ADSCI_Update_ticks(t,stat) \
  (stat)->acc_ticks += t; \
  if ((t) < (stat)->min_ticks) (stat)->min_ticks = t; \
  if ((t) > (stat)->max_ticks) (stat)->max_ticks = t;
#else
/* update statistic times */
void ADSCI_Update_ticks (longlong_t ticks, 	  /* I: ticks measured */
			ADSCI_statistics_t * stat  /* I: run-time statistics */
);
#endif
#endif

/* display runtime statistics */
void ADSCI_Runtime_statistics( void );
/* turn on/off or reset runtime statistics */
void ADSCI_Set_statistics (int );

#endif






// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
