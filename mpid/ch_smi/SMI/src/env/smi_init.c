/* $Id$ */
#ifdef WIN32
/* #include <fcntl.h> */

/* We have to avoid to include fcntl.h here, because it seems to conflict with 
   the unix definitions */
/* stefan: that's not true */
#define O_CREAT        0x0100  /* create and open file */
#define O_TRUNC        0x0200  /* open and truncate */
#define O_EXCL         0x0400  /* open only if file doesn't already exist */
#define O_WRONLY       0x0001  /* open for writing only */
/*#include <stdio.h>*/
#include <io.h>
#include <getopt.h>
#include <process.h>
#endif

#include "smi_init.h"
#include "setup.h"
#include "smidebug.h"
#include "startup/startup.h"
#include "memory/shmem.h"
#include "message_passing/setup_comm.h"
#include "synchronization/sync_init.h"
#include "regions/create_shreg.h"
#include "dyn_mem/dyn_mem.h"
#include "synchronization/barrier.h"
#include "synchronization/store_barrier.h"
#include "switch_consistency/init_switching.h"
#include "proc_node_numbers/first_proc_on_node.h"
#include "utility/smi_time.h"
#include "utility/query.h"
#include "loop_scheduling/loop_interface.h"
#include "proper_shutdown/watchdog.h"
#include "proper_shutdown/resource_list.h"
#include "regions/memtree.h"
#include "utility/statistics.h"
#include "regions/internal_regions.h"
#include "env/error_count.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define TEST_LOCAL_SYNC 0

static boolean _is_smi_statistics_initialized = FALSE;
static int _smi_use_signals = TRUE;

static void _smi_resetGetOpt(void)
{
  opterr = 1;
  optind = 1;
  optopt = 0;
  optarg = 0;
}

