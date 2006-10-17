/* $Id$ */

/******************************************************************************/
/******************************************************************************/
/***                                                                        ***/
/*** This module contains some general definitions, required by several SMI ***/
/*** modules which are not contained already in the 'smi.h' file.           ***/
/***                                                                        ***/
/******************************************************************************/
/******************************************************************************/

#ifndef _SMI_GENERAL_DEFINITIONS_H_
#define _SMI_GENERAL_DEFINITIONS_H_

#define SMI_VERSION "SMI library version 2.9"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/*** includes.                                                              ***/
/******************************************************************************/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
	
#ifdef HAVE_CONFIG_H
/* WIN32 does not have config.h - see below */
#include "smiconfig.h"
#elif defined WIN32
#include "smiconfig_win32.h"
#else
#error No configuration file found! (smiconfig.h)
#endif

#ifndef WIN32
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#else

#include <windows.h>
#endif

#ifndef NO_SISCI
#include <sisci_api.h>
#include "proper_shutdown/sci_desc.h"
#else
typedef int sci_error_t;
#endif

#include "safety.h"
#include "smi.h"
#include "smi_fifo.h"


/******************************************************************************/
/*** definitions.                                                           ***/
/******************************************************************************/

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#ifdef WIN32
#ifndef true
#define boolean BOOL
#define false FALSE
#define true  TRUE
#endif
#else
#ifndef true
typedef int boolean;
#define false 0
#define true  1
#endif
#endif

#ifndef SCALI_SISCI
typedef unsigned int unsigned32;
typedef unsigned char unsigned8;
#endif

#ifdef HAVE_SCIINITIALIZE
/* adaptions to different SISCI API versions */
#if API_VER_MINOR == 10
#define SCIStoreBarrier(seq, flag, err) SCIStoreBarrier(seq, flag)
#endif
#endif

#define DEV_GLOBAL  0
#define DEV_SMP     1
#define DEV_LOCAL   2

#define MPI_SUCCESS 0


/* for parsing commandline parameters */ 
#define MAX_FILENAME_LEN 256
#define MAX_HOSTNAME_LEN 256

/* this port is used for the initial startup */
#define SMI_DEFAULT_TCP_PORT 51069

/* The default size INT_SHMSEG of the "internal region" is sufficient 
   for INT_SHMSEG_SCALE processes. If more processes are used, this
   size needs to be scaled up accordingly. See configure.in for the
   default value of the related INT_SHMSEG_BASESIZE. */
#define INT_SHMSEG_SCALE  8

/* A parameter in that this bit is set indicates that the value, */
/* ignoring this bit, has a special meaning                      */ 
#define INTERNAL   0x10000  /* 65536 */
#define LOCAL_ONLY 0x100000

/* internal flags for shared region creation */
#define SHREG_NONFIXED 1
#define SHREG_DELAYED  (1<<1)
#define SHREG_REGISTER (1<<2)
#define SHREG_PRIVATE  (1<<3)
#define SHREG_ASYNC    (1<<4)
#define SHREG_PT2PT    (1<<5)
#define SHREG_LOCAL    (1<<6)
#define SHREG_NOMAP    (1<<7)
#define SHREG_CALLBACK (1<<8)

#define SHREG_MAX_REGION SMI_SHM_RDMA

/* define machine and process ranks based on hostnames or SCI-IDs ? */
#define RANKS_BY_HOSTNAME 0

/******************************************************************************/
/*** if your PCI-SCI adapter has more stream buffers (wow!), adjust this    ***/
/*** to a reasonable size. It is only needed for an array definition in     ***/
/*** _smi_store_barrier.c                                                   ***/
/******************************************************************************/
#define MAX_NBR_STREAMBUFS 64
#ifndef NO_SISCI
#define ALLSTREAMS_SIZE    (_smi_StreambufSize*_smi_NbrStreambufs)
#define INTS_PER_STREAM    (_smi_StreambufSize/sizeof(int))
#else 
#define ALLSTREAMS_SIZE    (64*64)
#define INTS_PER_STREAM    8
#endif

