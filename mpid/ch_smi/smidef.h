/* $Id$ */

#include <sys/types.h>
#include <limits.h>

#include "sbcnst2.h"

#include "smitypes.h"
#include "mpichconf.h"
#ifndef WIN32
#include "smi_conf.h"
#endif
#ifndef _MPID_SMI_DEF
#define _MPID_SMI_DEF

/* Declaration of global variables and of all configuration parameters
   (and their default values). This file should be included by all
   source files. */


#ifdef MPID_USE_DEVTHREADS
/* thread/synchronization type definitons */
#define MPID_SMI_LOCK_T      pthread_mutex_t
#define MPID_SMI_THREADKEY_T pthread_key_t

/* mutex' for multithreaded usage of the device */
#ifdef MPIR_USE_LIBTHREADS

#endif

/* different types of thread */
#define APPLICATION_THREAD 0
#define DEVICE_THREAD      1

#define MPID_SMI_SIGNAL_SEND(rank) if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) { \
                                           MPID_STAT_CALL(signal_send); \
                                           SMI_Signal_send((rank)|SMI_SIGNAL_ANY); \
                                           MPID_STAT_RETURN(signal_send); }


#define MPID_SMI_INIT_LOCK(mtx) pthread_mutex_init(mtx, NULL);
#define MPID_SMI_ASYNC_LOCK(mtx) { if (MPID_SMI_cfg.ASYNC_PROGRESS) pthread_mutex_lock(mtx); }
#define MPID_SMI_ASYNC_UNLOCK(mtx) { if (MPID_SMI_cfg.ASYNC_PROGRESS) pthread_mutex_unlock(mtx); }
#define MPID_SMI_LOCK(mtx) pthread_mutex_lock(mtx)
#define MPID_SMI_UNLOCK(mtx) pthread_mutex_unlock(mtx)
#define MPID_SMI_TRYLOCK(mtx) pthread_mutex_trylock(mtx)
#define MPID_SMI_DESTROY_LOCK(mtx) pthread_mutex_destroy (mtx);
#else /* MPID_USE_DEVTHREADS */
/* no multi-threading -> dummy macros */
#define MPID_SMI_LOCK_T    int
#define MPID_SMI_THREADKEY_T int

#define MPID_SMI_INIT_LOCK(mtx)
#define MPID_SMI_LOCK(mtx) 
#define MPID_SMI_UNLOCK(mtx)
#define MPID_SMI_TRYLOCK(mtx) 1
#define MPID_SMI_DESTROY_LOCK(mtx)
#define MPID_SMI_ASYNC_LOCK(mtx)
#define MPID_SMI_ASYNC_UNLOCK(mtx)
#define MPID_SMI_SIGNAL_SEND(rank)
#endif /* MPID_USE_DEVTHREADS */

/* define the mode to check remote memory transfers */
#define CHECK_MODE SMI_CHECK_NOBARRIER

/* just in case, should be in limits.h */
#if !(defined ULONG_MAX)
#define	ULONG_MAX	0xffffffffUL	/* max of "unsigned long int" */
#endif

/* misc. defines */
#define MPID_SMI_STRINGLEN      1024
#define MPID_SMI_INVALID_REGID  -1

/* SCI topology fundamentals */
#define MAX_SCI_DIMS            3     /* max. number of SCI-torus-dimensions supported */
#define MAX_NODES_PER_DIM       15    /* max. number of SCI-nodes per such dimension */
#define X_DIM 0
#define Y_DIM 1
#define Z_DIM 2

#ifdef DOLPHIN_SCI
#define MIN_SCI_ID              4
#define SCI_ID_STRIDE           4
#define SCI_ID_YOFFSET          (16*4)
#define SCI_ID_ZOFFSET          (15*64) /* XXX correct? */
#elif defined SCALI_SCI
#define MIN_SCI_ID              0x0100
#define SCI_ID_STRIDE           0x0100
#define SCI_ID_YOFFSET          0x1000
#define SCI_ID_ZOFFSET          0xffff      /* XXX how? */
#else
/* dummy for SMP */
#define MIN_SCI_ID              1
#define SCI_ID_STRIDE           1
#define SCI_ID_YOFFSET          1
#define SCI_ID_ZOFFSET          1 
#endif

/* Are the nodes arranged in a torus topology as defined by the Dolphin or by the
   Scali drivers? If both are set to 0, a central-switch topology is assumed. */
#define IS_DOLPHIN_TORUS        1     
#define IS_SCALI_TORUS          0

/* available resource scheduling strategies */
typedef enum { IMMEDIATE = 0, LRU, LFU, BEST_FIT, RANDOM, NONE, SCHED_DUMMY } MPID_SMI_rsrc_sched_strategy_t;

