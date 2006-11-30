/* $Id$ */

#ifndef _MPID_SMI_STAT_H
#define _MPID_SMI_STAT_H

#include <sys/types.h>

#include "getus.h"


typedef enum { STAT_TIMER, STAT_COUNTER, STAT_PROBE, STAT_PERIOD, STAT_END } stat_type;

typedef enum {
    stat_overhead,

    /* control-messages */
    device_init,         /* time to set up device, excl. SMI_Init() */
    getsendpkt, 
    no_sendpkt,
    sendcontrol,
    signal_send,
    signal_wait,
    checkdev,
    readcontrol, 
    no_readpkt,
    thread_wakeup,
    thread_got_msg,
    calc_csum,
    csum_retry_ctrl,
    csum_retry_data,

    /* low-level SCI events */
    check_seq,
    sci_flush,
    nodemem_alloc,
    nodemem_free,
    sci_transm_error,
    sci_read_error,
    create_sgmnt,
    create_sgmnt_fail,
    thread_yield,
    
    /* sequentialization */
    recv_postponed,
    send_postponed,
    no_incpy_lock,
    no_outcpy_lock,
    ch_smi_mutex_lock,
    ch_smi_mutex_unlock,    

    /* short protocol */
    short_send, 
    short_recv, 
    short_pack, 

    /* eager protocol */
    eager_sgmnt_cnct,
    eager_sgmnt_discnct,
    no_eagerbuf,
    eager_isend,
    eager_send,
    eager_scopy,
    eager_dma,
    eager_recv,
    eager_rcopy,

    /* rndv protocol: sync send */      
    rndv_sgmnt_cnct,
    rndv_sgmnt_discnct,
    rndv_pushsend,
    rndv_isend,
    rndv_send,
    rndv_send_w_ack,
    rndv_ack_send,
    rndv_pushscopy,
    rndv_scopy,
    rndv_dma,

    /* rndv protocol: sync recv */  
    setup_rndvadr,
    rndvmem_split,
    rndv_irecv,
    rndv_ack_recv,
    rndv_pushrecv,
    rndv_pushrcopy,
    rndv_rcopy,

    /* rndv protocol: async send */  
    arndv_isend, 
    arndv_buf2ls,
    arndv_dma, 
    arndv_pio, 
    arndv_send,
    arndv_send_w_ack,
    arndv_ack_send,
    arndv_ack_sendzc,
    arndv_pushsend,

    /* rndv protocol: async recv */  
    arndv_irecv,
    arndv_ls2buf,
    arndv_ack_recv,
    arndv_pushrecv,
    arndv_setup_sendbuf,
    arndv_setup_recvbuff,
    arndv_cnct_dstbuf,
    arndv_discnct_dstbuf,

    /* rndv protocol: blocking send/recv */  
    brndv_send,
    brndv_isend,
    brndv_irecv,
    brndv_ack_send,
    brndv_ack_recv,
    
    /* rndv transfer control */
    nbr_rndv_recvs,
    nbr_rndv_sends,
    rndv_inbuf_size,
    rndv_sync_delay,

    /* single sided */
    sside_put_contig,
    sside_put_cont_cpy,
    sside_put_sametype,
    sside_put_emulate,
    sside_put_emu_remote,
    sside_get_contig,
    sside_get_sametype,
    sside_get_emulate,
    sside_get_emu_remote,
    sside_accu_emulate,
    sside_accu_emu_remote,
    sside_win_create,
    sside_win_free,
    sside_win_sync,
    sside_memcpy,
    sside_delay_store,
    sside_delay_flush,
    sside_delay_process,
    sside_add_tgt_job,
    sside_rm_tgt_job,
    sside_add_jobreq,
    sside_rm_jobreq,
    sside_cmplt_tgt_jobs,
    sside_cmplt_jobreqs,
    
    /* direct pack */
    directPack,
    directUnpack,
    directPackLeave,
    directUnpackLeave,
    directWRITE_direct,
    directWRITE_lbuf,
    directWRITE_part,
    directFLUSH_lbuf,
    directREAD,
    directREAD_part,

    /* resource management */
    acquire_rsrc,
    release_rsrc,
    rmt_mem_map,
    rmt_mem_release,
    loc_mem_create,
    loc_mem_register,
    loc_mem_release,
    rmt_reg_connect,
    rmt_reg_release,

    rsrc_reuse,             /* using existing resource */
    rsrc_create,            /* creating new resource */
    rsrc_release,           /* releasing resource that was used */
    rsrc_unused,            /* a resource that was released has a usage counter of 0 */
    rsrc_failed,            /* resource could not be created */
    rsrc_destroy,           /* destroying resource that is not in use */
    rsrc_cb_disconnect,     /* destroying a resource because it's remote connection was withdrawn (callback) */
    rsrc_release_req,       /* request another process on this node to release a resource */
    rsrc_release_ack,       /* request another process on this node to release a resource */
    rsrc_release_nack,       /* request another process on this node to release a resource */

    /* zero-copy */
    sendbuf_registered,
    recvbuf_registered,
    destbuf_imported,
    zerocopy_canceled,

    /* custom collectives */
    comm_init,
    coll_init,
    barrier,
    reduce,
    allreduce,
    scan,
    allgather,
    alltoall,
    bcast,
    bcast_dma,
    bcast_scopy,
    bcast_rcopy,
    bcast_pcopy,
    rdcpipe_send,
    rdcpipe_scopy,
    rdcpipe_recv,
    rdcpipe_rcopy,
    rdcpipe_sendrecv,
    rdcpipe_pcopy,

    /* MPI memory allocation */
    alloc_mem,
    free_mem,
    alloc_failed,
    alloc_priv,
    alloc_shared_pool,
    alloc_shared_reg,

    /*** stop marker ***/
    fun_dummy
} function_number_t;


