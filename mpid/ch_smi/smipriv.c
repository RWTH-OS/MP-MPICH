/* $Id$

   internal setup and basic message and memory control 
 */

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "smi.h"

#include "adi2config.h"
#include "mpichconf.h"
#include "fifo.h"
#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "smistat.h"
#include "getus.h"
#include "smidef.h"
#include "smicheck.h"
#include "smiperf.h"
#include "smirndv.h"

/* imports */
/* process-local information for sending & receiving control packets (from smishort.c) */
extern MPID_SMI_lShortsend_t *MPID_SMI_lShortsend;
extern MPID_SMI_lShortrecv_t *MPID_SMI_lShortrecv;
extern int *MPID_SMI_Shregid_rndv;
extern int MPID_SMI_Locregid_rndv;

/* exports */
/* system dependand constants */
int MPID_SMI_PAGESIZE;    /* the effective SMI pagesize */
int MPID_SYS_PAGESIZE;    /* the local system pagesize */
int MPID_SMI_STREAMSIZE;
int MPID_SMI_SCI_TA_SIZE;
int MPID_SMI_SCI_TA_SLOTS;
int MPID_SMI_QUEUE_SIZE;
int MPID_SMI_NBRADPTS;    /* number of PCI-SCI adapters */
int MPID_SMI_DEFADPT;     /* number of default PCI-SCI adapter */
int MPID_SMI_DMA_SIZE_ALIGN;
int MPID_SMI_DMA_OFFSET_ALIGN;

/* referenced in smiperf.h */
longlong_t _mpid_smi_perf_tickref;
size_t _mpid_smi_perf_size;

/* contains all SCI-MPICH configuration constants (initialized at startup from 
   device configuration file, constant throughout execution) */
MPID_SMI_CFG_T MPID_SMI_cfg;

#ifdef MPID_USE_DEVTHREADS
#include <pthread.h>
/* the threads need to know who they are */
MPID_SMI_THREADKEY_T MPID_SMI_thread_type;

/* mutex' for multithreaded usage of the device */
MPID_SMI_LOCK_T MPID_SMI_incoming_lck;
MPID_SMI_LOCK_T MPID_SMI_freepkts_lck;
MPID_SMI_LOCK_T MPID_SMI_getpkts_lck;
MPID_SMI_LOCK_T MPID_SMI_connect_lck;
MPID_SMI_LOCK_T MPID_SMI_allocate_lck;
MPID_SMI_LOCK_T MPID_SMI_async_check_lck;
MPID_SMI_LOCK_T MPID_SMI_counters_lck;
MPID_SMI_LOCK_T MPID_SMI_waitmsg_lck;
#endif

#ifdef MPIR_ENABLE_THREADS
#include <pthread.h>
/* mutex' for multithreaded usage of the library */

#endif

/* ptr to remote memory copy-function actually used */
mpid_smi_memcpy_fcn_t MPID_SMI_memcpy;


/*
  internal communication buffers
*/

/* MPID_SMI_Int_info_imp[proc] points to memory into which we import information
   from proc; this memory is located in SCI memory on this node */
MPID_SMI_Int_data **MPID_SMI_Int_info_imp;

/* MPID_SMI_Int_info_exp[proc] points to memory into which we xeport information
   to proc; this memory is located in SCI memory on proc's node */
MPID_SMI_Int_data **MPID_SMI_Int_info_exp;

/* size per proc of SCI memory that is allocated behind the short buffers */
int MPID_SMI_Int_bufsize;



/* pre-allocated buffers for stream buffer flushing */
MPID_SMI_MSGFLAG_T *MPID_SMI_flagbuf_0;
MPID_SMI_MSGFLAG_T *MPID_SMI_flagbuf_1;

/* id's of the SMI locks for memcpy() synchronization */ 
int *MPID_SMI_memlocks_in;
int *MPID_SMI_memlocks_out;

/* local globals */
static int MPID_SMI_SCI_TA_LD;     /* log base 2 of the SCI transaction size */
static int MPID_SMI_SHORTSIZE_LD;  /* log base 2 of size of the packet size for short msgs */
static int MPID_SMI_STREAMSIZE_LD; /* log base 2 of the streambuffer size */
static int MPID_SMI_MSGFLAG_LD;    /* log base 2 of the size of the msg flag */
static int MPID_SMI_ta_payload;    /* payload of the first SCI data-move transaction in a short msg */
static int max_load_first_slot;
static int max_load_end_slot;
static int check_drank;            /* which incoming control-queue to check next (device-rank) */

static MPID_FIFO_t MPID_SMI_postponed_recvs;
static MPID_FIFO_t MPID_SMI_postponed_sends;

#define MSGID_T_BITS (sizeof(MPID_SMI_MSGID_T)*8)

#if (SENDCTRL_ALIGN_LONGS != 1) && (SENDCTRL_ALIGN_LONGS != 2)
#error valid values for SENDCTRL_ALIGN_LONGS are 1 and 2! (see smidef.h)
#endif

/* for remote-write optimizations (long writes instead int on Alpha) */
#ifdef MPI_LINUX_ALPHA
#define MSGFLAG_SIZE sizeof(long)
#define CHKSUM_SIZE  sizeof(long)
#else
#define MSGFLAG_SIZE sizeof(MPID_SMI_MSGFLAG_T)
#define CHKSUM_SIZE  sizeof(CSUM_VALUE_TYPE)
#endif

static int MPID_op = 0;
static int MPID_readcnt = 0;
static int MPID_freecnt = 0;
static int MPID_pktflush;

static int *next_slot = 0;    
static int *out_of_order_slots = 0;

/* local prototypes */
void MPID_SMI_lbarrier (void);

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* bcopy requires a wrapper because src and dest are swapped in the parameter list. */
void _mpid_smi_bcopy (void *dest, const void *src, size_t n)
{
#ifdef WIN32
    memcpy(dest,src,n);
#else
    bcopy (src, dest, n);
#endif
}

void _mpid_smi_smi_memcpy (void *dest, const void *src, size_t n)
{
  SMI_Memcpy(dest, (void *)src, n, SMI_MEMCPY_FAST);
}

/* returns the logarithm to base 2 of v 
   CAUTION: v needs to be 2^x, x > 0 */
uint MPID_SMI_ld(uint v) 
{
    uint ld_v = 0;
    
    while (!(v & 1)) {
	v = v >> 1;
	ld_v++;
    }

    return ld_v;
}

/* performs a 'rot'-times rotate_right for an wide'-bit number */
uint MPID_SMI_rotate_right(uint value, uint wide, uint rot) 
{
    return ((value >> rot) | (value << (wide - rot))); 
}

/* Compute the number of slots that a short message with a given payload occupied. */
uint MPID_SMI_Get_Slots(uint size)
{
    int size_middle;

    if (size > max_load_first_slot) {
	size_middle = size - max_load_first_slot;

	if (size_middle <= max_load_end_slot)
	    return 2;
	else
	    return (3 + ((size_middle - MPID_SMI_SCI_TA_SIZE + sizeof(MPID_SMI_MSGFLAG_T) + sizeof(CSUM_VALUE_TYPE) - 1)
			 / MPID_SMI_SCI_TA_SIZE));
    } else
      return 1;
}

#define MPID_SMI_NBR_SLOTS(size) (size) <= max_load_first_slot ? 1 : MPID_SMI_Get_Slots(size)


/* returns the value of the highest bit in 'v' which is non-zero  */
uint MPID_SMI_msb(uint v) 
{
    uint i;
    uint msb = 0;
    
    for (i = 1; i <= sizeof(v)*8; i++) {
	if (v & 1)
	    msb = i;
	v >>= 1;
    }

    return (msb > 0 ? 1 << (msb-1) : 0);
}

