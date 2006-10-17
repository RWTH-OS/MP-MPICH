/* $Id$ */

/********************************************************************************/
/*** This file includes definitions of all constants, data-types and          ***/
/*** functions of the shared memory interface.                                ***/
/********************************************************************************/


#ifndef _SMI_H_
#define _SMI_H_

#ifdef WIN32
#include <wtypes.h>
#define pthread_t HANDLE
#else
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* just to fix indendation problems while editing */
#if 0
}
#endif

/********************************************************************************/
/*** Error codes                                                              ***/
/********************************************************************************/
#define SMI_SUCCESS       0
#define SMI_ERR_OTHER     1
#define SMI_ERR_NOINIT    2
#define SMI_ERR_PARAM     3
#define SMI_ERR_BADADR    4
#define SMI_ERR_MAPFAILED 5
#define SMI_ERR_NODEVICE  6
#define SMI_ERR_NOSEGMENT 7
#define SMI_ERR_NOMEM     8
#define SMI_ERR_NOTIMPL   9
#define SMI_ERR_TRANSFER  10  
#define SMI_ERR_PENDING   11
#define SMI_ERR_NOTPOSTED 12
#define SMI_ERR_BADALIGN  13
#define SMI_ERR_BUSY      14

/********************************************************************************/
/*** Shared memory region's physical distribution policy                      ***/
/********************************************************************************/
/* types */
#define SMI_SHM_UNDIVIDED   0     /* one globally shared segment */
#define SMI_SHM_BLOCKED     1     /* globally shared, block-distributed segments */
#define SMI_SHM_CYCLIC      2     /* globally shared, cylic-distributed segments */
#define SMI_SHM_CUSTOMIZED  3     /* globally shared, custom-distributed segments */
#define SMI_SHM_SMP         4     /* SMP-wide shared segment */
#define SMI_SHM_PT2PT       5     /* segment shared between two procs */
#define SMI_SHM_FRAGMENTED  6     /* non-contignously block-distributed global segments */
#define SMI_SHM_LOCAL       7     /* locally generated and exported SCI segment */
#define SMI_SHM_REMOTE      8     /* import an existing remote SCI segment */
#define SMI_SHM_RDMA        9     /* remote DMA, no PIO access */

/* flags */
#define SMI_SHM_DELAYED     (1<<10)  /* indicate delayed client connection */
#define SMI_SHM_NONFIXED    (1<<11)  /* indicate nonfixed mapping is allowed (implicit for some region types) */
#define SMI_SHM_REGISTER    (1<<12)  /* indicate that local memory is to be registered for
				      the SCI segment to be created */
#define SMI_SHM_PRIVATE     (1<<13)  /* the SCI segment is not to be exported (i.e. DMA source memory) */ 
#define SMI_SHM_INTERN      (1<<14)  /* enforce local, non-SCI shared memory */
#define SMI_SHM_SHRINKABLE  (1<<15)  /* the size of the region may be reduced if not enough resources 
					are available for the given size */
#define SMI_SHM_CALLBACK    (1<<16)  /* enable callbacks for this region */

/************************************************************************************/
/*** SMI_Check_transfer() flags, correspond to SISCI flags for SCICheckSequence() ***/
/************************************************************************************/
#define SMI_CHECK_FULL      0
#define SMI_CHECK_NOFLUSH   1
#define SMI_CHECK_NOBARRIER 2
#define SMI_CHECK_FAST      (SMI_CHECK_NOFLUSH | SMI_CHECK_NOBARRIER)
#define SMI_CHECK_PROBE     4     /* do only probe the state, do not wait until it changes */

/************************************************************************************/
/*** region callbacks                                                             ***/
/************************************************************************************/
/* reasons for a callback */
#define SMI_CB_REASON_CONNECT      1
#define SMI_CB_REASON_DISCONNECT   (1<<1)

/* callback function */
typedef int (*smi_region_callback_t)(int region_id, int cb_reason);