static int _smi_get_args(int* argc, char*** argv, smi_args_t* pArgs)
{
  REMDSECTION("get_arg");
  int n, i;
  int iLastOpt = 0;
  int optG=0, optI=0,optR=0,optF=0,optN=0,optL=0,optH=0,optM=0,optP=0,optS=0,optT=0,optT_=0,optW=0,optV=0;
  char c;

  *pArgs = _smi_default_args;
  
  strcpy(pArgs->szExecName,(*argv)[0]);
  pArgs->szPIDfile[0] = '\0';
  DNOTICES("szExeName:",pArgs->szExecName);
  
  while((c = getopt(*argc, *argv, "i:r:n:f:h:m:p:lsvwtTg")) != EOF)
    switch(c) {
    case 'i':
      DNOTICE("Option -i found");
      pArgs->iSCIId = atoi(optarg);
      DNOTICEI("SCIId has been set to",pArgs->iSCIId);
      optI++;
      break;

    case 'r':
      DNOTICE("Option -r found");
      pArgs->iProcRank = atoi(optarg);
      DNOTICEI("ProcRank has been set to",pArgs->iProcRank);
      optR++;
      break;

    case 'n': 
      DNOTICE("Option -n found");
      pArgs->iNumProcs = atoi(optarg);
      DNOTICEI("NumProcs has been set to",pArgs->iNumProcs);
      optN++;
      break;

    case 'f':
      DNOTICE("Option -f found");
      strcpy(pArgs->szPIDfile, optarg);
      DNOTICES("PID file is named",pArgs->szPIDfile); 
      optF++;
      break;

    case 'm':
      DNOTICE("Option -m found");
      pArgs->iMagicNumber = atoi(optarg);
      DNOTICEI("MagicNumber has been set to",pArgs->iMagicNumber);
      optM++;
      break;

     case 'p':
      DNOTICE("Option -p found");
      pArgs->iPortNumber = atoi(optarg);
      DNOTICEI("PortNumber has been set to",pArgs->iPortNumber);
      optP++;
      break;  
      
    case 'h':
      DNOTICE("Option -h found");
      strcpy(pArgs->szSyncHost, optarg);
      DNOTICES("SyncHost is named",pArgs->szSyncHost);
      optH++;
      break;

    case 'l':
      DNOTICE("Option -l found");
      optL++;
      break;

    case 's':
      DNOTICE("Option -s found");
      optS++;
      break;

    case 't':
      DNOTICE("Option -t found");
      optT++;
      break;
    
    case 'T':
      DNOTICE("Option -T found");
      optT_++;
      break;
    
    case 'v':
      DNOTICE("Option -v found");
      optV++;
      break;

    case 'w':
      DNOTICE("Option -w found");
      optW++;
      break;

    case 'g':
      DNOTICE("Option -g found");
      optG++;
      break;

    default:
      DNOTICE("Illegal Option found");
    } 

  pArgs->eStartupMethod = smi_startup_use_tcp;
 
  ASSERT_R(optR > 0,"Missing definition of process rank (-r)",-1);
  ASSERT_R(optN > 0,"Missing definition of number of processes (-n)",-1); 

  ASSERT_R(optH > 0,"Missing definition of SyncHost (-h)",-1); 
  ASSERT_R(optM > 0,"Missing definition of MagicNumber (-m)",-1); 
  ASSERT_R(optH < 2,"Multiple definition of SyncHost (-h)",-1); 
  ASSERT_R(optP < 2,"Multiple definition of PortNumber (-p)",-1); 
  ASSERT_R(optM < 2,"Multiple definition of MagicNumber (-m)",-1); 
  ASSERT_R(strlen(pArgs->szSyncHost)>0, "Definition of SyncHoste without hostname", -1); 
  
  ASSERT_R(optI < 2,"Multiple definition of SCIId (-i)",-1);
  ASSERT_R(optR < 2,"Multiple definition of ProcRank (-r)",-1);
  ASSERT_R(optN < 2,"Multiple definition of NumProcs (-n)",-1);
  ASSERT_R(optL < 2,"Multiple definition of locality (-l)",-1);
  ASSERT_R(optS < 2,"Multiple definition of smidebug (-s)",-1);
  ASSERT_R(optT < 2,"Multiple definition of statistics (-t)",-1);
  ASSERT_R(optV < 2,"Multiple definition of verbose (-v)",-1);
  ASSERT_R(optW < 2,"Multiple definition of watchdog (-w)",-1);
  
  for (i=0;i<(*argc);i++) {
    if (strcmp("--",(*argv)[i])==0) {
      iLastOpt=i;
      break;
    }
  }

  ASSERT_R((iLastOpt>0),"Missing '--'",-1);
  
  if (optL) {
      pArgs->iNumProcs = -abs(pArgs->iNumProcs);
      _smi_all_on_one = TRUE;
  } else {
      _smi_all_on_one = FALSE;
  }
  
  if (optS) {
      SMI_Debug(TRUE);
  }

  if (optV)
      _smi_verbose = TRUE;

  if (optW)
      _smi_use_watchdog = FALSE;

  if (optG)
      _smi_use_signals = FALSE;

  if (optT)
      _is_smi_statistics_initialized = TRUE;

  if (optT_)
      SMI_Debug_time(TRUE);

  for(n = 1; (n+iLastOpt) < *argc; n++)
      (*argv)[n] = (*argv)[n + iLastOpt];
  (*argc) -= (iLastOpt);

  _smi_resetGetOpt();

  return(0);
}


static int _smi_write_pidfile(char *pidfile)
{
  int fh, rc;
  int rval = SMI_SUCCESS;
  char pidval[32];

  if (pidfile != NULL && strlen(pidfile) > 0) {
    fh = rs_CreateTempfile (pidfile, O_CREAT |O_EXCL | O_WRONLY);
    if (fh == -1) {
      DERRORI("Could not open PID file, errno", errno);
      return SMI_ERR_OTHER;
    }
    sprintf (pidval, "%d", getpid());
    rc = write (fh, pidval, (unsigned int) strlen(pidval));
    close (fh);
    if (rc != strlen(pidval)) {
	DERRORI("Could not write to PID file, errno", errno);
	return SMI_ERR_OTHER;
    }
    DNOTICES("Wrote PID to", pidfile);
  }

  return rval;
}


