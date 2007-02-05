/*
 * $Id$
 *
 * (C) 1995 by Argonne National Laboratory and Mississipi State University. All
 * rights reserved.  See COPYRIGHT in top-level directory.
 *
 * Metacomputing code (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 *
 */

#include <stdio.h>

#ifdef CH_GM_PRESENT
#include <unistd.h>
#include "../ch_gm/gmpi.h"
#endif /* CH_GM */

#if (_MSC_VER >=1400)
#pragma warning (disable:4996)
#endif

#include "mpi.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "../../src/coll/coll.h"
#include "../util/cmnargs.h"
#include "../util/queue.h"
#include "reqalloc.h"
#include "mpid_debug.h"
#include "adi2config.h"
#include "../../src/routing/rdebug.h"
#include "calltrace.h"

#include "patchlevel.h"

#ifdef META
#include "metampi.h"
#include "../../src/routing/rhlist.h"
#endif /*  /META */

#include "multidevice.h"



/* Home for these globals */
int MPID_MyWorldSize = -1, MPID_MyWorldRank = -1;
int MPID_MyHostSize  = -1, MPID_MyHostRank  = -1;
int MPID_MyLocalSize = -1, MPID_MyLocalRank = -1;
int MPID_MyMetaSize  = -1, MPID_MyMetaRank  = -1;
int MPID_MyAllSize   = -1, MPID_MyAllRank   = -1;

/* flag to indicate an -mpichtv-switch */
int mpichtv_flag = 0;

#ifdef META

extern MPID_Device **MPID_Tunnel_native_dev;

/* uncomment if you want to use the meta_barrier in this file (only for development purposes) */
/* extern int      meta_barrier; */

#endif /* META */

int             MPID_Print_queues = 0;
MPID_SBHeader   MPIR_rhandles;
MPID_SBHeader   MPIR_shandles;

/* This is a prototype for this function used to provide a debugger hook */
void *          MPIR_Breakpoint(void);

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are out-standing                                      */
/***************************************************************************/
int             MPID_n_pending = 0;
/*
 * Create the MPID_DevSet device from the requested devices, and initialize
 * the device mapping
 */

/*
 * This COULD be a single piece of permanent storage, but that is awkward for
 * shared-memory versions (hot-spot-references).
 */
MPID_DevSet    *MPID_devset = 0;

#ifdef META
void debug_print_devset();
#endif


int             MPID_Complete_pending(void);

static int      MPID_Short_len = -1;

#ifdef CH_GM_PRESENT
static MPID_Config * gm_config_info_copy;

extern int MPID_GM_rank;
extern int MPID_GM_size;

/* This is a prototype for this function used to get the configuration */
MPID_Config * MPID_GM_GetConfigInfo (int *, char *** );

/* ch_gm gets a special init function, because its initialization differs from the other devices' */
int MPID_GM_special_init( int *, char *** );

/* for ch_gm, there must be something done after the initialization */
void MPID_GM_post_init( int *, char *** );

/* in contrast to the other "native" device types, ch_gm initializes multiple devices instead of just one */
MPID_DevSet    *MPID_GM_devset = 0;
#endif /* CH_GM */

#ifdef CH_WSOCK_PRESENT
/*  mixed usage:
	mixed is set to true if the number of processors is smaller than MPID_numids
	i.e. two processes on a doubleprocessor means mixed is false
*/
int mixed;
extern unsigned long *Global2Local;
extern int MPID_numids;
#endif