/* return values for callback function: what should the SMI callback (thread) do?  */
#define SMI_CB_ACTION_NOTHING    1       /* do nothing with the region and continue running */
#define SMI_CB_ACTION_FREEREG    (1<<1)  /* remove the region and terminate */   
#define SMI_CB_ACTION_TERMINATE  (1<<2)  /* just terminate */   

/************************************************************************************/
/*** implicit specification of PCI-SCI-adapter to use                             ***/
/************************************************************************************/
#define SMI_ADPT_DEFAULT         0xf0     /* use default adapter */
#define SMI_ADPT_CYCLIC          0xf1     /* cyclic usage of all available adapters */
#define SMI_ADPT_IMPEXP          0xf2     /* use separate adapters for exporting and importing
					     segments/regions (if possible) */
#define SMI_ADPT_SMP             0xf3     /* for multiple local SMI processes of an application,
					     each process shall use a different adapter */
#define SMI_ADPT_BIDIRECTIONAL   0xf4     /* in a bidirectional torus, use the adapter which has
					     the shortest distance towards the related process. */
#define SMI_ADPT_INVALID         -1

/********************************************************************************/
/*** Different combination policies when switching between replicated and     ***/
/*** shared state of memory regions and replication policies.                 ***/
/********************************************************************************/
#define SMI_SHR_SINGLE_SOURCE    1
#define SMI_SHR_LOOP_SPLITTING   2
#define SMI_SHR_ADD              4
#define SMI_REP_NOTHING          8
#define SMI_SHR_NOTHING          8
#define SMI_REP_EVERYTHING       16
#define SMI_REP_LOCAL_AND_BEYOND 32
#define SMI_SHR_EVERY_LOCAL      64

#define SMI_SHR_SPARSE           256
#define SMI_SHR_BAND             512
#define SMI_REP_ONE_PER_NODE     1024

#define SMI_DTYPE_FIXPOINT         2048
#define SMI_DTYPE_FLOATINGPOINT    4096
#define SMI_DTYPE_HIGHPRECISION    8192


/********************************************************************************/
/*** Different strategies to determine a loop splitting.                      ***/
/********************************************************************************/
#define SMI_SPLIT_REGULAR    0
#define SMI_SPLIT_OWNER      1


/********************************************************************************/
/*** Memory types for SMI_Memcpy / SMI_Imemcpy                                ***/
/***                                                                          ***/
/***   You can supply the types of source and dest memory to speed up the     ***/
/***   copy operation. If none of these flags is supplied, SMI will choose    ***/
/***   the appropiate method of copying the memory itself.                    ***/
/***                                                                          ***/
/***   Syntax: LP = local privat memory                                       ***/
/***           LS = local shared memory segment                               ***/
/***           RS = remote shared memory segment                              ***/
/***   therewith: SMI_MEMCPY_sourcetype_desttype                              ***/
/********************************************************************************/
#define SMI_MEMCPY_NOBARRIER 1   /* no store barrier is issued after the copy operation -
				    do only use if memory addresses and size are well aligned */
#define SMI_MEMCPY_NOVERIFY  2   /* no verification (sequence check) and retry is done after the 
  				    copy operation - may be used if you want to check several 
				    smaller operations at once using SMI_Check_transfer() */
#define SMI_MEMCPY_ENQUEUE   4   /* for Imemcpy: do not immeadetely start the copy, but instead
				     queue the operation */
#define SMI_MEMCPY_FORCE_DMA 8   /* */
#define SMI_MEMCPY_ALIGN     16  /* align each copy operation to the size of a streambuffer - 
				     care must been taken the destination is big enough! */
#define SMI_MEMCPY_FAST      3

#define SMI_MEMCPY_LP_LP 128
#define SMI_MEMCPY_LS_LP 128
#define SMI_MEMCPY_LS_LS 128
#define SMI_MEMCPY_LP_LS 256
#define SMI_MEMCPY_LP_RS 512
#define SMI_MEMCPY_RS_RS 512
#define SMI_MEMCPY_RS_LP 1024
#define SMI_MEMCPY_LS_RS 2048
#define SMI_MEMCPY_RS_LS 4096