#ifndef MPID_SMI_STATIC_CONFIG
/* type for device configuration variables (non protocol-specific variables */
#define CFG_VAR_TYPE int
typedef struct _MPID_SMI_CFG_T {
	/* short/control packet protocol */
    CFG_VAR_TYPE MSGCHK_TYPE;        /* poll or block (and how?) for new messages ? */
	double       MSGCHK_DELAY;       /* delay parameter for hybrid blocking-polling (us) */
	CFG_VAR_TYPE MSGCHK_REPEAT;      /* loop count for hybrid blocking-polling */
    CFG_VAR_TYPE MAX_SHORT_PAYLOAD;  /* maximum payload of a short msg */
    CFG_VAR_TYPE SENDCTRL_PAD;       /* padding of sendcontrol alignment */
	
	/* low-level communication configuration */
    CFG_VAR_TYPE MEMCPY_TYPE;        /* which memcpy function to use? Default: auto detect */
    CFG_VAR_TYPE MEMCPYSYNC_MODE;    /* how to synchronize remote memcpy operations ? */
    CFG_VAR_TYPE MEMCPYSYNC_MIN;     /* no synchronization for memcpy operations smaller than this */
    CFG_VAR_TYPE SEGMODE;            /* the mode in which the SMI segments are created */
    CFG_VAR_TYPE ADPTMODE;           /* how are adapters assigned to processes? */
    CFG_VAR_TYPE DO_VERIFY;          /* do data transfer verfication ? */
	
	/* rendez-vous variants */
    CFG_VAR_TYPE ASYNC_PROGRESS;     /* true asynchronous send/recv operations? */
    CFG_VAR_TYPE ZEROCOPY;           /* do zero-copy transfers if possible? */
    CFG_VAR_TYPE REGISTER;           /* try to register user-allocated buffers with SCI? */
	CFG_VAR_TYPE CACHE_REGISTERED;   /* keep registered memory as it is until it's finally free'd */
	CFG_VAR_TYPE CACHE_CONNECTED;    /* keep up connections to remote regions after a DMA transfer */
    CFG_VAR_TYPE USE_DMA_PT2PT;      /* use DMA for long rendez-vous transfers ? */
    CFG_VAR_TYPE USE_DMA_COLL;       /* use DMA for collective operations (pipelining) ? */
    CFG_VAR_TYPE SYNC_DMA_MINSIZE;   /* min. blocksize to use DMA for blocking pt2pt transfers */
    CFG_VAR_TYPE ASYNC_DMA_MINSIZE;  /* min. blocksize to use DMA for non-blocking pt2pt transfers */
	CFG_VAR_TYPE MAX_RECVS;          /* maximum number of concurrent recvs */
	CFG_VAR_TYPE MAX_SENDS;          /* maximum number of concurrent sends */
	
	/* non-contiguous data transfer */
    CFG_VAR_TYPE NC_ENABLE;          /* enable the use of direct non-contig send and receive? */
    CFG_VAR_TYPE NC_MINSIZE;	     /* set a threshold for the use of direct copy */
    CFG_VAR_TYPE NC_ALIGN;   	     /* align direct copy operations to this address boundary */
	
	/* memory allocation through MPI_Alloc_mem */
    CFG_VAR_TYPE ALLOC_MINSIZE;		 /* min. size to alloc in shared region */
    CFG_VAR_TYPE ALLOC_POOLSIZE;	 /* size of shreg pools */
    
	/* single sided communication */
    CFG_VAR_TYPE SSIDED_ENABLED;
	CFG_VAR_TYPE SSIDED_RMTPUT_PRIVATE;	/* min size to do a remote put */
	CFG_VAR_TYPE SSIDED_RMTPUT_SHARED;
	CFG_VAR_TYPE SSIDED_FRAGMENT_MAX;	/* max. fragmentation to allow direct copy. in percent (0-100) */
	CFG_VAR_TYPE SSIDED_SIGNAL;         /* Use signals to enforce completion in emulation? */
	CFG_VAR_TYPE SSIDED_NONBLOCKING;    /* Use non-blocking send/recv in emulation? */
	CFG_VAR_TYPE SSIDED_DELAY;          /* Delay operations up to X bytes until synchronization? */
	CFG_VAR_TYPE SSIDED_NOSYNC;         /* Perform all operations immedeately, which allows to ommit sync. */

	/* performance modelling, see smiperf.h */
	CFG_VAR_TYPE PERF_GAP_LTNCY;
	CFG_VAR_TYPE PERF_SEND_LTNCY;
	CFG_VAR_TYPE PERF_RECV_LTNCY;
	CFG_VAR_TYPE PERF_BW_LIMIT;
	CFG_VAR_TYPE PERF_BW_REDUCE;
	
	/* resource scheduling, see smiregionmngmt.h */
    MPID_SMI_rsrc_sched_strategy_t RESOURCE_SCHED;

	/* collective operations */
    CFG_VAR_TYPE COLLOPS;                 /* use optimized collective operations? */
    CFG_VAR_TYPE COLL_BARRIER;            /* use shared-memory barrier? */
	CFG_VAR_TYPE COLL_BARRIER_FANIN;      /* fan-in of shared-memory barrier */
	CFG_VAR_TYPE COLL_REDUCE_SHORT_TYPE;  /* algorithm for custom reduce() of "short" vectors */
	CFG_VAR_TYPE COLL_REDUCE_LONG;        /* threshold for "long" vector for custom reduce()? [bytes] */
	CFG_VAR_TYPE COLL_REDUCE_LONG_TYPE;   /* algorithm for custom reduce() of "long" vectors */
    CFG_VAR_TYPE COLL_REDUCE_FANIN;       /* fan-in of reduce operation (for small message sizes)  */
    CFG_VAR_TYPE COLL_DIRECT_REDUCE_OK;   /* does the eager/rndv protocol setup allow for direct reduce? */
	CFG_VAR_TYPE COLL_ALLREDUCE_TYPE;     /* choice of allreduce() algorithm */
	CFG_VAR_TYPE COLL_SCATTER_MAX;        /* max msg size for scatter optimization  */
	CFG_VAR_TYPE COLL_ALLGATHER_BARRIER;  /* threshold to use barrier in allgather()  */
	CFG_VAR_TYPE COLL_ALLTOALL_BARRIER;   /* msgsize threshold to use barrier in alltoall()  */
	CFG_VAR_TYPE COLL_ALLTOALL_MIN;       /* msgsize threshold to use custom alltoall()  */
	CFG_VAR_TYPE COLL_ALLTOALL_TYPE;      /* algorithm for custom alltoall() */
	CFG_VAR_TYPE COLL_BCAST_TYPE;         /* type of bcast to use (point-to-point, pipe, mult-dim pipe*/	
	CFG_VAR_TYPE COLL_PIPE_MIN;           /* msgsize threshold to use custom pipe()  */
	CFG_VAR_TYPE COLL_PIPE_DMA_MIN;       /* msgsize threshold to use custom pipe() with DMA */
	CFG_VAR_TYPE COLL_PIPE_BLOCKSIZE;     /* blocksize for pcast */
	CFG_VAR_TYPE COLL_PIPE_NBRBLOCKS;     /* nbr of blocks in a pcast buffer*/
	CFG_VAR_TYPE COLL_PIPE_DYNAMIC;       /* decrease BLOCKSIZE for small broadcasts */

	/* misc */
    CFG_VAR_TYPE SENDSELF;           /* use shortcut to send to itself (zero-copy) */
    CFG_VAR_TYPE USE_WATCHDOG;       /* indicates setting for SMI-Watchdog */
    CFG_VAR_TYPE VERBOSE;            /* verbose on startup ? */
    CFG_VAR_TYPE CONNECT_ON_INIT;    /* connect all remote segment in MPI_Init? */
} MPID_SMI_CFG_T;
#else
/* 
 * static configuration 
 */