#define SMI_DEFAULT_STREAMBUF_SIZE 64
#define SMI_DEFAULT_STREAMBUF_NBR 8

#define SMI_DEFAULT_ADAPTER 0     /* the one and only adapter which is always
                                     present if using SCI-devices at all */  
#define MAX_ADAPTER       3       /* how many PCI-SCI adapters are supported ? */
#define MAX_DIMS          3       /* max. number of phys. SCI dimensions */
#define MAX_NODES_PER_DIM 15      /* max. number of nodes per dimension. */
#ifdef DOLPHIN_SISCI
#define MIN_SCI_ID        4
#define SCI_ID_STRIDE     4
#else /* Scali */
#define MIN_SCI_ID        0x0100
#define SCI_ID_STRIDE     (1 << 8)
#endif

/* retry and delay values for SCI segment operations */
#define SCI_CONNECT_TRIES 1000
#define SCI_CONNECT_DELAY 100  /* in us */

#define SCI_REMOVE_TRIES 100
#define SCI_REMOVE_DELAY 10 /* in us */

#define SCI_PROBE_TRIES 10
#define SCI_PROBE_DELAY 2    /* in seconds */

#define SCI_STARTUP_TIMEOUT 20 /* in seconds */

#define SCI_NOT_OPERATIONAL_DELAY 10 /* in seconds */

/* align segment size to internal SCI page size - only necessary for NT as it has 
   different system- and SCI-related "page sizes" (allocation granularity is more
   exact) */
#ifdef WIN32
#define SEGSIZE_ALIGNMENT (64*1024)
#else
#define SEGSIZE_ALIGNMENT 0
#endif

/* disable multiadapter support */
#define WITHOUT_MULTIADAPTER 0

/******************************************************************************/
/*** locks for multi-threaded usage                                         ***/
/******************************************************************************/
#ifndef DISABLE_THREADS
#define SMI_THREAD_T pthread_t
#define SMI_THREADKEY_T pthread_key_t
#define SMI_LOCK_T pthread_mutex_t
#define SMI_INIT_LOCK(mtx) pthread_mutex_init(mtx, NULL);
#define SMI_LOCK(mtx) pthread_mutex_lock(mtx)
#define SMI_UNLOCK(mtx) pthread_mutex_unlock(mtx)
#define SMI_TRYLOCK(mtx) pthread_mutex_trylock(mtx)
#define SMI_DESTROY_LOCK(mtx) pthread_mutex_destroy (mtx);
#else
#define SMI_THREAD_T int
#define SMI_THREADKEY_T int
#define SMI_LOCK_T int
#define SMI_INIT_LOCK(mtx) 
#define SMI_LOCK(mtx) 
#define SMI_UNLOCK(mtx) 
#define SMI_TRYLOCK(mtx) 1
#define SMI_DESTROY_LOCK(mtx) 
#endif

extern SMI_LOCK_T _smi_region_lock;

/********************************************************************************/
/*** Different SVM-Consistency policies                                       ***/
/********************************************************************************/
#define SVM_SRSW 1024
#define SVM_MRSW 2048

#define SVM_ALGO SVM_MRSW


/******************************************************************************/
/*** Constants for IO Redirection                                           ***/
/******************************************************************************/
#define SMI_IO_ASIS     0
#define SMI_IO_FILE     1


/*****************************************************************************/
/*** Describe the Mutex-Algorithm                                          ***/
/*****************************************************************************/
#define L_MUTEX     0    /* A Mutex-Algorithm from Leslie Lamport            */
#define BL_MUTEX    1    /* A Mutex-Algorithm from Burns & Lynch             */
#define SCH_MUTEX   2    /* Martin Schulz' implementation with sisci use     */