/* initialize the SMI run-time environment with all its data 
   structures and the basic MPI-like communication mechanisms */
smi_error_t SMI_Init_direct(smi_args_t *pArgs) {
    smi_args_t sArgs;
    smi_error_t error;
    int i;
    char *tmp;
    
    DSECTION ("SMI_Init_direct");
    DSECTENTRYPOINT;
    
    /* Initialize only, if not already done */
    
    if (_smi_initialized == false) {
	/* set it right now because some functions that will be called during init
	   depend on it */
	_smi_initialized = true;       
	
	/* create lock for mis-structure */
	SMI_INIT_LOCK(&_smi_mis_lock);
	
	_smi_init_signal_handler();
	
	memcpy(&sArgs, pArgs, sizeof(smi_args_t));
	
	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, SMI_VERSION);
#if defined(_DEBUG)
	    fprintf (stdout, " (DEBUG)");
#else
	    fprintf (stdout, " (RELEASE)");
#endif	    
	    fprintf (stdout, "\n");fflush(stdout);
	}

	error = _smi_init_resource_list();
	ASSERT_X(error == SMI_SUCCESS,"_smi_init_resource_list failed",error);

	error = _smi_write_pidfile(sArgs.szPIDfile);
	ASSERT_X(error == SMI_SUCCESS,"_smi_write_pidfile failed", error);

	error = _smi_init_query(sArgs.iNumProcs);
	ASSERT_X(error == SMI_SUCCESS,"_smi_init_query failed",error);

	_smi_init_timer();
        DTIMERESET;    

	/* initialize the low-level communication layer */
	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, "  Lowlevel Init - "); fflush (stdout);
	}
	error = _smi_lowlevel_init(&sArgs);
	ASSERT_X(error == MPI_SUCCESS, "_smi_lowlevel_init failed",1000+error);
    	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, "ok.\n"); fflush (stdout);
	}

	/* initialize subsystems and global variables */	
	error = _smi_get_no_processes();
	ASSERT_X(error == SMI_SUCCESS,"_smi_get_no_processes failed",error);
	
	error = _smi_get_loc_mpi_rank();
	ASSERT_X(error == SMI_SUCCESS,"_smi_get_loc_mpi_rank failed",error);
	
	error = _smi_set_ranks();
	ASSERT_X(error == SMI_SUCCESS,"_smi_set_ranks failed",error);

	error = _smi_get_pids();
	ASSERT_X(error == SMI_SUCCESS,"_smi_get_pids failed",error);

	error = _smi_set_no_machines();
	ASSERT_X(error == SMI_SUCCESS,"_smi_set_no_machines failed",error); 
	
	error = _smi_determine_closeness();
	ASSERT_X(error == SMI_SUCCESS,"_smi_determine_closeness failed",error); 
	
	error = _smi_get_page_size();
	ASSERT_X(error == SMI_SUCCESS,"_smi_get_page_size failed",error); 
	
	error = _smi_init_shared_segment_subsystem();
	ASSERT_X(error == SMI_SUCCESS,"_smi_init_shared_segment_subsystem failed",error); 

	/* initialize the low-level communication layer */
	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, "  Topology Init - "); fflush (stdout);
	}
	error = _smi_topology_init();
	ASSERT_X(error == SMI_SUCCESS, "_smi_init_topology failed",error); 
    	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, "ok.\n"); fflush (stdout);
	}
	if (_smi_verbose & !sArgs.iProcRank) {
	    int topo_type, ndims, dimextnt;
	    char topology[256], tmp1[16];
	    
	    fprintf (stdout, "  Running %d procs on %d nodes:\n", _smi_nbr_procs, _smi_nbr_machines);
	    
	    SMI_Query (SMI_Q_SCI_SCI_TOPOLOGY_TYPE, 0, &topo_type);
	    switch (topo_type) {
	    case SMI_SCI_TOPOLOGY_UNKNOWN:
		sprintf (topology, "UNKNOWN");
		break;
	    case SMI_SCI_TOPOLOGY_SWITCH:
		sprintf (topology, "Switch");
		break;
	    case SMI_SCI_TOPOLOGY_TORUS:
		SMI_Query (SMI_Q_SCI_NBR_PHYS_DIMS, 0, &ndims);
		sprintf (topology, "%dD-torus ( ", ndims);
		for (i = 0; i < ndims; i++) {
		    SMI_Query (SMI_Q_SCI_PHYS_DIM_EXTENT, i, &dimextnt);
		    if (i + 1 < ndims)
			sprintf(tmp1, "%d X ", dimextnt);
		    else 
			sprintf(tmp1, "%d )", dimextnt);
		    strcat(topology, tmp1);
		}
		break;
	    case SMI_SCI_TOPOLOGY_SMP:
		sprintf (topology, "SMP");
		break;
	    }
	    fprintf (stdout, "    phys. SCI topology: %s\n", topology);
	    
	    if (topo_type == SMI_SCI_TOPOLOGY_TORUS) {
		SMI_Query (SMI_Q_SMI_NBR_APP_DIMS, 0, &ndims);
		sprintf (topology, "%dD-torus ( ", ndims);
		for (i = 0; i < ndims; i++) {
		    SMI_Query (SMI_Q_SMI_APP_DIM_EXTENT, i, &dimextnt);
		    if (i + 1 < ndims)
			sprintf(tmp1, "%d X ", dimextnt);
		    else 
			sprintf(tmp1, "%d )", dimextnt);
		    strcat(topology, tmp1);
		}
		fprintf (stdout, "    virt. SCI topology: %s\n", topology);
	    }	    
	}

	_smi_statistics_init();
	_smi_set_statistics(_is_smi_statistics_initialized);

	error = _smi_init_error_counter(_smi_nbr_machines);
	ASSERT_X(error == SMI_SUCCESS,"_smi_init_error_counter failed",error); 
	
