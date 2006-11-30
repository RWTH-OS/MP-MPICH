/* $Id$ */

#ifndef _SMI_STATISTICS_H
#define _SMI_STATISTICS_H

#include <sys/types.h>

#include "env/general_definitions.h"

/* 
 * types
 */

typedef enum {
    /* segment creation */
    create_shreg,
    allocate,
    export_sci_segment,
    import_sci_segment,
    map_sci_segment,
    
    /* calls to SISCI */
    sci_create,
    sci_connect,
    
    /* syncronisation */
    barrier,
    bcast,
    allgather,
    alltrue,

    /* error counters */
    err_check,
    err_rtrbl,
    err_not_rtrbl,
    err_not_rtrbl_retry,
    err_pndng,
    err_pndng_retry,
    err_pndng_not_rtrbl,
    err_pndng_not_rtrbl_retry,
    err_pndng_ok,

    /* internals */
    stat_overhead,
    /*** stop marker ***/
    fun_dummy
} _smi_statistics_fnbr_t;


typedef struct {
    char name[26];		             /* name of function */
    int number_calls;		          /* how many times called */
    longlong_t min_ticks;		       /* minimal ticks spent */
    longlong_t max_ticks;		       /* maximal ticks spent */
    longlong_t acc_ticks;		       /* accumulated ticks spent */
} _smi_statistics_t;


/* 
 * function prototypes  
 */

/* initialize statistics module */
void _smi_statistics_init ( void );

#if defined(INLINE)
#define _smi_update_ticks(t,stat) \
  (stat)->acc_ticks += t; \
  if ((t) < (stat)->min_ticks) (stat)->min_ticks = t; \
  if ((t) > (stat)->max_ticks) (stat)->max_ticks = t;
#else
/* update statistic times */
void _smi_update_ticks (longlong_t ticks,	  /* I: ticks measured */
			_smi_statistics_t * stat  /* I: run-time statistics */
);
#endif

/* display runtime statistics */
void _smi_runtime_statistics( void );
/* turn on/off or reset runtime statistics */
void _smi_set_statistics (int );


/* 
 * variables
 */
extern longlong_t _smi_stat_t0, _smi_stat_t1;
extern int _smi_do_statistics;	       /* do runtime statistics */
extern _smi_statistics_t _smi_statistics[fun_dummy + 1];

extern double processor_wait_time;


/* 
 * macros
 */
#define CAL_LOOPS 100
#define FRQ_BASE  1000000
#define FRQ_LOOPS (25*FRQ_BASE)

/* measure a complete function */
#define SMI_STAT_ENTRY(fun) \
  longlong_t t0, t1; \
  if (_smi_do_statistics) \
    { \
      ++_smi_statistics[fun].number_calls; \
      SMI_Get_ticks(&t0); \
    }
#define SMI_STAT_EXIT(fun) \
  if (_smi_do_statistics) \
    { \
      SMI_Get_ticks(&t1); \
      _smi_update_ticks(t1 - t0, &_smi_statistics[fun]); \
    }

/* measure a function call - attention: uses global variables! */
#define SMI_STAT_CALL(fun) \
  if (_smi_do_statistics) \
    { \
      ++_smi_statistics[fun].number_calls; \
      SMI_Get_ticks(&_smi_stat_t0); \
    }

#define SMI_STAT_RETURN(fun) \
  if (_smi_do_statistics) \
    { \
      SMI_Get_ticks(&_smi_stat_t1); \
      _smi_update_ticks(_smi_stat_t1 - _smi_stat_t0, &_smi_statistics[fun]); \
    }

/* special macros to stop times between different functions.
   _smi_stat_t0 and _smi_stat_t1 must be defined visible to both macro
   calls. Take care that you really know which timespan you are 
   measuring! */
#define SMI_STAT_START(timespan) \
  if (_smi_do_statistics) \
    { \
      ++_smi_statistics[timespan].number_calls; \
      SMI_Get_ticks(&_smi_stat_t0); \
    }

#define SMI_STAT_END(timespan) \
  if (_smi_do_statistics) \
    { \
      SMI_Get_ticks(&_smi_stat_t1); \
      _smi_update_ticks(_smi_stat_t1 - _smi_stat_t0, &_smi_statistics[timespan]); \
    }

/* event counter */
#define SMI_STAT_COUNT(event) \
  if (_smi_do_statistics) \
    { \
      ++_smi_statistics[event].number_calls; \
    }


#endif


