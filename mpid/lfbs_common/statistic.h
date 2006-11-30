/* $Id$ */

#ifndef MPID_STAT_H
#define MPID_STAT_H

#ifndef WIN32
#include <sys/types.h>
#endif

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
#define SMI_Get_ticks(v) QueryPerformanceCounter((LARGE_INTEGER*)v);
#endif

#if defined(MPID_STATISTICS)



typedef enum {
    /*** SEND ***/
    /* MPIR */
    mpi_send=0, 
    /* ADI */
    mpi_senddatatype, 
    mpi_sendcontig, 
     /* ch_smi */
    mpid_getsendpkt, 
    mpid_sendcontrol,

    mpid_send_short, 

    mpid_eager_isend,
    mpid_eager_send,
    mpid_eager_scopy,
    mpid_eager_dma,

    mpid_rndv_pushsend,
    mpid_rndv_senddelay,
    mpid_rndv_isend,
    mpid_rndv_send,
    mpid_rndv_send_w_ack,
    mpid_rndv_ack_send,

    mpid_sendself,

    /*** RECV ***/
    /* MPIR */
    mpi_recv, 
    mpi_recvdatatype,
	mpid_short_recv,
	mpid_message_arrived,
    /* ch_smi */
    mpid_read_control, 

    mpid_eager_recv,
    mpid_eager_rcopy,

    mpid_setup_rndvadr,
    mpid_rndv_irecv,
    mpid_rndv_ack_recv,
    mpid_rndv_pushrecv,

    mpid_recvself,

    /*** non-blocking rendez-vous ***/
    mpid_nrndv_isend, 
    mpid_nrndv_buf2ls,
    mpid_nrndv_dma, 
    mpid_nrndv_send,
    mpid_nrndv_send_w_ack,
    mpid_nrndv_ack_send,
    mpid_nrndv_pushsend,
    mpid_nrndv_senddelay,
    mpid_nrndv_irecv,
    mpid_nrndv_ls2buf,
    mpid_nrndv_ack_recv,
    mpid_nrndv_pushrecv,

    /*** GENERAL ***/
    /* internal */
	shmem_latency,
	mpid_check_seq,
    create_sgmnt,
	stat_overhead,
    
    /*** EVENT COUNTERS ***/
    sci_transm_error,
    create_sgmnt_fail,
    no_sendpkt,
    no_readpkt,
    no_eagerbuf,
    rndvmem_split,
	
    
    /*** stop marker ***/
    fun_dummy
} function_number_t;


typedef struct {
    char name[21];		             /* name of function */
    int number_calls;		             /* how many times called */
    longlong_t min_ticks;		     /* minimal ticks spent */
    longlong_t max_ticks;		     /* maximal ticks spent */
    longlong_t acc_ticks;		     /* accumulated ticks spent */
} MPID_statistics_t;

#endif				       /* defined(MPID_STATISTICS) */


/* 
 * variables
 */
#if defined(MPID_STATISTICS)
extern longlong_t _stat_t0, _stat_t1;
extern int MPID_do_statistics;	       /* do runtime statistics */
extern MPID_statistics_t MPID_statistics[fun_dummy + 1];
#endif

extern double processor_wait_time;


/* 
 * macros
 */
#define CAL_LOOPS 100
#define FRQ_BASE  1000000
#define FRQ_LOOPS (25*FRQ_BASE)

#if defined(MPID_STATISTICS)
/* measure a complete function */
#define MPID_STAT_ENTRY(fun) \
  longlong_t t0, t1; \
  if (MPID_do_statistics) \
    { \
      ++MPID_statistics[fun].number_calls; \
      SMI_Get_ticks(&t0); \
    }
#define MPID_STAT_EXIT(fun) \
  if (MPID_do_statistics) \
    { \
      SMI_Get_ticks(&t1); \
      MPID_Update_ticks(t1 - t0, &MPID_statistics[fun]); \
    }

/* measure a function call - attention: uses global variables! */
#define MPID_STAT_CALL(fun) \
  if (MPID_do_statistics) \
    { \
      ++MPID_statistics[fun].number_calls; \
      SMI_Get_ticks(&_stat_t0); \
    }

#define MPID_STAT_RETURN(fun) \
  if (MPID_do_statistics) \
    { \
      SMI_Get_ticks(&_stat_t1); \
      MPID_Update_ticks(_stat_t1 - _stat_t0, &MPID_statistics[fun]); \
    }

/* special macros to stop times between different functions.
   _stat_t0 and _stat_t1 must be defined visible to both macro
   calls. Take care that you really know which timespan you are 
   measuring! */
#define MPID_STAT_START(timespan) \
  if (MPID_do_statistics) \
    { \
      ++MPID_statistics[timespan].number_calls; \
      SMI_Get_ticks(&_stat_t0); \
    }

#define MPID_STAT_END(timespan) \
  if (MPID_do_statistics) \
    { \
      SMI_Get_ticks(&_stat_t1); \
      MPID_Update_ticks(_stat_t1 - _stat_t0, &MPID_statistics[timespan]); \
    }

/* event counter */
#define MPID_STAT_COUNT(event) \
  if (MPID_do_statistics) \
    { \
      ++MPID_statistics[event].number_calls; \
    }

#define MPID_STAT_LATENCY(start,end) \
	if (MPID_do_statistics) \
		{ \
			++MPID_statistics[shmem_latency].number_calls;\
			MPID_Update_ticks((end)-(start), &MPID_statistics[shmem_latency]); \
		}	
#else
/* dummy defines */
#define MPID_STAT_ENTRY(fun) 
#define MPID_STAT_EXIT(fun) 
#define MPID_STAT_CALL(fun) 
#define MPID_STAT_RETURN(fun) 
#define MPID_STAT_START(timespan) 
#define MPID_STAT_END(timespan) 
#define MPID_STAT_COUNT(event) 
#define MPID_STAT_LATENCY(start,end)
#endif

/* 
 * function prototypes  
 */

/* initialize statistics module */
extern void MPID_Statistics_init ( void );

#if defined(MPID_STATISTICS)
#if defined(INLINE)
#define MPID_Update_ticks(t,stat) \
  (stat)->acc_ticks += t; \
  if ((t) < (stat)->min_ticks) (stat)->min_ticks = t; \
  if ((t) > (stat)->max_ticks) (stat)->max_ticks = t;
#else
/* update statistic times */
void MPID_Update_ticks (longlong_t ticks,	  /* I: ticks measured */
			MPID_statistics_t * stat  /* I: run-time statistics */
);
#endif
#endif

/* display runtime statistics */
void MPID_Runtime_statistics( void );
/* turn on/off or reset runtime statistics */
void MPID_Set_statistics (int );

#endif