void
MPID_Init(argc, argv, config, error_code)
    int            *argc, *error_code;
    void           *config;
    char         ***argv;
{
    int             i = 0;
    MPID_Device    *dev = NULL;
    MPID_Config    *config_info = (MPID_Config *) config;
    int use_ch_gm = 0;  /* are we running with ch_gm ? */
    int symbol_res_error = 0;
    MPID_Device * (*InitMsgPassPt)(int *, char ***, int, int); /* pointer to device initialization function */
#ifdef CH_GM_PRESENT
    int dev_type = 0;
    int dev_rank = 0;
#endif

    TR_PUSH("MPID_Init");

#ifdef CH_SMI_PRESENT
    TR_MSG("CH_SMI_PRESENT defined");
#endif
#ifdef CH_SHMEM_PRESENT
    TR_MSG("CH_SHMEM_PRESENT defined");
#endif
#ifdef CH_P4_PRESENT
    TR_MSG("CH_P4_PRESENT defined");
#endif
#ifdef CH_USOCK_PRESENT
    TR_MSG("CH_USOCK_PRESENT defined");
#endif
#ifdef CH_MPX_PRESENT
    TR_MSG("CH_MPX_PRESENT defined");
#endif
#ifdef CH_GM_PRESENT
    TR_MSG("CH_GM_PRESENT defined");

    /* the initialization procedure for ch_gm is quite different from the other devices; meaning that ch_gm
       was built into the libray does not mean that we are running with ch_gm; therefore use_ch_gm indicates whether we
       are actually using ch_gm in this run */
    if( MPID_selected_primary_device == DEVICE_ch_gm_nbr )
	use_ch_gm = 1;
#endif

    MPID_Init_queue();

    /* Initialize the send/receive handle allocation system */
    /*
     * Use the persistent version of send/receive (since we don't have
     * separate MPIR_pshandles/MPIR_prhandles)
     */
    MPIR_shandles = MPID_SBinit(sizeof(MPIR_PSHANDLE), 100, 100);
    MPIR_rhandles = MPID_SBinit(sizeof(MPIR_PRHANDLE), 100, 100);
    /*
     * Needs to be changed for persistent handles.  A common request
     * form?
     */

	  /*
       * check for verbose startup
       * this is necessary to be able to have verbose output before MPIR_Init has been called
       */
      for (i=1; i<*argc; i++) {
      	if ((*argv)[i]) {
			if (strcmp((*argv)[i],"-mpichtv" ) == 0) {
				(*argv)[i] = 0; /* Eat it up so the user doesn't see it */
				mpichtv_flag = 1;
			}
#ifdef META
			else if (strcmp((*argv)[i],"-metaverbose" ) == 0) {
			    RDEBUG_verbose = 1;
			}
#endif /*META*/
      	}
      }

      MPID_ArgSqueeze( argc, *argv );


    if (config_info) {
	/* config_info != NULL means that we are running a meta computer (with at least two meta hosts) */

#ifdef META
      MPID_Config    *p;
      int global_rank, metahost_rank;
      MPID_Device *primary_dev;

      if (RDEBUG_verbose){
	  	strcpy(RDEBUG_dbgprefix, "[");
	  	strcat(RDEBUG_dbgprefix, MPIR_meta_cfg.my_metahostname);
	  	strcat(RDEBUG_dbgprefix, "|" );
	  	strcat(RDEBUG_dbgprefix, MPIR_meta_cfg.nodeName);
	  	strcat(RDEBUG_dbgprefix, "]" );
      }

      if( use_ch_gm ) {
#ifdef CH_GM_PRESENT
	  /* ch_gm is a very special case */
	  MPID_GM_special_init( argc, argv );
	  MPID_devset = MPID_GM_devset;

	  MPID_GM_post_init( argc, argv );

	  MPID_devset = (MPID_DevSet *) MALLOC(sizeof(MPID_DevSet));
	  if (!MPID_devset) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  /* Make devset safe for initializations errors */
	  MPID_devset->req_pending = NULL;
	  MPID_devset->dev_list = NULL;
	  MPID_devset->active_dev = NULL;

	  /* the native devices of ch_gm and the gateway device, these
	     numbers are later increased for the routers, which use ch_tunnel, too  */
	  MPID_devset->ndev      = MPID_GM_devset->ndev + 1;
	  MPID_devset->ndev_list = MPID_GM_devset->ndev_list + 1;

	  MPID_devset->dev = (MPID_Device **) MALLOC( (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) * sizeof(MPID_Device *) );
	  if (!MPID_devset->dev) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  /* pointers to native devices for processes running on this metahost */
	  global_rank = MPIR_meta_cfg.metahost_firstrank;
	  for( metahost_rank = 0; metahost_rank < MPID_MyWorldSize; metahost_rank++ ) {
	      MPID_devset->dev[global_rank] = MPID_GM_devset->dev[metahost_rank];
	      global_rank++;
	  }

	  /* list of different devices */
	  MPID_devset->dev_list = MPID_GM_devset->dev_list;


	  /* for each ch_gm device we have to change the mapping from global ranks to device ranks */
	  primary_dev = MPID_devset->dev_list;
	  while( primary_dev ) {
	      FREE( primary_dev->grank_to_devlrank); /* old mapping */
	      primary_dev->grank_to_devlrank = (int *)MALLOC( (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) * sizeof(int) );
	      global_rank = MPIR_meta_cfg.metahost_firstrank;
	      for( metahost_rank = 0; metahost_rank < MPID_MyWorldSize; metahost_rank++ ) {
		  primary_dev->grank_to_devlrank[global_rank] = metahost_rank;
		  global_rank++;
	      }
	      primary_dev = primary_dev->next;
	  }

	  primary_dev = MPID_devset->dev_list;
	  p = config_info;
#endif /* CH_GM_PRESENT */
      }
      else {
	  /* initialization of primary device when not running ch_gm */

	  MPID_devset = (MPID_DevSet *) MALLOC(sizeof(MPID_DevSet));
	  if (!MPID_devset) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  /* Make devset safe for initialization errors */
	  MPID_devset->req_pending = NULL;
	  MPID_devset->dev_list = NULL;
	  MPID_devset->active_dev = NULL;

	  /* the native device and the gateway device, these numbers are later increased for
	     the routers, which use ch_tunnel, too  */
	  MPID_devset->ndev = 2;
	  MPID_devset->ndev_list = 2;

	  MPID_devset->dev = (MPID_Device **) MALLOC( (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) * sizeof(MPID_Device *) );
	  if (!MPID_devset->dev) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  p = config_info;

	  /* **************
	     PRIMARY DEVICE
	     ************** */

	  /* initialize device */
	  PRVERBOSE("Initializing primary device...\n");
	  dev = (p->device_init) (argc, argv, MPID_Short_len, -1);
	  if (!dev) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }
	  PRVERBOSE("Primary device initialized.\n");
	  /* needed for the initialization of ch_tunnel below */
	  primary_dev = dev;

	  /* include device in list of different devices */
	  dev->next = MPID_devset->dev_list;
	  MPID_devset->dev_list = dev;

	  /*  for any process with number j in the whole system ( rank in
	      MPI_COMM_ALL for META-enviranments ),
	      MPID_devset->dev[j] points to the device via which
	      we send messages from here to that process */
	  for (i = 0; i < p->num_served; i++)
	      MPID_devset->dev[p->granks_served[i]] = dev;

	  /* set up mapping global ranks to local device ranks and vice versa */
	  dev->grank_to_devlrank = (int *)MALLOC(sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp));
	  for( i = 0; i < p->num_served; i++ )
	      dev->grank_to_devlrank[p->granks_served[i]] = i;
      } /* end of initialization of primary device when not running ch_gm */

      /* set MPID_MyWorldRank / MPID_MyWorldSize for those devices that don't do this anymore (which
	 should be all devices in the near future) */
      if( MPID_MyWorldRank == -1 )
	  MPID_MyWorldRank = dev->lrank;
      if( MPID_MyWorldSize == -1 )
	  MPID_MyWorldSize = dev->lsize;

      /* p should now point to the primary device */

      MPIR_meta_cfg.my_routingid = -1;
      for( i = 0; i < rh_getNumRouters(); i++ ) {
	  if( routerlist[i].metahostrank == MPID_MyWorldRank )
	      MPIR_meta_cfg.my_routingid = i;
      }
      MPIR_meta_cfg.my_rank_on_metahost = MPID_MyWorldRank;

      /* now it is time to check if we are a router */
      if (!MPIR_meta_cfg.router && MPIR_meta_cfg.routerAutoCfg &&
	  (MPIR_meta_cfg.dedicated_rp ||
	   ( MPIR_meta_cfg.my_routingid != -1 ) ) ) {


	  /* this proc will become a router proc */
	  MPIR_meta_cfg.router = 1;
	  (MPID_devset->ndev)++;
	  (MPID_devset->ndev_list)++;
	  /* since this is a router, set mpichtv_flag to 0, so process will
	   * not be attached to by totalview
	   */
	  mpichtv_flag = 0;

	  /* if we also use a secondary device, p->next points to it, otherwise,
	     p->next points to the gateway device */
	  if( MPIR_meta_cfg.useSecondaryDevice )
	      p->next->next->next = &tn_device;
	  else
	      p->next->next = &tn_device;

	  MPIR_meta_cfg.my_localrank = MPID_MyWorldRank;
      }
      else {
	  /* here we calculate MPIR_meta_cfg.my_localrank; this value
	     later becomes the rank of this process in MPI_COMM_LOCAL;
	     if MPIR_meta_cfg.my_localrank = j, then this process is the
	     jth application process on this metahost */
	  int i;

	  MPIR_meta_cfg.my_localrank = 0;
	  i = 0;
	  while( i < MPIR_meta_cfg.my_rank_on_metahost ) {
	      if( !(MPIR_meta_cfg.isRouter[i]) )
		  MPIR_meta_cfg.my_localrank++;
	      i++;
	  }
      }

      /* need to get to know the other MPI procs on this machine */
      MPID_MyLocalSize = MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank];
      /* local rank is assigned from command line */
      MPID_MyLocalRank = MPIR_meta_cfg.my_localrank;

      MPID_MyHostRank = MPIR_meta_cfg.my_rank_on_metahost;
      MPID_MyHostSize = MPID_MyLocalSize + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank];

      MPID_MyMetaRank = 0;
      for (i = 0; i < MPIR_meta_cfg.my_metahost_rank; i++)
	  MPID_MyMetaRank += MPIR_meta_cfg.np_metahost[i];
      MPID_MyMetaRank += MPID_MyLocalRank;
      MPID_MyMetaSize = MPIR_meta_cfg.np;

      MPID_MyWorldRank = MPID_MyMetaRank;
      MPID_MyWorldSize = MPID_MyMetaSize;

      MPID_MyAllRank = MPIR_meta_cfg.metahost_firstrank + MPIR_meta_cfg.my_rank_on_metahost;
      MPID_MyAllSize = MPIR_meta_cfg.np + MPIR_meta_cfg.nrp;

      /* ****************
	 SECONDARY DEVICE
	 **************** */
      if( MPIR_meta_cfg.useSecondaryDevice ) {

	  /* advance pointer to secondary device */
	  p = p->next;

	  /* build rank mapping for secondary device, the result is stored in
	     MPID_SecondaryDevice_grank_to_devlrank */
	  if( MPID_buildRankMappingForSecondaryDevice() != MPI_SUCCESS ) {
	      fprintf( stderr, "MPID_buildRankMappingForSecondaryDevice() returned with an error\n" );
	      fflush( stderr );

	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  /* build fake command line arguments for secondary device and put them into the rest of the command line,
	     the result is stored in MPID_SecondaryDevice_argc and MPID_SecondaryDevice_argv */
	  if( MPID_buildSecondaryDeviceArgs( MPIR_meta_cfg.secondaryDevice, argv ) != MPI_SUCCESS ) {
	      fprintf( stderr, "MPID_buildSecondaryDeviceArgs() returned with an error\n" );
	      fflush( stderr );

	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }


	  if (RDEBUG_verbose){
	  	fprintf(stderr, "%sStarting secondary device initialization with cmdLine:", RDEBUG_dbgprefix);
	  	for (i = 0; i < MPID_SecondaryDevice_argc; i++ ) {
	  		fprintf(stderr, " %s", MPID_SecondaryDevice_argv[i]);
	  	}
	  	fprintf(stderr, "\n");
	  	fflush(stderr);
	  }

	  /* call device initialization function for secondary device with fake command line arguments */
	  dev = (p->device_init)( &MPID_SecondaryDevice_argc, &MPID_SecondaryDevice_argv, MPID_Short_len, -1 );

	  PRVERBOSE("Secondary device initialization done.\n");

	  /* include device in list of different devices */
	  dev->next = MPID_devset->dev_list;
	  MPID_devset->dev_list = dev;

	  /*  for any process with number j in the whole system ( rank in
	      MPI_COMM_ALL for META-environments ),
	      MPID_devset->dev[j] points to the device via which
	      we send messages from here to that process */
	  for( i = 0; i < p->num_served; i++ )
	      MPID_devset->dev[p->granks_served[i]] = dev;

	  /* The rank mappings for the seondary device have already been set */
	  dev->grank_to_devlrank = MPID_SecondaryDevice_grank_to_devlrank;
      } /* end of secondary device initialization */

      /* p should now point to the device before the gateway device (primary or secondary) */


      if( MPIR_meta_cfg.useRouters ) {

	  /* advance pointer to the gateway device */
	  p = p->next;

	  /* **************
	     gateway device
	     ************** */

	  /* initialize device */
	  dev = (p->device_init) (argc, argv, MPID_Short_len, -1);
	  if (!dev) {
	      *error_code = MPI_ERR_INTERN;
	      RETURN;
	  }

	  /* include device in list of different devices */
	  dev->next = MPID_devset->dev_list;
	  MPID_devset->dev_list = dev;

	  /*  for any process with number j in the whole system ( rank in
	      MPI_COMM_ALL for META-enviranments ),
	      MPID_devset->dev[j] points to the device via which
	      we send messages from here to that process */
	  for (i = 0; i < p->num_served; i++)
	      MPID_devset->dev[p->granks_served[i]] = dev;

	  /* set mapping global ranks -> local device ranks */
	  dev->grank_to_devlrank = (int *)MALLOC(sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp));
	  for (i = 0; i < (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp); i++)
	      dev->grank_to_devlrank[i] = i;

	  /* *advance pointer to the tunnel device (if any) */
	  p = p->next;


	  if( p ) {

	      /* ***************************
		 tunnel device (only routers)
		 **************************** */

	      /* initialize device */
	      dev = (p->device_init) (argc, argv, MPID_Short_len, -1);
	      if (!dev) {
		  *error_code = MPI_ERR_INTERN;
		  RETURN;
	      }

	      /* include device in list of different devices */
	      dev->next = MPID_devset->dev_list;
	      MPID_devset->dev_list = dev;

	      /* for ch_tunnel, we save the pointers to the devices on the metahost, before
		 they are overwritten later (we save all pointers including those to ch_gateway,
		 but it's only the pointers to the native devices that we are interested in)*/
	      MPID_Tunnel_native_dev = (MPID_Device **)MALLOC( sizeof(MPID_Device *) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) );
	      for( i = 0; i < MPIR_meta_cfg.np + MPIR_meta_cfg.nrp; i++ )
		  MPID_Tunnel_native_dev[i] = MPID_devset->dev[i];

	      /*  for any process with number j in the whole system ( rank in
		  MPI_COMM_ALL for META-enviranments ),
		  MPID_devset->dev[j] points to the device via which
		  we send messages from here to that process */
	      /* here, we overwrite pointers to the native device */
	      for (i = 0; i < p->num_served; i++)
		  MPID_devset->dev[p->granks_served[i]] = dev;

	      /* set mapping global ranks -> local device ranks */
	      dev->grank_to_devlrank = (int *)MALLOC(sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp));
	      for( i = 0; i < p->num_served; i++ )
		  dev->grank_to_devlrank[p->granks_served[i]] =
		      primary_dev->grank_to_devlrank[p->granks_served[i]];

	  } /* end of code for ch_tunnel */
      } /* end of initialization of ch_gateway and ch_tunnel */