void MPID_SMI_Lowlevel_Init () 
{
    long i, j, wc;
    smi_error_t smi_err;
    ulong tick_scale;
    int  first_proc, ps_per_tick;
    double us_per_tick;

    /* init timers etc. */
    MPID_SMI_Set_statistics(2);

    check_drank = (MPID_SMI_myid + 1) % MPID_SMI_numids;

    /* Which adapters can we use? However, using multiple adapters is mostly
       transparent by the SMI library. */
    SMIcall(SMI_Query(SMI_Q_SCI_NBRADAPTERS, MPID_SMI_myid, &MPID_SMI_NBRADPTS));
    SMIcall(SMI_Query(SMI_Q_SCI_DEFADAPTER, 0, &MPID_SMI_DEFADPT));

    SMIcall(SMI_Set_adapter (MPID_SMI_cfg.ADPTMODE));

    /* allocate and initialize the id / padding data */
    ALLOCATE(MPID_SMI_flagbuf_0, MPID_SMI_MSGFLAG_T *, MPID_SMI_STREAMSIZE*(MPID_SMI_SHORTID+1));
    ALLOCATE(MPID_SMI_flagbuf_1, MPID_SMI_MSGFLAG_T *, MPID_SMI_STREAMSIZE*(MPID_SMI_SHORTID+1));
    for (i = 0; i <= MPID_SMI_SHORTID; i++) {
	for (j = 0; j < MPID_SMI_STREAMSIZE/sizeof(MPID_SMI_MSGFLAG_T); j++) {
	    MPID_SMI_flagbuf_0[i*MPID_SMI_STREAMSIZE/sizeof(MPID_SMI_MSGFLAG_T) + j] = 
		i << MSGID_T_BITS;
	    MPID_SMI_flagbuf_1[i*MPID_SMI_STREAMSIZE/sizeof(MPID_SMI_MSGFLAG_T) + j] = 
		(i << MSGID_T_BITS) | 1;
	}
    }

    /* to make some arithmetic operations more efficient */
    MPID_SMI_SCI_TA_LD = MPID_SMI_ld(MPID_SMI_SCI_TA_SIZE);
    MPID_SMI_STREAMSIZE_LD = MPID_SMI_ld(MPID_SMI_STREAMSIZE);
    MPID_SMI_SHORTSIZE_LD= MPID_SMI_ld(MPID_SMI_SHORTSIZE);
    MPID_SMI_MSGFLAG_LD = MPID_SMI_ld(sizeof(MPID_SMI_MSGFLAG_T));

    /* for internal single-sided information exchange */
    MPID_SMI_Int_bufsize = MPID_SMI_numids * (sizeof(MPID_SMI_Int_data) + MPID_SMI_EAGERBUFS*sizeof(char**));
    ALLOCATE (MPID_SMI_Int_info_exp, MPID_SMI_Int_data **, MPID_SMI_numids*sizeof(MPID_SMI_Int_data *));
    ALLOCATE (MPID_SMI_Int_info_imp, MPID_SMI_Int_data **, MPID_SMI_numids*sizeof(MPID_SMI_Int_data *));
    /* the pointer initialization is done in MPID_SMI_Short_MemSetup() */

    /* create locks to synchronize remote memcpy operations: in this case, the locks guarantee
       that only a single process at a time writes to a memory segment exported by a single 
       node or a single adapter. Locking a single node is the easiest implementation, but does 
       not consider multiple adapters inside a node. */
    if (MPID_SMI_cfg.MEMCPYSYNC_MODE != MEMCPYSYNC_NONE) {
	/* XXX for now, use the per-node locking scheme. Efficient use of multiple SCI adapters
	   will require a per-adapter locking scheme. */
	if (MPID_SMI_cfg.MEMCPYSYNC_MODE | MEMCPYSYNC_OUT) {
	    ZALLOCATE (MPID_SMI_memlocks_out, int *, MPID_SMI_numNodes*sizeof(int));
	    for (i = 0; i < MPID_SMI_numNodes; i++) {
		SMIcall( SMI_First_proc_on_node(i, &first_proc) );
		smi_err = SMI_Mutex_init_with_locality(&MPID_SMI_memlocks_out[i], first_proc );
		if (smi_err != SMI_SUCCESS) {
		    /* If the mutex_init fails (not enough memory, ...), we fully disable this
		       memcpy-sync mode. All these mutex_operations are collective, this means
		       we won't get deadlocks. */ 
		    if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
			fprintf (stderr, "*** SCI-MPICH warning:could not initialize memsync-locks!\n");
			fprintf (stderr, " -> disabling outgoing memcpy synchronization.)\n");
		    }
		    for (j = 0; j < i; j++) {
			SMIcall (SMI_Mutex_destroy(MPID_SMI_memlocks_out[j]));
		    }
		    FREE (MPID_SMI_memlocks_out);
		    MPID_SMI_cfg.MEMCPYSYNC_MODE &= ~MEMCPYSYNC_OUT;
		    break;
		}
	    }
	}
	if (MPID_SMI_cfg.MEMCPYSYNC_MODE | MEMCPYSYNC_IN) {
	    ZALLOCATE (MPID_SMI_memlocks_in, int *, MPID_SMI_numNodes*sizeof(int));
	    for (i = 0; i < MPID_SMI_numNodes; i++) {
		SMIcall( SMI_First_proc_on_node(i, &first_proc) );
		smi_err = SMI_Mutex_init_with_locality(&MPID_SMI_memlocks_in[i], first_proc );
		if (smi_err != SMI_SUCCESS) {
		    /* If the mutex_init fails (not enough memory, ...), we fully disable this
		       memcpy-sync mode. All these mutex_operations are collective, this means
		       we won't get deadlocks. */ 
		    if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
			fprintf (stderr, "*** SCI-MPICH warning:could not initialize memsync-locks!\n");
			fprintf (stderr, " -> disabling incoming memcpy synchronization.)\n");
		    }
		    for (j = 0; j < i; j++) {
			SMIcall (SMI_Mutex_destroy(MPID_SMI_memlocks_in[j]));
		    }
		    FREE (MPID_SMI_memlocks_in);
		    MPID_SMI_cfg.MEMCPYSYNC_MODE &= ~MEMCPYSYNC_IN;
		    break;
		}
	    }
	}
    }
    /* The init of the mutex's is collective, but single-process nodes don't need to 
       do memcpy_syncs for outgoing communication. */
    if (MPID_SMI_numProcsOnNode[MPID_SMI_myNode] == 1)
	MPID_SMI_cfg.MEMCPYSYNC_MODE &= ~MEMCPYSYNC_OUT;
    
    /* is any padding required to make the available size for data in short packets
       a multiple of SENDCTRL_ALIGN ? */
    MPID_SMI_cfg.SENDCTRL_PAD = 
	(MPID_SMI_SCI_TA_SIZE - MPID_PKT_SHORT_SIZE - sizeof(MPID_SMI_MSGFLAG_T) - CSUMHEAD_SIZE) 
      % SENDCTRL_ALIGN;
    /* IMPORTANT: MPID_SMI_ta_payload must be divisible by SENDCTRL_ALIGN ! Therefor, 
       MPID_SMI_cfg.SENDCTRL_PAD will be initialized accordingly */
    MPID_SMI_ta_payload = MPID_SMI_SCI_TA_SIZE - MPID_PKT_SHORT_SIZE - sizeof(MPID_SMI_MSGFLAG_T) 
      - CSUMHEAD_SIZE - MPID_SMI_cfg.SENDCTRL_PAD;

    max_load_first_slot = MPID_SMI_ta_payload;
    max_load_end_slot = MPID_SMI_SCI_TA_SIZE - sizeof(MPID_SMI_MSGFLAG_T) - sizeof(CSUM_VALUE_TYPE);

    /* Number of available Slots of MPID_SMI_SCI_TA_SIZE Bytes */
    MPID_SMI_SCI_TA_SLOTS = (MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS) / MPID_SMI_SCI_TA_SIZE;

    /* Sizeof MessageQueue in Bytes */
    MPID_SMI_QUEUE_SIZE = MPID_SMI_SCI_TA_SLOTS * MPID_SMI_SCI_TA_SIZE;

    /* allocate and initialize counter fields for FreeRecvPkt() */
    ALLOCATE (next_slot, int *, MPID_SMI_numids*sizeof(int));
    ALLOCATE (out_of_order_slots, int *, MPID_SMI_numids*sizeof(int));
    for (i = 0; i < MPID_SMI_numids; i++) {
	next_slot[i] = 0;
	out_of_order_slots[i] = 0;
    }
    
    MPID_SMI_CSUM_INIT;
    
    MPID_SMI_INIT_LOCK(&MPID_SMI_incoming_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_freepkts_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_getpkts_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_connect_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_allocate_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_async_check_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_counters_lck);
    MPID_SMI_INIT_LOCK(&MPID_SMI_waitmsg_lck);

    /* Translate the settings for performance modelling: they are specified 
       in us and MB/s, but we use 'ticks' internally to allow for low-overhead timing. 
       Therefore, the latencies are given in 'ticks', and the max bandwidth in bytes/megaticks. */
    SMI_Query (SMI_Q_SYS_TICK_DURATION, 0, &ps_per_tick);
    us_per_tick = ps_per_tick/1e+6;
    tick_scale = 1 << PERF_TICKSCALE;
    MPID_SMI_cfg.PERF_GAP_LTNCY   = (CFG_VAR_TYPE)(MPID_SMI_cfg.PERF_GAP_LTNCY/us_per_tick);
    MPID_SMI_cfg.PERF_SEND_LTNCY  = (CFG_VAR_TYPE)(MPID_SMI_cfg.PERF_SEND_LTNCY/us_per_tick);
    MPID_SMI_cfg.PERF_RECV_LTNCY  = (CFG_VAR_TYPE)(MPID_SMI_cfg.PERF_RECV_LTNCY/us_per_tick);
    MPID_SMI_cfg.PERF_BW_LIMIT    = (CFG_VAR_TYPE)(MPID_SMI_cfg.PERF_BW_LIMIT * us_per_tick 
						  * tick_scale / 1e+6) ;

    if (MPID_SMI_cfg.MEMCPY_TYPE == MEMCPY_TYPE_AUTO) {
	/* choose the best memcpy function for remote memory writes */
	SMI_Query(SMI_Q_SYS_WRITECOMBINING, 0, &wc);
#ifdef MPI_LINUX
	/* this query does currently only work for Linux */
	switch (wc) {
	    int cacheline_len;
	case SMI_WC_ENABLED:
	    SMI_Query (SMI_Q_SYS_CPU_CACHELINELEN, 0, &cacheline_len);
	    MPID_SMI_memcpy = (cacheline_len == 64) ? 
		(mpid_smi_memcpy_fcn_t)_mpid_smi_wc64_memcpy : 
		(mpid_smi_memcpy_fcn_t)_mpid_smi_wc32_memcpy;
	    break;
	case SMI_WC_DISABLED:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_prefetchnta_memcpy;
	    break;
	default:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_prefetchnta_memcpy;
	}
#elif defined MPI_LINUX_X86_64 || defined MPI_LINUX_IA64
	/* use remote memcpy optimized in SISCI */
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_smi_memcpy;
#elif defined MPI_solaris
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)memcpy;
#elif defined MPI_solaris86
	/* until MTRR support on Solaris is available & integrated in the IRM */
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_memcpy;
#elif defined WIN32
#if defined(_M_IX86)
	/* under Win32, write-combining is always (?) established */
	/* defaults to 64 for p4 systems */
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_wc64_memcpy;
#elif defined(_M_AMD64)
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_sse64_memcpy;
#else
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)memcpy;
#endif
#elif defined MPI_LINUX_ALPHA
#if 0
	/* XXX experimental */
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_alpha_memcpy;
#else
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t) memcpy;
#endif
#else
	MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)memcpy;
