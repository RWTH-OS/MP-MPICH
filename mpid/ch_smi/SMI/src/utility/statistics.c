/* $Id$ */

/* function call & execution time statistics */

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#ifdef SOLARIS
#include <sys/processor.h>
#include <sys/procset.h>
#endif

#include "statistics.h"
#include "message_passing/lowlevelmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* 
 * global exported variables 
 */

longlong_t _smi_stat_t0, _smi_stat_t1;
int _smi_do_statistics = 0;	 /* do runtime statistics */

/* runtime statistics */
_smi_statistics_t _smi_statistics[fun_dummy + 1] =
{
    /* segment creation */
    {"create_shreg", 0, LLONG_MAX, 0, 0},
    {"allocate", 0, LLONG_MAX, 0, 0},
    {"export_sci_segment", 0, LLONG_MAX, 0, 0},
    {"import_sci_segment", 0, LLONG_MAX, 0, 0},
    {"map_sci_segment", 0, LLONG_MAX, 0, 0},
    
    /* calls to SISCI */
    {"sci_create", 0, LLONG_MAX, 0, 0},
    {"sci_connect", 0, LLONG_MAX, 0, 0},
    
    /* syncronisation */
    {"barrier", 0, LLONG_MAX, 0, 0},
    {"bcast", 0, LLONG_MAX, 0, 0},
    {"allgather", 0, LLONG_MAX, 0, 0},
    {"alltrue", 0, LLONG_MAX, 0, 0},	 
    
    /* error counters */
    {"err_check", 0, LLONG_MAX, 0, 0},
    {"err_rtrbl", 0, LLONG_MAX, 0, 0},
    {"err_not_rtrbl", 0, LLONG_MAX, 0, 0},
    {"err_not_rtrbl_retry", 0, LLONG_MAX, 0, 0},
    {"err_pndng", 0, LLONG_MAX, 0, 0},
    {"err_pndng_retry", 0, LLONG_MAX, 0, 0},
    {"err_pndng_not_rtrbl", 0, LLONG_MAX, 0, 0},
    {"err_pndng_not_rtrbl_retry", 0, LLONG_MAX, 0, 0},
    {"err_pndng_ok", 0, LLONG_MAX, 0, 0},
    
    /* internals */
    {"stat_overhead", 0, LLONG_MAX, 0, 0},

    /*** stop marker ***/
    {"dummy", 0, LLONG_MAX, 0, 0}
};


/*
 * local variables
 */
static longlong_t total_ticks;        /* start time of program */
static int cpu_frq;


/* 
 * initialize statistics module 
 */
void _smi_statistics_init(void)
{
    longlong_t t0, t1; 
    int i, do_it;

    /* get CPU information */
    SMI_Query (SMI_Q_SYS_CPUFREQ, 0, (void *)&cpu_frq);
    
    for (i = 0; i < fun_dummy; ++i) {
	_smi_statistics[i].number_calls = 0;
	_smi_statistics[i].min_ticks = LLONG_MAX;
	_smi_statistics[i].max_ticks = 0;
	_smi_statistics[i].acc_ticks = 0;
    }
    
    /* calibrate the measurement overhead */
    for (i = 0; i < CAL_LOOPS; i++) {
	if (i >= 0)
	    do_it = 1;
	if (do_it) { 
	    ++_smi_statistics[stat_overhead].number_calls; 
	    SMI_Get_ticks(&t0);
	}
	if (do_it) {
	    SMI_Get_ticks(&t1);
	    _smi_update_ticks(t1 - t0, &_smi_statistics[stat_overhead]);
	}
    }    
    
    /* get initial time */
    SMI_Get_ticks(&total_ticks);
}

/* update times for run-time library function calls */
void _smi_update_ticks( ticks, stat)
	 longlong_t ticks;		      /* I: time measured */