#define SMI_Mutex_init(id) SMI_MUTEX_INIT((id), BL_MUTEX, -1)
#define SMI_Mutex_init_with_locality(id,prank) SMI_MUTEX_INIT((id), BL_MUTEX, (prank))

/*****************************************************************************/
/*** Describe the Barrier-Algorithm                                        ***/
/*****************************************************************************/
#define PROGRESS_COUNTER_BARRIER 0 
#define SMI_Barrier() SMI_BARRIER(0)

/*****************************************************************************/
/*** adaptions to different SISCI versions                                 ***/
/*****************************************************************************/
#ifdef SCISTOREBARRIER_TWOARGS
#define SMI_SCIStoreBarrier(seq, flags, err) SCIStoreBarrier(seq, flags)
#else
#define SMI_SCIStoreBarrier(seq, flags, err) SCIStoreBarrier(seq, flags, err)
#endif


/* is a given region ID a valid ID ? */
#define IS_VALID_ID(ID) ( \
  (((ID) >= 0) && ( (ID) < _smi_mis.no_regions)) ? \
  ((_smi_mis.region[(ID)]->id) == (ID)) : 0 \
    )

/*****************************************************************************/
/*** return-code/error-check macros                                        ***/
/*****************************************************************************/

#define TEST_R(condition,message,return_code) {\
/* DNOTICES("testing if",message); */\
  if(!(condition)){\
    DWARNING(message);\
    DNOTICEI(" Test failed, return_code:",return_code);\
    DSECTLEAVE\
      return(return_code);\
  }\
}

#define ASSERT_A(condition,message,return_code) {\
/* DNOTICES("testing if",message); */ \
  if(!(condition)){\
    DERROR(message);\
    DNOTICEI(" Assertion failed, return_code:",return_code);\
    SMI_Abort(return_code);\
  }\
}
#define ASSERT_X(condition,message,return_code)\
ASSERT_A(condition,message,return_code)

#define ASSERT_R(condition,message,return_code) {\
  if(!(condition)){\
    DPROBLEM(message);\
    DNOTICEI(" Assertion failed, return_code:",return_code);\
    DSECTLEAVE; return(return_code);\
  }\
}

#define ASSERT_R_UNLOCK(condition,message,return_code,lock) {\
  if(!(condition)){\
    DPROBLEM(message);\
    DNOTICEI(" Assertion failed, return_code:",return_code);\
    SMI_UNLOCK(&lock);\
    DSECTLEAVE;\
    return(return_code);\
  }\
}

/* error handling macros for SISCI calls */
#define ABORT_IF_FAIL(message,sci_error,return_code) {\
  if(sci_error != SCI_ERR_OK) {\
    DERROR(message);\
    DNOTICEP("   SISCI errorcode:",sci_error);\
    SMI_Abort(return_code);\
  }\
}
#define EXIT_IF_FAIL(message,sci_error,return_code) \
ABORT_IF_FAIL(message,sci_error,return_code)

#define RETURN_IF_FAIL(message,sci_error,return_code) {\
  if(sci_error != SCI_ERR_OK) {\
    DPROBLEM(message);\
    DNOTICEP("   SISCI errorcode:",sci_error);\
    DSECTLEAVE return(return_code);\
  }\
}

#define WARN_IF_FAIL(message,sci_error) {\
  if(sci_error != SCI_ERR_OK) {\
    DWARNING(message);\
    DNOTICEP("SISCI errorcode:",sci_error); \
  }\
}

/* checked memory allocation */
#define ALLOCATE(ptr,type,size) \
    if (((ptr) = (type) malloc (size)) == NULL) { \
	DERROR("out of local memory");\
    	return (SMI_ERR_NOMEM); \
    } 

#define ZALLOCATE(ptr,type,size) \
    if (((ptr) = (type) malloc (size)) == NULL) { \
	DERROR("out of local memory");\
    	return (SMI_ERR_NOMEM); \
    } \
    memset (ptr, 0, size);