#ifndef SMI_NOCPP
	error = SMI_Loop_scheduling_init();
	ASSERT_X(error == SMI_SUCCESS,"SMI_Loop_scheduling_init failed",error); 
#endif
	
	error = _smi_ll_barrier();
	ASSERT_X(error == MPI_SUCCESS,"_smi_ll_barrier failed",1000+error);

	/* the watchdog had to be initialized with the sync ranks, but we want to use
	   the real ranks */
	if (_smi_use_watchdog) {
	    _smi_wd_set_rank(_smi_my_proc_rank);
	    _smi_wd_enable(WD_TICKS_TIMEOUT_DEF);
	}

	/* start synchronization and sync afterwards */
	error = _smi_synchronization_init(_smi_use_signals);
	ASSERT_X(error == SMI_SUCCESS, "_smi_synchronization_init failed", error);

	error = _smi_ll_barrier();
	ASSERT_X(error == SMI_SUCCESS,"_smi_lI_barrier failed", error);
	
	/* locks for mutithreaded usage */
	SMI_INIT_LOCK(&_smi_region_lock);

	/* Install a first, single shared memory segment on each machine which 
	   will be used for internal purposes dor dynamic memory allocation.   
	   Initialize a memory manager for each and allocate a lock to protect 
	   the memory managers data structures afterwards.                     */
	_smi_mis.no_regions = 0;
	_smi_mis.nbr_user_regions = 0;
	_smi_memtree = _smi_memtree_empty();
	
	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, "  Internal Segments"); fflush (stdout);
	}
	/* The size of the internal shared memory regions needs to be scaled
	   up for increasing numbers of processes to satisfy increasing
	   memory requirements (number of locks, etc.). It is scaled linearly 
	   with a granularity of INT_SHMSEG_SCALE processes. */
#if 1
	error = _smi_create_internal_regions(((_smi_nbr_procs - 1)/INT_SHMSEG_SCALE + 1)
					     *INT_SHMSEG_BASESIZE);