#endif /* META */
    } /* end of multidevice initialization in meta case */
    else {
	/* device initialization without meta devices (ch_gateway and ch_tunnel) or secondary device */

	if( !use_ch_gm ) {

	    /*
	     * Create the device set structure.  Currently, only handles one
	     * device and maps all operations to that device
	     */
	    MPID_devset = (MPID_DevSet *) MALLOC(sizeof(MPID_DevSet));
	    if (!MPID_devset) {
		*error_code = MPI_ERR_INTERN;
		RETURN;
	    }

	    /* Make devset safe for initializations errors */
	    MPID_devset->req_pending = NULL;
	    MPID_devset->dev_list = NULL;
	    MPID_devset->active_dev = NULL;
	}

#ifdef DEBUG_TRACE
TR_stack_debug =1;
#endif

#ifndef WIN32
	switch( MPID_selected_primary_device ) {
#ifdef CH_SMI_PRESENT
	    TR_MSG("CH_SMI_PRESENT defined");
	case DEVICE_ch_smi_nbr:
#endif
#ifdef CH_SHMEM_PRESENT
	    TR_MSG("CH_SHMEM_PRESENT defined");
	case DEVICE_ch_shmem_nbr:
#endif
#ifdef CH_P4_PRESENT
	    TR_MSG("CH_P4_PRESENT defined");
	case DEVICE_ch_p4_nbr:
#endif
#ifdef CH_USOCK_PRESENT
	    TR_MSG("CH_USOCK_PRESENT defined");
	case DEVICE_ch_usock_nbr:
#endif
#ifdef CH_MPX_PRESENT
	    TR_MSG("CH_MPX_PRESENT defined");
	case DEVICE_ch_mpx_nbr:
#endif
	    /* look up initialization function */
	    InitMsgPassPt =
		( MPID_Device *(*)(int *, char ***, int, int) ) MPID_GetInitMsgPassPt( MPID_selected_primary_device,
										       &symbol_res_error );

	    if( symbol_res_error == MPI_ERR_INTERN ) {
		*error_code = MPI_ERR_INTERN;
		RETURN;
	    }
	    else
		/* initialize device */
		dev = (*InitMsgPassPt)(argc, argv, MPID_Short_len, -1);
	    break;

#ifdef CH_GM_PRESENT
	    TR_MSG("CH_GM_PRESENT defined");
	case DEVICE_ch_gm_nbr:
	    /* ch_gm is a very special case */
	    *error_code = MPID_GM_special_init( argc, argv );
	    break;
#endif

	default:
	    fprintf( stderr, "Selected device was not built into the library\n" );
	    fflush( stderr );
	    *error_code = MPI_ERR_INTERN;
	    RETURN;
	    break;
	}
#else /* !WIN32 */
#ifdef CH_SHMEM_PRESENT
	    TR_MSG("CH_SHMEM_PRESENT defined");
		dev = (MPID_Device*)MPID_CH_NTSHMEM_InitMsgPass( argc, argv, MPID_Short_len, -1 );
#endif

#ifdef CH_SMI_PRESENT
		TR_MSG("CH_SMI_PRESENT defined");
	dev = (MPID_Device*)MPID_CH_SMI_InitMsgPass( argc, argv, MPID_Short_len, -1 );
#endif
#ifdef CH_WSOCK_PRESENT
	/*
	  MPID_Short_len default is -1 -> default MPID_PKT_MAX_DATA_SIZE
	  MPID_PKT_MAX_DATA_SIZE defined in packets.h: 16384
	  change with parameter -mpipktsize
	  long_len: parameter -1 -> default 128000*/
	TR_MSG("CH_WSOCK_PRESENT defined");
	dev = (MPID_Device*)MPID_CH_WSOCK_InitMsgPass( argc, argv, MPID_Short_len, -1 );
#endif
#ifdef CH_USOCK_PRESENT
	TR_MSG("CH_USOCK_PRESENT defined");
	dev = (MPID_Device*)MPID_CH_USOCK_InitMsgPass( argc, argv, MPID_Short_len, -1 );
#endif
#ifdef CH_MPX_PRESENT
	TR_MSG("CH_MPX_PRESENT defined");
	dev = (MPID_Device*)MPID_CH_MPX_InitMsgPass( argc, argv, MPID_Short_len, -1 );
#endif
#endif /* !WIN32 */

#ifdef DEBUG_TRACE
	TR_stack_debug =0;
#endif

	/* set MPID_MyWorldRank / MPID_MyWorldSize for those devices that don't do this anymore (which
	   should be all devices in the near future) */
	if( MPID_MyWorldRank == -1 )
	    MPID_MyWorldRank = dev->lrank;
	if( MPID_MyWorldSize == -1 )
	    MPID_MyWorldSize = dev->lsize;

	/* if we run with only one device, these should be all the same */
	MPID_MyHostRank = MPID_MyLocalRank = MPID_MyMetaRank = MPID_MyAllRank = MPID_MyWorldRank;
	MPID_MyHostSize = MPID_MyLocalSize = MPID_MyMetaSize = MPID_MyAllSize = MPID_MyWorldSize;

	if( !use_ch_gm ) {
	    /*
	     * Get the device type and the number of processors
	     */
	    if (!dev) {
		*error_code = MPI_ERR_INTERN;
		RETURN;
	    }

	    dev->grank_to_devlrank = (int *)MALLOC(sizeof(int) * MPID_MyAllSize);
	    for (i = 0; i < MPID_MyAllSize; i++)
		dev->grank_to_devlrank[i] = i;

#ifdef CH_WSOCK_PRESENT
	    if( dev->next ) {
		dev->next->grank_to_devlrank = (int *)MALLOC(sizeof(int) * MPID_MyAllSize);
		for( i = 0; i < MPID_MyAllSize; i++ )
		    dev->next->grank_to_devlrank[i] = i;
	    }
#endif

#ifdef CH_WSOCK_PRESENT
	    /* this preprocessor flag should be used only on windows;
	       the initialization function of ch_wsock maybe returned a pointer
	       to a list of 2 devices, meaning that ch_ntshmem is used for communication
	       with processes running on the same machine as this process */
	    MPID_devset->ndev = dev->next ? 2 : 1;
#else
	    MPID_devset->ndev = 1;
#endif

	    MPID_devset->dev = (MPID_Device **) MALLOC( MPID_MyAllSize * sizeof(MPID_Device *));
	    if (!MPID_devset->dev) {
		*error_code = MPI_ERR_INTERN;
		RETURN;
	    }

#ifdef CH_WSOCK_PRESENT
	    /* we have to take ch_ntshmem into account here */
	    for( i = 0; i < MPID_MyAllSize; i++ )
		if( Global2Local && Global2Local[i] < MPID_numids && dev->next )
		    MPID_devset->dev[i] = dev->next;/* NTSHMEM device */
		else
		    MPID_devset->dev[i] = dev;/* WSOCK device */

	    MPID_devset->ndev_list = MPID_devset->ndev = (dev->next ? 2 : 1);
#else
	    for( i = 0; i < MPID_MyAllSize; i++ )
		MPID_devset->dev[i] = dev;

	    MPID_devset->ndev_list = 1;
#endif

	    MPID_devset->dev_list = dev;
	}
	else {
#ifdef CH_GM_PRESENT
	    /* using ch_gm */
	    MPID_devset = MPID_GM_devset;
	    MPID_GM_post_init( argc, argv );
#endif
	}

    } /* end of initialization without meta devices (ch_gateway and ch_tunnel) */