/* short/control packet protocol */
#define MSGCHK_TYPE
#define MSGCHK_DELAY
#define MSGCHK_REPEAT
#define MAX_SHORT_PAYLOAD
#define SENDCTRL_PAD
	
/* low-level communication configuration */
#define MEMCPY_TYPE
#define MEMCPYSYNC_MODE
#define MEMCPYSYNC_MIN
#define SEGMODE
#define ADPTMODE   
#define DO_VERIFY  

/* rendez-vous variants */
#define ASYNC_PROGRESS
#define ZEROCOPY
#define REGISTER
#define CACHE_REGISTERED
#define CACHE_CONNECTED
#define USE_DMA_PT2PT
#define USE_DMA_COLL
#define SYNC_DMA_MINSIZE
#define ASYNC_DMA_MINSIZE
#define MAX_RECVS
#define MAX_SENDS
	
/* non-contiguous data transfer */
#define NC_ENABLE
#define NC_MINSIZE
#define NC_ALIGN
	
/* memory allocation through MPI_Alloc_mem */
#define ALLOC_MINSIZE
#define ALLOC_POOLSIZE
    
/* single sided communication */
#define SSIDED_ENABLED
#define SSIDED_RMTPUT_PRIVATE
#define SSIDED_RMTPUT_SHARED
#define SSIDED_FRAGMENT_MAX
#define SSIDED_SIGNAL
#define SSIDED_NONBLOCKING
#define SSIDED_DELAY
#define SSIDED_NOSYNC

/* performance modelling, see smiperf.h */
#define PERF_GAP_LTNCY
#define PERF_SEND_LTNCY
#define PERF_RECV_LTNCY
#define PERF_BW_LIMIT
#define PERF_BW_REDUCE

/* resource scheduling, see smiregionmngmt.h */
#define RESOURCE_SCHED

/* collective operations */
#define COLLOPS
#define COLL_BARRIER
#define COLL_BARRIER_FANIN
#define COLL_REDUCE_SHORT_TYPE
#define COLL_REDUCE_LONG
#define COLL_REDUCE_LONG_TYPE
#define COLL_REDUCE_FANIN
#define COLL_DIRECT_REDUCE_OK
#define COLL_ALLREDUCE_TYPE
#define COLL_SCATTER_MAX
#define COLL_ALLGATHER_BARRIER
#define COLL_ALLTOALL_BARRIER
#define COLL_ALLTOALL_MIN
#define COLL_ALLTOALL_TYPE
#define COLL_BCAST_TYPE
#define COLL_PIPE_MIN
#define COLL_PIPE_DMA_MIN
#define COLL_PIPE_BLOCKSIZE
#define COLL_PIPE_NBRBLOCKS
#define COLL_PIPE_DYNAMIC

/* misc */
#define SENDSELF
#define USE_WATCHDOG
#define VERBOSE
#define CONNECT_ON_INIT

#endif /* static configuration */

extern MPID_SMI_CFG_T MPID_SMI_cfg;


/* global rank of this process and number of processes (related to this device) */
extern int MPID_SMI_myid;
extern int MPID_SMI_numids;

extern boolean MPID_SMI_is_initialized;

extern int MPID_SMI_myNode;           /* number of node for this process */
extern int MPID_SMI_numNodes;         /* number of nodes */
extern int *MPID_SMI_numProcsOnNode;  /* mapping: number of processes running on each node */
extern int *MPID_SMI_procNode;        /* mapping: rank of node for each process */

extern int MPID_SMI_use_SMP;
extern int *MPID_SMI_use_localseg;
extern int *MPID_SMI_is_remote;

