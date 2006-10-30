/* $Id$ */

/* 
   This file contains the routines that initialize and finalize the device, 
   and provide the basic information on it
*/
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>

#ifdef WIN32
#define strcasecmp stricmp
#else
#include <strings.h>
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "mpid.h"
#include "mpid_debug.h"
#include "smidev.h"
#include "smimem.h"
#include "flow.h"
#include "smidef.h"
#include "smistat.h"
#include "smicoll.h"
#include "mmu.h"
#include "ssidesetup.h"
#include "smiregionmngmt.h"
#include "mutex.h"

/* Forward refs */
int MPID_SMI_End (MPID_Device *);
int MPID_SMI_Barrier(struct MPIR_COMMUNICATOR *);
void MPID_SMI_Eval_cmdline(int *, char ***);
void MPID_SMI_Read_settings(void);
void MPID_SMI_resetGetOpt(void);
void MPID_SMI_Print_config(void);
static void MPID_SMI_GetTopology(void);
void MPID_SMI_Get_version(char *version);
void MPID_SMI_watchdog_cb (void);
int MPID_SMI_long_len(int);
int MPID_SMI_vlong_len(int);
/*
 * exports 
 */

/* Topology information, all related to global (device) ranks. Each communicator holds 
   similar information for the included processes. */
/* global/device rank and size  */
int  MPID_SMI_myid = -1;
int  MPID_SMI_numids = 0;
/* proc/node mapping and node information */
int *MPID_SMI_procNode;		       /* mapping process_rank -> node_rank */
int *MPID_SMI_numProcsOnNode;	   /* nbr of processes on this node */
int  MPID_SMI_numNodes;		       /* number of nodes on which processes are running */
int  MPID_SMI_myNode;		       /* MPID_SMI_procNode[MPID_SMI_myid] */
int *MPID_SMI_localRankForProc;    /* rank of process on its node */
/* local/remote info for clustered SMPs */
int  MPID_SMI_use_SMP;             /* is any node running more than 1 process ? */
int *MPID_SMI_use_localseg;        /* use LOCAL shared memory to communicate with this proc? */
int *MPID_SMI_is_remote;           /* is this process (grank) located on a remote node? */
/* SCI topology information */
int  MPID_SMI_sci_type;
int  MPID_SMI_sci_dims;            /* phys. dimensions of SCI ring/torus topology */
int  MPID_SMI_sci_dim_extnt[MAX_SCI_DIMS];  /* extent of each of these dimensions. */
int  MPID_SMI_proc_dims;           /* nbr. of dimensions on which process of this app. are running */
int  MPID_SMI_active_dims;         /* logical/active dimensions (dimension with extent == 1 are not active) */
int  MPID_SMI_gnodes_in_dim[MAX_SCI_DIMS]; /* nodes in dimension '0' till dimension 'MPID_SMI_sci_dims - 1' */
int *MPID_SMI_dstnc_to_grank;      /* distance in nodes to other ranks */
int *MPID_SMI_dimdstnc_to_grank[MAX_SCI_DIMS]; /* same, but indexed by dimensions */
int *MPID_SMI_grank_to_sciid;      /* map global process rank to sciid of node it is running on */

int  MPID_SMI_Do_async_devcheck;   /* Flag: true = wait for another signal */
boolean MPID_SMI_is_initialized = false;

/*
 * imports 
 */
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

/* this variable is declared in smieager.c */
extern int MPID_SMI_Eager_DMA_working;


/*
 * internal globals 
 */

/* name of the config file for dynamic device configuration */
static char *devconf_name;

static int MPID_SMI_long_len_value;
static int MPID_SMI_vlong_len_value;

#ifdef MPID_USE_DEVTHREADS
#include <pthread.h>
/* thread-id of worker thread */
static pthread_t tid_signal_thread; 
static pthread_attr_t attr_scope_system;  
#endif

/*****************************************************************************
  Here begin the interface routines themselves
*****************************************************************************/

#define INIT_PRINT_MSG(msg) fprintf (stdout, msg); fprintf (stdout, "\n"); fflush (stdout);

/* 
   In addition, Chameleon processes many command-line arguments 
   
   This should return a structure that contains any relavent context
   (for use in the multiprotocol version)
   
   Returns a device.  
   This sets up a message-passing device (short/eager/rendezvous protocols)
*/

MPID_Device *MPID_CH_SMI_InitMsgPass(int *argc, char ***argv, int short_len, int long_len)
{
    MPID_Device *dev;
    int ret;

#if defined(MPIR_MEMDEBUG)
	MPID_trlevel(0x03);
#endif
    
	/* we want unbuffered terminal I/O */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    /* evaluate command line arguments */
    MPID_SMI_Eval_cmdline(argc, argv);
    
    /* initialize the SMI library and query the vital parameters */
    SMIcall (SMI_Init (argc, argv) );
    MPID_SMI_DEBUG_PRINT_MSG("SMI library initialized");

    MPID_SMI_GetTopology();
    MPID_SMI_Read_settings();
    MPID_SMI_Set_statistics(SMI_STAT_START);
	MPID_SMI_Lowlevel_Init();

	MPID_STAT_CALL(device_init);

    MPID_SMI_Rsrc_sched_init (MPID_SMI_cfg.RESOURCE_SCHED);
    MPID_SMI_DEBUG_PRINT_MSG("low-level initialization completed");

    if (!MPID_SMI_MMU_init())
		return NULL;
    MPID_SMI_DEBUG_PRINT_MSG("MMU initialization completed");
    if (MPID_SMI_cfg.SSIDED_ENABLED && !MPID_SMI_Mutex_module_init()) 
		return NULL;
    MPID_SMI_DEBUG_PRINT_MSG("Mutex module initialization completed");

    ZALLOCATE (dev, MPID_Device *, sizeof(MPID_Device));

    /* initialize all protocols */
	switch (MPID_SMI_cfg.CONNECT_ON_INIT) {
	case 0: MPID_INFO("Setting up protocols (no preconnect):\n");
		break;
	case 1: MPID_INFO("Setting up protocols (dynamic preconnect):\n");
		break;
	case 2: MPID_INFO("Setting up protocols (static preconnect):\n");
		break;
	}
    dev->short_msg    = MPID_SMI_Short_setup();
    dev->vlong_msg    = MPID_SMI_Rndv_setup();
    dev->long_msg     = MPID_SMI_Eagern_setup();

	/* disabled, because the blocking rendezvous protocol doesn't support "direct ff" yet, which is used by default */
	/*	dev->ready        = MPID_SMI_Brndv_setup(); */
	dev->ready        = 0;

    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->sside	      = MPID_SMI_cfg.SSIDED_ENABLED ? MPID_SMI_Sside_setup () : NULL;
    dev->lrank        = MPID_SMI_myid;
    dev->lsize        = MPID_SMI_numids;

	MPID_SMI_Pipe_setup();

    /* Set switching points between the protocols('+ 1' because in adi2send.c, 
       it is tested for 'len < ' and not for 'len <='. I don't want to change adi2send.c, however). */
    if (short_len < 0) 
		short_len = MPID_SMI_cfg.MAX_SHORT_PAYLOAD + 1;
    /* This size is the "peak" size for eager msgs: depending on the buffer setup of 
	   each process, the switch from eager to rndv may already take place for smaller msg sizes. */
    if (long_len < 0)  
		long_len  = MPID_SMI_EAGERSIZE + 1;
	MPID_SMI_long_len_value = short_len;
    dev->long_len     = &MPID_SMI_long_len;
	MPID_SMI_vlong_len_value = long_len;
    dev->vlong_len    = &MPID_SMI_vlong_len;

    /* additional, non-protocol specific device functions */
    dev->check_device = MPID_SMI_Check_incoming;
    dev->terminate    = MPID_SMI_End;
    dev->cancel       = MPID_SMI_SendCancelPacket;
    dev->abort	      = MPID_SMI_Abort;
    dev->get_version  = MPID_SMI_Get_version;
    dev->wtime        = SMI_Wtime;
    dev->collops_init = MPID_SMI_Collops_init;
    dev->alloc_mem    = MPID_SMI_Alloc_mem;
    dev->free_mem     = MPID_SMI_Free_mem;
    dev->comm_init    = MPID_SMI_Comm_init;
    dev->comm_free    = MPID_SMI_Comm_free;    
    dev->persistent_init = MPID_SMI_Persistent_init;
    dev->persistent_free = MPID_SMI_Persistent_free;
    dev->next	      = 0;

    /* if the eager protocol is not available, use rndv for all non-short msgs */
    if (dev->long_msg == NULL)
		MPID_SMI_vlong_len_value = MPID_SMI_long_len_value;

    /* copy the nc_enable def. to the device config, so also calling routines know if
       we can copy directly or not */
    dev->nc_enable = MPID_SMI_cfg.NC_ENABLE;
    
    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
    MPID_DEBUG_CODE(if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;);

#ifdef MPID_USE_DEVTHREADS

	pthread_attr_init(&attr_scope_system);
    pthread_attr_setscope(&attr_scope_system, PTHREAD_SCOPE_SYSTEM);

    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		/* Start signal thread for asynchronous transfer */
		MPID_SMI_ASYNC_LOCK(&MPID_SMI_async_check_lck);
		MPID_SMI_Do_async_devcheck = 0;
		
		/* XXX create and use macros for WIN32 and POSIX */
		pthread_key_create (&MPID_SMI_thread_type, NULL);
		pthread_create (&tid_signal_thread, &attr_scope_system, MPID_SMI_Async_devcheck, (void *)NULL);  
		
		/* synchronize with the thread */
		while (!MPID_SMI_Do_async_devcheck) 
			MPID_SMI_YIELD;
		
		MPID_SMI_DEBUG_PRINT_MSG("device-checking thread created");
    }
#endif
	/* This call may fail if SMI has watchdog disabled. */
    SMI_Watchdog_callback(MPID_SMI_watchdog_cb);

    /* print actual configuration */
    if ( (MPID_SMI_myid == 0) && MPID_SMI_cfg.VERBOSE ) {
		MPID_SMI_Print_config();
    }

    /* we need a barrier to ensure that all processes are ready to communicate -
       and to synchronize the execution time as good as possible */
    MPID_SMI_DEBUG_PRINT_MSG("waiting for other processes");
    MPID_SMI_lbarrier();

	/* Complete the initialization of the eager protocol using the information from
	   the remote processes. */
	MPID_SMI_Eagern_init_complete();
    
    if (MPID_SMI_cfg.CONNECT_ON_INIT) {
		/* Pre-connect all segments if fast startup if not selected (only possible 
		   for eager segments). */
		MPID_SMI_Eagern_preconnect();
		MPID_SMI_Rndv_preconnect();
		MPID_SMI_DEBUG_PRINT_MSG("all message segments are connected");
    }	

    MPID_SMI_is_initialized = true;
	MPID_STAT_RETURN(device_init);

    MPID_SMI_DEBUG_PRINT_MSG("leaving MPID_CH_SMI_InitMsgPass");
    return dev;
}