#ifdef MPIR_MEMDEBUG
    TR_MSG("MPIR_MEMDEBUG defined");
    MPID_trinit(MPID_MyAllRank);
#endif
    /*
     * Get the basic options.  Note that this must be AFTER the
     * initialization in case the initialization routine was responsible
     * for sending the arguments to other processors.
     */
    MPID_ProcessArgs(argc, argv);

    /*
     * does any of the channel devices offer a timing function? Use it as
     * it is probably better than gettimeofday()
     */
    /*
     * XXX dynamically choosing the clock with the highest
     * resolution/highest precision would be nice, but it would take some
     * time to determine it
     */

    dev = MPID_devset->dev_list;

    while (dev) {
      if (dev->wtime != NULL) {
	MPID_dev_wtime = dev->wtime;
	break;
      }
	  /* fails if next is not properly initialized (dev->next==0xcdcdcdcd)*/
      dev = dev->next;
    }
    *error_code = MPI_SUCCESS;
} /* end of MPID_Init() */

/*
 * Barry Smith suggests that this indicate who is aborting the program. There
 * should probably be a separate argument for whether it is a user requested
 * or internal abort.
 */
void
MPID_Abort(comm_ptr, code, user, str)
    struct MPIR_COMMUNICATOR *comm_ptr;
    int             code;
    char           *user, *str;
{
    MPID_Device    *dev;
    char            abortString[256];

    fprintf(stderr, "[%d] %s Aborting program %s\n", MPID_MyWorldRank,
	    user ? user : "", str ? str : "!");
    fflush(stderr);
    fflush(stdout);

    /*
     * Also flag a debugger that an abort has happened so that it can
     * take control while there's still useful state to be examined.
     * Remember, MPIR_Breakpoint is a complete no-op unless the debugger
     * is present.
     */
    sprintf(abortString, "%s Aborting program %s", user ? user : "",
	    str ? str : "!");
    MPIR_debug_abort_string = abortString;
    MPIR_debug_state = MPIR_DEBUG_ABORTING;
    MPIR_Breakpoint();

    /* We may be aborting before defining any devices */
    if (MPID_devset) {
	int             found_dev = 0;
	dev = MPID_devset->dev_list;
	while (dev) {
	    found_dev = 1;
	    MPID_Device_call_abort (comm_ptr, code, str, dev);
	    dev = dev->next;
	}
	if (!found_dev)
	    exit(code);
    } else {
	exit(code);
    }
}