#endif
    } else 
	switch (MPID_SMI_cfg.MEMCPY_TYPE) {
	case MEMCPY_TYPE_MEMCPY:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)memcpy;
	    break;
	case MEMCPY_TYPE_SMI:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_smi_memcpy;
	    break;
	case MEMCPY_TYPE_BCOPY:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_bcopy;
	    break;
#if defined MPI_LINUX || defined MPI_solaris86 || defined WIN32
#if !(defined(WIN32) && defined(_M_AMD64))
	/* these should be made available for Win x64, too - requires some assembly 
	   translation. */
	case MEMCPY_TYPE_WC32:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_wc32_memcpy;
	    break;
	case MEMCPY_TYPE_WC64:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_wc64_memcpy;
	    break;
	case MEMCPY_TYPE_MMX32:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx32_memcpy;
	    break;
	case MEMCPY_TYPE_MMX64:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx64_memcpy;
	    break;
	case MEMCPY_TYPE_MMX:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_memcpy;
	    break;
#endif
	case MEMCPY_TYPE_SSE32:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_sse32_memcpy;
	    break;
	case MEMCPY_TYPE_SSE64:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_sse64_memcpy;
	    break;
#ifndef WIN32
	    /* XXX these should be made available for Win32, too (at least 
	       the SSE based ones) - requires some assembly translation. */
	case MEMCPY_TYPE_MMX_PRE:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_prefetchnta_memcpy;
	    break;
#endif
#endif
#if defined MPI_LINUX_ALPHA
	case MEMCPY_TYPE_ALPHA:
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)_mpid_smi_alpha_memcpy;
	    break;
#endif
	default:
	    if (MPID_SMI_myid == 0)
		fprintf (stderr, "*** SCI-MPICH warning: unsupported MEMCPY_TYPE %d; using default memcpy().\n",
			 MPID_SMI_cfg.MEMCPY_TYPE);
	    MPID_SMI_memcpy = (mpid_smi_memcpy_fcn_t)memcpy;
	    break;
	}
    
    MPID_SMI_postponed_recvs = MPID_FIFO_init (MPID_UTIL_THREADSAFE);
    MPID_SMI_postponed_sends = MPID_FIFO_init (MPID_UTIL_THREADSAFE);

    /* check for DMA limits */
    SMIcall (SMI_Query(SMI_Q_SCI_DMA_SIZE_ALIGN, 0, &MPID_SMI_DMA_SIZE_ALIGN));
    SMIcall (SMI_Query(SMI_Q_SCI_DMA_OFFSET_ALIGN, 0, &MPID_SMI_DMA_OFFSET_ALIGN));

    return;
}


/* shared memory allocation and deallocation */
void *MPID_SMI_shmalloc(size, shreg_id)
    int size, shreg_id; 
{
    void *p = 0;

    MPID_SMI_ASYNC_LOCK(&MPID_SMI_allocate_lck);
    SMI_Imalloc( size, shreg_id, &p );
    MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_allocate_lck);
    return (p);
}

void MPID_SMI_shfree(ptr)
    char *ptr;
{
    int err, region_id;

    if (!ptr)
	return;

#if 0
    /* check if this is an address from the SCI shared memory */ 
    if ((err = SMI_Adr_to_region (ptr, &region_id)) == SMI_ERR_PARAM)
	free (ptr);
    else
#endif 
    {
        MPID_SMI_ASYNC_LOCK(&MPID_SMI_allocate_lck);
	SMIcall (SMI_Ifree (ptr));
	MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_allocate_lck);
    }
    return;
}


/* simply map this function to SMI_Barrier() */
void MPID_SMI_lbarrier()
{
    SMIcall( SMI_Barrier() );
    return;
}


/* high-resolution time via x86-clocks using the SMI library function*/
void MPID_Wtime(double *t)
{
    *t = SMI_Wtime();
}


void MPID_SMI_Get_version (version)
    char *version;
{
    int len;

    sprintf(version, " %s via ", MPIDTRANSPORT);
    len = MPID_MAX_VERSION_NAME - (strlen(MPIDTRANSPORT) + 7);
    
    SMI_Query (SMI_Q_SCI_API_VERSION, len, &(version[strlen(MPIDTRANSPORT)+6]));

    return;
}


/* Return the pid of the process id of the id corresponding to 
   MPI_COMM_WORLD */
/* host_name is used to get IP address for some network address valid for the
   processor running the process.
   image_name is the name of the executable image being run.
 */
static int MPID_SMI_proc_info( id, host_name, image_name )
    int id;
char **host_name;
char **image_name;
{
    /* XXX include code for SMI here. It would be best to 
       initialize the required data structures in p2p_init() as 
       a guaranteed global synchronisation point

       Until then, return the local PID if requested and and 
       bogus PID for other cases */
    
    *host_name = 0;	  /* TV assumes "the same as parent" if it sees 0 */
    *image_name= 0;         /*  ditto */
    
    /* We know that this array *is* now ordered with respect to COMM_WORLD
     * and that COMM_WORLD[0] is the parent who *isn't* in this array.
     */
    if ( id == MPID_SMI_myid )
	return getpid();
    else
	return 0;
}