static void MPID_SMI_GetTopology(void)
{
	char node_name[MPID_SMI_STRINGLEN];
    int i, j;
    int *procRankOnNodeTmp;
	int my_sci_id, ids_are_valid;
	int *target_sci_id, highest_sci_id;
	int this_node, highest_node, lowest_node;

    /* basic SCI & system parameters */
    SMIcall (SMI_Query (SMI_Q_SCI_PACKETSIZE, 0, &MPID_SMI_SCI_TA_SIZE));
    SMIcall (SMI_Query (SMI_Q_SCI_STREAMBUFSIZE, 0, &MPID_SMI_STREAMSIZE));
    SMIcall (SMI_Query (SMI_Q_SYS_PAGESIZE, 0, &MPID_SYS_PAGESIZE ));
    SMIcall (SMI_Page_size(&MPID_SMI_PAGESIZE));
    MPID_SMI_cfg.SEGMODE = SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED;

    /* gather topology information */
    SMIcall (SMI_Proc_rank(&MPID_SMI_myid));
    SMIcall (SMI_Proc_size(&MPID_SMI_numids));
    SMIcall (SMI_Node_rank(&MPID_SMI_myNode));	
    SMIcall (SMI_Node_size(&MPID_SMI_numNodes));

    /* mapping node_rank -> procs_on_node */
    ALLOCATE(MPID_SMI_numProcsOnNode, int *, MPID_SMI_numNodes*sizeof(int));
    i = 0; /* node rank */
    j = 0; /* procs processed */
    while ( i < MPID_SMI_numNodes - 1) {
		SMIcall( SMI_First_proc_on_node ( i+1, &MPID_SMI_numProcsOnNode[i] ));
	
		MPID_SMI_numProcsOnNode[i] -= j;
		j += MPID_SMI_numProcsOnNode[i];
		i++;
    }
    MPID_SMI_numProcsOnNode[i] = MPID_SMI_numids  - j;

    /* is any participating node running more than one process ? This
       will activate "SMP mode" for all processes */
    MPID_SMI_use_SMP = false;
    for (i = 0; i < MPID_SMI_numNodes; i++) {
		if (MPID_SMI_numProcsOnNode[i] > 1)
			MPID_SMI_use_SMP = true;
    }
	if (MPID_SMI_numNodes == 1)
		MPID_SMI_use_SMP = false;
    
    /* mapping proc_rank -> node_rank */
    ALLOCATE (MPID_SMI_procNode, int *, MPID_SMI_numids*sizeof(int));
    for (i = 0; i < MPID_SMI_numids; i++) {
		SMIcall (SMI_Proc_to_node(i, &MPID_SMI_procNode[i]));
    }

    /* calculate local rank of processes on their nodes */
    ZALLOCATE (MPID_SMI_localRankForProc, int *, MPID_SMI_numids * sizeof(int) );
    ZALLOCATE (procRankOnNodeTmp, int *, MPID_SMI_numNodes* sizeof(int));
    for( i = 0; i < MPID_SMI_numids; i++) {
		MPID_SMI_localRankForProc[i] = procRankOnNodeTmp[MPID_SMI_procNode[i]];
		(procRankOnNodeTmp[MPID_SMI_procNode[i]])++;    
    }
    FREE( procRankOnNodeTmp );

    /* Determine ranks to which this process will communicate via SMI-regions of type LOCAL 
       Note: if all procs are running on one node, LOCAL regions will *not* be used! */
    ALLOCATE (MPID_SMI_use_localseg, int *, MPID_SMI_numids * sizeof(int) );
    for( i = 0; i < MPID_SMI_numids; i++) {
		if (MPID_SMI_use_SMP && (MPID_SMI_procNode[i] == MPID_SMI_myNode))
			MPID_SMI_use_localseg[i] = true;
		else
			MPID_SMI_use_localseg[i] = false;
    }

    /* Determine ranks of processes which are remote to this process */ 
    ALLOCATE (MPID_SMI_is_remote, int *, MPID_SMI_numids * sizeof(int) );
    for( i = 0; i < MPID_SMI_numids; i++) {
		MPID_SMI_is_remote[i] = (MPID_SMI_procNode[i] != MPID_SMI_myNode);
    }
    
    /* how many PCI-SCI adapters do we have ? */
    SMIcall (SMI_Query(SMI_Q_SCI_NBRADAPTERS, MPID_SMI_myid, (void *)&MPID_SMI_NBRADPTS));

	/* warn the user if he is running more processes than CPUs on this node */
	if (MPID_SMI_cfg.VERBOSE) {
		size_t node_name_len = MPID_SMI_STRINGLEN;
		SMIcall (SMI_Get_node_name(node_name, &node_name_len));
		j = (int) node_name_len;
		SMIcall (SMI_Query(SMI_Q_SYS_NBRCPUS, 0, &j));
		if (j < MPID_SMI_numProcsOnNode[MPID_SMI_myNode] && 
			MPID_SMI_localRankForProc[MPID_SMI_myid] == 0)
			fprintf (stdout, "*** SCI-MPICH warning, node %s: running %d processes with only %d CPUs.\n",
					 node_name, MPID_SMI_numProcsOnNode[MPID_SMI_myNode], j);
	}

	/* 
	 * SCI topology information (mostly for collective operations)
	 */
	SMIcall (SMI_Query(SMI_Q_SCI_NBR_PHYS_DIMS, 0, &MPID_SMI_sci_dims));
	for (i = 0; i < MPID_SMI_sci_dims; i++) {
		SMIcall (SMI_Query(SMI_Q_SCI_PHYS_DIM_EXTENT, i, &MPID_SMI_sci_dim_extnt[i]));
	}
	SMIcall (SMI_Query(SMI_Q_SCI_SYSTEM_TYPE, 0, &MPID_SMI_sci_type));
	/* Scali has no SCI-id-dependent topology rules anylonger;  for now, use 
	   dummy values instead (for all dimensions) */
	if (MPID_SMI_sci_type == SMI_SCI_SCALI) {
#define MAX_DIM_SCALI    8

		MPID_SMI_proc_dims = MPID_SMI_sci_dims;
		MPID_SMI_gnodes_in_dim[2] = MPID_SMI_numids > MAX_DIM_SCALI*MAX_DIM_SCALI ? 
			(MPID_SMI_numids / MAX_DIM_SCALI*MAX_DIM_SCALI) : 1;
		MPID_SMI_gnodes_in_dim[1] = MPID_SMI_numids > MAX_DIM_SCALI ? 
			(MPID_SMI_numids / MAX_DIM_SCALI) : 1;
		MPID_SMI_gnodes_in_dim[0] = MPID_SMI_numids > MAX_DIM_SCALI ? 
			MAX_DIM_SCALI : MPID_SMI_numids;

		for (i = 0, MPID_SMI_active_dims = 0; i < MPID_SMI_proc_dims; i++) 
			if (MPID_SMI_gnodes_in_dim[i] > 1)
				MPID_SMI_active_dims++;

		ALLOCATE (MPID_SMI_grank_to_sciid, int *, MPID_SMI_numids * sizeof(int) );
		for (i = 0; i < MPID_SMI_numids; i++) 
			MPID_SMI_grank_to_sciid[i] = 4*(i + 1);

		ZALLOCATE (MPID_SMI_dstnc_to_grank, int *, MPID_SMI_numids * sizeof(int));
		for (j = 0; j < MPID_SMI_proc_dims; j++) {
			ZALLOCATE(MPID_SMI_dimdstnc_to_grank[j], int *, MPID_SMI_numids * sizeof(int));
		}
		for (i = 0; i < MPID_SMI_numids; i++) {
			for (j = 0; j < MPID_SMI_proc_dims; j++) {
				switch (j) {
					/* XXX calculations for Y and Z are still missing */
				case X_DIM:
					MPID_SMI_dimdstnc_to_grank[j][i] = (MPID_SMI_myid % MAX_DIM_SCALI) - (i % MAX_DIM_SCALI);
					if (MPID_SMI_dimdstnc_to_grank[j][i] < 0)
						MPID_SMI_dimdstnc_to_grank[j][i] = 0 - MPID_SMI_dimdstnc_to_grank[j][i];
					break;
				case Y_DIM:
					MPID_SMI_dimdstnc_to_grank[j][i] = 1;
					break;
				case Z_DIM:
					MPID_SMI_dimdstnc_to_grank[j][i] = 1;
					break;
				}
				MPID_SMI_dstnc_to_grank[i] += MPID_SMI_dimdstnc_to_grank[j][i];
			}
		}
		
		return;
	} 
		
	/* Get dimensions of SCI network in which our processes operate */
	SMIcall (SMI_Query(SMI_Q_SMI_NBR_APP_DIMS, 0, &MPID_SMI_proc_dims));
	for (i = 0; i < MPID_SMI_proc_dims; i++) {
		SMIcall (SMI_Query(SMI_Q_SMI_APP_DIM_EXTENT, i, &MPID_SMI_gnodes_in_dim[i]));
	}	
	/* Determine the number of acive SCI-dimensions. I.e., if all active nodes are located
	   in one ring (which is not located in the lowest dimension), the determined
	   number of SCI dimensions will be different from this one. */
	MPID_SMI_active_dims = 0;
	for (i = 0; i < MPID_SMI_proc_dims; i++) {
		if (MPID_SMI_gnodes_in_dim[i] > 1)
			MPID_SMI_active_dims++;
	}
	
	/* If the SCI'ids of the nodes do not match the topology, we use faked
	   id's instead for topology purposes. Drawback of this are potential peformance
	   bottlenecks due to non-optimal traffic routing in collective operations. */
    ALLOCATE (MPID_SMI_grank_to_sciid, int *, MPID_SMI_numids * sizeof(int) );
	SMIcall (SMI_Query (SMI_Q_SCI_SCIIDS_VALID, 0, &ids_are_valid));
	for (i = 0; i < MPID_SMI_numids; i++) {
		if (ids_are_valid) {
			SMIcall (SMI_Query(SMI_Q_SCI_PROC_ID, i, &MPID_SMI_grank_to_sciid[i]));
		} else {
			MPID_SMI_grank_to_sciid[i] = MIN_SCI_ID + i%MPID_SMI_sci_dim_extnt[X_DIM]*SCI_ID_STRIDE;
			if (MPID_SMI_sci_dim_extnt[Y_DIM] > 1)
				MPID_SMI_grank_to_sciid[i] +=  
					((i/MPID_SMI_sci_dim_extnt[X_DIM]) % MPID_SMI_sci_dim_extnt[Y_DIM]) * SCI_ID_YOFFSET;
			if (MPID_SMI_sci_dim_extnt[Z_DIM] > 1)
				MPID_SMI_grank_to_sciid[i] +=  
					i/(MPID_SMI_sci_dim_extnt[X_DIM]*MPID_SMI_sci_dim_extnt[Y_DIM]) * SCI_ID_ZOFFSET;
		}
	}

	/* Determine network path distance to each rank, dimension-independant
	   and indexed by dimension. */
    ZALLOCATE (MPID_SMI_dstnc_to_grank, int *, MPID_SMI_numids * sizeof(int));
	for (j = 0; j < MPID_SMI_proc_dims; j++) {
		ZALLOCATE(MPID_SMI_dimdstnc_to_grank[j], int *, MPID_SMI_numids * sizeof(int));
	}
	SMIcall (SMI_Query(SMI_Q_SCI_PROC_ID, MPID_SMI_myid, &my_sci_id));
    for (i = 0; i < MPID_SMI_numids; i++) {
		for (j = 0; j < MPID_SMI_proc_dims; j++) {
			MPID_SMI_dimdstnc_to_grank[j][i] = ((MPID_SMI_grank_to_sciid[i] 
											   + MPID_SMI_gnodes_in_dim[j]*( 4 << (4*j)) - my_sci_id)
										       / ( 4 << (4*j))) % MPID_SMI_gnodes_in_dim[j];
			MPID_SMI_dstnc_to_grank[i] += MPID_SMI_dimdstnc_to_grank[j][i];
		}
	}

	return;
}