void
MPID_End()
{
    MPID_Device    *dev, *ndev;
#ifdef CH_GM_PRESENT
    MPID_Config * config_info;
#endif /* CH_GM */


    DEBUG_PRINT_MSG("Entering MPID_End")
	/* Finish off any pending transactions */
	/*
	 * Should this be part of the device terminate routines instead ?
	 * Probably not, incase they need to be done in an arbitrary sequence
	 */
	MPID_Complete_pending();

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
    /* Eventually make this optional */

    if (MPID_Print_queues)
	MPID_Dump_queues();

    /*
     * We should really generate an error or warning message if there are
     * uncompleted operations...
     */
    dev = MPID_devset->dev_list;
    while (dev) {
	ndev = dev->next;
	/*
	 * Each device should free any storage it is using INCLUDING
	 * dev
	 */
	(*dev->terminate) (dev);
	dev = ndev;
    }

    /* Clean up request handles */
    MPID_SBdestroy(MPIR_shandles);
    MPID_SBdestroy(MPIR_rhandles);

    /* Free remaining storage */
    FREE(MPID_devset->dev);
    FREE(MPID_devset);

#ifdef CH_GM_PRESENT
    while (gm_config_info_copy != NULL)
    {
	free(gm_config_info_copy->device_init_name);
	free(gm_config_info_copy->granks_served);
	config_info = gm_config_info_copy;
	gm_config_info_copy = gm_config_info_copy->next;
	free(config_info);
    }
#endif /* CH_GM */

#if defined(MPIR_MEMDEBUG) && defined(MPID_ONLY)
    /* MPI_Finalize also does this */
    MPID_trdump(stdout);
#endif
}