extern int  MPID_SMI_sci_type;
extern int  MPID_SMI_sci_dim_extnt[MAX_SCI_DIMS];  /* extent of each of these dimensions. */
extern int  MPID_SMI_proc_dims;           /* nbr. of dimensions on which process of this app. are running */
extern int  MPID_SMI_sci_dims;            /* phys. dimensions of SCI ring/torus topology */
extern int  MPID_SMI_active_dims;         /* logical/active dimensions */
extern int  MPID_SMI_gnodes_in_dim[MAX_SCI_DIMS]; /* nodes in dimension '0' till dimension 'MPID_SMI_sci_dims - 1' */
extern int *MPID_SMI_dstnc_to_grank;      /* distance in nodes to other ranks */
extern int *MPID_SMI_dimdstnc_to_grank[MAX_SCI_DIMS];
extern int *MPID_SMI_grank_to_sciid;      /* map global process rank to sciid of node it is running on */

/* system-dependant constants */
extern int MPID_SMI_PAGESIZE;    /* SCI/SMI related "page size" (allocation granularity */
extern int MPID_SYS_PAGESIZE;    /* real system page size */ 
extern int MPID_SMI_STREAMSIZE;

extern int MPID_SMI_SCI_TA_SIZE; /* biggest SCI transaction size which is equal or 
									less the MPID_SMI_STREAMSIZE */
extern int MPID_SMI_NBRADPTS;    /* number of PCI-SCI adapters */
extern int MPID_SMI_DEFADPT;     /* number of default PCI-SCI adapter */

extern int MPID_SMI_DMA_SIZE_ALIGN;
extern int MPID_SMI_DMA_OFFSET_ALIGN;

extern MPID_SMI_THREADKEY_T MPID_SMI_thread_type;
extern MPID_SBHeader MPID_SMI_os_ta_allocator;

/* use job-management for one-sided communication? */
#define USE_JOBMNGMT            1
/* do MPI_Win_fence via shmem-barrier? */
#define WIN_FENCE_SHMEM_BARRIER 1

/* specify the checksum-algorithm (available algoritms are CRC32, FITS, NETDEV, NONE (, INET)
   (see smicheck.h for more info) */
#define MPID_SMI_CHECK_CRC32
/* Do CRC-checks for intra-node (shmem) communication, too? Normally, this shouldnt be 
   required, but it has been observed on SMP systems that the ordering of write accesses 
   is not as it would be expected. This can be verified with 
   examples/perftest/stress -size 1 1024 1
   and a device configuration of 
   shortbuf_size 1024    
   This effect should be evaluated in detail! For now, setting this to 1 helps.
*/
#define MPID_SMI_CHECK_SMP       1

/* DMA transfers */
#define NO_DMA     0 
#define MAPPED_DMA 2
#define RDMA       1
#define NAME_USE_DMA_PT2PT "USE_DMA_PT2PT"
#define MPID_SMI_USE_DMA_PT2PT_DEF NO_DMA      /* if != NO_DMA: use DMA transfers for rndv sends 
												  (see thresholds below)
												  if NO_DMA: always use PIO (memcpy()) */
#define NAME_USE_DMA_COLL "USE_DMA_COLL"
#define MPID_SMI_USE_DMA_COLL_DEF RDMA  /* if != NO_DMA: use DMA transfers for pipelined transfers
										   in collective operations - recommended!
										   if NO_DMA: always use PIO (memcpy()) */

/* parameters for memory allocation via MPI_Alloc_mem() */
#define NAME_ALLOC_MINSIZE "ALLOC_MINSIZE"
#define MPID_SMI_ALLOC_MINSIZE_DEF (64*1024)
#define NAME_ALLOC_POOLSIZE "ALLOC_POOLSIZE"
#define MPID_SMI_ALLOC_POOLSIZE_DEF (4*1024*1024)

/* replace generic collective operations by SCI-optimized versions? */
#define NAME_COLLOPS "COLL_CUSTOM_ENABLE"
#define MPID_SMI_COLLOPS_DEF 1

#define NAME_COLL_BARRIER "COLL_BARRIER"
#define MPID_SMI_BARRIER_DEF 1
#define NAME_COLL_BARRIER_FANIN "COLL_BARRIER_FANIN"
#define MPID_SMI_BARRIER_FANIN_DEF 8
#define MPID_SMI_BARRIER_FANIN_MAX 32

#define NAME_COLL_REDUCE_FANIN "COLL_REDUCE_FANIN"
#define MPID_SMI_REDUCE_FANIN_DEF 4
#define MPID_SMI_REDUCE_FANIN_MAX 32
/* tree-variants are designed for short vectors (short & eager),
   Rabenseifner- and pipeline-variants are optimal for long vectors. */
#define REDUCE_TREE              0
#define REDUCE_RABENSEIFNER      1
#define REDUCE_PIPELINE          2
#define NAME_COLL_REDUCE_SHORT_TYPE "COLL_REDUCE_SHORT_TYPE"
#define MPID_SMI_REDUCE_SHORT_TYPE_DEF REDUCE_TREE  
#define NAME_COLL_REDUCE_LONG "COLL_REDUCE_LONG"
#define MPID_SMI_REDUCE_LONG_DEF MPID_SMI_EAGERSIZE
#define NAME_COLL_REDUCE_LONG_TYPE "COLL_REDUCE_LONG_TYPE"
#define MPID_SMI_REDUCE_LONG_TYPE_DEF REDUCE_RABENSEIFNER

#define NAME_COLL_DIRECT_REDUCE_OK "COLL_REDUCE_DIRECT"
#define MPID_SMI_DIRECT_REDUCE_OK_DEF 1