void MPID_SMI_finalize( void )
{
    SMIcall ( SMI_Finalize() );

    FREE( MPID_SMI_flagbuf_0 );
    FREE( MPID_SMI_flagbuf_1 );
    FREE( MPID_SMI_Int_info_exp );
    FREE( MPID_SMI_Int_info_imp );
    if (MPID_SMI_cfg.MEMCPYSYNC_MODE != MEMCPYSYNC_NONE) {
	FREE( MPID_SMI_memlocks_in );
	FREE( MPID_SMI_memlocks_out );
    }
    FREE( next_slot );
    FREE( out_of_order_slots );

    MPID_SMI_DESTROY_LOCK(&MPID_SMI_incoming_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_freepkts_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_getpkts_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_connect_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_allocate_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_async_check_lck);
    MPID_SMI_DESTROY_LOCK(&MPID_SMI_counters_lck);
    
    MPID_FIFO_destroy (MPID_SMI_postponed_recvs);
    MPID_FIFO_destroy (MPID_SMI_postponed_sends);

    if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
	fprintf (stdout, "SCI-MPICH finalized.\n");
	fflush (stdout);
    }

}

/* If any of the incoming queues contains a packet, it returns the rank of 
   the sender of this packet (the first one found). Returns -1 if no packet
   is available. */
int MPID_SMI_pkt_avail( void )
{
    MPID_SMI_MSGFLAG_T msg_flag = 0;
    MPID_SMI_MSGID_T   msg_id;
    int          got_msg = 0;
    int          nbr_checks = 0;
    MPID_DEBUG_CODE (static int no_msg = 0;);
    
    /* check all control packet queues */
    while (!got_msg) {
	msg_flag = *MPID_SMI_lShortrecv[check_drank].msg_flag;
	msg_id   = msg_flag >> MSGID_T_BITS;
	got_msg = (MPID_SMI_lShortrecv[check_drank].newmsg_id == msg_id);
	if (got_msg || ++nbr_checks == MPID_SMI_numids) 
	    break;

	check_drank = (check_drank + 1) % MPID_SMI_numids;
    }

    if (!got_msg) {
	MPID_DEBUG_CODE (if (!no_msg))
	    MPID_SMI_DEBUG_PRINT_MSG("Leaving pkt_avail() (no messages)");
	MPID_DEBUG_CODE (no_msg = 1);
	return -1;
    }
    MPID_DEBUG_CODE (no_msg = 0);

    MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE, "[%d] found packet from %d, id %d, offset = %d\n", 
			       MPID_SMI_myid, check_drank, msg_id,
			       (size_t)(MPID_SMI_lShortrecv[check_drank].recv_ptr) -
			       (size_t)(MPID_SMI_lShortrecv[check_drank].recv_buffer)); );
    return check_drank;
}


/* 
   Poll all message queues (for *from == -1) for an incoming control packet or
   directly deliver a control packet from a specified queue 
 */