/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
*/
int MPID_SMI_Abort( comm_ptr, code, msg )
	 struct MPIR_COMMUNICATOR *comm_ptr;
	 int      code;
	 char     *msg;
{
    if (msg) {
		fprintf( stderr, "  [%d] %s\n", MPID_SMI_myid, msg );
    }
    else {
		fprintf( stderr, "  [%d] Aborting program!\n", MPID_SMI_myid );
    }

#ifdef MPID_USE_DEVTHREADS
    /* Stop signal thread - if this thread keeps waiting for a signal,
       Linux seems to have problems. Set the flag to "quit", then signal the 
       my own thread and the thread of the next process. */
    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		MPID_SMI_Do_async_devcheck = 0;

		/* terminate the device threads (local and remote) */
		SMI_Signal_send (((MPID_SMI_myid + 1) % MPID_SMI_numids)|SMI_SIGNAL_ANY);
		pthread_kill (tid_signal_thread, SIGTERM);

		pthread_join (tid_signal_thread, NULL); 
		pthread_key_delete (MPID_SMI_thread_type);
    }
#endif
	/* free the one-sided allocated resources */
    MPID_SMI_Rsrc_sched_finalize();

    /* terminate SMI library to free resources */
    SMI_Abort(code);

    /* we will never get here */
    exit(code);    
    return 0;
}

/* do some cleanup in case of a watchdog-triggered shutdown */
void MPID_SMI_watchdog_cb (void)
{
#ifdef MPID_USE_DEVTHREADS
    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		/* Terminate the local device thread. This is a call with local completion 
		   semantics - should succeed even in this situation.  Signalling the remote
		   threads might not succeed - we do not know  what happened to the remote process */

		MPID_SMI_Do_async_devcheck = 0;

#if 0
		/* This is the friendly way to cancel the thread - but it does not work if the
		   thread is not returning to its main routine regularly. */
		SMI_Signal_send (MPID_SMI_myid|SMI_SIGNAL_ANY);
		pthread_join (tid_signal_thread, NULL); 
#else
		/* This is the hard way to terminate the thread. */
		pthread_cancel (tid_signal_thread);
#endif
    }
#endif    
    return;
}

int MPID_SMI_End( dev )
	 MPID_Device *dev;
{
    int i;
    
    MPID_SMI_DEBUG_PRINT_MSG("Entering MPID_SMI_End\n");

#if defined(MPIR_MEMDEBUG)
	fprintf(stderr, "[%d] *** allocated memory at start of ch_smi shutdown:\n", MPID_SMI_myid);
	MPID_trdump(0);
#endif

    /* Finish off any pending transactions */
    /* MPID_SMI_Complete_pending(); */
    /* XXX We should really generate an error or warning message if there 
       are uncompleted operations. */
	
	/* Wait for all other processes before starting to shut down and release
	   the SCI resources. */
	SMIcall (SMI_Barrier());

    /* do not go further if there are still working DMA threads */
    while(MPID_SMI_Eager_DMA_working);
    
#ifdef MPID_USE_DEVTHREADS
    /* Stop signal thread: set the flag to "quit", then signal the thread of 
       the next process to trigger a quit */
    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		/* Release the device thread, whereever it hangs. We may or may not 
		   own the lock, but we release it anyway. trylock() does not
		   help here (as it seems). */
		MPID_SMI_Do_async_devcheck = 0;
		pthread_mutex_unlock (&MPID_SMI_async_check_lck);
		
		SMIcall (SMI_Barrier());
		/* this call may fail */
		SMI_Signal_send (((MPID_SMI_myid + 1) % MPID_SMI_numids)|SMI_SIGNAL_ANY);
		SMIcall (SMI_Barrier());

		pthread_join (tid_signal_thread, NULL); 
		pthread_key_delete (MPID_SMI_thread_type);
    }
#endif
	/* XXX include destroy function into protocol (see below) */
	MPID_SMI_Sside_destroy (dev->sside);

    (dev->vlong_msg->delete)( dev->vlong_msg );
	/*     (dev->ready->delete)( dev->ready );	 */ /* disabled, see initialization */
	MPID_SMI_Pipe_delete();

    /* eager protocol might be disabled */
    if (dev->long_msg)
		(dev->long_msg->delete)( dev->long_msg );	
	/* Short shall be the last protocol to be deleted! */    
	(dev->short_msg->delete)( dev->short_msg );	

    MPID_SMI_Rsrc_sched_finalize();

    FREE( dev->grank_to_devlrank );
    FREE( dev );
    
#ifdef MPID_SMI_STATISTICS
    MPID_SMI_Runtime_statistics();
#endif

    FREE(MPID_SMI_localRankForProc);
    FREE(MPID_SMI_is_remote);
    FREE(MPID_SMI_use_localseg);
	FREE(MPID_SMI_grank_to_sciid);
	FREE(MPID_SMI_dstnc_to_grank);

    MPID_SMI_finalize();
#if defined(MPIR_MEMDEBUG)
	fprintf(stderr, "[%d] *** allocated memory at end of ch_smi shutdown:\n", MPID_SMI_myid);
	MPID_trdump(0);
	MPID_trlevel(0);
#endif
    
    return 0;
}


void MPID_SMI_Print_config( void ) 
{
    char tmp1[256], tmp2[256], tmp3[256];
    int nbr, size;

    /* XXX replace stdout with configurable output file (MPID_DEBUG_FILE) */
#ifdef MPID_SMI_CHECK_CRC32
    strcpy (tmp1, "CRC32");
#elif defined MPID_SMI_CHECK_FITS
    strcpy (tmp1, "FITS");
#elif defined MPID_SMI_CHECK_INET
    strcpy (tmp1, "INET");
#elif defined MPID_SMI_CHECK_NETDEV
    strcpy (tmp1, "NETDEV");
#elif defined MPID_SMI_CHECK_NONE
    strcpy (tmp1, "NONE");
#else
    ERROR - illegal SMI_CHECK method selected!
#endif
    
		fprintf( stdout, "ADI version %4.2f - transport %s, locks %s\n", 
				 MPIDPATCHLEVEL, MPIDTRANSPORT, p2p_lock_name );
    fprintf( stdout, "  threads: %s - stats: %s - debug: %s - csum: %s\n", 
			 MPID_SMI_THREADS_INFO, MPID_SMI_STATISTICS_INFO, MPID_SMI_DEBUG_INFO, tmp1);

    fprintf (stdout, "Protocol memory configuration: \n");
    MPID_SMI_Short_GetConfig (&nbr, &size);
    fprintf (stdout, "  SHORT:  nbrbufs   = %d, bufsize  = %d [%d net]\n", nbr, size, 
			 MPID_SMI_cfg.MAX_SHORT_PAYLOAD);
    MPID_SMI_Eagern_GetConfig (&nbr, &size);
    if (MPID_SMI_EAGER_DYNAMIC) 
		fprintf (stdout, " dEAGER:  bufsize   = %dk, maxeager = %dk\n", size/1024, MPID_SMI_EAGERSIZE/1024);
    else
		fprintf (stdout, " sEAGER:  nbrbufs   = %d, bufsize  = %dk\n", nbr, size/1024);
    MPID_SMI_Rndv_GetConfig (&nbr, &size);
    MPID_SMI_RNDVBLOCKING_MODE ? 
		fprintf (stdout, " bRNDV :  blocksize = %dk, rcptsize = %dk, poolsize = %dk\n", 
				 nbr/1024, nbr*MPID_SMI_RNDVRECEIPT/1024, size/1024):
		fprintf (stdout, "  RNDV :  blocksize = %dk, rcptsize = %dk, poolsize = %dk\n", 
				 nbr/1024, nbr*MPID_SMI_RNDVRECEIPT/1024, size/1024);
    if (MPID_SMI_cfg.ASYNC_PROGRESS && MPID_SMI_cfg.USE_DMA_PT2PT) {
		MPID_SMI_Arndv_get_config (&size);
		fprintf (stdout, " aRNDV :  dma_outbuf = %dk, dma_blocksize = %dk\n", size/1024, MPID_SMI_RNDVDMASIZE/1024);
    }
    
    fprintf (stdout, "General configuration: \n");
	/* line 1 */
	switch (MPID_SMI_cfg.RESOURCE_SCHED) {
		case 0:
			strcpy(tmp1, "IMMEDIATE");
			break;
		case 1:
			strcpy(tmp1, "LRU");
			break;
		case 2:
			strcpy(tmp1, "LFU");
			break;
		case 3:
			strcpy(tmp1, "BEST_FIT");
			break;
		case 4:
			strcpy(tmp1, "RANDOM");
			break;
		case 5:
			strcpy(tmp1, "NONE");
			break;
	}
    MPID_SMI_cfg.ASYNC_PROGRESS == 0 ? (size_t)strcpy(tmp2, "NO") : (size_t)strcpy(tmp2, "YES");
    switch (MPID_SMI_cfg.MSGCHK_TYPE) {
		case MSGCHK_TYPE_POLLING:
			strcpy(tmp3, "POLL");
			break;
		case MSGCHK_TYPE_IRQ:
			strcpy(tmp3, "IRQ");
			break;
		case MSGCHK_TYPE_IRQ_POLL:
			sprintf(tmp3, "IRQ_POLL (%.0fus)", MPID_SMI_cfg.MSGCHK_DELAY*1e+6);
			break;
		case MSGCHK_TYPE_IRQ_BLOCK:
			sprintf(tmp3, "IRQ_BLOCK (%d x %.0fus)", MPID_SMI_cfg.MSGCHK_REPEAT, MPID_SMI_cfg.MSGCHK_DELAY*1e+6);
			break;
	}
    fprintf (stdout, "  Rsrc Strategy: %s - Async Progress: %s - Msgcheck: %s\n", tmp1, tmp2, tmp3);

	/* line 2 */
    switch (MPID_SMI_cfg.ADPTMODE) {
    case SMI_ADPT_DEFAULT:
		strcpy (tmp1, "DEFAULT"); break;
    case SMI_ADPT_IMPEXP:
		strcpy (tmp1, "IMPEXP"); break;
    case SMI_ADPT_SMP:
		strcpy (tmp1, "SMP"); break;
    }
    switch (MPID_SMI_cfg.MEMCPYSYNC_MODE) {
    case MEMCPYSYNC_NONE:
		sprintf (tmp2, "NONE"); break;
    case MEMCPYSYNC_IN:
		sprintf (tmp2, "IN (%d)", MPID_SMI_cfg.MEMCPYSYNC_MIN); break;
    case MEMCPYSYNC_OUT:
		sprintf (tmp2, "OUT (%d)", MPID_SMI_cfg.MEMCPYSYNC_MIN); break;
    case MEMCPYSYNC_IN|MEMCPYSYNC_OUT:
		sprintf (tmp2, "IN_OUT (%d)", MPID_SMI_cfg.MEMCPYSYNC_MIN); break;
    }
    if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)memcpy)
		strcpy(tmp3, "memcpy");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_smi_memcpy)
		strcpy(tmp3, "SMI/SISCI");