#define SMI_MEMCPY_FLAGS_ALL   (1+2+4+8+16+128+256+512+1024+2048+4096)
#define SMI_MEMCPY_MTFLAGS_ALL (        128+256+512+1024+2048+4096)

/******************************************************************************/
/*** Constants for IO Redirection                                           ***/
/******************************************************************************/
#define SMI_IO_ASIS     0
#define SMI_IO_FILE     1


/******************************************************************************/
/*** Type of the current SCI Topology                                       ***/
/******************************************************************************/
#define SMI_SCI_TOPOLOGY_UNKNOWN 0
#define SMI_SCI_TOPOLOGY_SWITCH  1
#define SMI_SCI_TOPOLOGY_TORUS   2
#define SMI_SCI_TOPOLOGY_SMP     3


/********************************************************************************/
/*** Query type for SMI_Query()                                               ***/
/********************************************************************************/
typedef enum  { 
    /*
     * SCI related queries
     */
    SMI_Q_SCI_STREAMBUFSIZE = 101,     /* size of the streambuffers on the PCI-SCI adapter 
				       arg: nbr of local adapter to query
				       result: int  */
    SMI_Q_SCI_NBRSTREAMBUFS = 102,     /* number of streambuffers on the PCI-SCI adapter 
				       arg: nbr of local adapter to query
				       result: int  */
    SMI_Q_SCI_NBRADAPTERS = 103,       /* number of configured PCI-SCI adapters which a process can access
				       arg: rank of a process
				       result: int  */
    SMI_Q_SCI_ADAPTERTYPE = 104,       /* type of the specified PCI-SCI adapter
				       arg: nbr of adapter to query
				       result: int  */
    SMI_Q_SCI_DEFADAPTER = 105,        /* return the number of the local default PCI-SCI adapter
				       arg: none
				       result: int */
    SMI_Q_SCI_VALIDADAPTER = 106,      /* test if the specified number of a PCI-SCI adapter is valid
				       arg: adapter number
				       result: int (to be interpreted as boolean) */
    SMI_Q_SCI_ID = 107,                /* ID of the specified local PCI-SCI adapter
				       arg: nbr of adapter to query
				       result: int  */
    SMI_Q_SCI_PROC_ID = 108,           /* SCI ID of the adapter another process uses to commumicate
				       with the local proc
				       arg: rank of proc to query
				       result: int (the ID of the adapter which exports the internal
				       segment of the proc) */
    SMI_Q_SCI_CONNECTION_STATE = 109,  /* the current state of the SCI connections:
				       arg: none
				       result: int, error code SUCCESS or PENDING */
    SMI_Q_SCI_ERRORS = 110,            /* the current state of the SCI connections:
				       arg: none
				       result: int, number of transmission errors noticed by the PCI-SCI
				       adapters since application start */
    SMI_Q_SCI_API_VERSION = 111,       /* The Version of the currently used sci-api
				       arg: size of the memory block provided by user
				       result: null-terminated string of size "arg" */
    SMI_Q_SCI_PACKETSIZE = 112,        /* the biggest SCI data-move packet (atomic transaction) 
					  available (currently 64 or 128 byte, identical for all 
					  adapters)
					  arg: none
					  result: int (transaction size in bytes) */
    SMI_Q_SCI_DMA_SIZE_ALIGN = 113, /* the required alignment for source/target addresses/offsets
					  of DMA transfers - misaligned transfers will not succeed
					  arg: none
					  result: int (alignment size in bytes) */
    SMI_Q_SCI_DMA_OFFSET_ALIGN = 114,/* the required alignment for the size of DMA transfers - 
					  misaligned transfers will not succeed
					  arg: none
					  result: int (alignment size in bytes) */
    SMI_Q_SCI_DMA_MINSIZE = 115,       /* the reocmmended minimum size for DMA transfers
					  arg: none
					  result: int (minimum size in bytes) */
    SMI_Q_SCI_DMA_MAXSIZE = 116,       /* the maximum size for DMA transfers
					  arg: none
					  result: int (minimum size in bytes) */
    SMI_Q_SCI_NBR_PHYS_DIMS = 117,    /* nbr of physical dimensions of the SCI topology 
					 arg: none
					 result: int (nbr of dimensions (1, 2, 3, ...)) */
    SMI_Q_SCI_PHYS_DIM_EXTENT = 118,  /* extension of an SCI dimension, measured in nodes
					 arg: rank of the dimension (1, 2, 3, ...)
					 result: int (nbr of nodes in this dimension) */
    SMI_Q_SCI_SCI_TOPOLOGY_TYPE = 119, /* type of the SCI topology
					  arg: none
					  result: SMI_SCI_TOPOLOGY_XY */
    SMI_Q_SCI_SCIIDS_VALID = 120,      /* do the SCI node id's match the physical topology?
					  arg: none
					  result: int (true or false) */
    SMI_Q_SCI_SYSTEM_TYPE = 121,          /* do the SCI node id's match the physical topology?
					  arg: none
					  result: int (SMI_SCI_xxx flag) */
    
    /*
     * SMI related queries
     */
    SMI_Q_SMI_INITIALIZED = 201,       /* is the SMI library initialized ?
				       arg: none
				       result : int 
				            = 0  no
                                            > 0  yes */
    SMI_Q_SMI_REGION_CONNECTED = 202,  /* connection status of the shared region
				       arg: SMI id of the shared region
				       result : int 
				            0: not conncted 
				            1: connected & ready to use */
    SMI_Q_SMI_REGION_SGMT_ID = 203,    /* low-level id of the segment of the region (SCI id or shmem id)
					  This can be:
					  - for a region with a local segment: the id of the local segment
					  - for a region with no local segment, but exactly one remote 
					    segment: the id of this remote segment
   				       arg: SMI id of the shared region
				       result : int 
				            > 0 o.k.
                                            < 0 error (illegal SMI region id, or more than 1 rmt segments) */
    SMI_Q_SMI_SIGNALS_AVAILABLE = 204, /* can signals be used (SMI_Signal_xy()) ? 
				       arg: none
				       result: int
				            = 0  no
                                            > 0  yes */
    SMI_Q_SMI_DMA_AVAILABLE = 205,    /* are DMA transfers available ? 
				      arg: none
				      result: int
				            = 0  no
                                            > 0  yes */				    
    SMI_Q_SMI_REGION_ADPTNBR = 206,    /* nbr of the local adapter used to export this region
				       arg: SMI id of the shared region
				       result : int 
				            > 0 o.k.
                                            < 0 error (i.e. no local segment) */
    SMI_Q_SMI_REGION_ADDRESS = 207,    /* local starting address of this region (if regions consists of multiple
					  segments, the starting address of the local segment is returned)
				       arg: SMI id of the shared region
				       result : void ** 
				         *result == NULL  invalid region
				         *result != NULL  address */
    SMI_Q_SMI_REGION_SIZE = 208,      /* Size of the complete region (in bytes). For FRAGMENTED, size of one
					 fragment.
					 arg: SMI id of the shared region
					 result: long
					    = 0  error (illegal region id)
					    > 0 o.k. */
    SMI_Q_SMI_REGION_OWNER = 209,    /* Process rank of the owner of the complete region. Only applicable for
					regions which consist of only one segment!
					arg: SMI id of the shared region
					result: int
					< 0  error (illegal region id, or illegal region type)
					>= 0 process rank of the owner */
    SMI_Q_SMI_REGION_OFFSET = 210,   /* Offset of the complete region (in bytes). Only valid for mapped regions
					consisting of one segment (REMOTE, LOCAL, UNDIVIDED, SMP)
					 arg: SMI id of the shared region
					 result: signed long
					    <  error (illegal region type)
					    >=  0 o.k. */
    SMI_Q_SMI_NBR_APP_DIMS = 211,    /* nbr of application dimensions of the SCI topology 
					arg: none
					result: int (nbr of dimensions (1, 2, 3, ...)) */
    SMI_Q_SMI_APP_DIM_EXTENT = 212,  /* extension of an application dimension, measured in nodes
					arg: rank of the dimension (1, 2, 3, ...)
					result: int (nbr of nodes in this dimension) */
    SMI_Q_SMI_APP_DIM_OFFSET = 213,  /* offset of the placement of the application grid within the physical grid
					arg: rank of the dimension (1, 2, 3, ...)
					result: int (nbr of nodes in this dimension) */

    /*
     * system related queries
     */
    SMI_Q_SYS_NBRCPUS = 301,          /* number of active CPUs in this node 
				      arg: none
				      result: int */
    SMI_Q_SYS_CPUFREQ = 302,          /* clock frequency of the CPUs
				      arg: none
				      result: int (MHz) */
    SMI_Q_SYS_PAGESIZE = 303,         /* VMM page size of this node
				      arg: none
				      result: int */
    SMI_Q_SYS_WRITECOMBINING = 304,   /* is write-combining enabled for the SCI remote memory?
				      arg: none
				      result: int (boolean) */
    SMI_Q_SYS_PID = 305,              /* Process id of process with certain smi_rank
				      arg: smi_rank of process to query
                                      result: int */
    SMI_Q_SYS_TICK_DURATION = 306,     /* How long does a 'tick' as measured by SMI_Get_ticks() and
					 SMI_Wticks() take in ps?
					 arg: none
					 result: int (nbr of ps) */
    SMI_Q_SYS_CPU_CACHELINELEN = 307  /* length of a CPU cache line
					 arg: none
					 result: int (bytes) */
} smi_query_t; 