/*
 * Returns 1 if something found, -1 otherwise (if is_blocking is
 * MPID_NOTBLOCKING)
 */
int
MPID_DeviceCheck(is_blocking)
    MPID_BLOCKING_TYPE is_blocking;
{
    MPID_Device *actual_dev;
    MPID_Device    *dev;
    int             found = 0;
    int             lerr;

    actual_dev = MPID_devset->active_dev;

	TR_PUSH("Starting DeviceCheck");
    DEBUG_PRINT_MSG("Starting DeviceCheck");
    while (!found) {
	dev = MPID_devset->dev_list;
	while (dev) {

	    /* if we have multiple devices running, we never block  */
	    if( MPID_devset->ndev > 1 )
	        lerr = MPID_Device_call_check_device (dev, MPID_NOTBLOCKING);
	    else {
		/* attention: using threads breaks multi-device-safety! */
#ifndef MPID_USE_DEVTHREADS
		/* If threads are used in the device, they will take care of
		   completing the message, and *this* thread has to block if
		   this is requested here!

		   If threads are *not* used, we will want to loop over all
		   devices, and thus enforce NOTBLOCKING. */
		is_blocking = MPID_NOTBLOCKING;
#endif
		lerr = MPID_Device_call_check_device (dev, is_blocking);
	    }

	    found |= (lerr == 0);
	    dev = dev->next;
	}
	if (is_blocking == MPID_NOTBLOCKING) {
	    break;
	}

    }

    MPID_devset->active_dev = actual_dev;

    DEBUG_PRINT_MSG("Exiting DeviceCheck");
	TR_POP;
    return (found) ? found : -1;
}

