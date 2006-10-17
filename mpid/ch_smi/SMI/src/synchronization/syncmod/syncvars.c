/*--------------------------------------------------------------------------*/
/* Global structurs */

/*.......................................................................*/
/* constants and macros */

#define ATOMIC_MANAGER		0
#define ATOMIC_MAINLOCK		1

#define ATOMIC_SCOPE            ATOMIC_MAINLOCK + 2

#ifdef SIMPLESYNC_NEWLOCK
#define ATOMIC_BARRIER		ATOMIC_SCOPE+2+MAX_CLUSTER_NODES
#else
#define ATOMIC_BARRIER		ATOMIC_SCOPE+2
#endif

#ifdef SIMPLESYNC_NEWBARRIER
#define ATOMIC_USERSTART	ATOMIC_BARRIER+MAX_CLUSTER_NODES
#else
#define ATOMIC_USERSTART	ATOMIC_BARRIER+1
#endif

#define SIZE_ATOMICMEM		128*1024

#ifdef SIMPLESYNC_NEWBARRIER
#define SIZE_SYNCMEM		SIZE_ATOMICMEM
#endif

#define ATOMIC_MASTER		0

#ifndef OLD_SCIAL
#define SIMPLESYNC_ATOMIC_SEGID 	0x4011
#define SIMPLESYNC_SYNCMEM_SEGID 	0x4023
#endif


/*.......................................................................*/
/* atomic memory management */

static uint		*atomStart			= NULL;
static uint		*atomStartDirect	= NULL;
static uint		atomLocal[SIZE_ATOMICMEM / sizeof(uint)];

#ifdef SIMPLESYNC_NEWBARRIER
static uint_p   syncMem[MAX_CLUSTER_NODES];

#ifdef WIN32
static HANDLE	localThreadBarrier[MAX_THREADS_PER_NODE];
#endif
#endif

#ifdef WIN32
static CRITICAL_SECTION localBarrier[SIZE_ATOMICMEM / sizeof(uint)];
#endif
#ifdef LINUX
static pthread_mutex_t localBarrier[SIZE_ATOMICMEM / sizeof(uint)];
static pthread_cond_t localCond[SIZE_ATOMICMEM / sizeof(uint)];
#endif

static atomic_t	atomEnd		 		= 0;
static BOOL		initialAtomSetup	= FALSE;

#ifndef OLD_SCIAL
static void* simplesync_atomic_segInfoPtr;
static void* simplesync_direct_segInfoPtr;
static void* simplesync_syncmem_segInfoPtr;
#endif


/*.......................................................................*/
/* scope management */

atomic_t consMod_globalCounterPointer;
atomic_t consMod_lastGlobalReleasePointer;
uint     consMod_lastCacheFlushLocal   = 0;
uint     consMod_lastLocalRelease      = 0;


/*.......................................................................*/
/* misc admin stuff */

static nodeNum_t	locNodeNum, locNodeCount;
static uint		locNodeID;

static BOOL simpleSync_started = FALSE;


/*.......................................................................*/
/* statistics for syncMod */

static syncMod_stat_t syncMod_globalStat;