#if (defined MPI_LINUX) || (defined MPI_solaris86) || (defined(WIN32))
	else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_sse32_memcpy)
		strcpy(tmp3, "SSE32");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_sse64_memcpy)
		strcpy(tmp3, "SSE64");
#endif
#if (defined MPI_LINUX) || (defined MPI_solaris86)
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_wc32_memcpy)
		strcpy(tmp3, "WC32");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_wc64_memcpy)
		strcpy(tmp3, "WC64");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_memcpy)
		strcpy(tmp3, "MMX");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx32_memcpy)
		strcpy(tmp3, "MMX32");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx64_memcpy)
		strcpy(tmp3, "MMX64");
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_mmx_prefetchnta_memcpy)
		strcpy(tmp3, "MMX_PRE");
#elif defined MPI_LINUX_ALPHA
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)_mpid_smi_alpha_memcpy)
		strcpy(tmp3, "alpha");
#endif
#ifndef WIN32
    else if (MPID_SMI_memcpy == (mpid_smi_memcpy_fcn_t)bcopy)
		strcpy(tmp3, "bcopy");
#endif
    else 
		strcpy(tmp3, "unknown");
	fprintf (stdout, "  Adapter Mode: %s - Memcpy Sync: %s - Rmtcpy: %s\n", tmp1, tmp2, tmp3);

	/* line 3 */
	switch (MPID_SMI_cfg.NC_ENABLE) {
	case 0: strcpy (tmp1, "NO"); break;
	case 1: sprintf (tmp1, "VECTOR (min %d, align %d)", MPID_SMI_cfg.NC_MINSIZE,
					 MPID_SMI_cfg.NC_ALIGN); break;
	case 2: sprintf (tmp1, "FULL (min %d, align %d)", MPID_SMI_cfg.NC_MINSIZE,
					 MPID_SMI_cfg.NC_ALIGN); break;
	}	
	fprintf (stdout, "  Non-contig: %s\n", tmp1);

	strcpy (tmp2, "");
	if (MPID_SMI_cfg.SSIDED_DELAY > 0)
		sprintf (tmp2, "DELAYED %d", MPID_SMI_cfg.SSIDED_DELAY);
	if (MPID_SMI_cfg.SSIDED_SIGNAL)
		strcat (tmp2, ", SIGNALED");
	if (strlen(tmp2) != 0) 
		fprintf (stdout, "  One-sided: %s\n", tmp2);
	

	/* line 4 */
    MPID_SMI_cfg.REGISTER == 0 ? (size_t)strcpy(tmp2, "NO") : 
		MPID_SMI_cfg.CACHE_REGISTERED ? (size_t)strcpy(tmp2, "YES (cached)") : (size_t)strcpy(tmp2, "YES");
    MPID_SMI_cfg.ZEROCOPY == 0 ? (size_t)strcpy(tmp3, "NO") : 
		MPID_SMI_cfg.CACHE_CONNECTED ? (size_t)strcpy(tmp3, "YES (cached)") : (size_t)strcpy(tmp3, "YES");
	if (MPID_SMI_cfg.USE_DMA_PT2PT) {
		MPID_SMI_cfg.ASYNC_PROGRESS ?
			sprintf(tmp1, "PT2PT-DMA: YES (sync: %dkB, async: %dkB)", MPID_SMI_cfg.SYNC_DMA_MINSIZE >> 10, 
					MPID_SMI_cfg.ASYNC_DMA_MINSIZE >> 10) :
		    sprintf(tmp1, "PT2PT-DMA: YES (sync: %dkB)", MPID_SMI_cfg.SYNC_DMA_MINSIZE >> 10);
		fprintf (stdout, "  %s - Register: %s - Zerocopy: %s\n", tmp1, tmp2, tmp3);
	} else {
		fprintf (stdout, "  PT2PT-DMA: NO\n");
	}

	/* additional lines for custom collectives */
	if (MPID_SMI_cfg.COLLOPS) {
		fprintf (stdout, "Collective operations:\n");
		
		/* Pipeline parameters */
		strcpy (tmp1, MPID_SMI_cfg.COLL_PIPE_DYNAMIC ? "dyn" : "fixed");
		switch (MPID_SMI_cfg.USE_DMA_COLL) {
		case 0: strcpy (tmp2, "no DMA"); break;
		case 1: 
		case 2: sprintf (tmp2, "DMA >%dk", MPID_SMI_cfg.COLL_PIPE_DMA_MIN >> 10); break;
		}
		fprintf (stdout, "  Pipe: PIO >%dk - %s - %dk x %d (%s)\n", MPID_SMI_cfg.COLL_PIPE_MIN >> 10,
				 tmp2, MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE >> 10, MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS, tmp1);

		/* Reduction parameters */
		strcpy (tmp1, MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK ? "YES" : "NO"); 
		switch (MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE) {
		case REDUCE_TREE: strcpy (tmp2, "TREE");    break;
		case REDUCE_RABENSEIFNER: strcpy (tmp2, "RABSEIF"); break;
		case REDUCE_PIPELINE: strcpy (tmp2, "PIPE");    break;
		}
		switch (MPID_SMI_cfg.COLL_ALLREDUCE_TYPE) {
		case ALLREDUCE_DEFAULT: strcpy (tmp3, "DEFAULT"); break;
		case ALLREDUCE_RABENSEIFNER: strcpy (tmp3, "RABSEIF"); break;
		case ALLREDUCE_PIPELINE: strcpy (tmp3, "PIPE");    break;
		case ALLREDUCE_ALLGATHER: strcpy (tmp3, "ALLGATH");    break;
		case ALLREDUCE_RABSCI: strcpy (tmp3, "RAB_SCI");    break;
		}
		/* Currently, short vectors are always reduce via a tree algorithm. */
		fprintf (stdout, "  Rdc: %s (< %dk: TREE w/ fan-in %d) - Allrdc: %s - Direct: %s\n", 
				 tmp2, MPID_SMI_cfg.COLL_REDUCE_LONG >> 10, MPID_SMI_cfg.COLL_REDUCE_FANIN, 
				 tmp3, tmp1);

		/* Other (barrier, alltoall) parameters */
		if (MPID_SMI_cfg.COLL_BARRIER)
			sprintf (tmp1, "fan-in %d", MPID_SMI_cfg.COLL_BARRIER_FANIN);
		else
			strcpy (tmp1, "NO");

		switch (MPID_SMI_cfg.COLL_ALLTOALL_TYPE) {
		case ALLTOALL_PLAIN:  strcpy (tmp2, "PLAIN");  break;
		case ALLTOALL_SCI_1D: strcpy (tmp2, "SCI-1D"); break;
		case ALLTOALL_SCI_2D: strcpy (tmp2, "SCI-2D"); break;
		case ALLTOALL_SCAMPI: strcpy (tmp2, "SCAMPI"); break;
		case ALLTOALL_MPICH:  strcpy (tmp2, "MPICH");  break;
		case ALLTOALL_STRAIGHT: strcpy (tmp2, "STRAIGHT");  break;
		}
		if (MPID_SMI_cfg.COLL_ALLTOALL_TYPE != ALLTOALL_MPICH)
			sprintf (tmp3, "(>= %dk)", MPID_SMI_cfg.COLL_ALLTOALL_MIN/1024);
		else
			sprintf (tmp3, "");		
		fprintf (stdout, "  Barrier: %s - A2a: type %s %s\n", tmp1, tmp2, tmp3);
	} else {
		fprintf (stdout, "Collective operations:\n  point-to-point\n");
	}
	
	/* Show all applied performance modelling parameters (if any are active).*/
	if (MPID_SMI_cfg.PERF_GAP_LTNCY > 0 || MPID_SMI_cfg.PERF_SEND_LTNCY > 0 || MPID_SMI_cfg.PERF_RECV_LTNCY > 0
		|| MPID_SMI_cfg.PERF_BW_LIMIT > 0 || MPID_SMI_cfg.PERF_BW_REDUCE > 0 ) {
		/* We have to recalculate the values into "real world" scale. */
		double us_per_tick;
		int ps_per_tick;
		ulong tick_scale;
		SMI_Query (SMI_Q_SYS_TICK_DURATION, 0, &ps_per_tick);
		us_per_tick = ps_per_tick/1e+6;
		tick_scale = 1 << PERF_TICKSCALE;

		fprintf (stdout, "Performance modelling activated:\n");
		if (MPID_SMI_cfg.PERF_GAP_LTNCY > 0 || MPID_SMI_cfg.PERF_SEND_LTNCY > 0 || MPID_SMI_cfg.PERF_RECV_LTNCY > 0)
			fprintf (stdout, "  Latency  : GAP +%4.1fus - SEND +%4.1fus - RECV +%4.1fus\n",
					 MPID_SMI_cfg.PERF_GAP_LTNCY*us_per_tick, 
					 MPID_SMI_cfg.PERF_SEND_LTNCY*us_per_tick, 
					 MPID_SMI_cfg.PERF_RECV_LTNCY*us_per_tick);

		if (MPID_SMI_cfg.PERF_BW_LIMIT > 0 || MPID_SMI_cfg.PERF_BW_REDUCE > 0 ) {
			if (MPID_SMI_cfg.PERF_BW_LIMIT > 0)
				sprintf (tmp1, "to %4.1f MB/s", (double)MPID_SMI_cfg.PERF_BW_LIMIT*1e+6/(us_per_tick*tick_scale*1024*1024));
			else 
				sprintf (tmp1, "none");
			if (MPID_SMI_cfg.PERF_BW_REDUCE > 0)
				sprintf (tmp2, "down to %d %%", MPID_SMI_cfg.PERF_BW_REDUCE);
			else 
				sprintf (tmp2, "none");
			fprintf (stdout, "  Bandwidth: LIMIT %s - REDUCE %s\n", tmp1, tmp2);
		}
	}

	/* Warning if transfer verification is disabled as it may lead to deadlocks*/
    if (!MPID_SMI_cfg.DO_VERIFY) {
		fprintf (stdout, "  SCI TRANSFER VERIFICATION DISABLED\n");
    }
    fprintf (stdout, "\n"); fflush (stdout);
    return;
}