#define AALLOCATE(ptr,type,size) \
    if (((ptr) = (type) malloc (size)) == NULL) { \
	DERROR("out of local memory");\
    	SMI_Abort(99); \
    } 

/*****************************************************************************/
/*** typedefs                                                              ***/
/*****************************************************************************/

/* define 64 bit ints and others */
#if (defined LINUX) || (defined DARWIN)
typedef long long int longlong_t;
#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif
#elif defined(_WIN32)
#include <wtypes.h>
typedef LONGLONG longlong_t;
#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807
#endif
typedef unsigned long ulong;
#endif

/* this needs to be changed depending on the SISCI API revision */
#ifndef HAVE_SCIINITIALIZE
typedef struct sci_query_adapter sci_query_adapter_t;
typedef struct sci_query_string sci_query_string_t;
typedef struct sci_query_system sci_query_system_t;
#endif 

typedef int device_t;

#ifndef NO_SISCI
typedef struct seglist_t_ {
  sci_remote_segment_t segment;
  sci_desc_t fd;
} seglist_t;
#endif 

typedef enum  smi_startup_method_t_ {
  smi_startup_use_file,
  smi_startup_use_tcp,
  smi_startup_none
} smi_startup_method_t;

typedef struct smi_args_t_ {
  int iSCIId;
  int iNumProcs;
  int iProcRank;
  int iMagicNumber;
  int iPortNumber;
  char szPIDfile[MAX_FILENAME_LEN+1];
  char szSyncHost[MAX_HOSTNAME_LEN+1];
  smi_startup_method_t eStartupMethod;
  char szExecName[MAX_FILENAME_LEN+1];
} smi_args_t;

/* structure, which contains all information about a single shared memory     */
/* segment, which constitutes to an entire shared memory region.              */
typedef struct 
{
  int      region_id;                     /* id of the region it belongs to  */
  char*    address;                       /* start address                   */
  size_t   size;                          /* size (in byte)                  */
  int      machine;                       /* machine rank, where it is       */
                                          /* physically located              */
  int      owner;                         /* rank of the process, that       */
                                          /* allocated it                    */
  int      partner;                       /* partner process for pt2pt       */

  unsigned int flags;                     /* properties of the segment       */

  device_t device;                        /* device, used to generate it     */
  unsigned int id;                        /* identifier or file descriptor   */
                                          /* to reference is                 */
  int handle;                             /* handle for POSIX shared memory  */
  size_t offset;                          /* for the SCI-mapping procedure  */
  unsigned int connect_flag;
 
  int adapter;                     /* local adapter used for this segment */
  int sci_id;                      /* SCI id of adapter which exports this segment */

#ifndef NO_SISCI  
  sci_desc_t fd;
  smi_sci_desc_t smifd;
  
  sci_map_t map;
  sci_local_segment_t localseg;
  sci_remote_segment_t segment;
  sci_sequence_t* node_sequence;  
#endif 
} shseg_t;
    
/* Structure, which contains all information about a single shared memory     */
/* region. If it's size is 0, this means, that this structure does temporary  */
/* not correspond to an existing shared memory region                         */
typedef struct {
    int       id;                           /* identifier                      */
    char**    addresses;                    /* start addresses (one address for all but FRAGMENTED regions) */
    int       type;                         /* type of the region */
    boolean   collective;                   /* flag: collecvtive or not? (related to 'type') */
    size_t    size;                         /* total size (in byte) */
    int       no_segments;                  /* number of shared segments, which state the entire shared   
                                               memory region                   */
    int       nbr_rmt_cncts;                /* For a local region: how many remote procs are connected? */

    pthread_t cb_thread;                    /* Thread and function ptr for callback events */
    smi_region_callback_t cb_fcn;

    int       counterpart_id;               /* id of another region, used for switching purposes between      
                                               shared and replicated  '-1' means, there is none       */
    boolean   replication;                  /* states, whether this region is the local replication of a  
                                               shared one or not.              */
    shseg_t** seg;                          /* its constituting individual segments */
} region_t;