#else
	/* As a bugfix, this mechanism is disabled and the size of the internal shared
	   memory region is set to 32K. The memory management seems to have problems
	   with offset greater than 32K into these regions. */
	error = _smi_create_internal_regions(INT_SHMSEG_BASESIZE);
#endif

	ASSERT_X(error == SMI_SUCCESS, "_smi_create_internal_regions failed",error); 
	
	/* before any SMI synchronization mechanisms are used that require */
	/* load and/or store barriers: initialize them                     */
	_smi_init_load_store_barriers();

	/* the mutex for each of the regions to guarantee mutual exclusion if */
	/* different processes concurrently try to allocate a piece of memory */
	/* can only be allocated after all shared region have been installed  */
	/* and initialized regarding all other stuff, because the             */
	/* SMI_Mutex_init already performs CMalloc`s.                         */
	for (i = 0; i < _smi_nbr_machines; i++) { 
	  DNOTICEI("allocating MMU lock for machine",i);
	  error = _smi_allocate_MMU_lock(_smi_int_shreg_id[i]);
	  ASSERT_X(error == SMI_SUCCESS, "_smi_allocate_MMU_lock failed",error);
	}
	if (_smi_verbose & !sArgs.iProcRank) {
	    fprintf (stdout, " - ok.\n"); fflush (stdout);
	}
    }
   
    /* start barriers */
    if (_smi_verbose & !sArgs.iProcRank) {
      fprintf (stdout, "  Barrier & Mutex Init - "); fflush (stdout);
    }
    DNOTICE ("initialize barriers");
    error = SMI_BARRIER_INIT((int*)&tmp, PROGRESS_COUNTER_BARRIER, TRUE);
    ASSERT_X(error == SMI_SUCCESS,"barrier_init failed",error);    
    if (_smi_verbose & !sArgs.iProcRank) {
      fprintf (stdout, "ok.\n"); fflush (stdout);
    }

#ifndef SMI_NOCPP
    /* init the module that is concerned with switching */
    /* between different modes of consistency           */
    error = _smi_init_switching();
    ASSERT_X(error == SMI_SUCCESS,"_smi_init_switching failed",error);
#endif
 
    /* initialize memcpy module */
    error = _smi_memcpy_init();
    ASSERT_X(error == SMI_SUCCESS,"_smi_memcpy_init failed",error);

    /* initialize message-passing module */
    error = _smi_init_mp(); 
    ASSERT_X(error == SMI_SUCCESS,"_smi_init_mp failed",error);

    DNOTICE("*** SMI_Init complete, setup information:");
    DNOTICEI("   pagesize         :", _smi_page_size);
    DNOTICEI("   nbr of processes :", _smi_nbr_procs);
    DNOTICEI("   this procs's rank:", _smi_my_proc_rank);
    DNOTICEI("   nbr of nodes     :", _smi_nbr_machines);
    DNOTICEI("   this node's rank :", _smi_my_machine_rank);
    DNOTICE("*****************************************");


#if TEST_LOCAL_SYNC
    /* additionally allocate an unix shared memory segment, for node-local operations */
    _smi_create_local_region(INT_SHMSEG);
#endif    

    if (_smi_verbose & !sArgs.iProcRank) {
	fprintf (stdout, "SMI Library initialized.\n"); fflush (stdout);
    }

    DSECTLEAVE;
    return(SMI_SUCCESS);
}

smi_error_t SMI_Init(int* argc, char*** argv) {
    smi_args_t sArgs;
    smi_error_t error;
    int i;
    char *tmp;
    
    if (_smi_initialized == false) {
	if (_smi_get_args(argc,argv, &sArgs) == -1) {
	    fprintf(stderr, "This is an SMI application. It should be started via a script like smirun.\n");
	    fprintf(stderr, "Startup options for manual startup are:\n");
	    fprintf(stderr, " %s -n numprocs -r rank -h synchost -m magicnumber [SMI args] -- [application args]\n",(*argv)[0]);
	    exit(0);
	}
    }
    
    return SMI_Init_direct(&sArgs);
}