/*
 * Currently, this is inactive because adi2init contains MPID_Version_name .
 */
static void MPID_SMI_Version_name(char *name)
{
    sprintf( name, "ADI version %4.2f - transport %s, locks %s", 
			 MPIDPATCHLEVEL, MPIDTRANSPORT, p2p_lock_name );
}

/* 
   function to evaluate the command line arguments and create an new set of
   argumants that can be read by SMI_INIT()
*/
void MPID_SMI_Eval_cmdline(argc, argv)

	 int *argc;
	 char ***argv;
{
    int c, n;
    extern char *optarg;
    int argument = 1; /* index for delargs */
    int *delargs;    /* flag-field of which args to strip from the command-line */

    ALLOCATE(delargs, int *, *argc * sizeof(int));
    for(c = 0; c < *argc; c++)
		delargs[c] = 0;
    /* set devconf_name to its default value */
    ALLOCATE(devconf_name, char *, strlen(DEVCONF_NAME_DEF) + 1);
    strcpy(devconf_name, DEVCONF_NAME_DEF);

    SMIcall( SMI_Debug(0) );
    /* these two parameters may get overwritten by command line args */
    MPID_SMI_cfg.USE_WATCHDOG = -MPID_SMI_WATCHDOG_DEF;
    MPID_SMI_cfg.VERBOSE      = MPID_SMI_VERBOSE_DEF;

    /* evaluate the command line arguments and make sure that all 
       necessary options are given */
    while((c = getopt(*argc, *argv, "h:p:m:i:r:n:d:f:lstwvg")) != EOF) 
		switch(c) {
			/* the following arguments will be passed to SMI */
		case 'h':
        case 'p':
        case 'm':
		case 'i':
		case 'r':
		case 'n':
		case 'f':
			argument += 2;
			break;
		case 'l':
		case 's':
		case 't':
		case 'g':
			argument++;
			break;	    
		case 'v':
			/* be verbose on startup */
			MPID_SMI_cfg.VERBOSE = 1;
			argument++;
			break;
		case 'w':
			/* disable watchdog for debugging */
			MPID_SMI_cfg.USE_WATCHDOG = 0;
			argument++;
			break;
			/* the following arguments are for SCI-MPCH only */
		case 'd':
			FREE(devconf_name);
			ALLOCATE(devconf_name, char *, strlen(optarg) + 1);
			strcpy(devconf_name, optarg);
	    
			delargs[argument] = 1;
			delargs[argument+1] = 1;
			argument += 2;
			break;
		}
    
    /* throw away the args special to SCI-MPICH */
    n = 1;
    while (n < *argc) {
		if (delargs[n]) {
			for (c = n; c < *argc - 1; c++) {
				(*argv)[c] = (*argv)[c+1];
				delargs[c] = delargs[c+1];
			}
			(*argc)--;
		} else
			n++;
    }
    
    FREE(delargs);
    MPID_SMI_resetGetOpt();
}

#define PRINT_CONFIG_WARNING fprintf (stdout, "*** SCI-MPICH warning: \n"); \
  			    fprintf (stdout, "   ch_smi device configuration '%s', line %d\n", devconf_name, line_nbr);