#define NAME_COLL_ALLREDUCE_TYPE "COLL_ALLREDUCE_TYPE"
#define ALLREDUCE_DEFAULT        0  /* reduce-bcast combiniation with the chosen reduce/bcast algorithms */
#define ALLREDUCE_RABENSEIFNER   1  /* always rabenseifner for long-enough vectors (MPICH for short ones) */
#define ALLREDUCE_PIPELINE       2  /* always pipeline for long-enough vectors (MPICH for short ones) */
#define ALLREDUCE_ALLGATHER      3  /* always allgather-like custom-allreduce */
#define ALLREDUCE_RABSCI         4  /* Rabenseifner optimized for SCI */
#define MPID_SMI_ALLREDUCE_TYPE_DEF ALLREDUCE_RABSCI

#define NAME_COLL_ALLGATHER_BARRIER "COLL_ALLGATHER_BARRIER"
#define MPID_SMI_ALLGATHER_BARRIER_DEF (8*1024)

#define NAME_COLL_SCATTER_MAX "COLL_SCATTER_MAX"
#define MPID_SMI_SCATTER_MAX_DEF MPID_SMI_EAGERSIZE

#define NAME_COLL_ALLTOALL_BARRIER "COLL_ALLTOALL_BARRIER"
#define MPID_SMI_ALLTOALL_BARRIER_DEF (8*1024)
#define NAME_COLL_ALLTOALL_MIN "COLL_ALLTOALL_MIN"
#define MPID_SMI_ALLTOALL_MIN_DEF (16*1024)
#define NAME_COLL_ALLTOALL_TYPE "COLL_ALLTOALL_TYPE"
#define ALLTOALL_PLAIN         0
#define ALLTOALL_SCI_1D        1
#define ALLTOALL_SCI_2D        2
#define ALLTOALL_SCAMPI        3
#define ALLTOALL_MPICH         4
#define ALLTOALL_STRAIGHT      5
#define MPID_SMI_ALLTOALL_TYPE_DEF ALLTOALL_SCAMPI

#define NAME_COLL_BCAST_TYPE "COLL_BCAST_TYPE"
#define BCAST_PLAIN       0
#define BCAST_PIPELINE    1
#define BCAST_MULTIDIM    2
/* XXX PIPELINE bcast is still buggy! */
#define MPID_SMI_BCAST_TYPE_DEF 0

#define NAME_COLL_PIPE_MIN "COLL_PIPE_MIN"
#define MPID_SMI_PIPE_MIN_DEF MPID_SMI_EAGERSIZE
#define NAME_COLL_PIPE_BLOCKSIZE "COLL_PIPE_BLOCKSIZE"
#define MPID_SMI_PIPE_BLOCKSIZE_DEF (128*1024)
#define NAME_COLL_PIPE_DMA_MIN "COLL_PIPE_DMA_MIN"
#define MPID_SMI_PIPE_DMA_MIN_DEF (2*MPID_SMI_PIPE_BLOCKSIZE_DEF)
#define NAME_COLL_PIPE_NBRBLOCKS "COLL_PIPE_NBRBLOCKS"
#define MPID_SMI_PIPE_NBRBLOCKS_DEF 8
#define NAME_COLL_PIPE_DYNAMIC "COLL_PIPE_DYNAMIC"
#define MPID_SMI_PIPE_DYNAMIC_DEF 1

/* poll for new messages or wait for signals which announce a new message? */
#define MSGCHK_TYPE_POLLING    0
#define MSGCHK_TYPE_IRQ        1
#define MSGCHK_TYPE_IRQ_POLL   2
#define MSGCHK_TYPE_IRQ_BLOCK  3
#define NAME_MSGCHK_TYPE "MSGCHK_TYPE"
#define MPID_SMI_MSGCHK_TYPE_DEF MSGCHK_TYPE_POLLING
#define NAME_MSGCHK_DELAY "MSGCHK_DELAY"
#define MPID_SMI_MSGCHK_DELAY_DEF 1000.0  /* continue polling for how many usec when doing an irq-device check? */
#define NAME_MSGCHK_REPEAT "MSGCHK_REPEAT"
#define MPID_SMI_MSGCHK_REPEAT_DEF 10     /* do a blocking check how often after an irq-device check? */


/* enable single-sided communication? requires addtional SCI resources */
#define NAME_SSIDED_ENABLED "SSIDED_ENABLED"
#define MPID_SMI_SSIDED_ENABLED_DEF 1 

/* parameters for decession wether emulating put/get */
#define NAME_SSIDED_RMTPUT_PRIVATE "SSIDED_RMTPUT_PRIVATE"
#define MPID_SMI_SSIDED_RMTPUT_PRIVATE_DEF	148		/* 148 Byte */
#define NAME_SSIDED_RMTPUT_SHARED "SSIDED_RMTPUT_SHARED"
#define MPID_SMI_SSIDED_RMTPUT_SHARED_DEF	122		/* 122 Byte */
#define NAME_SSIDED_FRAGMENT_MAX		"SSIDED_FRAGMENT_MAX"
#define MPID_SMI_SSIDED_FRAGMENT_MAX_DEF				75		/* 75% 	(0 always; 100 never) */

#define NAME_SSIDED_SIGNAL "SSIDED_SIGNAL"
#define MPID_SMI_SSIDED_SIGNAL_DEF 0

#define NAME_SSIDED_NONBLOCKING "SSIDED_NONBLOCKING"
#define MPID_SMI_SSIDED_NONBLOCKING_DEF 0

#define NAME_SSIDED_DELAY "SSIDED_DELAY"
#define MPID_SMI_SSIDED_DELAY_DEF 0