/* root structure, which contains all information about the SMI's data layout */
typedef struct {
    region_t** region;                /* all existing shared memory regions */
    int        no_regions;            /* nbr of existing regions, including internal regions  */
    int        nbr_user_regions;      /* nbr of regions created by the user */
} mis_t;


typedef struct {
    int sci_id[MAX_ADAPTER];
} adpt_rank_t;

extern mis_t _smi_mis;
extern SMI_LOCK_T _smi_mis_lock;



/******************************************************************************/
/*** global variables                                                         */
/******************************************************************************/

extern smi_args_t _smi_default_args;

extern int _smi_verbose;
extern int _smi_use_watchdog;

/* topology information */
extern int  _smi_page_size;       /* smallest common multiple of the page sizes of */
                                  /* all participating machine                     */
extern int  _smi_nbr_procs;       /* total number of processes                     */
extern int  _smi_my_proc_rank;    /* local process rank, accorting to an ordering  */
                                  /* in which processes, residing on the same      */
                                  /* machine get consecutive ranks.                */
extern int  _smi_nbr_machines;    /* total number of machines                      */
extern int* _smi_machine_rank;    /* ranks of the machines of all processes,       */
                                  /* indexed according their 'my_proc_rank'        */
extern int  _smi_my_machine_rank; /* machine rank of the local machine             */
extern boolean _smi_all_on_one;    /* this flag determines whether all processes    */
                                   /* reside on a single machine or not. If so,     */
                                   /* shared regions can be build solely out of     */
                                   /* Unix shared segments without SCI.             */

/* SMI internals */
extern int  _smi_mpi_rank;         /* rank of the process within MPI_COMM_WORLD (historical) */

extern boolean _smi_initialized;     /* SMI environment already initialized? */
extern boolean _smi_SISCI_MAP_FIXED; /*  SISCI supports the MAP_FIXED flag? */
extern int *_smi_int_shreg_id;       /* identifiers of the shared regions that are    */
extern int  _smi_int_smp_shreg_id;    /* used for internal purposes. */
extern int _smi_ll_sgmt_ok;
				  
/* machine specific */
extern int  _smi_Cachelinesize;
extern int  _smi_1stLevel_Cachesize;
extern int  _smi_2ndLevel_Cachesize; 

extern int *_smi_pids;

/* SCI specific */
extern int  _smi_DefAdapterNbr;    /* Number of the local adapter to be used by default
				      (the fist that can be found and is usable). */
extern int  _smi_adpt_policy;      /* The global adapter utilization policy which is applied if 
				      a region is specified with SMI_ADPT_DEFAULT. */
extern int  _smi_adpt_export;
extern int  _smi_adpt_import;
extern int  _smi_StreambufSize;    /* number of byte that the stream buffers are sized */
extern int  _smi_NbrStreambufs;    /* number of active stream buffers */
extern int  _smi_AdapterType;      /* SISCI-coded adapter type */
extern char _smi_AdapterTypeS[64]; /* SISCI-coded adapter type in printable form */
extern int  *_smi_sci_rank;        /* mapping of process ranks to node SCI ids (adapter 0) */
extern int  *_smi_nbr_adapters;    /* nbr of adapters each process can access */
extern adpt_rank_t *_smi_adpt_sci_id;     /* SCI ranks of each adapter a process can access */
/* transfer checking */
#ifndef NO_SISCI
extern sci_sequence_t *_smi_node_sequence;   /* one sequence towards every node */
#endif 
extern boolean *_smi_error_seq_initialized;

smi_error_t SMI_Debug_time(int);
smi_error_t SMI_Debug(int);

#ifdef __cplusplus
}
#endif
#include "internal_functions.h"
#endif