void MPID_SMI_Read_settings( void ) {
    
    FILE *config_file_handler;
    char buffer[MPID_SMI_STRINGLEN], keyword[MPID_SMI_STRINGLEN], valuestring[MPID_SMI_STRINGLEN];
	char *magn_symbol;
    int value, i, line_nbr = 0;
    uint align;
    
    /* 
     *  set  default values 
     */

    /* set to minimum size for minimal latency */
    MPID_SMI_SHORTSIZE = MPID_SMI_SHORTSIZE_DEF;
    /* set to zero here; if not defined in the device configuration file, set to
       default afterwards */
    MPID_SMI_SHORTBUFS = 0;
    
    MPID_SMI_EAGERSIZE = MPID_SMI_EAGERSIZE_DEF;
    MPID_SMI_EAGERBUFS = MPID_SMI_EAGERBUFS_DEF; 
    MPID_SMI_EAGER_MAXCHECKDEV = MPID_SMI_EAGER_MAXCHECKDEV_DEF; 
    MPID_SMI_EAGER_DYNAMIC = MPID_SMI_EAGER_DYNAMIC_DEF;
    MPID_SMI_EAGER_IMMEDIATE = MPID_SMI_EAGER_IMMEDIATE_DEF;

    MPID_SMI_RNDVSIZE  = MPID_SMI_RNDVSIZE_DEF;
    MPID_SMI_RNDVBLOCK = MPID_SMI_RNDVBLOCK_DEF;
    MPID_SMI_RNDVRECEIPT = MPID_SMI_RNDVRECEIPT_DEF;
    MPID_SMI_RNDVDMASIZE = MPID_SMI_RNDVDMASIZE_DEF;
    MPID_SMI_RNDVBLOCKING_MODE = MPID_SMI_RNDVBLOCKING_MODE_DEF;	

	MPID_SMI_cfg.MEMCPY_TYPE    = MPID_SMI_MEMCPY_TYPE_DEF;
    MPID_SMI_cfg.ASYNC_PROGRESS = MPID_SMI_ASYNC_PROGRESS_DEF;
    MPID_SMI_cfg.SENDSELF       = MPID_SMI_SENDSELF_DEF;
	MPID_SMI_cfg.MAX_RECVS      = MPID_SMI_MAX_RECVS_DEF;
	MPID_SMI_cfg.MAX_SENDS      = MPID_SMI_MAX_SENDS_DEF;
    MPID_SMI_cfg.USE_DMA_PT2PT  = MPID_SMI_USE_DMA_PT2PT_DEF;
    MPID_SMI_cfg.USE_DMA_COLL   = MPID_SMI_USE_DMA_COLL_DEF;
    MPID_SMI_cfg.SYNC_DMA_MINSIZE  = MPID_SMI_SYNC_DMA_MINSIZE_DEF;
    MPID_SMI_cfg.ASYNC_DMA_MINSIZE = MPID_SMI_ASYNC_DMA_MINSIZE_DEF;
    MPID_SMI_cfg.CONNECT_ON_INIT   = MPID_SMI_CONNECT_ON_INIT_DEF;
    MPID_SMI_cfg.DO_VERIFY      = !MPID_SMI_NO_VERIFY_DEF;
    MPID_SMI_cfg.MSGCHK_TYPE    = MPID_SMI_MSGCHK_TYPE_DEF;
    MPID_SMI_cfg.MSGCHK_DELAY   = MPID_SMI_MSGCHK_DELAY_DEF*1e-6;
    MPID_SMI_cfg.MSGCHK_REPEAT  = MPID_SMI_MSGCHK_REPEAT_DEF;
    MPID_SMI_cfg.ZEROCOPY       = MPID_SMI_ZEROCOPY_DEF;
    MPID_SMI_cfg.REGISTER       = MPID_SMI_REGISTER_DEF;
    MPID_SMI_cfg.CACHE_REGISTERED = MPID_SMI_CACHE_REGISTERED_DEF;

    MPID_SMI_cfg.COLLOPS = MPID_SMI_COLLOPS_DEF;
    MPID_SMI_cfg.COLL_BARRIER             = MPID_SMI_BARRIER_DEF;
    MPID_SMI_cfg.COLL_BARRIER_FANIN       = MPID_SMI_BARRIER_FANIN_DEF;
    MPID_SMI_cfg.COLL_REDUCE_SHORT_TYPE   = MPID_SMI_REDUCE_SHORT_TYPE_DEF;
    MPID_SMI_cfg.COLL_REDUCE_LONG         = MPID_SMI_REDUCE_LONG_DEF;
    MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE    = MPID_SMI_REDUCE_LONG_TYPE_DEF;
    MPID_SMI_cfg.COLL_REDUCE_FANIN        = MPID_SMI_REDUCE_FANIN_DEF;
    MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK    = MPID_SMI_DIRECT_REDUCE_OK_DEF;
    MPID_SMI_cfg.COLL_ALLREDUCE_TYPE      = MPID_SMI_ALLREDUCE_TYPE_DEF;
    MPID_SMI_cfg.COLL_ALLGATHER_BARRIER   = MPID_SMI_ALLGATHER_BARRIER_DEF;
    MPID_SMI_cfg.COLL_SCATTER_MAX         = MPID_SMI_SCATTER_MAX_DEF;
    MPID_SMI_cfg.COLL_ALLTOALL_BARRIER    = MPID_SMI_ALLTOALL_BARRIER_DEF;
    MPID_SMI_cfg.COLL_ALLTOALL_MIN        = -1;
    MPID_SMI_cfg.COLL_ALLTOALL_TYPE       = MPID_SMI_ALLTOALL_TYPE_DEF;
    MPID_SMI_cfg.COLL_PIPE_MIN            = MPID_SMI_PIPE_MIN_DEF;
	MPID_SMI_cfg.COLL_PIPE_DMA_MIN        = MPID_SMI_PIPE_DMA_MIN_DEF;
	MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE      = MPID_SMI_PIPE_BLOCKSIZE_DEF;
	MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS      = MPID_SMI_PIPE_NBRBLOCKS_DEF;
	MPID_SMI_cfg.COLL_PIPE_DYNAMIC        = MPID_SMI_PIPE_DYNAMIC_DEF;
	MPID_SMI_cfg.COLL_BCAST_TYPE          = MPID_SMI_BCAST_TYPE_DEF; 

    MPID_SMI_cfg.NC_ENABLE      = MPID_SMI_NC_ENABLE_DEF;
    MPID_SMI_cfg.NC_MINSIZE     = MPID_SMI_NC_MINSIZE_DEF;
    MPID_SMI_cfg.NC_ALIGN       = MPID_SMI_NC_ALIGN_DEF;
    MPID_SMI_cfg.ALLOC_MINSIZE  = MPID_SMI_ALLOC_MINSIZE_DEF;
    MPID_SMI_cfg.ALLOC_POOLSIZE = MPID_SMI_ALLOC_POOLSIZE_DEF;
    MPID_SMI_cfg.RESOURCE_SCHED = MPID_SMI_RESOURCE_SCHEDULING_DEF;
    MPID_SMI_cfg.SSIDED_ENABLED = MPID_SMI_SSIDED_ENABLED_DEF;
	MPID_SMI_cfg.SSIDED_RMTPUT_PRIVATE	
								= MPID_SMI_SSIDED_RMTPUT_PRIVATE_DEF;
	MPID_SMI_cfg.SSIDED_RMTPUT_SHARED
								= MPID_SMI_SSIDED_RMTPUT_SHARED_DEF;
	MPID_SMI_cfg.SSIDED_FRAGMENT_MAX	= MPID_SMI_SSIDED_FRAGMENT_MAX_DEF;
	MPID_SMI_cfg.SSIDED_SIGNAL  = MPID_SMI_SSIDED_SIGNAL_DEF;
	MPID_SMI_cfg.SSIDED_DELAY   = MPID_SMI_SSIDED_DELAY_DEF;
	MPID_SMI_cfg.SSIDED_NOSYNC  = MPID_SMI_SSIDED_NOSYNC_DEF;

#if 0
    /* make a smart default choice of the adapter mode */
    if (MPID_SMI_NBRADPTS <= 1) {
		MPID_SMI_cfg.ADPTMODE = SMI_ADPT_DEFAULT;
    } else {
		if ((MPID_SMI_numProcsOnNode[MPID_SMI_myNode] > 1)
			&& (MPID_SMI_NBRADPTS == MPID_SMI_numProcsOnNode[MPID_SMI_myNode])) {
			/* we are running SMP and have one adapter for each process -> SMP mode */
			MPID_SMI_cfg.ADPTMODE = SMI_ADPT_SMP;
		} else {
			/* we have at least two adapters, but are not runnig SMP or have not
			   one adapter for every process -> IMPEXP mode */
			MPID_SMI_cfg.ADPTMODE = SMI_ADPT_IMPEXP;
		}
    }
#else
    /* XXX: for now, make this feature only available on expicit demand of the user */
    MPID_SMI_cfg.ADPTMODE = SMI_ADPT_DEFAULT;
#endif

    /* - use INgoing memcpy synchronization if running with more than 2 nodes 
	     (effective for LC-2 systems only)
       - use OUTgoing synchronization if running in SMP mode (> 1 proc on local node) */
	MPID_SMI_cfg.MEMCPYSYNC_MODE = MEMCPYSYNC_NONE;
	MPID_SMI_cfg.MEMCPYSYNC_MIN  = MPID_SMI_MEMCPYSYNC_MIN_DEF;
	if (MPID_SMI_use_SMP) {
		/* This setting will be reverted for non-SMP nodes lateron (lowlevel_init). */
		MPID_SMI_cfg.MEMCPYSYNC_MODE = MEMCPYSYNC_OUT;
	}
	SMI_Query(SMI_Q_SCI_ADAPTERTYPE, MPID_SMI_DEFADPT, &value);
	if (MPID_SMI_numNodes > 2 && value < 330) {
		MPID_SMI_cfg.MEMCPYSYNC_MODE |= MEMCPYSYNC_IN;
	}
		
    MPID_SMI_Set_statistics(MPID_SMI_STATISTICS_DEF);
    
    /* read configuration file, if possible, and set variables according to the values therein;
       if the file cannot be opened or a keyword is not contained in the file, the according
       variable remains at its default value                                                   */
    /* short buffer size, eager buffer size and rendez-vous block size *must* be a 
       multiple of the streamsize - or at least the SCI transaction size */
    
    config_file_handler = fopen(devconf_name, "r");
    if (config_file_handler != NULL) {
		while (fgets(buffer, MPID_SMI_STRINGLEN, config_file_handler) != NULL) {
			line_nbr++;
			/* empty lines */
			if (strlen(buffer) <= 1)
				continue;
			/* comment lines */
			if ((buffer[0] == '#') || (buffer[0] == ';'))
				continue;
			/* parse the numerical argument */	    
			if (sscanf(buffer, "%s%s", keyword, valuestring) == 2) {
				value = (int)strtol(valuestring, &magn_symbol, 10);

				if (magn_symbol != NULL) {
					if ((magn_symbol[0] == 'k') || (magn_symbol[0] == 'K'))
						value *= 1024;
					else if ((magn_symbol[0] == 'm') || (magn_symbol[0] == 'M'))
						value *= 1048576;
					else if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid && (strlen(magn_symbol) > 1)) {
						PRINT_CONFIG_WARNING;
						fprintf(stdout, "    Illegal value '%s'", valuestring);
						fprintf(stdout, "    -> ignoring this line\n");
						continue;
					}
				}

				/* SHORT_bufsize */
				if (strcasecmp(keyword, NAME_SHORTSIZE) == 0) {
					/* SHORTSIZE must be a power of 2 */
					if (MPID_SMI_msb(value) == value) {
						MPID_SMI_SHORTSIZE = value;
					} else {
						MPID_SMI_SHORTSIZE = MPID_SMI_msb(value)*2;
						if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
							PRINT_CONFIG_WARNING;
							fprintf(stdout, "    SHORT_bufsize must be a power of 2 -> aligning to %d\n", 
									MPID_SMI_SHORTSIZE);
						}
					}
					if (MPID_SMI_SHORTSIZE > MPID_SMI_PAGESIZE) {
						/* max. SHORTSIZE is the systems pagesize */
						MPID_SMI_SHORTSIZE = MPID_SMI_PAGESIZE;
						if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
							PRINT_CONFIG_WARNING;
							fprintf(stdout, "    SHORT_bufsize > Pagesize -> aligning to %d\n", 
									MPID_SMI_SHORTSIZE);
						}
					}
					if (MPID_SMI_SHORTSIZE < MPID_SMI_SCI_TA_SIZE) {
						/* min. SHORTSIZE is the SCI packet size */
						MPID_SMI_SHORTSIZE = MPID_SMI_SCI_TA_SIZE;
						if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
							PRINT_CONFIG_WARNING;
							fprintf(stdout, "    SHORT_bufsize < SCI packet size -> aligning to %d\n", 
									MPID_SMI_SHORTSIZE);
						}
					}
				}
				/* SHORT_nbrbufs */
				else if (strcasecmp(keyword, NAME_SHORTBUFS) == 0) 
					MPID_SMI_SHORTBUFS = value;
				/* EAGER_bufsize */
				else if (strcasecmp(keyword, NAME_EAGERSIZE) == 0) {
					MPID_SMI_STREAMBUF_ALIGN (value);
					MPID_SMI_EAGERSIZE = value;
				} 
				/* EAGER_nbrbufs */
				else if (strcasecmp(keyword, NAME_EAGERBUFS) == 0)
					MPID_SMI_EAGERBUFS = value;
				/* EAGER_maxcheckdev */
				else if (strcasecmp(keyword, NAME_EAGERMAXCHECKDEV) == 0)
					MPID_SMI_EAGER_MAXCHECKDEV = value;
				/* EAGER_dynamic */
				else if (strcasecmp(keyword, NAME_EAGERDYNAMIC) == 0) {
					MPID_SMI_EAGER_DYNAMIC = value;		    
					/* if DYNAMIC is selected, IMMEDIATE must be set, too */
					if (MPID_SMI_EAGER_DYNAMIC)
						MPID_SMI_EAGER_IMMEDIATE = 1;
				/* EAGER_immediate */
				} else if (strcasecmp(keyword, NAME_EAGERIMMEDIATE) == 0)
					MPID_SMI_EAGER_IMMEDIATE = value;
				/* RNDV_memorysize */
				else if (strcasecmp(keyword, NAME_RNDVSIZE) == 0)
					MPID_SMI_RNDVSIZE = value;
				/* RNDV_blocksize */
				else if (strcasecmp(keyword, NAME_RNDVBLOCK) == 0) {
					MPID_SMI_STREAMBUF_ALIGN (value);
					MPID_SMI_RNDVBLOCK = value;
				}
				/* RNDV_receiptsize */
				else if (strcasecmp(keyword, NAME_RNDVRECEIPT) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0) {
						if (!MPID_SMI_myid) {
							PRINT_CONFIG_WARNING;
							fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0 \n", value, keyword);
							fprintf (stdout, "   -> running with default value\n");
						}
					} else {
						MPID_SMI_RNDVRECEIPT = value;
					}
				}
				/* RNDV_DMAblocksize */
				else if (strcasecmp(keyword, NAME_RNDVDMASIZE) == 0) {
					MPID_SMI_STREAMBUF_ALIGN (value);
					MPID_SMI_RNDVDMASIZE = value;
				}
				/* activate asynchronous data transfer ? */
				else if (strcasecmp(keyword, NAME_RNDVBLOCKING_MODE) == 0) {
					MPID_SMI_RNDVBLOCKING_MODE = value;
				}
				/* ASYNC_PROGRESS */
				else if (strcasecmp(keyword, NAME_ASYNC_PROGRESS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with nonblocking Isends\n");
					} else {
						MPID_SMI_cfg.ASYNC_PROGRESS = value;	
						if (MPID_SMI_cfg.ASYNC_PROGRESS) {
#ifdef MPID_USE_DEVTHREADS
							/* check if non-blocking mode can be used */
							if (MPID_SMI_numids == 1) {
								if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   %s mode not available for single-process applications\n", keyword);
								}
								MPID_SMI_cfg.ASYNC_PROGRESS = 0;
							}
							SMIcall(SMI_Query(SMI_Q_SMI_SIGNALS_AVAILABLE, 0, &value));
							if (!value) {
								if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   SMI signals not available -> %s mode disabled\n", keyword);
								}
								MPID_SMI_cfg.ASYNC_PROGRESS = 0;
							}
#else
							if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
								PRINT_CONFIG_WARNING;
								fprintf (stdout, "   %s mode not available without threads\n", keyword);
								fprintf (stdout, "   use --enable-devthreads option for configuring\n");
							}
							MPID_SMI_cfg.ASYNC_PROGRESS = 0;