#define NAME_SSIDED_NOSYNC "SSIDED_NOSYNC"
#define MPID_SMI_SSIDED_NOSYNC_DEF 0

/* do zero-copying if possible or never? */
#define NAME_ZEROCOPY "ZEROCOPY"
#define MPID_SMI_ZEROCOPY_DEF 1

/* try to register user-allocated buffers? */
#define NAME_REGISTER "REGISTER"
#define MPID_SMI_REGISTER_DEF 1
#define NAME_CACHE_REGISTERED "CACHE_REGISTERED"
#define MPID_SMI_CACHE_REGISTERED_DEF 0
#define NAME_CACHE_CONNECTED "CACHE_CONNECTED"
#define MPID_SMI_CACHE_CONNECTED_DEF 0

#define NAME_SYNC_DMA_MINSIZE "SYNC_DMA_MINSIZE"
#define MPID_SMI_SYNC_DMA_MINSIZE_DEF (256*1024)  
#define NAME_ASYNC_DMA_MINSIZE "ASYNC_DMA_MINSIZE"
#define MPID_SMI_ASYNC_DMA_MINSIZE_DEF (32*1024)  

#define NAME_MAX_RECVS "RNDV_MAXRECVS"
#define MPID_SMI_MAX_RECVS_DEF 0
#define NAME_MAX_SENDS "RNDV_MAXSENDS"
#define MPID_SMI_MAX_SENDS_DEF 4
#define NAME_MAX_TRANSFERS "RNDV_MAXTRANSFERS"
#define MPID_SMI_MAX_TRANSFERS_DEF 4

/* shortcut if sending to itself */
#define NAME_SENDSELF "SENDSELF"
#define MPID_SMI_SENDSELF_DEF (1 | MPID_SMI_USE_DMA_PT2PT_DEF) 
                             /* if 1: use shortcut if sending messages to youself 
				  			    if 0: send msg to yourself the usual (slower) way 
							    NOTE: USE_DMA_PT2PT implies SEND_SELF */

/* id's of the locks for memcpy() synchronization */ 
extern int *MPID_SMI_memlocks_in;
extern int *MPID_SMI_memlocks_out;

/* internal communication buffers to "publish" relevant data to other processes */
struct MPID_SMI_EagerSetup {
    uint sgmt_id;
    uint adptr;
    int nbrbufs;
    int bufsize;
};

struct MPID_SMI_RndvSetup {
    uint sgmt_id;
    uint adptr;
};

struct MPID_SMI_BarrierSetup {
	ulong bmem_offset;
};

typedef struct {
	ulong short_msgcnt;              /* counter for number of short msgs read */
    struct MPID_SMI_EagerSetup Eager_setup;
#if 0
	/* this information is now contained in the control packets */
    struct MPID_SMI_RndvSetup Rndv_setup;
#endif
	struct MPID_SMI_BarrierSetup Barrier_setup;
	int Sside_tag_cnt;
} MPID_SMI_Int_data;

extern MPID_SMI_Int_data **MPID_SMI_Int_info_imp;
extern MPID_SMI_Int_data **MPID_SMI_Int_info_exp;
extern int MPID_SMI_Int_bufsize;


/* misc. defines */
#define MPID_SMI_BACKOFF_LMT 200    /* max. backoff counter for polling situations */

/* 
SHO:   For many systems, it is important to align data structures on 
   cache lines, and to insure that separate structures are in
   different cache lines.  Currently, the largest cache line that we've seen
   is 128 bytes, so we pick that as the default.
 */
/* these are currently not used in ch_smi */
#ifndef MPID_CACHE_LINE_SIZE
#define MPID_CACHE_LINE_SIZE 128
#define MPID_CACHE_LINE_LOG_SIZE 7
#endif

/* default name of the configuration file for dynamic memory configuration */
#define DEVCONF_NAME_DEF "ch_smi.conf"

/* name of the switch in the device configuration file to turn on/off the 
   transfer verification */
#define NAME_NO_VERIFY "NO_VERIFY"
#define MPID_SMI_NO_VERIFY_DEF 0

/* use real asynchronous communication for Irecv/Isend? */
extern int MPID_SMI_Do_async_devcheck;  /* continue-flag for signal thread */
/* name of the switch in the device configuration file to turn on/off the 
   use of asynchronous Isends */
#define NAME_ASYNC_PROGRESS "ASYNC_PROGRESS"
#define MPID_SMI_ASYNC_PROGRESS_DEF 0
/* XXX debug switches ! */ 
#define DO_ASYNC_CHECK   1  /* Does the thread do device checks (default: 1)? */
#define WAIT_FOR_SIGNAL  1  /* Let the thread block on a signal or do busy polling (default: 1)? */

/* name of the switch in the device configuration file to turn on/off the 
   collection of runtime-statistics */
#define NAME_STATISTICS "STATISTICS"
#define MPID_SMI_STATISTICS_DEF 0

/* name of the switch in the device configuration file to set the 
   mininum block size for non-contig optimization  */
#define NAME_NC_MINSIZE "NC_MINSIZE"
#define MPID_SMI_NC_MINSIZE_DEF 16
/* name of the switch in the device configuration file to set the 
   mininum block size for non-contig optimization  */
#define NAME_NC_ALIGN "NC_ALIGN"
#define MPID_SMI_NC_ALIGN_DEF 32

/* name of the switch in the device configuration file to enable the
   direct send for non-contig datatypes  
   (0 = no, 1 = simple types (always MPI conform), 2 = full */