/********************************************************************************/
/*** various system characteristics                                           ***/
/********************************************************************************/
/* write-combining status */
#define SMI_WC_ENABLED  1
#define SMI_WC_DISABLED 0
#define SMI_WC_UNKNOWN  -1

/* watchdog control */
#define SMI_WATCHDOG_DISABLE 0
#define SMI_WATCHDOG_OFF     -1

/* SCI type */
#define SMI_SCI_NONE    0
#define SMI_SCI_DOLPHIN 1
#define SMI_SCI_SCALI   (1<<1)
#define SMI_SCI_UNKNOWN (1<<8)

/********************************************************************************/
/*** Different modes to get/set the global/local index range of a loop.       ***/
/********************************************************************************/
#define SMI_LOOP_SET_GLOBAL  0
#define SMI_LOOP_GET_GLOBAL  1
#define SMI_LOOP_SET_LOCAL   2
#define SMI_LOOP_GET_LOCAL   3

/********************************************************************************/
/*** Progrss Counters                                                         ***/
/********************************************************************************/
#define SMI_OWNPC -1


/********************************************************************************/
/*** loop scheduling                                                          ***/
/********************************************************************************/
/* loop scheduling modes */
#define SMI_PART_BLOCKED         1
#define SMI_PART_CYCLIC          2
#define SMI_PART_ADAPTED_BLOCKED 3
#define SMI_PART_TIMED_BLOCKED   4
/* loop scheduling status */
#define SMI_LOOP_READY  0
#define SMI_LOOP_LOCAL  1
#define SMI_LOOP_REMOTE 2
/* loop scheduling adaptionMode */
#define SMI_NO_ADAPT     1
#define SMI_ADAPT_EXPO   2
#define SMI_ADAPT_LINEAR 3
#define SMI_ADAPT_OPT    4
/* loop scheduling misc */
#define SMI_HELP_ONLY_SMP -1
#define SMI_NO_CHANGE      0
  