int MPID_SMI_ReadControl( pkt, from )
    MPID_PKT_T **pkt;
    int *from;
{
    MPID_PKT_T *inpkt;
    MPID_SMI_MSGID_T msg_id, msg_cont;
    MPID_SMI_MSGFLAG_T msg_flag;
    MPID_SMI_MSGFLAG_T mask = 0;
    MPID_CSUM_T * volatile csum_ptr;
    uint backoff = 0, wait, count = 0, remaining = 0, end_reached = 0, this_slot;
    uint csum_ok, csum_retry = 0;
    int from_grank = 0;
    int got_msg = 0;
    int sci_ta_nbr = 0;
    ulong queue_end; 
    size_t next_recv_ptr = 0, flag_ptr = 0;
    CSUM_VALUE_TYPE csum;
    
    MPID_DEBUG_CODE (int show_ids = 1;)

    MPID_STAT_ENTRY(readcontrol);
    MPID_TRACE_CODE("ReadControl", *from);
    MPID_SMI_DEBUG_PRINT_MSG("Entering ReadControl");

    inpkt = NULL;
    backoff = 1;
    mask = ~mask;

    if (*from < 0){
	/* poll all control packet queues until any msg arrives */
	while (!got_msg) {
	    msg_flag = *(MPID_SMI_lShortrecv[from_grank].msg_flag);
	    msg_id   = msg_flag >> MSGID_T_BITS;
	    MPID_DEBUG_IFCODE (if(show_ids) {
		fprintf( MPID_DEBUG_FILE,
			 "[%d] checking packet from (%d), offset = %d, expct'd msg_id %d, found %d\n",
			 MPID_SMI_myid, from_grank,
			 (size_t)(MPID_SMI_lShortrecv[from_grank].recv_ptr) -
			 (size_t)(MPID_SMI_lShortrecv[from_grank].recv_buffer),
			 MPID_SMI_lShortrecv[from_grank].newmsg_id, msg_id);
		fflush( MPID_DEBUG_FILE ); })

	    if (got_msg = (MPID_SMI_lShortrecv[from_grank].newmsg_id == msg_id)) {
		break;
	    }
	    if (from_grank == MPID_SMI_numids - 1) {
		MPID_STAT_COUNT(no_readpkt);
		MPID_DEBUG_CODE (show_ids = 0;)

		/* backoff to release bus - this code should not be optimized! */
		if (backoff < MPID_SMI_BACKOFF_LMT) 
		    backoff++;		
		wait = backoff;
		while (wait)
		    wait--;

		from_grank = 0;
	    } else
		from_grank++;
	}
	*from = from_grank;
    } else {
	msg_flag = *(MPID_SMI_lShortrecv[*from].msg_flag);
	msg_id   = msg_flag >> MSGID_T_BITS;

	from_grank = *from;
    }

    this_slot = ((size_t)MPID_SMI_lShortrecv[from_grank].recv_ptr 
		 - (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer) >> MPID_SMI_SCI_TA_LD;
    remaining = MPID_SMI_SCI_TA_SLOTS - (this_slot + 1);

    MPID_SMI_DEBUG_PRINT_RECV_PKT("found packet", (MPID_PKT_T *)(MPID_SMI_lShortrecv[from_grank].recv_ptr
							+ CSUMHEAD_SIZE));

    /* wait until the checksum of the first slot is correct to ensure that really the 
      *complete* data has arrived */
    csum_ptr = (MPID_CSUM_T *)MPID_SMI_lShortrecv[from_grank].recv_ptr;

    /* CSUM of 0 is invalid, and the lenght of the checksum needs to be reasonable -
       if it is inconsistent, make sure it isn't to long to avoid SEGVs! */
    backoff  = 0;
    while (csum_ptr->csum == 0 || csum_ptr->csum_len > MPID_SMI_SHORTSIZE) {
	/* backoff to release bus - this code should not be optimized! */
	if (backoff < MPID_SMI_BACKOFF_LMT) 
	    backoff++;		
	wait = backoff;
	while (wait)
	    wait--;
    }
    
    backoff = 0;
    MPID_SMI_CSUM_OK(csum_ptr->csum, (unsigned char *)csum_ptr + CSUMHEAD_SIZE, 
		     csum_ptr->csum_len, (unsigned char)msg_id, from_grank, csum_ok);
    while (!csum_ok) {
	MPID_STAT_COUNT(csum_retry_ctrl);
	if (++csum_retry > MAX_CSUM_RETRIES) {
	    MPID_ABORT ("Unresolvable CSUM mismatch in Readcontrol()" );
	}
	MPID_DEBUG_CODE (if (csum_retry % (MAX_CSUM_RETRIES/10) == 0) \
			 fprintf (stderr, "[%d] ReadControl: high CSUM error rate!\n", MPID_SMI_myid); )
	    
        /* backoff to release bus - this code should not be optimized! */
	if (backoff < MPID_SMI_BACKOFF_LMT) 
	    backoff++;		
	wait = backoff;
	while (wait)
	    wait--;

	MPID_SMI_CSUM_OK(csum_ptr->csum, (unsigned char *)csum_ptr + CSUMHEAD_SIZE, 
			 csum_ptr->csum_len, (unsigned char)msg_id, from_grank, csum_ok);
    }

    MPID_SMI_DEBUG_PRINT_MSG("CSUM o.k., waiting for complete msg");
    /* 'msg_cont' SCI transactions will follow which make up the complete short message */
    msg_cont = msg_flag | (mask << MSGID_T_BITS);

    /* Deliver the found packet to the caller */
    MPID_SMI_DEBUG_PRINT_MSG("msg complete, delivering to caller");
    *pkt  = (MPID_PKT_T *)(MPID_SMI_lShortrecv[from_grank].recv_ptr + CSUMHEAD_SIZE);


    /* advance the pointers to the next packet to come */
    next_recv_ptr = (size_t)MPID_SMI_lShortrecv[from_grank].recv_ptr + ((msg_cont + 1) << MPID_SMI_SCI_TA_LD);
    queue_end = (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer + MPID_SMI_QUEUE_SIZE;
    if (next_recv_ptr  >= ((size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer + MPID_SMI_QUEUE_SIZE))
	MPID_SMI_lShortrecv[from_grank].recv_ptr = (MPID_SMI_lShortrecv[from_grank].recv_buffer + 
					       (next_recv_ptr - queue_end));
    else
	MPID_SMI_lShortrecv[from_grank].recv_ptr = (char *)next_recv_ptr;

    /* the flag (pointer and value) to poll on */
    MPID_SMI_lShortrecv[from_grank].msg_flag = 
	(MPID_SMI_MSGFLAG_T *)(MPID_SMI_lShortrecv[from_grank].recv_ptr 
			       + MPID_SMI_SCI_TA_SIZE - sizeof(MPID_SMI_MSGFLAG_T));
    MPID_SMI_lShortrecv[from_grank].newmsg_id = 
	(MPID_SMI_lShortrecv[from_grank].newmsg_id % MPID_SMI_SHORTID) + 1;    
    
    PERF_RECV_LATENCY(MPID_SMI_cfg.PERF_RECV_LTNCY)
    MPID_STAT_EXIT(readcontrol);
    return MPI_SUCCESS;
}


/* Free a packet 'pkt' of the short protocol which was received from
   process 'from_grank'. The 'size' is only relevant for inlined data to
   indicate how much data has been transported. For all control messages,
   IS_CTRL_MSG should be passed as 'size'. */
void MPID_SMI_FreeRecvPkt( MPID_PKT_T *pkt, int from_grank, int size)
{
    int this_slot, vacant_slots, recv_slot, used_slots;

    MPID_TRACE_CODE("FreeRecvPkt", from_grank);
    /* XXX need to translate grank to device rank ? */
    
    MPID_SMI_ASYNC_LOCK(&MPID_SMI_freepkts_lck);

    /* adjust the pkt ptr by the space required for the CSUM */
    pkt = (MPID_PKT_T *)((char *)pkt - CSUMHEAD_SIZE);
    this_slot = ((size_t)pkt - (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer)
	>> MPID_SMI_SCI_TA_LD;
    used_slots = (size == IS_CTRL_MSG) ? 1 : MPID_SMI_NBR_SLOTS(size);

     MPID_DEBUG_IFCODE (fprintf(MPID_DEBUG_FILE, "[%d] from (%d): this_slot = %d  next_slot = %d\n",
	MPID_SMI_myid, from_grank, this_slot, next_slot[from_grank]); fflush (MPID_DEBUG_FILE);)
    if (this_slot == next_slot[from_grank]) {
	/* One more packet has been transfered from SCI to local memory - 
	   the packet is available again (because it is freed in-order). */
	*(MPID_SMI_lShortrecv[from_grank].read_msgs) += used_slots;

	next_slot[from_grank] = (next_slot[from_grank] + used_slots) % MPID_SMI_SCI_TA_SLOTS;

	MPID_DEBUG_IFCODE (fprintf(MPID_DEBUG_FILE, "[%d] Freeing pkt from (%d) at address 0x%p (offset = %d)\n",
				   MPID_SMI_myid, from_grank, pkt,
				   (size_t)pkt - (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer);
			   fflush( MPID_DEBUG_FILE );)

	if (out_of_order_slots[from_grank] > 0) {
	    /* check if the packets that have been free'd out-of-order before 
	       can be free'd now */
	    recv_slot = ((size_t)MPID_SMI_lShortrecv[from_grank].recv_ptr - 
			 (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer)
	                  >> MPID_SMI_SCI_TA_LD;
	    vacant_slots = (recv_slot > next_slot[from_grank]) ? 
		recv_slot - next_slot[from_grank] :
		recv_slot + (MPID_SMI_SCI_TA_SLOTS - next_slot[from_grank]);

	    /* we can only free all out-of-order pkts at once */
	    if (vacant_slots == out_of_order_slots[from_grank]) {
		MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE, "[%d] Freeing %d out-of-order packets from (%d)\n",
					 MPID_SMI_myid, out_of_order_slots[from_grank], from_grank);
				fflush( MPID_DEBUG_FILE ););

		*(MPID_SMI_lShortrecv[from_grank].read_msgs) += out_of_order_slots[from_grank];
		next_slot[from_grank] = (next_slot[from_grank] + out_of_order_slots[from_grank]) % MPID_SMI_SCI_TA_SLOTS;
		out_of_order_slots[from_grank] = 0;
	    }
	}
    } else {
	/* Just remember that a pkt was free'd out-of-order - and free all out-of-order
	   pkts if *all* pkts have been free'd out-of-order. */
	out_of_order_slots[from_grank] += used_slots;

	MPID_DEBUG_IFCODE (fprintf(MPID_DEBUG_FILE, "[%d] Could not free pkt from (%d) at address 0x%p (offset = %d). %d out-of-order pkts.\n",
				   MPID_SMI_myid, from_grank, pkt,
				   (size_t)pkt - (size_t)MPID_SMI_lShortrecv[from_grank].recv_buffer, out_of_order_slots[from_grank]);
			   fflush( MPID_DEBUG_FILE );)

	if (out_of_order_slots[from_grank] >= MPID_SMI_SCI_TA_SLOTS) {
            /* Will we ever get here? We shouldn't - because at latest, the last packet will 
               be freed *in* order. */
	    MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE, "[%d] Freeing ALL %d out-of-order pkts from (%d)\n",
				       MPID_SMI_myid, out_of_order_slots[from_grank], from_grank);
			      fflush( MPID_DEBUG_FILE ););
	    
	    *(MPID_SMI_lShortrecv[from_grank].read_msgs) += out_of_order_slots[from_grank];
	    out_of_order_slots[from_grank] = 0;
	}
    }
    MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE, "[%d] for (%d): %d out_of_order_slots, next_slot = %d\n", 
	MPID_SMI_myid, from_grank, out_of_order_slots[from_grank], next_slot[from_grank]); fflush( MPID_DEBUG_FILE );)
    MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_freepkts_lck);
    return;
}


/* Before calling MPID_SMI_GetSendPkt(), the rank of the destination process has to be stored 
   in pkt_desc->dest. When MPID_SMI_GetSendPkt() has returned MPI_SUCCESS, the address of the 
   target buffer is stored in pkt_desc->pkt and the Message ID is stored in pkt_desc->msgid. */
int MPID_SMI_GetSendPkt(nonblock, pkt_desc)
    int                nonblock;