typedef struct {
    char name[21];		             /* name of function */
    int number_calls;		             /* how many times called */
    longlong_t min_ticks;		     /* minimal ticks spent (or values counted) */
    longlong_t max_ticks;		     /* maximal ticks spent (or values counted) */
    longlong_t acc_ticks;		     /* accumulated ticks spent (or values counted) */
    stat_type type;
} MPID_SMI_statistics_t;



/* 
 * variables
 */
#ifdef MPID_SMI_STATISTICS
extern longlong_t _mpid_smi_stat_t0, _mpid_smi_stat_t1;
extern int MPID_SMI_do_statistics;	       /* do runtime statistics */
extern MPID_SMI_statistics_t MPID_SMI_statistics[fun_dummy + 1];
#endif

extern double processor_wait_time;


/* 
 * macros
 */
#define CAL_LOOPS 100
#define FRQ_BASE  1000000
#define FRQ_LOOPS (25*FRQ_BASE)

/* commands to MPID_SMI_Set_statistics() */
#define SMI_STAT_DISABLE 0
#define SMI_STAT_ENABLE  1
#define SMI_STAT_RESET   2
#define SMI_STAT_START   3

#ifdef MPID_SMI_STATISTICS
/* measure a complete function */
#define MPID_STAT_ENTRY(fun) \
  longlong_t t0, t1; \
  if (MPID_SMI_do_statistics) \
    { \
      ++MPID_SMI_statistics[fun].number_calls; \
      SMI_Get_ticks(&t0); \
    }
#define MPID_STAT_EXIT(fun) \
  if (MPID_SMI_do_statistics) \
    { \
      SMI_Get_ticks(&t1); \
      MPID_SMI_Update_ticks(t1 - t0, &MPID_SMI_statistics[fun]); \
    }

/* measure a function call - attention: uses global variables! */
#define MPID_STAT_CALL(fun) \
  if (MPID_SMI_do_statistics) \
    { \
      ++MPID_SMI_statistics[fun].number_calls; \
      SMI_Get_ticks(&_mpid_smi_stat_t0); \
    }