/********************************************************************************/
/*** Signalling modes                                                         ***/
/********************************************************************************/
#define SMI_SIGNAL_ANY   (256*256*256*64)
#define SMI_SIGNAL_BCAST (SMI_SIGNAL_ANY-1)


/********************************************************************************/
/*** Flushing modes                                                             */
/********************************************************************************/
#define SMI_FLUSH_ALL    -1


/********************************************************************************/
/*** synchronization algorithms                                                 */
/********************************************************************************/
#define PROGRESS_COUNTER_BARRIER 0 
#define SCHULZ_BARRIER           1

#define L_MUTEX           0 /* A Mutex-Algorithm from Leslie Lamport            */
#define BL_MUTEX          1 /* A Mutex-Algorithm from Burns & Lynch             */
#define SCH_MUTEX         2 /* A Mutex-Algorithm from Martin Schulz             */ 


/********************************************************************************/
/*** Data Structures                                                            */
/********************************************************************************/
    typedef struct smi_sgmt_locator_t_ {
	int sci_id;
	int segment_id;
    } smi_sgmt_locator_t;

    typedef struct {
	size_t size;         /* all: size of the region to use */

	int owner;        /* undiv, pt2pt, remote: process who owns the region  */
	size_t offset;    /* undiv, pt2pt, remote: offset for import of the segment */
	
	int sgmt_id;      /* pt2pt, remote: id of the segment (for delayed connections) */
	int partner;      /* pt2pt: connecting process */

	int nbr_sgmts;    /* custom, cyclic: nbr of segments the region consists of */

	int *sgmt_owner;  /* custom: mapping of segment nbr to segment owner */
	size_t *sgmt_size;/* custom: mapping of segment nbr to segment size */

	int adapter;      /* all: which *local* PCI-SCI adapter to use? */
	int rmt_adapter;  /* remote: which *remote* PCI-SCI adapter to use? */

	int shrinkable;   /* indicates if region can be shrinked if there not
			     enough memory fol allocation */
	smi_region_callback_t cb_fcn;  /* callback function for segment events */
    } smi_region_info_t;

    