#endif
						}
					}
				}
				/* SSIDED_ENABLED */
				else if (strcasecmp(keyword, NAME_SSIDED_ENABLED) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> using default setting (enabled)\n");
					} else {
						MPID_SMI_cfg.SSIDED_ENABLED = value;	
					}
				}
				/* SSIDED_SIGNAL */
				else if (strcasecmp(keyword, NAME_SSIDED_SIGNAL) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> using default setting (disabled)\n");
					} else {
						MPID_SMI_cfg.SSIDED_SIGNAL = value;	
					}
				}
				/* SSIDED_NOSYNC */
				else if (strcasecmp(keyword, NAME_SSIDED_NOSYNC) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> using default setting (disabled)\n");
					} else {
						MPID_SMI_cfg.SSIDED_NOSYNC = value;	
					}
				}
				/* SSIDED_NONBLOCKING */
				else if (strcasecmp(keyword, NAME_SSIDED_NONBLOCKING) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> using default setting (disabled)\n");
					} else {
						MPID_SMI_cfg.SSIDED_NONBLOCKING = value;	
					}
				}
				/* SSIDED_DELAY */
				else if (strcasecmp(keyword, NAME_SSIDED_DELAY) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >= 0\n", value, keyword);
						fprintf (stdout, "   -> using default setting (disabled)\n");
					} else {
						MPID_SMI_cfg.SSIDED_DELAY = value;	
					}
				}
				/* SSIDED_RMTPUT_PRIVATE */
				else if (!strcasecmp (keyword, NAME_SSIDED_RMTPUT_PRIVATE)) {
					if (MPID_SMI_cfg.VERBOSE && value < 0) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s has a negative value (%d)\n",
											keyword, value);
					}
					MPID_SMI_cfg.SSIDED_RMTPUT_PRIVATE = value;
				}
				/* SSIDED_RMTPUT_SHARED */
				else if (!strcasecmp (keyword, NAME_SSIDED_RMTPUT_SHARED)) {
					if (MPID_SMI_cfg.VERBOSE && value < 0) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s has a negative value (%d)\n",
											keyword, value);
					}
					MPID_SMI_cfg.SSIDED_RMTPUT_SHARED = value;
				}
				/* SSIDED_FRAGMENT_MAX */
				else if (!strcasecmp (keyword, NAME_SSIDED_FRAGMENT_MAX)) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 100)) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   illegal value %d for %s - "
										 "must be between 0 - 100\n",  
											value, keyword);
					}
					MPID_SMI_cfg.SSIDED_FRAGMENT_MAX = value;
				}	
				/* STATISTICS */
				else if (strcasecmp(keyword, NAME_STATISTICS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
#ifdef MPID_SMI_STATISTICS
						if (value == 0)
							MPID_SMI_Set_statistics(SMI_STAT_DISABLE);
					if (value == 1)
						MPID_SMI_Set_statistics(SMI_STAT_ENABLE);
#else
					if (MPID_SMI_cfg.VERBOSE && value == 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "    No support for %s - reconfigure with --enable-statistics as ch_smi device option\n", keyword);
						fprintf (stdout, "    -> running with disabled statistics\n");
					}
#endif
				}
				/* SENDSELF */
				else if (strcasecmp(keyword, NAME_SENDSELF) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.SENDSELF = value;
				}
				/* MAX_RECVS */
				else if (strcasecmp(keyword, NAME_MAX_RECVS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MAX_RECVS = value;
				}
				/* MAX_SENDS */
				else if (strcasecmp(keyword, NAME_MAX_SENDS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MAX_SENDS = value;
				}
				/* MAX_TRANSFERS */
				else if (strcasecmp(keyword, NAME_MAX_TRANSFERS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value <= 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MAX_RECVS = value;
						MPID_SMI_cfg.MAX_SENDS = value;
				}
				/* COLLOPS */
				else if (strcasecmp(keyword, NAME_COLLOPS) == 0) {
#if 1 || MPI_SHARED_LIBS || defined (WIN32)
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLLOPS = value;
#else
					if (MPID_SMI_cfg.VERBOSE && value != 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Optimized collective ops. only available with dynamic libraries\n", value);
						fprintf (stdout, "   -> running with default collective operations\n");
					}			
#endif
				}
				/* COLL_BARRIER */
				else if (strcasecmp(keyword, NAME_COLL_BARRIER) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 1) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", 
								 value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_BARRIER = value;
				}
				/* COLL_BARRIER_FANIN */
				else if (strcasecmp(keyword, NAME_COLL_BARRIER_FANIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value <= 0 || value > MPID_SMI_BARRIER_FANIN_MAX) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0 and <= %d\n", 
								 value, keyword, MPID_SMI_BARRIER_FANIN_MAX);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_BARRIER_FANIN = value;
				}
				/* COLL_REDUCE_SHORT_TYPE */
				else if (strcasecmp(keyword, NAME_COLL_REDUCE_SHORT_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 2) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_REDUCE_SHORT_TYPE = value;
				}
				/* COLL_REDUCE_LONG_TYPE */
				else if (strcasecmp(keyword, NAME_COLL_REDUCE_LONG_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 2) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE = value;
				}
				/* COLL_REDUCE_LONG */
				else if (strcasecmp(keyword, NAME_COLL_REDUCE_LONG) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 128 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 128\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_REDUCE_LONG = value;
				}
				/* COLL_REDUCE_FANIN */
				/* XXX: Check for 2**n condition! */
				else if (strcasecmp(keyword, NAME_COLL_REDUCE_FANIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value <= 0 || value > MPID_SMI_REDUCE_FANIN_MAX) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >0 and <= %d\n", 
								 value, keyword, MPID_SMI_REDUCE_FANIN_MAX);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_REDUCE_FANIN = value;
				}
				/* COLL_DIRECT_REDUCE */
				else if (strcasecmp(keyword, NAME_COLL_DIRECT_REDUCE_OK) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 1) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK = value;
				}
				/* COLL_ALLREDUCE_TYPE */
				else if (strcasecmp(keyword, NAME_COLL_ALLREDUCE_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 4) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_ALLREDUCE_TYPE = value;
				}
				/* COLL_ALLTOALL_BARRIER */
				else if (strcasecmp(keyword, NAME_COLL_ALLTOALL_BARRIER) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value <= 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_ALLTOALL_BARRIER = value;
				}
				/* COLL_ALLTOALL_MIN */
				else if (strcasecmp(keyword, NAME_COLL_ALLTOALL_MIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_ALLTOALL_MIN = value;
				}
				/* COLL_ALLTOALL_TYPE */
				else if (strcasecmp(keyword, NAME_COLL_ALLTOALL_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > ALLTOALL_MPICH) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_ALLTOALL_TYPE = value;
				}
				/* COLL_PIPE_MIN */
				else if (strcasecmp(keyword, NAME_COLL_PIPE_MIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_PIPE_MIN = value;
				}
				/* COLL_PIPE_DMA_MIN */
				else if (strcasecmp(keyword, NAME_COLL_PIPE_DMA_MIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_PIPE_DMA_MIN = value;
				}
				/* COLL_PIPE_BLOCKSIZE */
				else if (strcasecmp(keyword, NAME_COLL_PIPE_BLOCKSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else {
						MPID_SMI_STREAMBUF_ALIGN(value);
						MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE = value;
					}
				}
				/* COLL_PIPE_NBRBLOCKS */
				else if (strcasecmp(keyword, NAME_COLL_PIPE_NBRBLOCKS) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 3 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 2 \n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS = value;
				}
				/* COLL_PIPE_DYNAMIC */
				else if (strcasecmp(keyword, NAME_COLL_PIPE_DYNAMIC) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 1) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_PIPE_DYNAMIC = value;
				}
				/* COLL_BCAST */
				else if (strcasecmp(keyword, NAME_COLL_BCAST_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > BCAST_MULTIDIM) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_BCAST_TYPE = value;
				}
				/* COLL_ALLGATHER_BARRIER */
				else if (strcasecmp(keyword, NAME_COLL_ALLGATHER_BARRIER) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value <= 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be > 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_ALLGATHER_BARRIER = value;
				}
				/* COLL_SCATTER_MAX */
				else if (strcasecmp(keyword, NAME_COLL_SCATTER_MAX) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.COLL_SCATTER_MAX = value;
				}
				/* USE_DMA_PT2PT */
				else if (strcasecmp(keyword, NAME_USE_DMA_PT2PT) == 0) {
					/* XXX debug */
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 3) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else {
						MPID_SMI_cfg.USE_DMA_PT2PT = value;
						if (MPID_SMI_cfg.USE_DMA_PT2PT) {
							/* is DMA available ? */
							SMI_Query(SMI_Q_SMI_DMA_AVAILABLE, 0, (void *)&value);
							if (!value) {
								if  (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   DMA transfers not available -> DMA_PT2PT mode disabled\n");
								}
								MPID_SMI_cfg.USE_DMA_PT2PT = 0;
							}				
						}
					}
				}
				/* USE_DMA */
				else if (strcasecmp(keyword, NAME_USE_DMA_COLL) == 0) {
					/* XXX debug */
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 3) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else {
						MPID_SMI_cfg.USE_DMA_COLL = value;
						if (MPID_SMI_cfg.USE_DMA_COLL) {
							/* is DMA available ? */
							SMI_Query(SMI_Q_SMI_DMA_AVAILABLE, 0, (void *)&value);
							if (!value) {
								if  (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   DMA transfers not available -> DMA_COLL mode disabled\n");
								}
								MPID_SMI_cfg.USE_DMA_COLL = 0;
							}				
						}
					}
				}
				/* SYNC_DMA_MINSIZE */
				else if (strcasecmp(keyword, NAME_SYNC_DMA_MINSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0  && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >= 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.SYNC_DMA_MINSIZE = value;
				}
				/* ASYNC_DMA_MINSIZE */
				else if (strcasecmp(keyword, NAME_ASYNC_DMA_MINSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0  && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >= 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.ASYNC_DMA_MINSIZE = value;
				}
				/* WATCHDOG */
				else if (strcasecmp(keyword, NAME_WATCHDOG) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >=  0 \n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else {
						if (MPID_SMI_cfg.USE_WATCHDOG == -MPID_SMI_WATCHDOG_DEF)
							MPID_SMI_cfg.USE_WATCHDOG = value;
					}
					/* turn SMI-watchdog on/off according to settings */
					if (MPID_SMI_cfg.USE_WATCHDOG > 0) {
						SMIcall( SMI_Watchdog( MPID_SMI_cfg.USE_WATCHDOG) );
					} else {
						SMIcall( SMI_Watchdog( SMI_WATCHDOG_OFF ) );
					}			
				}
				/* MSGCHK_TYPE */
				else if (strcasecmp(keyword, NAME_MSGCHK_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 3) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch.\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else {
						MPID_SMI_cfg.MSGCHK_TYPE = value;
						if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
#ifdef MPID_USE_DEVTHREADS
							/* we need the device-check thread for this to work! */
							/* check if non-polling mode can be used */
							if (MPID_SMI_numids == 1) {
								if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   non-POLLING mode not available for single-process applications\n");
								}
								MPID_SMI_cfg.MSGCHK_TYPE = MSGCHK_TYPE_POLLING;
							}
							SMI_Query(SMI_Q_SMI_SIGNALS_AVAILABLE, 0, &value);
							if (!value) {
								if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
									PRINT_CONFIG_WARNING;
									fprintf (stdout, "   SMI signals not available -> non-POLLING mode disabled\n");
								}
								MPID_SMI_cfg.MSGCHK_TYPE = MSGCHK_TYPE_POLLING;
							}
							/* We need to start the device thread for non-polling msgchecks. */
							if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
								MPID_SMI_cfg.ASYNC_PROGRESS = 1;
							}
#else
							if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
								PRINT_CONFIG_WARNING;
								fprintf (stdout, "   non-POLLING mode not available without threads\n");
								fprintf (stdout, "   use --enable-devthreads option for configuring\n");
							}
							MPID_SMI_cfg.MSGCHK_TYPE = MSGCHK_TYPE_POLLING;
#endif
						}
					}
				}
				/* MSGCHK_DELAY */
				else if (strcasecmp(keyword, NAME_MSGCHK_DELAY) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0  && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >= 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MSGCHK_DELAY = ((double)value)*1e-6;
				}
				/* MSGCHK_REPEAT */
				else if (strcasecmp(keyword, NAME_MSGCHK_REPEAT) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0  && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must >= 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MSGCHK_REPEAT = value;
				}
				/* ZEROCOPY */
				else if (strcasecmp(keyword, NAME_ZEROCOPY) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.ZEROCOPY = value;
				}
				/* REGISTER */
				else if (strcasecmp(keyword, NAME_REGISTER) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.REGISTER = value;
				}
				/* CACHE_REGISTERED */
				else if (strcasecmp(keyword, NAME_CACHE_REGISTERED) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.CACHE_REGISTERED = value;
				}
				/* CACHE_CONNECTED */
				else if (strcasecmp(keyword, NAME_CACHE_CONNECTED) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.CACHE_CONNECTED = value;
				}
				/* CONNECT_ON_INIT */
				else if (strcasecmp(keyword, NAME_CONNECT_ON_INIT) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value < 0 && value > 2 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.CONNECT_ON_INIT = value;
				}
				/* NO_VERIFY */
				else if (strcasecmp(keyword, NAME_NO_VERIFY) == 0) {
					if (MPID_SMI_cfg.VERBOSE && value != 0 && value != 1 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0 or 1\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.DO_VERIFY = !value;
				}
				/* MEMCPY_TYPE */
				else if (strcasecmp(keyword, NAME_MEMCPY_TYPE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > MPID_SMI_MAX_MEMCPY_TYPE) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be between 0 and %d\n", value, keyword, MPID_SMI_MAX_MEMCPY_TYPE);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MEMCPY_TYPE = value;
				}
				/* MEMCPYSYNC_MODE */
				else if (strcasecmp(keyword, NAME_MEMCPYSYNC_MODE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 3) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be between 0 and 3\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MEMCPYSYNC_MODE = value;
				}
				/* MEMCPYSYNC_MIN */
				else if (strcasecmp(keyword, NAME_MEMCPYSYNC_MIN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be greater 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.MEMCPYSYNC_MIN = value;
				}
				/* NC_MINSIZE */
				else if (strcasecmp(keyword, NAME_NC_MINSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.NC_MINSIZE = value;
				}
				/* NC_ALIGN */
				else if (strcasecmp(keyword, NAME_NC_ALIGN) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.NC_ALIGN = value;
				}
				/* NC_ENABLE */
				else if (strcasecmp(keyword, NAME_NC_ENABLE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && ((value < 0) || (value > 2)) && !MPID_SMI_myid) { 
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.NC_ENABLE = value;
				}
				/* ADAPTER_MODE */
				else if (strcasecmp(keyword, NAME_ADPTMODE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 2) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be 0, 1 or 2\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						switch (value) {
						case 0:
							MPID_SMI_cfg.ADPTMODE = SMI_ADPT_DEFAULT;
							break;
						case 1:
							MPID_SMI_cfg.ADPTMODE = SMI_ADPT_IMPEXP;
							break;
						case 2:
							MPID_SMI_cfg.ADPTMODE = SMI_ADPT_SMP;
							break;
						}
				}		
				/* ALLOC_MINSIZE */
				else if (strcasecmp(keyword, NAME_ALLOC_MINSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be greater 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.ALLOC_MINSIZE = value;
				}
				/* ALLOC_POOLSIZE */
				else if (strcasecmp(keyword, NAME_ALLOC_POOLSIZE) == 0) {
					if (MPID_SMI_cfg.VERBOSE && (value < 0) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be greater 0\n", value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.ALLOC_POOLSIZE = value;
				}
				/* RESOURCE_SCHEDULING */
				else if (strcasecmp(keyword, NAME_RESOURCE_SCHEDULING) == 0) {
					if (MPID_SMI_cfg.VERBOSE && ((value < 0)||(value >= SCHED_DUMMY)) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be between 0 and %d\n", 
								 value, keyword, SCHED_DUMMY);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.RESOURCE_SCHED = (MPID_SMI_rsrc_sched_strategy_t)value;
				}
				/* GAP_LATENCY */
				else if (strcasecmp(keyword, NAME_GAP_LTNCY) == 0) {
#ifdef MPID_DEBUG_ALL  
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.PERF_GAP_LTNCY = value;
#else
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s switch: only available in DEBUG mode (see configure options)\n", 
								 keyword);
					}
#endif
				}
				/* SEND_LATENCY */
				else if (strcasecmp(keyword, NAME_SEND_LTNCY) == 0) {
#ifdef MPID_DEBUG_ALL  
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, keyword);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.PERF_SEND_LTNCY = value;
#else
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s switch: only available in DEBUG mode (see configure options)\n", 
								 keyword);
					}
#endif
				}
				/* RECV_LATENCY */
				else if (strcasecmp(keyword, NAME_RECV_LTNCY) == 0) {
#ifdef MPID_DEBUG_ALL  
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, NAME_RECV_LTNCY);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.PERF_RECV_LTNCY = value;
#else
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s switch: only available in DEBUG mode (see configure options)\n", 
								 keyword);
					}
