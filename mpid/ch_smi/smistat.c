/* $Id$ */

/* function call & execution time statistics */

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#ifdef MPI_solaris86
#include <sys/processor.h>
#include <sys/procset.h>
#endif

#include "smidef.h"
#include "mpid.h"
#include "smidev.h"
#include "smistat.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* 
 * global exported variables 
 */

#ifdef MPID_SMI_STATISTICS
longlong_t _mpid_smi_stat_t0, _mpid_smi_stat_t1;
int MPID_SMI_do_statistics = 0;	 /* do runtime statistics */

/* runtime statistics */
MPID_SMI_statistics_t MPID_SMI_statistics[fun_dummy + 1] =
{
    {"stat_overhead", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* control-messages */
    {"device_init", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"getsendpkt", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"no_sendpkt", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sendcontrol", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"signal_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"signal_wait", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"checkdev", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"readcontrol", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"no_readpkt", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"thread_wakeup", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"thread_got_msg", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"calc_csum", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"csum_retry_ctrl", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"csum_retry_data", 0, LLONG_MAX, 0, 0, STAT_COUNTER},

    /* low-level SCI events */
    {"check_seq", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sci_flush", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"nodemem_alloc", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"nodemem_free", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sci_transm_error", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sci_read_error", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"create_sgmnt", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"create_sgmnt_fail", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"thread_yield", 0, LLONG_MAX, 0, 0, STAT_COUNTER},

    /* sequentialization */
    {"recv_postponed", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"send_postponed", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"no_incpy_lock", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"no_outcpy_lock", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"ch_smi_mutex_lock", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"ch_smi_mutex_unlock", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    
    /* short protocol */
    {"short_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"short_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"short_pack", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* eager protocol */
    {"eager_sgmnt_cnct", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"eager_sgmnt_discnct", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"no_eagerbuf", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"eager_isend", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"eager_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"eager_scopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"eager_dma", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"eager_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"eager_rcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
        
    /* rndv protocol: sync send */      
    {"rndv_sgmnt_cnct", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rndv_sgmnt_discnct", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rndv_pushsend", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_isend", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_send_w_ack", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_ack_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_pushscopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_scopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_dma", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    
    /* rndv protocol: sync recv */  
    {"setup_rndvadr", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndvmem_split", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rndv_irecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_ack_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_pushrecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_pushrcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rndv_rcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* rndv protocol: async send */  
    {"arndv_isend", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_buf2ls", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_dma", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_pio", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_send_w_ack", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_ack_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_ack_sendzc", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_pushsend", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* rndv protocol: async recv */  
    {"arndv_irecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_ls2buf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_ack_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_pushrecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_setup_sendbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_setup_recvbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_cnct_dstbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"arndv_discnct_dstbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},    

    /* rndv protocol: blocking send/recv */  
    {"brndv_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"brndv_isend", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"brndv_irecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"brndv_ack_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"brndv_ack_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    
    /* rndv transfer control */
    {"nbr_rndv_recvs", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"nbr_rndv_sends", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"rndv_inbuf_size", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"rndv_sync_delay", 0, LLONG_MAX, 0, 0, STAT_PERIOD},

    /* single sided */
    {"sside_put_contig", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_put_cont_cpy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_put_sametype", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_put_emulate", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_put_emu_remote", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_get_contig", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_get_sametype", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_get_emulate", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_get_emu_remote", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_accu_emulate", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_accu_emu_remote", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_win_create", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_win_free", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_win_sync", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_memcpy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_delay_store", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_delay_flush", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_delay_process", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_add_tgt_job", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sside_rm_tgt_job", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sside_add_jobreq", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sside_rm_jobreq", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"sside_cmplt_tgt_jobs", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"sside_cmplt_jobreqs", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* direct pack */
    {"directPack", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directUnpack", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directPackLeave", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directUnpackLeave", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directWRITE_direct", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directWRITE_lbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directWRITE_part", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directFLUSH_lbuf", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directREAD", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"directREAD_part", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* resource management */
    {"acquire_rsrc", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"release_rsrc", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rmt_mem_map", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rmt_mem_release", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"loc_mem_create", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"loc_mem_register", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"loc_mem_release", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rmt_reg_connect", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rmt_reg_release", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    {"rsrc_reuse", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_create", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_release", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_unused", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_failed", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_destroy", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_cb_disconnect", 0, LLONG_MAX, 0, 0, STAT_COUNTER},    
    {"rsrc_release_req", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_release_ack", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"rsrc_release_nack", 0, LLONG_MAX, 0, 0, STAT_COUNTER},

     /* zero-copy */
    {"sendbuf_registered", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"recvbuf_registered", 0, LLONG_MAX, 0, 0, STAT_COUNTER},
    {"destbuf_imported", 0, LLONG_MAX, 0, 0, STAT_COUNTER},    
    {"zerocopy_canceled", 0, LLONG_MAX, 0, 0, STAT_COUNTER},

    /* custom collectives */
    {"comm_init", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"coll_init", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"barrier", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"reduce", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"allreduce", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"scan", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"allgather", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"alltoall", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"bcast", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"bcast_dma", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"bcast_scopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"bcast_rcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"bcast_pcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_send", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_scopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_recv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_rcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_sendrecv", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"rdcpipe_pcopy", 0, LLONG_MAX, 0, 0, STAT_TIMER},

    /* MPI memory allocation */
    {"alloc_mem", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"free_mem", 0, LLONG_MAX, 0, 0, STAT_TIMER},
    {"alloc_failed", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"alloc_priv", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"alloc_shared_pool", 0, LLONG_MAX, 0, 0, STAT_PROBE},
    {"alloc_shared_reg", 0, LLONG_MAX, 0, 0, STAT_PROBE},

    /*** stop marker ***/
    {"dummy", 0, LLONG_MAX, 0, 0, STAT_END}
};

#endif				       /* defined(MPID_STATISTICS) */


/*
 * local variables
 */

#ifdef MPID_SMI_STATISTICS
static longlong_t total_ticks;        /* start time of program */
static int cpu_frq;
#endif


/* 
 * initialize statistics module 
 */
void MPID_SMI_Statistics_init(void)
{
#ifdef MPID_SMI_STATISTICS
    longlong_t t0, t1; 
    int i, do_it;
    
    /* get CPU information */
    SMI_Query (SMI_Q_SYS_CPUFREQ, 0, (void *)&cpu_frq);

    for (i = 0; i < fun_dummy; ++i) {
	MPID_SMI_statistics[i].number_calls = 0;
	MPID_SMI_statistics[i].min_ticks = LLONG_MAX;
	MPID_SMI_statistics[i].max_ticks = 0;
	MPID_SMI_statistics[i].acc_ticks = 0;
    }

    /* calibrate the measurement overhead */
    for (i = 0; i < CAL_LOOPS; i++) {
	if (i >= 0)
	    do_it = 1;
	if (do_it) { 
	    ++MPID_SMI_statistics[stat_overhead].number_calls; 
	    SMI_Get_ticks(&t0);
	}
	if (do_it) {
	    SMI_Get_ticks(&t1);
	    MPID_SMI_Update_ticks(t1 - t0, &MPID_SMI_statistics[stat_overhead]);
	}
    }    

    /* get initial time */
    SMI_Get_ticks(&total_ticks);

#if !(defined MPI_LINUX) && !(defined MPI_LINUX_ALPHA)
    /* XXX this causes problems with Linux (kernel lockup!) on proc 0's node */

    /* we want to print the statistics even for abnormal termination */
    if (MPID_SMI_cfg.USE_WATCHDOG > 0)
	SMI_Watchdog_callback(MPID_SMI_Runtime_statistics);
#endif

#endif
}

/* update times for run-time library function calls */
#if defined MPID_SMI_STATISTICS && !defined UPDATE_INLINE
void MPID_SMI_Update_ticks( ticks, stat)
longlong_t ticks;	       /* I: time measured */
MPID_SMI_statistics_t * stat;  /* I: run-time statistics */
{
  assert(stat != NULL);

  /* XXX ticks == 0 can only happen in a multithreaded environment. We could avoid it
     by locking the global variables, but this would make the timing much coarser. Instead,
     we live with this and just don't count such an "illegal" event. */
  if (ticks == 0)
      return;

  stat->acc_ticks += ticks;
  if (ticks < stat->min_ticks)
    stat->min_ticks = ticks;
  if (ticks > stat->max_ticks)
    stat->max_ticks = ticks;
}
#endif				       /* defined(MPID_STATISTICS) */

#define PRINT_TIME(t,s)  if (t > 1e+6) sprintf (s, "%6.3f ", t/1e+6); \
			 else if (t > 1e+3) sprintf (s, "%6.3fm", t/1e+3); \
		         else sprintf (s, "%6.3fu", t);

/* display runtime statistics */
void MPID_SMI_Runtime_statistics ( void )
{
#ifdef MPID_SMI_STATISTICS
    longlong_t ticks;
    int i, proc;
    int calls = 0;
    double tns, tus, overhead, pcnt, ttime, ptime, time;
    char smin[10], smax[10], savg[10], sacc[13], spcnt[7];

    if (MPID_SMI_do_statistics) {
	assert(MPID_SMI_statistics != NULL);
	
	/* get global ticks spent in program */
	SMI_Get_ticks (&ticks);
	total_ticks = ticks - total_ticks;
	
	/* synchronized output */
	for (proc = 0; proc < MPID_SMI_numids; proc++) {
	    if (proc != MPID_SMI_myid) {
		usleep(1000);
	    } else {
		/* print header */
		tns = (double)1000/((double)cpu_frq);
		tus = (double)1/((double)cpu_frq);
		fprintf(stderr, "\n[%d] ch_smi run-time statistics (CPU clock %d MHz):\n",
			MPID_SMI_myid, cpu_frq);
		fprintf(stderr, "  %-20s %9s  %9s %9s %9s %9s %6s\n",
			"event", "#calls", "min", "max", "avg", "acc", "[%]");
		fprintf(stderr, "======================================"
			"============================================\n");
	
		/* print total time */
		ttime = total_ticks*tus;
		PRINT_TIME(ttime, savg);
		pcnt = 100;
		fprintf(stderr, "  %-20s %9d  %9s %9s %9s %12s %4.1f\n",
			"total time", 1, "--", "--", "--", savg, pcnt);

		/* calculate & print overhead */
		for (i = 0; i < fun_dummy; ++i) {
		    calls += MPID_SMI_statistics[i].number_calls;
		}
		overhead = (2*calls*MPID_SMI_statistics[stat_overhead].acc_ticks*tus)
	            / (MPID_SMI_statistics[stat_overhead].number_calls*1000000);
		PRINT_TIME(overhead, savg);
		sprintf (spcnt, "%4.2f", (overhead*100)/ttime);
		fprintf(stderr, "  %-20s %9d  %9s %9s %9s %12s %5s\n",
			"total overhead", 1, "--", "--", "--", savg, spcnt);
	
		/* go through all functions */
		for (i = 0; i < fun_dummy; ++i) {
		    if (MPID_SMI_statistics[i].number_calls > 0) {
			switch (MPID_SMI_statistics[i].type) {
			case STAT_COUNTER:
			    fprintf(stderr, "C %-20s %9d  %9s %9s %9s %12s %5s\n",
				    MPID_SMI_statistics[i].name,
				    MPID_SMI_statistics[i].number_calls,
				    "--", "--", "--", "--", "--"
				    );
			    break;
			case STAT_TIMER:
			case STAT_PERIOD:
			    time = MPID_SMI_statistics[i].min_ticks*tus;
			    PRINT_TIME(time, smin);
			    time = MPID_SMI_statistics[i].max_ticks*tus;
			    PRINT_TIME(time, smax);
			    time = (MPID_SMI_statistics[i].acc_ticks*tus)
				/ MPID_SMI_statistics[i].number_calls;
			    PRINT_TIME(time, savg);
			    ptime = MPID_SMI_statistics[i].acc_ticks*tus;
			    PRINT_TIME(ptime, sacc);
			    sprintf (spcnt, "%4.2f", (ptime*1e+2)/ttime);
			    fprintf(stderr, "T %-20s %9d  %9s %9s %9s %12s %5s\n",
				    MPID_SMI_statistics[i].name,
				    MPID_SMI_statistics[i].number_calls,
				    smin, smax, savg, sacc, spcnt);
			    break;
			case STAT_PROBE:
			    sprintf (smin, "%9lld", MPID_SMI_statistics[i].min_ticks);
			    sprintf (smax, "%9lld", MPID_SMI_statistics[i].max_ticks);
			    sprintf (savg, "%6.3f", MPID_SMI_statistics[i].acc_ticks
				     / (double)MPID_SMI_statistics[i].number_calls);
			    sprintf (sacc, "%9lld", MPID_SMI_statistics[i].acc_ticks);
			    fprintf(stderr, "P %-20s %9d  %9s %9s %9s %12s %5s\n",
				    MPID_SMI_statistics[i].name,
				    MPID_SMI_statistics[i].number_calls,
				    smin, smax, savg, sacc, "--");
			    break;
			}
		    }
		}
		/* the interal SCI error counter of the SMI library, giving insight to the
		   "quality" of the SCI link */
		SMI_Query (SMI_Q_SCI_ERRORS, 0, &i);
		if (i > 0) {
		    fprintf(stderr, "E %-20s %9d  %9s %9s %12s %6s\n",
			    "SCI err+retry", i, "--", "--", "--", "--");
		    fflush(stderr);
		}
	    }
#if 0
	    /* could cause problems if this function is called from the SMI_Abort handler */
	    SMIcall (SMI_Barrier());
#endif
	}
    }
#endif
}


/* turn on/off or reset runtime statistics */
void MPID_SMI_Set_statistics ( what_to_do )
int what_to_do;
{
#ifdef MPID_SMI_STATISTICS
  int i;

  switch (what_to_do) {
  case SMI_STAT_DISABLE:
      /* don't do statistics */
      MPID_SMI_do_statistics = 0;
      break;
  case SMI_STAT_ENABLE:
      /* do statistics */
      MPID_SMI_do_statistics = 1;
      break;
  case SMI_STAT_RESET:
      /* reset statistics */
      MPID_SMI_Statistics_init();
      break;
  case SMI_STAT_START:
      /* get initial time */
      SMI_Get_ticks(&total_ticks);
      break;
  }
#else
  /* always init the frequency value */
  MPID_SMI_Statistics_init();    
#endif				       /* defined(MPID_STATISTICS) */
}