int
MPID_Collops_init(struct MPIR_COMMUNICATOR * comm, MPIR_COMM_TYPE comm_type)
{
    MPIR_COLLOPS    new_collops;
    /*
     * If we have only one device, and this device offers a collops_init
     * function, we call it. The behaviour for multipe devices is not yet
     * defined - should each device look at the new communicator and see
     * if it can do something with it?
     */
    if ((MPID_devset->ndev == 1) && MPID_devset->dev_list->collops_init) {
	/*
	 * this device may replace one or more function ptrs =>
	 * create a new ptr table from the current (generic) one
	 */
	new_collops = (MPIR_COLLOPS) MALLOC(sizeof(struct _MPIR_COLLOPS));
	memcpy(new_collops, comm->collops, sizeof(struct _MPIR_COLLOPS));
	comm->collops = new_collops;
	comm->collops->ref_count = 0;

	MPID_devset->dev_list->collops_init(comm, comm_type);
    }
    return MPI_SUCCESS;
}

int
MPID_CommInit(struct MPIR_COMMUNICATOR * oldcomm,
	      struct MPIR_COMMUNICATOR * newcomm)
{
    /*
     * If we have only one device, and this device offers a comm_init
     * function, we call it. The behaviour for multipe devices is not yet
     * defined - should each device look at the new communicator and see
     * if it can do something with it?
     */
    if ((MPID_devset->ndev == 1) && MPID_devset->dev_list->comm_init) {
	MPID_devset->dev_list->comm_init(oldcomm, newcomm);
    } else {
	MPID_CH_Comm_msgrep(newcomm);
    }

    return MPI_SUCCESS;
}

int
MPID_CommFree(struct MPIR_COMMUNICATOR * comm)
{
    if ((MPID_devset->ndev == 1) && MPID_devset->dev_list->comm_free) {
	MPID_devset->dev_list->comm_free(comm);

	/*
	 * decrementing the refcounter and potentially freeing the
	 * collops is done in MPI_Comm_free()
	 */
    }
    return MPI_SUCCESS;
}

int
MPID_Complete_pending()
{
    MPID_Device    *dev;
    int             lerr;

    DEBUG_PRINT_MSG("Starting Complete_pending");
    while (MPID_n_pending > 0) {
	dev = MPID_devset->dev_list;
	while (dev) {
	    lerr = MPID_Device_call_check_device (dev, MPID_NOTBLOCKING);
	    if (lerr > 0) {
		return lerr;
	    }
	    dev = dev->next;
	}
    }

    DEBUG_PRINT_MSG("Exiting Complete_pending");
    return MPI_SUCCESS;
}

void
MPID_SetPktSize(len)
    int             len;
{
    MPID_Short_len = len;
}

/*
 * Perhaps this should be a util function
 */
int
MPID_WaitForCompleteSend(request)
    MPIR_SHANDLE   *request;
{
    while (!request->is_complete)
	MPID_DeviceCheck(MPID_NOTBLOCKING);
    return MPI_SUCCESS;
}

int
MPID_WaitForCompleteRecv(request)
    MPIR_RHANDLE   *request;
{
    while (!request->is_complete)
	MPID_DeviceCheck(MPID_NOTBLOCKING);
    return MPI_SUCCESS;
}

void
MPID_Version_name(name)
    char           *name;
{
    MPID_Device    *dev;
    char            chdev_version[MPID_MAX_VERSION_NAME];

    sprintf(name, "ADI-2 layer version %s, ", ADI2PATCHLEVEL);

    /*
     * determine the names of the transport facilities (the channel
     * devices)
     */
    dev = MPID_devset->dev_list;
    while (dev) {
	if (dev->get_version) {
	    MPID_Device_call_get_version(chdev_version, dev);
	    strncat(name, chdev_version, MPID_MAX_VERSION_NAME - strlen(name));
	}
	dev = dev->next;
    }

    return;
}


#ifdef META

/* this functions is meant as an aid for debugging, it prints out some information
   about the device setup */
void debug_print_devset()
{
    MPID_Device *dev;
    int n;

    printf( "\n-----------------------------------------\n" );
    printf( "MPID_devset:\n" );
    printf( "  ndev = %d, dev =", MPID_devset->ndev );
    for( n = 0; n < MPIR_meta_cfg.np + MPIR_meta_cfg.nrp; n++ )
	printf( " 0x%p", MPID_devset->dev[n] );
    printf( "\n  dev_list: " );
    dev = MPID_devset->dev_list;
    while( dev ) {
	printf( "0x%p ", dev );
	dev = dev->next;
    }
    printf( "\nDevices:\n" );
    dev = MPID_devset->dev_list;
    while( dev ) {
	printf( "Address: 0x%p, grank_to_devlrank:", dev );
	for( n = 0; n < MPIR_meta_cfg.np + MPIR_meta_cfg.nrp; n++ )
	    printf( " %d", dev->grank_to_devlrank[n] );
	printf( "\n" );
	dev = dev->next;
    }
    printf( "\n-----------------------------------------\n" );
}

#endif