#define NAME_NC_ENABLE "NC_ENABLE"
#define MPID_SMI_NC_ENABLE_DEF 1

/* activate the watchdog ? */
#define NAME_WATCHDOG "WATCHDOG"
#define MPID_SMI_WATCHDOG_DEF 3
#define MPID_SMI_WATCHDOG_INC_PER_PROC 0.5

/* name of the switch to turn on fast startup with delayed connection of segments */
#define NAME_CONNECT_ON_INIT "CONNECT_ON_INIT"
/* This option is mostly for benchmarks or cluster resource evaluation.
   Possible values:
   0: no connection on startup except for control messages
   1: dynamically pre-connect eager and rndv on startup
   2: statically pre-connect eager and rndv on startup 
      (enforce - abort if resources are insufficient) */
#define MPID_SMI_CONNECT_ON_INIT_DEF 0

/* name of the switch to specify the scheduling strategy of segment allocation */
#define NAME_RESOURCE_SCHEDULING "RESOURCE_SCHED"
#define MPID_SMI_RESOURCE_SCHEDULING_DEF LRU

/* be verbose on startup ? */
#define MPID_SMI_VERBOSE_DEF 0

/* use locks to synchronize copy operations towards a specific remote adapter or
   via a specific local adapter ? 
   (0 = no sync, 1 = incoming, 2 = outgoing, 3 = in & out)*/
#define NAME_MEMCPYSYNC_MODE "MEMCPYSYNC_MODE"
#define MPID_SMI_MEMCPYSYNC_MODE_DEF 3
#define MEMCPYSYNC_NONE 0
#define MEMCPYSYNC_IN   1
#define MEMCPYSYNC_OUT  2
/* threshold value to do memcpy-synchronization */
#define NAME_MEMCPYSYNC_MIN "MEMCPYSYNC_MIN"
#define MPID_SMI_MEMCPYSYNC_MIN_DEF 2048
/* type of PIO memcpy function to use */
#define NAME_MEMCPY_TYPE "MEMCPY_TYPE"
#define MPID_SMI_MEMCPY_TYPE_DEF 0
#define MPID_SMI_MAX_MEMCPY_TYPE 12
#define MEMCPY_TYPE_AUTO     0
#define MEMCPY_TYPE_SMI      1
#define MEMCPY_TYPE_MEMCPY   2
#define MEMCPY_TYPE_MMX      3
#define MEMCPY_TYPE_MMX32    4
#define MEMCPY_TYPE_MMX64    5
#define MEMCPY_TYPE_WC32     6
#define MEMCPY_TYPE_WC64     7
#define MEMCPY_TYPE_SSE32    8
#define MEMCPY_TYPE_SSE64    9
#define MEMCPY_TYPE_ALPHA    10
#define MEMCPY_TYPE_MMX_PRE  11
#define MEMCPY_TYPE_BCOPY    12

#define MPID_SMI_USE_SMI_MEMCPY 0     /* use SMI_Memcpy or built-in macro? */
#define MPID_SMI_BUFFERCPY_VERIFY 1   /* verify data buffers after they have been written? */

/* adapter scheduling strategy (0 = DEFAULT, 1 = IMPEXP, 2 = SMP) */
#define NAME_ADPTMODE "ADAPTER_MODE"
#define MPID_SMI_ADPTMODE_DEF SMI_ADPT_DEFAULT
extern int MPID_SMI_NBRADPTS;

/* performance modelling */
#define NAME_GAP_LTNCY "PERF_GAP_LTNCY"
#define MPID_SMI_GAP_LTNCY_DEF 0
#define NAME_SEND_LTNCY "PERF_SEND_LTNCY"
#define MPID_SMI_SEND_LTNCY_DEF 0
#define NAME_RECV_LTNCY "PERF_RECV_LTNCY"
#define MPID_SMI_RECV_LTNCY_DEF 0
#define NAME_BW_LIMIT "PERF_BW_LIMIT"
#define MPID_SMI_BW_LIMIT_DEF 0
#define NAME_BW_REDUCE "PERF_BW_REDUCE"
#define MPID_SMI_BW_REDUCE_DEF 0

/* S H O R T  protocol */

/* 
   datastructures for the control packets - also serves as transport protocol
   for SHORT messages of MPID_SMI_SHORTSIZE bytes in total, arranged in a FIFO
   queue of MPID_SMI_SHORTBUFS entries.
*/

extern int MPID_SMI_SHORTSIZE;
extern int MPID_SMI_SHORTBUFS;
/* config-file names */ 
#define NAME_SHORTSIZE "SHORT_bufsize"
#define NAME_SHORTBUFS "SHORT_nbrbufs"
/* default values */
#define MPID_SMI_SHORTSIZE_DEF (8*MPID_SMI_STREAMSIZE) /* size of the short messages incl. header */
#define MPID_SMI_SHORTBUFS_DEF 63     

/* align buffer sizes to be written to SENDCTRL_ALIGN long
   valid values are 1 or 2  (and nothing else!) */
#ifdef MPI_LINUX_ALPHA
#define SENDCTRL_ALIGN_LONGS 1
#else
#define SENDCTRL_ALIGN_LONGS 2
#endif
#define SENDCTRL_ALIGN (SENDCTRL_ALIGN_LONGS*sizeof(long))

#define SHORT_FRAGMENTED 0  /* choose between old (fragmented) and new (contigous) short data
			       transmission technique */