MPID_SMI_CTRLPKT_T *pkt_desc;
{
    MPID_SMI_lShortsend_t *send_info = &MPID_SMI_lShortsend[pkt_desc->dest];
    uint read_msgs, unread_msgs, remaining_slots = 0, used_slots = 0;

    MPID_STAT_ENTRY(getsendpkt);
    MPID_TRACE_CODE("GetSendPkt", pkt_desc->dest);
    MPID_SMI_ASYNC_LOCK(&MPID_SMI_getpkts_lck);

    used_slots = MPID_SMI_NBR_SLOTS(pkt_desc->dsize);

    /* check if we have packets available */
    if (send_info->avail_msgs < used_slots) {
	while (1) {
	    MPID_STAT_COUNT(no_sendpkt);

	    /* Check how many packets the receiver has read in the meantime.
	       This is a remote SCI read access. */
	    SMIcall(SMI_Flush_read(pkt_desc->dest));
	    read_msgs = *(send_info->read_msgs);
	    while (MPID_SMI_cfg.DO_VERIFY 
		   && (SMI_Check_transfer_proc(pkt_desc->dest, CHECK_MODE) != SMI_SUCCESS)) {
	      read_msgs = *(send_info->read_msgs);
	      MPID_STAT_COUNT (sci_read_error);
	    }

	    /* overflow-proof check */
	    if (send_info->sent_msgs >= read_msgs)
		unread_msgs = send_info->sent_msgs - read_msgs;
	    else
		unread_msgs = ULONG_MAX - read_msgs + send_info->sent_msgs;

	    if (unread_msgs <= MPID_SMI_SCI_TA_SLOTS - used_slots) {
		send_info->avail_msgs = MPID_SMI_SCI_TA_SLOTS - unread_msgs;
		break;
	    } else {
#ifdef MPID_USE_DEVTHREADS
		/* trigger remote device thread (if available) to drain incoming queues */
		if (MPID_SMI_cfg.ASYNC_PROGRESS)
		    SMI_Signal_send(pkt_desc->dest|SMI_SIGNAL_ANY);
#endif
		/* If not blocking, return error code. */
		if (nonblock) {
		    MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_getpkts_lck);
		    return (MPI_ERR_BUFFER);
		} else {
		    MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_getpkts_lck);
		    MPID_DeviceCheck(MPID_NOTBLOCKING );
		    MPID_SMI_ASYNC_LOCK(&MPID_SMI_getpkts_lck);
		}
	    }
	}
    }

    /* now we have a packet */
    pkt_desc->pkt = (MPID_PKT_T *) send_info->recv_ptr;
    send_info->avail_msgs -= used_slots;
    send_info->sent_msgs += used_slots;

    remaining_slots = ((size_t)send_info->recv_buffer + MPID_SMI_QUEUE_SIZE
                           - (size_t)send_info->recv_ptr) >> MPID_SMI_SCI_TA_LD;

    /* increment with wrap-around */
    if (used_slots >= remaining_slots)
	send_info->recv_ptr = send_info->recv_buffer + ((used_slots - remaining_slots) << MPID_SMI_SCI_TA_LD);
    else
	send_info->recv_ptr += (used_slots << MPID_SMI_SCI_TA_LD);

    /* get & increment message ID */
    pkt_desc->msgid = MPID_SMI_lShortsend[pkt_desc->dest].msg_id;
    MPID_SMI_lShortsend[pkt_desc->dest].msg_id = 
	(MPID_SMI_lShortsend[pkt_desc->dest].msg_id % MPID_SMI_SHORTID) + 1;
    
    MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE,
			       "[%d] Got packet for (%d) at address 0x%p (offset = %d) with id = %d\n",
			       MPID_SMI_myid, pkt_desc->dest, pkt_desc->pkt,
			       (size_t)(pkt_desc->pkt) - (size_t)(send_info->recv_buffer),
			       pkt_desc->msgid );
		      fflush( MPID_DEBUG_FILE ););    

    MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_getpkts_lck);
    MPID_STAT_EXIT(getsendpkt);
    return MPI_SUCCESS;
}