#ifdef CH_GM_PRESENT
int MPID_GM_special_init( int *argc, char ***argv )
{
  MPID_Config *gm_config_info, *p;
  int ndev = 0;
  int np, i;
  MPID_Device *dev;

  /*
     We call MPID_GM_GetConfigInfo() to fill up the MPID_Config structure to describe
     the different devices and the routes.
  */
  gm_config_info = MPID_GM_GetConfigInfo( argc, argv );

  gm_config_info_copy = gm_config_info;

  np = 0;
  if( gm_config_info ) {

    /* calculate the number of devices */
    p = gm_config_info;
    while (p) {
      ndev++;

      for (i = 0; i < p->num_served; i++) {
	if (p->granks_served[i] > np)
	  np = p->granks_served[i];
      }

      p = p->next;
    }

    np++;

    /*
     * Create the device set structure
     */
    MPID_GM_devset = (MPID_DevSet *) MALLOC(sizeof(MPID_DevSet));
    if (!MPID_GM_devset) {
      return MPI_ERR_INTERN;
    }
    /* Make devset safe for initializations errors */
    MPID_GM_devset->req_pending = 0;
    MPID_GM_devset->dev_list = 0;

    MPID_GM_devset->ndev = ndev;
    MPID_GM_devset->ndev_list = ndev-1;
    MPID_GM_devset->dev = (MPID_Device **) MALLOC( np * sizeof(MPID_Device *));
    if (!MPID_GM_devset->dev) {
      return MPI_ERR_INTERN;
    }
    p = gm_config_info;

    while (p) {
      dev = (p->device_init)( argc, argv, MPID_Short_len, -1 );
      if (!dev) {
	return MPI_ERR_INTERN;
      }

      dev->grank_to_devlrank = (int *)MALLOC( sizeof(int) * np );
      for (i = 0; i < np; i++)
	dev->grank_to_devlrank[i] = i;

      dev->next = MPID_GM_devset->dev_list;
      MPID_GM_devset->dev_list = dev;
      for (i=0; i<p->num_served; i++)
	MPID_GM_devset->dev[p->granks_served[i]] = dev;
      p = p->next;
    }

  }
  else  {
    /* MPID_GM_GetConfigInfo() returned NULL pointer */
    fprintf (stderr, "Configuration retrieval failed\n");
    gmpi_abort (0);
  }


  MPID_MyWorldRank = MPID_GM_rank;
  MPID_MyWorldSize = MPID_GM_size;

  return  MPI_SUCCESS;
}

void MPID_GM_post_init( int *argc, char ***argv )
{
  int mypid, err, j;
  char hostname[GM_MAX_HOST_NAME_LEN];
  char execname[256];
  int msgrp = 0;
  MPI_Status status;
  struct MPIR_COMMUNICATOR *comm = 0;

#ifdef NEEDS_PROCESS_GROUP_FIX
  /* Call this to force each MPI job into a separate process group
     if there is no attached terminal.  This is necessary on some
     systems to overcome runtime libraries that kill a process group
     on abnormal exit (the proces group can contain the invoking
     shell scripts and programs otherwise) */
  MPID_FixupProcessGroup();
#endif

  /* Initialize the array on node with MPI process 0 that contains
     pid information for all processes. The index of the array is the same
     as the mpi rank of the process. We associate this with a node by
     matching it to the values in gmpi.node_name[].
  */
  mypid = (int) getpid();
  gm_get_host_name (gmpi_gm_port, hostname);
  strcpy (execname, ((const char **) (*argv))[0]);

  if (MPID_MyWorldRank == 0) {
    gmpi.mpi_pids[0] = mypid;
    gmpi.host_names[0] = (char *) malloc (strlen (hostname) + 1);
    gmpi_malloc_assert(gmpi.host_names[0],
		       "MPID_Init",
		       "malloc: host names");
    strcpy (gmpi.host_names[0], hostname);
    gmpi.exec_names[0] = (char *) malloc (strlen (execname) + 1);
    gmpi_malloc_assert(gmpi.exec_names[0],
		       "MPID_Init",
		       "malloc: exec names");
    strcpy (gmpi.exec_names[0], execname);
    for (j=1; j<MPID_MyWorldSize; j++)
      {
	MPID_RecvContig(&(gmpi.mpi_pids[j]), sizeof(int), j,
			0, 0, &status ,&err);
	MPID_RecvContig(hostname, GM_MAX_HOST_NAME_LEN
			* sizeof(char), j, 0, 0, &status ,&err);
	gmpi.host_names[j] = (char *) malloc (strlen (hostname) + 1);
	gmpi_malloc_assert(gmpi.host_names[j],
			   "MPID_Init",
			   "malloc: host names");
	strcpy (gmpi.host_names[j], hostname);
	MPID_RecvContig(execname, 256 * sizeof(char),
			j, 0, 0, &status ,&err);
	gmpi.exec_names[j] = (char *) malloc (strlen (execname) + 1);
	gmpi_malloc_assert(gmpi.exec_names[j],
			   "MPID_Init",
			   "malloc: exec names");
	strcpy (gmpi.exec_names[j], execname);
      }
  }
  else
    {
      MPID_SendContig(&mypid, sizeof(int), MPID_MyWorldRank,
		      0, 0, 0, msgrp, &err);
      MPID_SendContig(hostname, GM_MAX_HOST_NAME_LEN * sizeof(char),
		      MPID_MyWorldRank, 0, 0, 0, msgrp, &err);
      MPID_SendContig(execname, 256 * sizeof(char),
		      MPID_MyWorldRank, 0, 0, 0, msgrp, &err);
      gmpi.host_names[MPID_MyWorldRank] = (char *) malloc (strlen (hostname) + 1);
      gmpi_malloc_assert(gmpi.host_names[MPID_MyWorldRank],
			 "MPID_Init",
			 "malloc: host names");
      strcpy (gmpi.host_names[MPID_MyWorldRank], hostname);
      gmpi.exec_names[MPID_MyWorldRank] = (char *) malloc (strlen (execname) + 1);
      gmpi_malloc_assert(gmpi.exec_names[MPID_MyWorldRank],
			 "MPID_Init",
			 "malloc: exec names");
      strcpy (gmpi.exec_names[MPID_MyWorldRank], execname);
    }

  return;
}


#endif /* CH_GM_PRESENT */