#define MPID_STAT_RETURN(fun) \
  if (MPID_SMI_do_statistics) \
    { \
      SMI_Get_ticks(&_mpid_smi_stat_t1); \
      MPID_SMI_Update_ticks(_mpid_smi_stat_t1 - _mpid_smi_stat_t0, &MPID_SMI_statistics[fun]); \
    }

/* special macros to stop times between different functions.
   _mpid_smi_stat_t0 and _mpid_smi_stat_t1 must be defined visible to both macro
   calls. Take care that you really know which timespan you are 
   measuring! */
#define MPID_STAT_START(timespan) \
  if (MPID_SMI_do_statistics) \
    { \
      ++MPID_SMI_statistics[timespan].number_calls; \
      SMI_Get_ticks(&_mpid_smi_stat_t0); \
    }

#define MPID_STAT_END(timespan) \
  if (MPID_SMI_do_statistics) \
    { \
      SMI_Get_ticks(&_mpid_smi_stat_t1); \
      MPID_SMI_Update_ticks(_mpid_smi_stat_t1 - _mpid_smi_stat_t0, &MPID_SMI_statistics[timespan]); \
    }

/* event counter */
#define MPID_STAT_COUNT(event) \
  if (MPID_SMI_do_statistics) \
    { \
      ++MPID_SMI_statistics[event].number_calls; \
    }

/* status probing (for integer values) */
#define MPID_STAT_PROBE(probe, value) \
  if (MPID_SMI_do_statistics) \
    { \
      ++MPID_SMI_statistics[probe].number_calls; \
      MPID_SMI_statistics[probe].acc_ticks += value; \
      if (value < MPID_SMI_statistics[probe].min_ticks) \
          MPID_SMI_statistics[probe].min_ticks = value; \
      if (value > MPID_SMI_statistics[probe].max_ticks) \
          MPID_SMI_statistics[probe].max_ticks = value; \
    }

/* Period probing (for ticks). First, store the start tick value in a location *specified
   by the user* (type longlong). Finally, store the difference of the value found there 
   and the current tick value in the statistics struct. This is useful to measure longer
   periods, distributed via multiple functions etc.  */
#define MPID_STAT_PERIOD_START(value) \
  if (MPID_SMI_do_statistics) \
    { \
      SMI_Get_ticks(&(value)); \
    }

#define MPID_STAT_PERIOD_END(period, value) \
  if (MPID_SMI_do_statistics) \
    { \
      longlong_t _period_t; \
      SMI_Get_ticks(&_period_t); \
      ++MPID_SMI_statistics[period].number_calls; \
      MPID_SMI_Update_ticks(_period_t - value, &MPID_SMI_statistics[period]); \
    }
#else
/* dummy defines if compiling without statistics support */
#define MPID_STAT_ENTRY(fun) 
#define MPID_STAT_EXIT(fun) 
#define MPID_STAT_CALL(fun) 
#define MPID_STAT_RETURN(fun) 
#define MPID_STAT_START(timespan) 
#define MPID_STAT_END(timespan) 
#define MPID_STAT_COUNT(event) 
#define MPID_STAT_PROBE(probe, value)
#define MPID_STAT_PERIOD_START(value)
#define MPID_STAT_PERIOD_END(probe, value)
#endif

/* 
 * function prototypes  
 */

/* initialize statistics module */
extern void MPID_SMI_Statistics_init ( void );

#ifdef MPID_SMI_STATISTICS
#ifdef UPDATE_INLINE
#define MPID_SMI_Update_ticks(t,stat) \
  (stat)->acc_ticks += t; \
  if ((t) < (stat)->min_ticks) (stat)->min_ticks = t; \
  if ((t) > (stat)->max_ticks) (stat)->max_ticks = t;
#else
/* update statistic times */
void MPID_SMI_Update_ticks (longlong_t ticks,	  /* I: ticks measured */
			MPID_SMI_statistics_t * stat  /* I: run-time statistics */
);
#endif
#endif

/* display runtime statistics */
void MPID_SMI_Runtime_statistics( void );
/* turn on/off or reset runtime statistics */
void MPID_SMI_Set_statistics (int );

#endif