typedef struct {
   size_t nsegments;    /* number of comprising segments             */
   size_t size;      /* total region size                         */
   char* adr;        /* region start address                      */
   size_t* seg_size;    /* size of each segment                      */
   char** seg_adr;   /* start address of each segment             */
   int* seg_machine; /* physical machine location of each segment */
 } smi_rlayout_t;

typedef int smi_error_t;

typedef struct smi_memcpy_handle_t_ {
    int otid;                              /* the Object Type Id is used to verify
					    * if handle is valid */
#ifdef _SISCI_API_H
    sci_dma_queue_t dq;                    /* the dma_queue assosiated to the transaction */
#endif 
    void *dma_thread_args;                 /* thread which is performing long DMA transfers. */

    smi_error_t dma_err;
    int dma_used;                          /* this is set if dma transfer is used */   
    int dma_entries;                       /* the actual number of entries in the queue */
    int dma_remoteseg;                     /* regionid of remote segment used by dma-transfer */
    int dma_rmt_proc;                      /* rank of the remote process */
    size_t dma_offset;                        /* offset, used for dma-transfer (remotesegment) */
    int dma_direction;                     /* direction used within dma-transfer */
    char* dma_localadr;                    /* adress of local data */
    void* mc_dest;                         /* destination adddress */
    void* mc_src;                          /* source adddress */
    size_t mc_size;                           /* given size  */
    int mc_flags;                          /* given flags */

    struct smi_memcpy_handle_t_** pHandle; /* Adress where pointer to this struct is stored */
    struct smi_memcpy_handle_t_* pNext;    /* this is used to store unused handles in a stack for reuse */
} smi_memcpy_handle_t;
typedef smi_memcpy_handle_t* smi_memcpy_handle;

typedef struct smi_signal_handle_t_ {
  pthread_t ptThread; 
  int iProcRank;
  void (*callback_fcn)(void *);
  void *callback_arg;
} smi_signal_handle_t;
typedef smi_signal_handle_t* smi_signal_handle;


/********************************************************************************/
/*** function headers                                                         ***/
/********************************************************************************/
smi_error_t SMI_Init(int* argc, char*** argv);
smi_error_t SMI_Finalize(void);
void SMI_Abort(int error_code);