/* 
   Send a control packet aka short message via the SCI interconnect making best
   use of the streambuffers while ensuring data integrity even for the case that
   the SCI transaction does not arrive in the remote memory in the same order as
   they were written into the stream. 
   This behaviour may theoretically occur when the size of the streambuffers is bigger
   than a single SCI data transatction: the D321 boards from Dolphin do have 128byte
   streambuffers but still 64 byte SCI transactions.

   SendControl() is designed to work with any combination of these two parameters.

   XXX: Due to the required "sequence check" after sending a control message, a 
   significant delay occurs after the message has been written into the receivers
   buffer. This increases the latency, but can normallay not be avoided. However, for 
   collective operations performed via the short protocol, an improvement could be
   achieved by designing a technique like "control-message burst mode": send *one*
   control/short-message to a number of processes and perform *one single* sequence
   check after sending all messages. If it returns an error (in most case, it will not
   do so), transfer all control messages again. This requires that all required state 
   information to re-send the messages is stored, which again causes some (minor) overhead 
   when sending the original messages.
*/
int MPID_SMI_SendControl( MPID_SMI_CTRLPKT_T *pkt_desc )
{
    MPID_SMI_MSGFLAG_T id_flag;
    MPID_SMI_MSGFLAG_T *flagbuf;
    MPID_SMI_lShortsend_t *send_info = &MPID_SMI_lShortsend[pkt_desc->dest];
    long align_buf[SENDCTRL_ALIGN_LONGS];
    int dest_offset, src_offset, remaining, copysize, firstcopysize = 0;
    int sizeavail, flush_size, buf_size, buf_size_LD, remaining_space;
    int sendctrl_buf_len = 0, align_size = 0, first_align_size = 0, flag_size = 0;
    char *sendctrl_buf = (char *)(pkt_desc->pkt);


    MPID_STAT_ENTRY(sendcontrol);
    MPID_TRACE_CODE("SendControl", pkt_desc->dest);
    MPID_SMI_DEBUG_PRINT_MSG("Entering SendControl");
    PERF_SEND_LATENCY(MPID_SMI_cfg.PERF_SEND_LTNCY)

    remaining_space = buf_size = (MPID_SMI_NBR_SLOTS(pkt_desc->dsize)) << MPID_SMI_SCI_TA_LD;
    
    /* check if this is a control packet (w/o data) or a "short message" */
    if (pkt_desc->dsize == 0) {
	/* this is a control packet or a short packet with length 0, thus no data */
	/* first, write the checksum */
	MPID_SMI_CSUM_GEN (((MPID_CSUM_T *)sendctrl_buf)->csum, pkt_desc->header, pkt_desc->hsize, 
			   (char)pkt_desc->msgid, pkt_desc->dest);
	((MPID_CSUM_T *)sendctrl_buf)->csum_len = pkt_desc->hsize;
	dest_offset = CSUMHEAD_SIZE;
	
	MEMCPY_S (sendctrl_buf + dest_offset, pkt_desc->header, pkt_desc->hsize);
	dest_offset += pkt_desc->hsize;
	remaining_space -= (CSUMHEAD_SIZE + pkt_desc->hsize);
    } else {
        /* This is a short message. */
        copysize    = MPID_MIN(MPID_SMI_ta_payload, pkt_desc->dsize);
	remaining   = pkt_desc->dsize - copysize;

	/* Calculate the CSUM for the data *in the first* SCI transaction (or what we intend 
	   to be an atomic transaction, but not necessarily is) and write the short msg header. */
	 MPID_SMI_CSUM_GEN (pkt_desc->header->short_pkt.csum, pkt_desc->data, copysize, 0xff, pkt_desc->dest);
	/* Now, calculate & write the CSUM of the header, followed by the header itself. */
	 MPID_SMI_CSUM_GEN (((MPID_CSUM_T *)sendctrl_buf)->csum, pkt_desc->header, pkt_desc->hsize, 
			    (char)pkt_desc->msgid, pkt_desc->dest);
	((MPID_CSUM_T *)sendctrl_buf)->csum_len = pkt_desc->hsize;
	/* This is the message header. */
	dest_offset = CSUMHEAD_SIZE;
	MEMCPY_S (sendctrl_buf + dest_offset, pkt_desc->header, pkt_desc->hsize);

	dest_offset += pkt_desc->hsize;
	src_offset   = 0;
	/* The smallest entity to be copied is SENDCTRL_ALIGN bytes for performance reasons. 
	   The easiest way is to read past the end of the user send buffer - but if we do
	   not want to that, we need to use memcpy() */
	if ((align_size = copysize & (SENDCTRL_ALIGN - 1)) != 0) {
	    copysize &= ~(SENDCTRL_ALIGN - 1);
	    /* This assignment may created SIGBUS (i.e. on Sparc 64bit) if the generated
	       address is not aligned as required by the CPU architecture. 
	       Thus, we use memcpy instead on all 64bit-architectures. Same below. */
#if !defined POINTER_64_BITS && !defined MPI_solaris
	    mpid_smi_peelcpy_l ((char *)align_buf, pkt_desc->data + copysize, align_size);
#else
	    /* memory access alignment requirements do not allow peelcpy() */
	    MEMCPY_S (align_buf, pkt_desc->data + copysize, align_size);
#endif
	}
	while (copysize > 0) {
 	    /* this "redundant" loop increases the performance on Athlon CPUs */
  	    size_t bsize = (MPID_SMI_WCSIZE > 0) ? 
	      MPID_MIN(copysize, MPID_SMI_WCSIZE - (dest_offset%MPID_SMI_WCSIZE)) : copysize;
	    MEMCPY_S (sendctrl_buf + dest_offset, pkt_desc->data + src_offset, bsize);
	    dest_offset += bsize;
	    src_offset  += bsize;
	    copysize -= bsize;
	}
	if (align_size > 0) {
#if !defined POINTER_64_BITS && defined MPI_solaris
	    mpid_smi_peelcpy_l (sendctrl_buf + dest_offset, (char *)align_buf, SENDCTRL_ALIGN);
#else
	    /* memory access alignment requirements do not allow peelcpy() */
	    MEMCPY_S (sendctrl_buf + dest_offset, align_buf, SENDCTRL_ALIGN);
#endif
	    dest_offset += SENDCTRL_ALIGN;
	    src_offset  += align_size;
	}

	/* New protocol for long short-msg, based on dynamic buffer-allocation via slots. */
	if (remaining > 0) {
	    /* calculate checksum on remaining data */
	    CSUM_VALUE_TYPE data_csum;
	    MPID_SMI_MSGID_T nbr_ta; 
	    int lsize;

	    MPID_SMI_CSUM_GEN (data_csum, pkt_desc->data + src_offset, remaining, 0xff, pkt_desc->dest);
	    /* We need to know the nbr of transactions required to transfer the remaining data 
	       because the msg flag we write *now* needs to indicate the next positition on 
	       which the receiver should poll for the final msg id. */
	    nbr_ta = (remaining + MSGFLAG_SIZE + CHKSUM_SIZE) >> MPID_SMI_SCI_TA_LD;
	    if ((remaining + MSGFLAG_SIZE + CHKSUM_SIZE) & (MPID_SMI_SCI_TA_SIZE-1)) 
		nbr_ta++;

	    /* terminate the prior transaction by writing the msg flag */
	    id_flag = (pkt_desc->msgid << MSGID_T_BITS)|nbr_ta;
#ifndef MPI_LINUX_ALPHA
	    while (dest_offset < MPID_SMI_SCI_TA_SIZE) {
	      *(MPID_SMI_MSGFLAG_T *)(sendctrl_buf + dest_offset) = id_flag;
	      WC_FLUSH;
	      dest_offset += sizeof(MPID_SMI_MSGFLAG_T);
	    }
#else
	    /* int writes destroy performance on Alpha-SCI -> convert to long-write */
	    {
		long lid_flag = 0;
		lid_flag |= id_flag;
		lid_flag <<= 32;
		((long *)(sendctrl_buf + dest_offset))[0] = lid_flag;
		dest_offset += sizeof(long);
	    }
#endif
	    remaining_space = (size_t)send_info->recv_buffer + MPID_SMI_QUEUE_SIZE 
		- (size_t)(sendctrl_buf + dest_offset);

	    /* Was the previous slot already the last slot of the queue ?
	       If not, copy the amount of data that fits into the queue */
	    if (remaining_space < (nbr_ta * MPID_SMI_SCI_TA_SIZE) 
		&& remaining_space >= MPID_SMI_SCI_TA_SIZE) {
		firstcopysize = (remaining < remaining_space) ? remaining : remaining_space;

	      /* Is the msg to be copied an aligned one ? */		
		if ((first_align_size = firstcopysize & (SENDCTRL_ALIGN - 1)) != 0) {	    
		    firstcopysize &= ~(SENDCTRL_ALIGN - 1);
		    mpid_smi_peelcpy_l ((char *)align_buf, pkt_desc->data + src_offset + firstcopysize,
					first_align_size);
		}

		lsize = firstcopysize;
		while (lsize > 0){
		  /* this "redundant" loop increases the performance on Athlon CPUs */
		  size_t bsize = (MPID_SMI_WCSIZE > 0) ? 
		    MPID_MIN(lsize, MPID_SMI_WCSIZE - (dest_offset%MPID_SMI_WCSIZE)) : lsize;
		  MEMCPY_S (sendctrl_buf + dest_offset, pkt_desc->data + src_offset, bsize);
		  WC_FLUSH;
		  dest_offset += bsize;
		  src_offset += bsize;
		  lsize -= bsize;
		}
		if (first_align_size > 0) {
		  remaining = 0;
		  mpid_smi_peelcpy_l (sendctrl_buf + dest_offset, (char *)align_buf, 
				      SENDCTRL_ALIGN);
		  /* XXX we will never get here because the slot lenght is always aligned !? */
		  src_offset += first_align_size;
		  /* dest_offset will be set to 0 below - this is a wrap-around! */
		} else
		  remaining -= firstcopysize;

		sendctrl_buf = send_info->recv_buffer;
		dest_offset = 0;
	    }

	    if (remaining_space < MPID_SMI_SCI_TA_SIZE) {
		sendctrl_buf = send_info->recv_buffer;
		dest_offset = 0;
	    }
	    /* copy data with alignment & write checksum */
	    copysize = remaining;
	    align_size = 0;
	    if (!first_align_size) {
		if ((align_size = copysize & (SENDCTRL_ALIGN - 1)) != 0) {
		copysize &= ~(SENDCTRL_ALIGN - 1);
		mpid_smi_peelcpy_l ((char *)align_buf, pkt_desc->data + src_offset + copysize,
				    align_size);
	      }
	    }
	    lsize = copysize;
	    while (lsize > 0) {
  	        /* this "redundant" loop increases the performance on Athlon CPUs */
	        size_t bsize = (MPID_SMI_WCSIZE > 0) ? 
		  MPID_MIN(lsize, MPID_SMI_WCSIZE - (dest_offset%MPID_SMI_WCSIZE)) : lsize;
		MEMCPY_S (sendctrl_buf + dest_offset, pkt_desc->data + src_offset, bsize);
		WC_FLUSH;
		src_offset  += bsize;
		dest_offset += bsize;
		lsize -= bsize;
	    }
	    if (align_size > 0) {
		mpid_smi_peelcpy_l (sendctrl_buf + dest_offset, (char *)align_buf, SENDCTRL_ALIGN);
		dest_offset += SENDCTRL_ALIGN;
	    }
#ifndef MPI_LINUX_ALPHA
	    *(CSUM_VALUE_TYPE *)(sendctrl_buf + dest_offset) = data_csum;
	    dest_offset += sizeof(CSUM_VALUE_TYPE);
#else
	    /* int writes destroy performance on Alpha-SCI -> convert to long-write */
	    {
		long lcsum = 0;
		lcsum |= data_csum;
		((long *)(sendctrl_buf + dest_offset))[0] = lcsum;
		dest_offset += sizeof(long);
	    }
#endif
	}
    } 

    /* write padding bytes which are also the msg flag and flush the stream buffer by this
       to let the remote process "see" the new packet 
       (using >> and << instead of division and multiplication) */
   /* explizit flushing of WC buffers - req. for Athlon */

    buf_size_LD = MPID_SMI_ld (buf_size);

    if ((remaining_space == buf_size - MPID_SMI_SCI_TA_SIZE) 
	&& (buf_size % MPID_SMI_STREAMSIZE != 0) && (dest_offset > sizeof(CSUM_VALUE_TYPE)))
	flush_size = MPID_SMI_ld(MPID_SMI_SCI_TA_SIZE);
    else
	flush_size = buf_size < MPID_SMI_STREAMSIZE ? buf_size_LD : MPID_SMI_STREAMSIZE_LD;
    
    flag_size = (((dest_offset >> flush_size) + 1) << flush_size) - dest_offset;
#if 0
    MEMCPY_S (sendctrl_buf + dest_offset,
	      (char *)&MPID_SMI_flagbuf_0[(pkt_desc->msgid << MPID_SMI_STREAMSIZE_LD)>> MPID_SMI_MSGFLAG_LD],
	      flag_size);
#else
    while (flag_size > 0) {
      size_t flush_len = (MPID_SMI_WCSIZE > 0) ? 
	MPID_MIN(flag_size, MPID_SMI_WCSIZE - (dest_offset%MPID_SMI_WCSIZE)) : flag_size;
      MEMCPY_S (sendctrl_buf + dest_offset,
		(char *)&MPID_SMI_flagbuf_0[(pkt_desc->msgid << MPID_SMI_STREAMSIZE_LD)>> MPID_SMI_MSGFLAG_LD],
		flush_len);
      WC_FLUSH;

      dest_offset += flush_len;
      flag_size -= flush_len;
    }
#endif

    if (MPID_SMI_cfg.DO_VERIFY 
	&& (SMI_Check_transfer_proc(pkt_desc->dest, CHECK_MODE) != SMI_SUCCESS)) {
	/* the caller has to try again */
	MPID_STAT_COUNT(sci_transm_error);
	return (MPI_ERR_INTERN);
    }    

    MPID_SMI_SIGNAL_SEND(pkt_desc->dest);
    PERF_GAP_LATENCY(MPID_SMI_cfg.PERF_GAP_LTNCY)
    MPID_STAT_EXIT(sendcontrol);

    return (MPI_SUCCESS);
}

