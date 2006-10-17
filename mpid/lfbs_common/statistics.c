/* $Id$ */

/* function call & execution time statistics */
#ifndef WIN32
#include "getus.h"
#endif
#include "statistic.h"
#include "mpid.h"

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#ifndef WIN32
#include <sys/time.h>
#else 
#include <wtypes.h>
#include <winbase.h>
#endif
#ifdef MPI_solaris86
#include <sys/processor.h>
#include <sys/procset.h>
#endif



/* 
 * global exported variables 
 */

#if defined(MPID_STATISTICS)
longlong_t _stat_t0, _stat_t1;
int MPID_do_statistics = 0;	 /* do runtime statistics */

/* runtime statistics */
MPID_statistics_t MPID_statistics[fun_dummy + 1] =
{
    /*** SEND ***/
    /* MPIR */
    {"mpi_send", 0, LLONG_MAX, 0, 0},
    /* ADI */
    {"mpi_senddatatype", 0, LLONG_MAX, 0, 0},
    {"mpi_sendcontig", 0, LLONG_MAX, 0, 0},
    /* ch_smi */
    {"mpid_getsendpkt", 0, LLONG_MAX, 0, 0},
    {"mpid_sendcontrol", 0, LLONG_MAX, 0, 0},
    
    {"mpid_send_short", 0, LLONG_MAX, 0, 0},
    
    {"mpid_eager_isend", 0, LLONG_MAX, 0, 0},
    {"mpid_eager_send", 0, LLONG_MAX, 0, 0},
    {"mpid_eager_scopy", 0, LLONG_MAX, 0, 0},
    {"mpid_eager_dma", 0, LLONG_MAX, 0, 0},
    
    {"mpid_rndv_pushsend", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_senddelay", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_isend", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_send", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_send_w_ack", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_ack_send", 0, LLONG_MAX, 0, 0},
    
    {"mpid_sendself", 0, LLONG_MAX, 0, 0},
    
    /*** RECV ***/
    /* MPIR */
    {"mpi_recv", 0, LLONG_MAX, 0, 0},
    {"mpi_recvdatatype", 0, LLONG_MAX, 0, 0},
	{"mpid_short_recv",0,LLONG_MAX,0,0},
	{"mpid_message_arrived",0,LLONG_MAX,0,0},
	
    /* ch_smi */
    {"mpid_readcontrol",0,LLONG_MAX, 0, 0},
    
    {"mpi_eager_recv", 0, LLONG_MAX, 0, 0},
    {"mpi_eager_rcopy", 0, LLONG_MAX, 0, 0},
    
    {"mpid_setup_rndvadr", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_irecv", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_ack_recv", 0, LLONG_MAX, 0, 0},
    {"mpid_rndv_pushrecv", 0, LLONG_MAX, 0, 0},
    
    {"mpid_recvself", 0, LLONG_MAX, 0, 0},
    
    /*** non-blocking rendez-vous ***/
    {"mpid_nrndv_isend", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_buf2ls", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_dma", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_send", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_send_w_ack", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_ack_send", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_pushsend", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_senddelay", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_irecv", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_ls2buf", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_ack_recv", 0, LLONG_MAX, 0, 0},
    {"mpid_nrndv_pushrecv", 0, LLONG_MAX, 0, 0},
    
    /*** GENERAL ***/
    /* internal */
    {"shmem_latency",0,LLONG_MAX, 0, 0},
    {"mpid_check_seq", 0, LLONG_MAX, 0, 0},
    {"create_sgmnt", 0, LLONG_MAX, 0, 0},
    {"stat_overhead", 0, LLONG_MAX, 0, 0},
    
     /*** EVENT COUNTERS ***/
    {"sci_transm_error", 0, LLONG_MAX, 0, 0},
    {"create_sgmnt_fail", 0, LLONG_MAX, 0, 0},
    {"no_sendpkt", 0, LLONG_MAX, 0, 0},
    {"no_readpkt", 0, LLONG_MAX, 0, 0},
    {"no_eagerbuf", 0, LLONG_MAX, 0, 0},
    {"rndvmem_split", 0, LLONG_MAX, 0, 0},
    {"dummy", 0, LLONG_MAX, 0, 0}
};

#endif				       /* defined(MPID_STATISTICS) */


/*
 * local variables
 */

#if defined(MPID_STATISTICS)
static longlong_t total_ticks;        /* start time of program */
static int cpu_frq;
#endif

#if defined(WIN32) && !defined SMI_Get_ticks
void SMI_Get_ticks(longlong_t *v) {
	__asm {
		mov edi, dword ptr [v];
		RDTSC ;
		MOV dword ptr [EDI+4],EDX;
		MOV dword ptr [EDI],EAX;
	}
}
#endif
/* 
 * initialize statistics module 
 */
void MPID_Statistics_init(void)
{
    longlong_t t0=0, t1=0; 
    int i, do_it;

    
#if defined(MPID_STATISTICS)
#ifndef _WIN32
    /* get CPU information */
    SMI_Query (SMI_Q_SYS_CPUFREQ, 0, (void *)&cpu_frq);
#else
	LARGE_INTEGER Frequ;
	QueryPerformanceFrequency(&Frequ);
	cpu_frq = Frequ.QuadPart*1e-6;
#endif

   for (i = 0; i < fun_dummy; ++i) {
	MPID_statistics[i].number_calls = 0;
	MPID_statistics[i].min_ticks = LLONG_MAX;
	MPID_statistics[i].max_ticks = 0;
	MPID_statistics[i].acc_ticks = 0;
    }

    /* calibrate the measurement overhead */
    for (i = 0; i < CAL_LOOPS; i++) {
	if (i >= 0)
	    do_it = 1;
	if (do_it) { 
	    ++MPID_statistics[stat_overhead].number_calls; 
	    SMI_Get_ticks(&t0);
	}
	if (do_it) {
	    SMI_Get_ticks(&t1);
	    MPID_Update_ticks(t1 - t0, &MPID_statistics[stat_overhead]);
	}
    }    

    /* get initial time */
    SMI_Get_ticks(&total_ticks);
#endif
}

/* update times for run-time library function calls */
#if defined(MPID_STATISTICS) && !defined(INLINE)
void MPID_Update_ticks( ticks, stat)
longlong_t ticks;		             /* I: time measured */
MPID_statistics_t * stat;  /* I: run-time statistics */
{
  assert(stat != NULL);
  assert(ticks >= 0);

  stat->acc_ticks += ticks;
  if (ticks < stat->min_ticks)
    stat->min_ticks = ticks;
  if (ticks > stat->max_ticks)
    stat->max_ticks = ticks;
}
#endif				       /* defined(MPID_STATISTICS) */

/* display runtime statistics */
void MPID_Runtime_statistics ( void )
{
#if defined(MPID_STATISTICS)
    longlong_t ticks;
    int i;
    int calls = 0;
    double tns, tus, overhead,acc;
    char smin[50], smax[50], savg[50], sacc[53];

    if (MPID_do_statistics) {
	assert(MPID_statistics != NULL);
	
	/* get global ticks spent in program */
	SMI_Get_ticks (&ticks);
	total_ticks = ticks - total_ticks;
	
	/* print header */
	tns = (double)1000/((double)cpu_frq);
	tus = (double)1/((double)cpu_frq);
	fprintf(stderr, "\nMPICH/ch_smi run-time library statistics (node %d):\n",
		MPID_MyWorldRank);
	fprintf(stderr, "based on CPU frequency of %d MHz\n",
		cpu_frq, tns);
	fprintf(stderr, "    %-20s %-9s %-9s %-9s %-9s %-9s\n",
		"function", "#calls", "min [us]", "max [us]", "avg [us]", "acc [s]");
	fprintf(stderr, "====================================="
		"=========================================\n");
	
	/* print total time */
	fprintf(stderr, "[%d] %-20s %-9d %-9s %-9s %-9s %6.3f\n",
		MPID_MyWorldRank,"total time", 1, "--", "--", "--", total_ticks*tus/1000000);

	/* calculate & print overhead */
	for (i = 0; i < fun_dummy; ++i) {
	    calls += MPID_statistics[i].number_calls;
	}
	overhead = (2*calls*MPID_statistics[stat_overhead].acc_ticks*tus)
	            / (MPID_statistics[stat_overhead].number_calls*1000000);
	fprintf(stderr, "[%d] %-20s %-9d %-9s %-9s %-9s %6.3f\n",
		MPID_MyWorldRank,"total overhead", 1, "--", "--", "--", overhead);
	
	/* go through all functions */
	for (i = 0; i < fun_dummy; ++i) {
	    if (MPID_statistics[i].number_calls > 0) {
		if (MPID_statistics[i].min_ticks == LLONG_MAX)
		    fprintf(stderr, "[%d] %-20s %-9d %-9s %-9s %-9s %-9s\n",
			    MPID_MyWorldRank,
				MPID_statistics[i].name,
			    MPID_statistics[i].number_calls,
			    "--",
			    "--",
			    "--",
			    "--"
			    );
		else {
			if(MPID_statistics[i].number_calls>10) {
				acc = (double) (MPID_statistics[i].acc_ticks-
					  MPID_statistics[i].min_ticks-
					  MPID_statistics[i].max_ticks);
			    acc *= tus;
				acc /= MPID_statistics[i].number_calls-2;
			} else 
				acc =(MPID_statistics[i].acc_ticks*tus)/ MPID_statistics[i].number_calls;
		    sprintf (smin, "%6.3f", MPID_statistics[i].min_ticks*tus);
		    sprintf (smax, "%6.3f", MPID_statistics[i].max_ticks*tus);
		    sprintf (savg, "%6.3f", acc);
		    sprintf (sacc, "%9.6f", MPID_statistics[i].acc_ticks*tus/1000000);
		    fprintf(stderr, "[%d] %-20s %-9d %-9s %-9s %-9s %-12s\n",
			    MPID_MyWorldRank,
				MPID_statistics[i].name,
			    MPID_statistics[i].number_calls,
			    smin, smax, savg, sacc);
		}

	    }
	}
	
	
	fflush(stderr);
    }
#endif
}


/* turn on/off or reset runtime statistics */
void MPID_Set_statistics ( what_to_do )
int what_to_do;
{
#if defined(MPID_STATISTICS)
  switch (what_to_do) {
  case 0:
      /* don't do statistics */
      MPID_do_statistics = 0;
      break;
  case 1:
      /* do statistics */
      MPID_do_statistics = 1;
      break;
  default:
      /* reset statistics */
      MPID_Statistics_init();
      break;
  }
#else
  /* always init the frequency value */
  MPID_Statistics_init();    
#endif				       /* defined(MPID_STATISTICS) */
}