smi_error_t SMI_Proc_rank(int* rank);
smi_error_t SMI_Local_proc_rank(int* rank);
smi_error_t SMI_Node_rank(int *node);
smi_error_t SMI_Proc_size(int *size);
smi_error_t SMI_Local_proc_size(int *size);
smi_error_t SMI_Max_local_proc_size(int *size);
smi_error_t SMI_Get_node_name(char *nodename, size_t *namelen);
smi_error_t SMI_Node_size(int *size);
smi_error_t SMI_Proc_to_node(int proc, int *node);
smi_error_t SMI_First_proc_on_node(int node, int *proc);
smi_error_t SMI_Query (smi_query_t cmd, int arg, void *result);
smi_error_t SMI_Page_size(int *size);

smi_error_t SMI_Create_shreg(int region_type, smi_region_info_t *region_desc, int *id, void **address);
smi_error_t SMI_Init_reginfo(smi_region_info_t *region_desc, size_t size, size_t offset, int owner, int loc_adpt, 
			 int rmt_adpt, int sgmt_id, smi_region_callback_t cb_fcn);
smi_error_t SMI_Connect_shreg (int id, void **address);
smi_error_t SMI_Free_shreg(int id);
smi_error_t SMI_Set_adapter (int adapter);
smi_error_t SMI_Set_region_callback (int region_id, smi_region_callback_t cb_fcn);
smi_error_t SMI_Adr_to_region (void *address, int *region_id);
smi_error_t SMI_Range_to_region (void *address, size_t len, int *region_id);
smi_error_t SMI_Region_layout(int region_id, smi_rlayout_t** r);
smi_error_t SMI_Get_region_locator (int region_id, smi_sgmt_locator_t *region_locator);

smi_error_t SMI_Init_shregMMU(int region_id);
smi_error_t SMI_Imalloc(size_t size, int region_id, void **address);
smi_error_t SMI_Cmalloc(size_t size, int region_id, void **address);
smi_error_t SMI_Ifree(void *address);
smi_error_t SMI_Cfree(void *address);
void *SMI_Nodemem_address (int node_rank);
void *SMI_Nodemem_alloc (size_t bufsize);
smi_error_t SMI_Nodemem_free (void *buf);

#define SMI_Barrier() SMI_BARRIER(0)
#define SMI_Mutex_init(id) SMI_MUTEX_INIT((id), BL_MUTEX, -1)
#define SMI_Mutex_init_with_locality(id,prank) SMI_MUTEX_INIT((id), BL_MUTEX, (prank))
smi_error_t SMI_Mutex_destroy(int);
smi_error_t SMI_Mutex_lock(int);
smi_error_t SMI_Mutex_trylock(int, int*);
smi_error_t SMI_Mutex_unlock(int);
smi_error_t SMI_Location_of_mutex(int, int*);

smi_error_t SMI_Flush_read(int);
smi_error_t SMI_Flush_write(int);

smi_error_t SMI_Loop_split_init(int *loop_id);
smi_error_t SMI_Loop_index_range(int loop_id, int *lower, int *upper, int mode);
smi_error_t SMI_Determine_loop_splitting(int loop_id, int mode, int param1, int param2);
smi_error_t SMI_Loop_time_start(int loop_id);
smi_error_t SMI_Loop_time_stop(int loop_id);
smi_error_t SMI_Loop_balance_index_range(int loop_id);

smi_error_t SMI_Switch_to_replication(int id, int mode, int param1, int param2, int param3);
smi_error_t SMI_Switch_to_sharing(int id, int comb_mode, int comb_param1, int comb_param2);
smi_error_t SMI_Ensure_consistency(int id, int comb_mode, int comb_param1, int comb_param2);

smi_error_t SMI_Loop_init(int* const id, const int globalLow, const int globalHigh ,int mode);
smi_error_t SMI_Get_iterations(const int id, int* const status, int* const low, int* const high);
smi_error_t SMI_Loop_free(const int id);
smi_error_t SMI_Evaluate_speed(double* const speedArray); 
smi_error_t SMI_Use_evaluated_speed(const int id);
smi_error_t SMI_Set_loop_param(const int id, const double kNew,
		       const int minChunkSizeLocal, const int minChunkSizeRemote,
		       const int maxChunkSizeLocal, const int maxChunkSizeRemote);