/* This is for local copy small blocklength speedup */
/* XXX maybe replace these typedef-construction with macros ? */
typedef unsigned short b2_t;
typedef unsigned int b4_t;
typedef struct { unsigned int b1; unsigned int b2; } b8_t;
typedef struct { unsigned int b1; unsigned int b2; unsigned int b3; unsigned int b4; } b16_t;

void mpid_smi_peelcpy_l (char *dest, char *src, ulong len)
{
    char *end;
    
    switch (len) {
	/* First common cases are handled fast */
    case 1:
	*dest = *src;
	break;
    case 2:
	*(((b2_t *)dest)) = *(((b2_t *)src));
	break;
    case 4:
	*(((b4_t *)dest)) = *(((b4_t *)src));
	break;
    case 8:
	*(((b8_t *)dest)) = *(((b8_t *)src));
	break;
    case 16:
	memcpy (dest,src,len);
	break;
    default:
	/* If we have no common case we try to give memcpy() a
	   length of n*16, don't ask why that's good for performance,
	   but it is ;-) */

	/* adjust until length is n*16 */
	end = src + len;
	if (len & 0x1) 
	    *(dest++) = *(src++);
	if (len & 0x2) {
	    *(short *)dest = *(short *)src;
	    dest += 2; src += 2;
	}
	if (len & 0x4) {
	    *(int *)dest = *(int *)src;
	    dest += 4; src += 4;
	}
	if (len & 0x8) {
	    *(int *)dest = *(int *)src;
	    dest += 4; src += 4;
	    *(int *)dest = *(int *)src;
	    dest += 4; src += 4;
	}

	if (len > 16)
	    /* copy the rest */
	    memcpy (dest,src,end-src);
	break;
    }
}


/*
 * Suspending and resuming incoming send_requests (for rendez-vous protocol)
 * to control the number of active transfers and thus avoid saturation.
 * The related recv-handles are stored in some kind of queue (simplest type: FIFO)
 * and are resumed if no other incoming packets did arrive and the number
 * of active recvs is low enough.
 */
#define POSTPONE_TRACE 0 

void MPID_SMI_Send_postpone (MPIR_SHANDLE *shandle)
{
#if POSTPONE_TRACE
    fprintf(stderr, "[%d] postponing send to %d (#sends = %d)\n", MPID_SMI_myid, shandle->partner,
	   MPID_SMI_Rndvsends_in_progress);
#endif

    MPID_FIFO_push (MPID_SMI_postponed_sends, shandle);
    MPID_STAT_COUNT( send_postponed );
    
    return;
}

void MPID_SMI_Recv_postpone (MPIR_RHANDLE *rhandle)
{
#if POSTPONE_TRACE
    fprintf(stderr, "[%d] postponing recv from %d (#recvs = %d)\n", MPID_SMI_myid, rhandle->partner,
	   MPID_SMI_Rndvrecvs_in_progress);
#endif

    MPID_FIFO_push (MPID_SMI_postponed_recvs, rhandle);
    MPID_STAT_COUNT( recv_postponed );
    
    return;
}

void MPID_SMI_Check_postponed(void)
{
    MPIR_RHANDLE *rhandle;
    MPIR_SHANDLE *shandle;
    
#if POSTPONE_TRACE
    fprintf(stderr, "[%d] checking for postponed transfers...\n", MPID_SMI_myid);
#endif

    if (MPID_SMI_Rndvrecvs_in_progress < MPID_SMI_cfg.MAX_RECVS 
	|| (MPID_SMI_Rndvrecvs_in_progress == 0 && MPID_SMI_Rndvrecvs_scheduled < MPID_SMI_cfg.MAX_RECVS)) {
	rhandle = (MPIR_RHANDLE *)MPID_FIFO_pop (MPID_SMI_postponed_recvs);
	if (rhandle != NULL) {
#if POSTPONE_TRACE
	    fprintf(stderr, "[%d] pushing recv from %d (#recvs = %d)\n", MPID_SMI_myid, rhandle->partner,
		   MPID_SMI_Rndvrecvs_in_progress);
#endif
	    rhandle->push (rhandle, NULL);
	}
#if POSTPONE_TRACE
	else
	    fprintf(stderr, "[%d] no recv to push (#recvs = %d)\n", MPID_SMI_myid, MPID_SMI_Rndvrecvs_in_progress);
#endif
    }

    if (MPID_SMI_Rndvsends_in_progress < MPID_SMI_cfg.MAX_SENDS 
	|| (MPID_SMI_Rndvsends_in_progress == 0 && MPID_SMI_Rndvsends_scheduled < MPID_SMI_cfg.MAX_SENDS)) {
	shandle = (MPIR_SHANDLE *)MPID_FIFO_pop (MPID_SMI_postponed_sends);
	if (shandle != NULL) {
#if POSTPONE_TRACE
	    fprintf(stderr, "[%d] pushing send to %d (#sends = %d)\n", MPID_SMI_myid, shandle->partner,
		   MPID_SMI_Rndvsends_in_progress);
#endif
	    shandle->push (shandle);
	}
#if POSTPONE_TRACE
	else
	    fprintf(stderr, "[%d] no send to push (#sends = %d)\n", MPID_SMI_myid, MPID_SMI_Rndvsends_in_progress);
#endif
    }
#if POSTPONE_TRACE
    fprintf(stderr, "[%d] ...done (#sends: %d i_p / %d q, #recvs: %d i_p / %d q).\n", MPID_SMI_myid,
	   MPID_SMI_Rndvsends_in_progress, MPID_SMI_postponed_sends->nbr_entries,
	   MPID_SMI_Rndvrecvs_in_progress, MPID_SMI_postponed_recvs->nbr_entries);
#endif

    return;
}
 

/*
 * Debugging support - not yet tested for ch_smi
 */
void MPID_SMI_Print_internals( FILE *fp )
{
    int i;
    char *state;

    /* Print state */
    state = "Not in device";
    switch (MPID_op) 
	{
	case 0: break;
	case 1: state = "MPID_ReadControl"; break;
	case 2: state = "MPID_GetSendPkt" ; break;
	case 3: state = "MPID_SendControl"; break;
	}
    fprintf( fp, "[%d] State is %s\n", MPID_SMI_myid, state );

    /* 
       XXX print relevant internals,
       i.e.  determine & print number of received, processed & avail packets 
       (towards each remote process)
    */
}

/* What is this for ? */
#if 0
int MAGPIE_cluster_of_process(MPI_Comm comm, int rank, int *cluster){
    SMI_Node_rank(cluster);
    return MPI_SUCCESS;
}

void MAGPIE_reset_cluster_info(void){
    /* do nothing -- will always be the same */
}
#endif