#define SHORT_MAX_MEMCPY_LEN 64  /* data up to this lenght is copied via memcpy(), longer data
				    blocks are copied with special remote memory copy functions */

#define MPID_SMI_SHORTID    1753      /* msg_ids are from 1 to MPID_SMI_SHORTID */

#define MPID_SMI_MSGTAG_T   unsigned char
#define MPID_SMI_MSGID_T    unsigned short
#define MPID_SMI_MSGFLAG_T  unsigned int 


/* data structure for the sender - one of these for each receiver 
   This data is kept in non-SCI memory */
typedef struct {
    char *recv_buffer;
    char *recv_ptr;

    ulong sent_msgs;
    ulong avail_msgs;
    ulong *read_msgs;       /* address of read_msgs counter on the remote host */

    MPID_SMI_MSGID_T msg_id;
} MPID_SMI_lShortsend_t;

/* data structures for receiving messages - one for each other process */
/* this is kept in local memory*/
typedef struct {
    char *recv_buffer;
    VOLATILE char *recv_ptr;

    VOLATILE MPID_SMI_MSGFLAG_T *msg_flag;
    MPID_SMI_MSGID_T newmsg_id;

    VOLATILE ulong *read_msgs;
    ulong delivered_msgs;
} MPID_SMI_lShortrecv_t;


/* protocols for short and ctrl messages are identical */
typedef MPID_SMI_lShortrecv_t MPID_SMI_lCtrlrecv_t;
typedef MPID_SMI_lShortsend_t MPID_SMI_lCtrlsend_t;


/* E A G E R   protocol */

/* The EAGER protocoll uses a MPID_SMI_EAGERBUFS of buffers for every (directed) 
   sender-receiver pair, each buffer holding MPID_SMI_SHORTBUFS bytes of payload 
   They are managed by a pointer field (serving as flags also) which is maintained by
   the receiver and read by sender if he's out of buffers */

/* the number and size of the buffers for the eager protocol */
extern int MPID_SMI_EAGERSIZE;
extern int MPID_SMI_min_EAGERSIZE;
extern int MPID_SMI_EAGERBUFS; 
extern int MPID_SMI_EAGER_MAXCHECKDEV;
extern int MPID_SMI_EAGER_DYNAMIC;
extern int MPID_SMI_EAGER_IMMEDIATE;

/* align copy operations in eager protocol to this size */
#define MPID_SMI_EAGER_ALIGN_SIZE 1
/* config-file names */
#define NAME_EAGERSIZE  "EAGER_bufsize"
#define NAME_EAGERBUFS  "EAGER_nbrbufs"
#define NAME_EAGERMAXCHECKDEV "EAGER_maxcheckdev"
#define NAME_EAGERDYNAMIC "EAGER_dynamic"
#define NAME_EAGERIMMEDIATE "EAGER_immediate"
/* default values */
#define MPID_SMI_EAGERSIZE_DEF  (128*1024)
#define MPID_SMI_EAGERBUFS_DEF  4
/* if no free eager send buffer could be found, check for outstanding receives how often? */
#define MPID_SMI_EAGER_MAXCHECKDEV_DEF  1
#define MPID_SMI_EAGER_DYNAMIC_DEF 0
#define MPID_SMI_EAGER_IMMEDIATE_DEF 0


/* Ringbuffer structure to manage eager buffers in the static eager protocol */
typedef struct {
    volatile unsigned int *offsets; /* array of buffer offsets*/
    volatile unsigned int *ptr;     /* pointer to current entry */

    int slotindex;     /* number of current ring buffer slot to read / write */
} MPID_SMI_lEagerRingbuf_t;

/* this structure is the equivalent to the above for the dynamic implementation of the
   eager protocol */
typedef struct {
    volatile unsigned int *r;
    volatile unsigned int *w;
} MPID_SMI_lEageroffset_t;


/* R E N D E Z V O U S  protocol */

/* The RENDEZVOUS protocoll uses dynamic buffers allocated from a buffer 
   of MPID_SMI_RNDVSIZE bytes each for all processes. The address of a 
   free buffer is sent to the requesting process. */
/* MPID_SMI_RNDVBLOCK is the size of message blocks which are transmitted
   in an pipelined manner. */
/* MPID_SMI_RNDVRECEIPT is the size of the shared memory buffer reserved for
   incoming data of a pipelined transfer (expressed as multiple of MPID_SMI_RNDVBLOCK) */
extern int MPID_SMI_RNDVSIZE;
extern int MPID_SMI_RNDVBLOCK;
extern int MPID_SMI_RNDVRECEIPT;
extern int MPID_SMI_RNDVDMASIZE;
extern int MPID_SMI_RNDVBLOCKING_MODE;

#define USE_REGION_MNGMT  1

/* config-file names */
#define NAME_RNDVSIZE    "RNDV_poolsize"
#define NAME_RNDVBLOCK   "RNDV_blocksize"
#define NAME_RNDVRECEIPT "RNDV_receipt"
#define NAME_RNDVDMASIZE "RNDV_dmasize"
#define NAME_RNDVBLOCKING_MODE "RNDV_blocking"
/* default values */
#define MPID_SMI_RNDVSIZE_DEF    (512*MPID_SYS_PAGESIZE)
#define MPID_SMI_RNDVBLOCK_DEF   (32*4096)
#define MPID_SMI_RNDVRECEIPT_DEF 12
#define MPID_SMI_RNDVDMASIZE_DEF (64*4096)
#define MPID_SMI_RNDVBLOCKING_MODE_DEF 0

#endif





/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