smi_error_t SMI_Set_loop_help_param(const int id, const int maxHelpDist);
smi_error_t SMI_Loop_k_adaption_mode(const int id, const int adaptionMode, const int maxCalcDist);

smi_error_t SMI_Get_timer(int *sec, int *microsec);
smi_error_t SMI_Get_timespan(int *sec, int *microsec);
double  SMI_Wtime(void);
long    SMI_Wticks(void);
void SMI_Get_ticks(void *ticks);  /* "ticks" has to be a 64 bit integer */

smi_error_t SMI_Init_PC(int *id);
smi_error_t SMI_Reset_PC(int id);
smi_error_t SMI_Increment_PC(int id, int val);
smi_error_t SMI_Get_PC(int pcid, int proc_id, int* pc_val);
smi_error_t SMI_Wait_individual_PC(int id, int proc_rank, int val);
smi_error_t SMI_Wait_collective_PC(int id, int val);

smi_error_t SMI_Memcpy (void *dest, void *src, size_t size, int flags);
smi_error_t SMI_Imemcpy (void *dest, void *src, size_t size, int flags, smi_memcpy_handle* h);
smi_error_t SMI_Memwait (smi_memcpy_handle h);
smi_error_t SMI_Memtest (smi_memcpy_handle h);
smi_error_t SMI_MemwaitAll (int count, smi_memcpy_handle *h, smi_error_t *status); 
smi_error_t SMI_MemtestAll (int count, smi_memcpy_handle *h, smi_error_t *status);
smi_error_t SMI_Put (int dest_region_id, int offset, void *src, size_t size);
smi_error_t SMI_Get (void *dst, int src_region_id, int offset, size_t size);
smi_error_t SMI_Iput (int dest_region_id, int offset, void *src, size_t size, smi_memcpy_handle* pHandle);
smi_error_t SMI_Iget (void *dst, int src_region_id, int offset, size_t size, smi_memcpy_handle* pHandle);

smi_error_t SMI_Signal_wait (int proc_rank);
smi_error_t SMI_Signal_send ( int proc_rank );
smi_error_t SMI_Signal_setCallBack (int proc_rank, void (*callback_fcn)(void *), 
				void *callback_arg, smi_signal_handle* h);
smi_error_t SMI_Signal_joinCallBack (smi_signal_handle* h);

smi_error_t SMI_Send(void *buf, int count, int dest);
smi_error_t SMI_Recv(void *buf, int count, int dest);
smi_error_t SMI_Isend(void *buf, int count, int dest);
smi_error_t SMI_Send_wait(int dest);
smi_error_t SMI_Sendrecv(void *send_buf, void *recv_buf, int count, int dest);

smi_error_t SMI_Check_transfer( int flags );
smi_error_t SMI_Check_transfer_addr( void* address, int flags );
smi_error_t SMI_Check_transfer_proc( int node, int flags );

smi_error_t SMI_Debug(int bool_switch);
smi_error_t SMI_Watchdog(int iWatchdogTimeout);
smi_error_t SMI_Watchdog_callback( void (*callback_function)(void) );
smi_error_t SMI_Redirect_IO(int err, void* errparam, 
			int out, void* outparam, 
			int in, void* inparam);

/********************************************************************************/
/*** Just for internal purposes.                        .                    ***/
/********************************************************************************/

/*** This constant is used in Mutex-lock and -unlock operations to be      ***/
/*** 'ored' with the lock-id, to denote, that it is not required to ensure ***/
/*** a correct memory model within this operations, i.e. to perform a      ***/
/*** load- or store-barrier.                                               ***/
#define NOCONSISTENCY (1024*1024*1024)

void print_region(void);
smi_error_t SMI_MUTEX_INIT(int *, int, int);
smi_error_t SMI_BARRIER_INIT(int *, int, int);
smi_error_t SMI_BARRIER_DESTROY(int);
smi_error_t SMI_BARRIER(int);

#ifdef __cplusplus
}
#endif



#endif