_smi_statistics_t * stat;  /* I: run-time statistics */
{
	 assert(stat != NULL);
	 assert(ticks >= 0);

	 stat->acc_ticks += ticks;
	 if (ticks < stat->min_ticks)
		  stat->min_ticks = ticks;
	 if (ticks > stat->max_ticks)
		  stat->max_ticks = ticks;
}

/* display runtime statistics */
void _smi_runtime_statistics ( void )
{
    longlong_t ticks;
    int i, proc;
    int calls = 0;
    double tus, overhead;
    char smin[10], smax[10], savg[10], sacc[13];
	 
    if (_smi_do_statistics) {
	assert(_smi_statistics != NULL);
		  
	/* get global ticks spent in program */
	SMI_Get_ticks (&ticks);
	total_ticks = ticks - total_ticks;
		  
	/* synchronized output */
	for (proc = 0; proc < _smi_nbr_procs; proc++) {
	    if (proc != _smi_my_proc_rank) {
		sleep(1);
	    } else {
		/* print header */
		tus = (double)1/((double)cpu_frq);
		fprintf(stderr, "\nSMI run-time library statistics (proc %d):\n",
			_smi_my_proc_rank);
		fprintf(stderr, "based on CPU frequency of %d MHz\n",
			cpu_frq);
		fprintf(stderr, "%-25s %-9s %-9s %-9s %-9s %-9s\n",
			"function", "#calls", "min [us]", "max [us]", "avg [us]", "acc [s]");
		fprintf(stderr, "====================================="
			"=========================================\n");
		
		/* print total time */
		fprintf(stderr, "%-25s %-9d %-9s %-9s %-9s %6.3f\n",
			"total time", 1, "--", "--", "--", total_ticks*tus/1000000);
		  
		/* calculate & print overhead */
		for (i = 0; i < fun_dummy; ++i) {
		    calls += _smi_statistics[i].number_calls;
		}
		overhead = (2*calls*_smi_statistics[stat_overhead].acc_ticks*tus)
		    / (_smi_statistics[stat_overhead].number_calls*1000000);
		fprintf(stderr, "%-25s %-9d %-9s %-9s %-9s %6.6f\n",
			"total overhead", 1, "--", "--", "--", overhead);
		  
		/* go through all functions */
		for (i = 0; i < fun_dummy; ++i) {
		    if (_smi_statistics[i].number_calls > 0) {
			if (_smi_statistics[i].min_ticks == LLONG_MAX)
			    fprintf(stderr, "%-25s %-9d %-9s %-9s %-9s %-9s\n",
				    _smi_statistics[i].name,
				    _smi_statistics[i].number_calls,
				    "--",
				    "--",
				    "--",
				    "--"
				    );
			else {
			    sprintf (smin, "%6.3f", _smi_statistics[i].min_ticks*tus);
			    sprintf (smax, "%6.3f", _smi_statistics[i].max_ticks*tus);
			    sprintf (savg, "%6.3f", (_smi_statistics[i].acc_ticks*tus)
				     / _smi_statistics[i].number_calls);
			    sprintf (sacc, "%9.6f", _smi_statistics[i].acc_ticks*tus/1000000);
			    fprintf(stderr, "%-25s %-9d %-9s %-9s %-9s %-12s\n",
				    _smi_statistics[i].name,
				    _smi_statistics[i].number_calls,
				    smin, smax, savg, sacc);
			}

		    }
		}
		fflush(stderr);
	    }
	    _smi_ll_barrier();
	}
    }
}


/* turn on/off or reset runtime statistics */
void _smi_set_statistics ( what_to_do )
	 int what_to_do;
{
	 switch (what_to_do) {
	 case 0:
		  /* don't do statistics */
		  _smi_do_statistics = 0;
		  break;
	 case 1:
		  /* do statistics */
		  _smi_do_statistics = 1;
		  break;
	 default:
		  /* reset statistics */
		  _smi_statistics_init();
		  break;
	 }
}