#endif
				}
				/* BANDWIDTH_LIMIT */
				else if (strcasecmp(keyword, NAME_BW_LIMIT) == 0) {
#ifdef MPID_DEBUG_ALL  
					if (MPID_SMI_cfg.VERBOSE && value < 0 && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0\n", 
								 value, NAME_BW_LIMIT);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.PERF_BW_LIMIT = value;
#else
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s switch: only available in DEBUG mode (see configure options)\n", 
								 keyword);
					}
#endif
				}
				/* BANDWIDTH_REDUCE */
				else if (strcasecmp(keyword, NAME_BW_REDUCE) == 0) {
#ifdef MPID_DEBUG_ALL  
					if (MPID_SMI_cfg.VERBOSE && (value < 0 || value > 100) && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   Illegal value %d for %s switch - must be >= 0 amd <= 100\n", 
								 value, NAME_BW_REDUCE);
						fprintf (stdout, "   -> running with default setting\n");
					} else
						MPID_SMI_cfg.PERF_BW_REDUCE = value;
#else
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						PRINT_CONFIG_WARNING;
						fprintf (stdout, "   %s switch: only available in DEBUG mode (see configure options)\n", 
								 keyword);
					}
#endif
				}
				/* unrecognized entry */
				else {
					if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
						fprintf (stdout, "*** SCI-MPICH warning: \n");
						fprintf (stdout, "    Ignoring unknown keyword `%s` in device configuration `%s`, line %d\n", 
								 keyword, devconf_name, line_nbr);
					}
				}
			}
			/* illegal syntax */
			else {
				if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) {
					fprintf (stdout, "*** SCI-MPICH warning: \n");
					fprintf (stdout, "    Illegal syntax in device configuration `%s`, line %d\n", 
							 devconf_name, line_nbr);
				}
			}
		}
		fclose(config_file_handler);
		fflush(stdout);
    }
    
    /* determine a usable number of control packet slots / short message buffers */
    if (MPID_SMI_SHORTBUFS == 0) {
		MPID_SMI_SHORTBUFS = MPID_SMI_SHORTBUFS_DEF;
		if (MPID_SMI_SHORTBUFS*MPID_SMI_SHORTSIZE > MPID_SMI_PAGESIZE) 
			MPID_SMI_SHORTBUFS = MPID_SMI_PAGESIZE/MPID_SMI_SHORTSIZE - 1;
    }
    
    /* the number of shortbufs must not be a multiple of the ID counter */
    value = MPID_SMI_SHORTBUFS / MPID_SMI_SHORTID;
    if (value * MPID_SMI_SHORTID == MPID_SMI_SHORTBUFS)
		MPID_SMI_SHORTBUFS--;

    /* adjust the watchdog to a reasonable value if no specific value
       has been choosen */
    if (MPID_SMI_cfg.USE_WATCHDOG == -MPID_SMI_WATCHDOG_DEF) {
		MPID_SMI_cfg.USE_WATCHDOG = MPID_SMI_WATCHDOG_DEF 
			+ MPID_SMI_numids*MPID_SMI_WATCHDOG_INC_PER_PROC;
		/* This call may fail if SMI has watchdog disabled. */
		SMI_Watchdog( MPID_SMI_cfg.USE_WATCHDOG);
    }

	/* Zerocopy and registering do not (yet) make sense without DMA. For DMA transfers
	   in collective operations, zerocopy/registering is not required (yet).
	   XXX: This may change with new SCI driver capabilities! */
	if (!MPID_SMI_cfg.USE_DMA_PT2PT) {
		MPID_SMI_cfg.REGISTER = 0;
		MPID_SMI_cfg.ZEROCOPY = 0;
	}

	/* Limiting *both*, the number of sends *and* recvs, may lead to deadlocks - warn the user! */
	if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid
		&& MPID_SMI_cfg.MAX_RECVS != 0 && MPID_SMI_cfg.MAX_SENDS != 0) {
		fprintf (stdout, "*** SCI-MPICH warning: \n");
		fprintf (stdout, "    Limiting the number of concurrent send *and* recvs at the same time\n");
		fprintf (stdout, "    may lead to deadlocks (MAX_RECVS and MAX_SENDS keywords).\n");
	}
	/* If the maximum nbr of concurrent sends or recvs is set to 0, it
	   means 'unlimited' number of sends and recvs. */
	if (MPID_SMI_cfg.MAX_RECVS == 0)
		MPID_SMI_cfg.MAX_RECVS = INT_MAX;
	if (MPID_SMI_cfg.MAX_SENDS == 0)
		MPID_SMI_cfg.MAX_SENDS = INT_MAX;
	
	/* adjust the rendez-vous parameters if necessary */
	if (MPID_SMI_RNDVBLOCK * MPID_SMI_RNDVRECEIPT >= MPID_SMI_RNDVSIZE)
		MPID_SMI_RNDVRECEIPT = MPID_SMI_RNDVSIZE/MPID_SMI_RNDVBLOCK >> 2;
	
	/* You get around 1/4 from rndv_pool. If RNDV_SIZE is to low, decrease PIPE_BLOCKSIZE.
	   Otherwise you have one big block in pipelined broadcast */
	MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE = MPID_MIN(MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE * MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS,
												MPID_SMI_RNDVSIZE / 4)  / MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS;
	MPID_SMI_STREAMBUF_ALIGN(MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE);
	
	/* The DMA variant of the pcast protocol does only work for minimum messsage
	   lenght > 2*PCAST_BLOCKSIZE */
	MPID_SMI_cfg.COLL_PIPE_DMA_MIN = MPID_MAX(MPID_SMI_cfg.COLL_PIPE_DMA_MIN,
											  2*MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE);
	
	/* Check if direct-reduce is safe for eager and rendez-vous transfers. */
	MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK = MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK && 
		!MPID_SMI_cfg.USE_DMA_PT2PT && !MPID_SMI_EAGER_DYNAMIC && !MPID_SMI_RNDVBLOCKING_MODE;

	if (MPID_SMI_cfg.COLL_ALLTOALL_MIN < 0) 
		MPID_SMI_cfg.COLL_ALLTOALL_MIN = MPID_SMI_EAGERSIZE;

	/* We can not use the shared-memory barrier if we are not polling (but blocking) 
	   for new messages. */
	if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
		MPID_SMI_cfg.COLL_BARRIER = 0;
	}

    FREE(devconf_name);
    
    return;
}


void MPID_SMI_resetGetOpt( void ) {
    opterr = 1;
    optind = 1;
    optopt = 0;
    optarg = 0;
}

int MPID_SMI_long_len( int partner_devlrank )
{
	return MPID_SMI_long_len_value;
}

int MPID_SMI_vlong_len( int partner_devlrank )
{
	return MPID_SMI_vlong_len_value;
}


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
