/*
*  $Id$
*
*  (C) 1993 by Argonne National Laboratory and Mississipi State University.
*      See COPYRIGHT in top-level directory.
*/

/* 
define MPID_NO_FORTRAN if the Fortran interface is not to be supported
(perhaps because there is no Fortran compiler)
*/
#include "mpiimpl.h"
#include "../mpid/ch2/adi2config.h"
#include "../mpid/util/cmnargs.h" /* for MPID_ArgSqueeze */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>

#include "calltrace.h"
#include "sbcnst2.h"
/* Error handlers in pt2pt */
#include "mpipt2pt.h"

/* META */
#include "comm.h"
#include "mpicoll.h"

#ifdef META
#include <errno.h>

/* use the parser from rconf_parser.tab.c */
#include "../routing/router_config.h"

/* for debugging output */
#include "../routing/rdebug.h"

#ifndef WIN32
#include <unistd.h>
#endif
/* we need MPID_Config for multi-device-setup */
/*#include "dev.h"*/
/*#include "mpiddev.h" */
/*#include "mpiddevbase.h"*/
#include "metampi.h"
#include "../routing/mpi_router.h"
#endif     
/* /META */

#include "multidevice.h"

#if defined(MPID_HAS_PROC_INFO)
/* This is needed to use select for a timeout */
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
/* For nice, sleep */
#include <unistd.h>
#endif

#if defined(MPE_USE_EXTENSIONS) && !defined(MPI_NO_MPEDBG)
#include "../../mpe/mpeexten.h"
#endif

#ifndef PATCHLEVEL_SUBMINOR
#define PATCHLEVEL_SUBMINOR 0
#endif

/* #define DEBUG(a) {a}  */
#define DEBUG(a)

/* need to change these later */
MPI_Info *MPIR_Infotable = NULL;
int MPIR_Infotable_ptr = 0, MPIR_Infotable_max = 0;

#ifdef FORTRANCAPS
#define mpir_init_fcm_   MPIR_INIT_FCM
#define mpir_init_flog_  MPIR_INIT_FLOG
#define mpir_init_bottom_ MPIR_INIT_BOTTOM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_fcm_   mpir_init_fcm__
#define mpir_init_flog_  mpir_init_flog__
#define mpir_init_bottom_ mpir_init_bottom__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_fcm_   mpir_init_fcm
#define mpir_init_flog_  mpir_init_flog
#define mpir_init_bottom_ mpir_init_bottom
#endif


#ifdef WIN32_FORTRAN_STDCALL
/* This is used on Windows to distinguish between
FORTRAN compilers that use STDCALL convention ond those
that use C convention. */
#define FORTRAN_API __stdcall
#else
#define FORTRAN_API
#endif

#ifdef VIOLAIO
/* MPI_COMM_WORLD needs to be a variable as it is changed on runtime */
/*int MPI_COMM_WORLD = MPI_COMM_META;*/

/* defines and variables needed von tunnelfs io */
#include "ad_tunnelfs_common.h"
#include "ad_tunnelfs_msg.h"
#endif

/* Prototypes for Fortran interface functions */
void FORTRAN_API mpir_init_fcm_ ( void );
void FORTRAN_API mpir_init_flog_ ( MPI_Fint *, MPI_Fint * );
#ifndef WIN32
void FORTRAN_API mpir_init_bottom_ ( void * );
#endif
/* Global memory management variables for fixed-size blocks */
void *MPIR_errhandlers;  /* sbcnst Error handlers */
void *MPIR_qels;      /* sbcnst queue elements */
void *MPIR_fdtels; /* sbcnst flat datatype elements */
void *MPIR_topo_els;/* sbcnst topology elements */

/* Global communicators.  Initialize as null in case we fail during startup */
/* We need the structure that MPI_COMM_WORLD refers to so often, 
   we export it */
struct MPIR_COMMUNICATOR *MPIR_COMM_WORLD = 0;
struct MPIR_COMMUNICATOR *MPIR_COMM_SELF  = 0;
/* additional global communicators for the MetaMPICH:
   _LOCAL for the local procs w/o routing procs
   _HOST  for the local procs w/ routing procs
   _META for all real MPI procs in the system
   _ALL for all procs in the system (MPI & routing)
   for non-meta configurations, these are present, but currently not
   initialized with "real" values
*/
struct MPIR_COMMUNICATOR *MPIR_COMM_LOCAL = 0;
struct MPIR_COMMUNICATOR *MPIR_COMM_HOST  = 0;
struct MPIR_COMMUNICATOR *MPIR_COMM_META  = 0;
struct MPIR_COMMUNICATOR *MPIR_COMM_ALL   = 0;

char *MPIR_process_name = 0;

#ifdef META
void debug_print_communicator_setup();
#endif

/* META */
/* this is for debugging the router */
int meta_barrier = 0;

#ifdef META
extern MPID_Device *MPID_Gateway_InitMsgPass ANSI_ARGS(( int *, char ***, int, int ));
extern MPID_Device *MPID_Tunnel_InitMsgPass ANSI_ARGS(( int *, char ***, int, int ));

/* multi-host and meta-environment configuration*/
MPIR_MetaConfig MPIR_meta_cfg;

/* multi-device configuration */
MPID_Config *MPID_multidev_cfg = (MPID_Config *) 0;
MPID_Config primary_device, secondary_device, tn_device, gw_device;

#ifdef VIOLAIO
/* additional global communicator for tunnelfs io servers:
_LOCAL_REDUCED: seperating extra procs from the rest on a single metahost
_META_REDUCED:  seperating extra procs from the rest in global view
*/
/*
struct MPIR_COMMUNICATOR *MPIR_COMM_LOCAL_REDUCED = 0;
struct MPIR_COMMUNICATOR *MPIR_COMM_META_REDUCED = 0;
*/
MPI_Comm MPI_COMM_LOCAL_REDUCED  = MPI_COMM_NULL;
MPI_Comm MPI_COMM_META_REDUCED   = MPI_COMM_NULL;
MPI_Comm MPI_COMM_TUNNELFS_SELF  = MPI_COMM_NULL;
MPI_Comm MPI_COMM_TUNNELFS_WORLD = MPI_COMM_NULL;
#endif 
/* /VIOLAIO */

#endif
/* /META */


struct MPIR_GROUP *MPIR_GROUP_EMPTY = 0;

/* Home for this variable (used in MPI_Initialized) */
int MPIR_Has_been_initialized = 0;

/* MPI_Comm MPI_COMM_SELF = 0, MPI_COMM_WORLD = 0; */
/* MPI_Group MPI_GROUP_EMPTY = 0; */

/* Global MPIR process id (from device) */
int MPIR_tid;

/* Permanent attributes */
/* Places to hold the values of the attributes */
static int MPI_TAG_UB_VAL, MPI_HOST_VAL, MPI_IO_VAL, MPI_WTIME_IS_GLOBAL_VAL;

/* Command-line flags */
int MPIR_Print_queues = 0;
#ifdef MPIR_MEMDEBUG
int MPIR_Dump_Mem = 0;
#else
int MPIR_Dump_Mem = 1;
#endif

/* Fortran logical values */
MPI_Fint MPIR_F_TRUE=-1, MPIR_F_FALSE=-1;

/* 
Location of the Fortran marker for MPI_BOTTOM.  The Fortran wrappers
must detect the use of this address and replace it with MPI_BOTTOM.
This is done by the macro MPIR_F_PTR.
*/
void *MPIR_F_MPI_BOTTOM = 0;
/* Here are the special status ignore values in MPI-2 */
void *MPIR_F_STATUS_IGNORE = 0;
void *MPIR_F_STATUSES_IGNORE = 0;

/* MPICH extension keyvals */
int MPICHX_QOS_BANDWIDTH = MPI_KEYVAL_INVALID;

/* META */

#ifdef META

/* this function parses the meta-configuration file 
* and sets up the devices
*
*/
int MPIR_init_systemconfig (metacfg_file, dev_cfg)
char *metacfg_file;
MPID_Config **dev_cfg;
{
    static char myname[] = "MPIR_init_systemconfig";
    int nall = 0;   /* counter of all procs found while parsing the config file */
    int metaHost;
    int rankOffset;
    int i,j;
    int error, secondaryDeviceNbr; 
    int global_rank, rank_array_index;
    Snode *node;
    MPID_Device *(*InitMsgPassPt)(int *, char ***, int, int); /* pointer to device initialization function */

    TR_PUSH(myname);

    /* read the meta configuration file;
       the data that the parser collects is stored in
       MPIR_routerConfig, metahostlist and routerlist */
    if (!MPIR_read_metaconfig(metacfg_file, &MPIR_RouterConfig, MPIR_meta_cfg.my_metahostname,
			      MPIR_meta_cfg.np_override, 0  )) {
	TR_POP;
	return MPIR_ERR_PRE_INIT;
    };

    /* initialize MPIR_meta_cfg further */
    MPIR_meta_cfg.nbr_metahosts = MPIR_RouterConfig.nbr_metahosts;
    MPIR_meta_cfg.np            = MPIR_RouterConfig.np;
    for( i = 0; i < META_MPI_MAX_METAHOSTS; i++ ) {
	MPIR_meta_cfg.nrp_metahost[i] = MPIR_RouterConfig.nrp_metahost[i];
	MPIR_meta_cfg.my_nrp[i]       = MPIR_RouterConfig.my_nrp[i];
	strcpy( MPIR_meta_cfg.metahostnames[i], MPIR_RouterConfig.metahostnames[i] );
	MPIR_meta_cfg.np_metahost[i] = MPIR_RouterConfig.np_metahost[i];
    }
    MPIR_meta_cfg.nrp = MPIR_RouterConfig.nrp;
    MPIR_meta_cfg.secondaryDevice = MPIR_RouterConfig.secondaryDevice;
    MPIR_meta_cfg.secondaryDeviceOpts = MPIR_RouterConfig.secondaryDeviceOpts;
    MPIR_meta_cfg.is_hetero = MPIR_RouterConfig.isHetero; /* <- obsolete? */
    strcpy( MPIR_meta_cfg.my_metahostname, MPIR_RouterConfig.my_metahostname );
    MPIR_meta_cfg.np_override = MPIR_RouterConfig.np_override;
    MPIR_meta_cfg.my_metahost_rank = MPIR_RouterConfig.my_metahost_rank;
    MPIR_meta_cfg.isRouter = (int*) malloc ( sizeof(int) *
					     (MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] +
					      MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank]) );
    
    MPIR_meta_cfg.npExtra=	MPIR_RouterConfig.npExtra;
    for (i=0; i< MPIR_meta_cfg.nbr_metahosts; i++)
	MPIR_meta_cfg.npExtra_metahost[i] = MPIR_RouterConfig.npExtra_metahost[i];

    for( i = 0; i < (MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] +
		     MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank]); i++ )
	MPIR_meta_cfg.isRouter[i] = 0;
    
    for( i = 0; i < rh_getNumRouters(); i++ )
	MPIR_meta_cfg.isRouter[routerlist[i].metahostrank] = 1;

    gethostname(MPIR_meta_cfg.nodeName, MPI_MAX_PROCESSOR_NAME);

    MPIR_meta_cfg.myNodeNr=0;

    /* let's see if we are about to use the secondary device, router procs or both */
    MPIR_meta_cfg.useRouterToMetahost = (int *)calloc( MPIR_meta_cfg.nbr_metahosts, sizeof(int) ); /* init to zero */
    for( i = 0; i < rh_getNumRouters(); i++ )
	MPIR_meta_cfg.useRouterToMetahost[routerlist[i].host] = 1;

    for( i = 0; i < MPIR_meta_cfg.nbr_metahosts; i++ ) {

	/* if there is some metahost which we reach via routers, well, then we have to use routers */
	if( MPIR_meta_cfg.useRouterToMetahost[i] )
	    MPIR_meta_cfg.useRouters = 1;
	    
	/* if there is a metahost other than our own which we don't reach via router,
	   we have to use the secondary device */
	if( (MPIR_meta_cfg.useRouterToMetahost[i] == 0) && (i != MPIR_meta_cfg.my_metahost_rank) )
	    MPIR_meta_cfg.useSecondaryDevice = 1;

	/* if there is a metahost j which is not reached from metahost i via routers,
	   metahost i uses the secondary device */
	MPIR_meta_cfg.metahostUsesSecondaryDevice[i] = 0;
	for( j = 0; j < MPIR_meta_cfg.nbr_metahosts; j++ )
	    if( MPIR_RouterConfig.useRouterFromTo[i][j] == 0 )
		MPIR_meta_cfg.metahostUsesSecondaryDevice[i] = 1;

    }
    
    /* ----------------------------------------------- */
    /* TODO */
    if (MPIR_meta_cfg.npExtra > 0)
	MPIR_meta_cfg.npExtra_ranks = 
	    (int*) malloc( sizeof(int) * MPIR_meta_cfg.npExtra);
    
#if 0
    j=0;
    rankOffset = 0;
    MPIR_meta_cfg.extra = 0;
    /* for each metahost ... */
    for (metaHost=0; metaHost < MPIR_meta_cfg.nbr_metahosts; metaHost++) 
	{
	    /* ... get the ranks of extra procs */
	    for (i=0; i<MPIR_meta_cfg.npExtra_metahost[metaHost]; i++) 
		{
		    MPIR_meta_cfg.npExtra_ranks[j] = rankOffset +
			MPIR_meta_cfg.np_metahost[metaHost] -
			MPIR_meta_cfg.npExtra_metahost[metaHost]+i;
		    /* FIXME: This should be parsed from a commandline parameter or a
		     * similar method, as the user should specify where the initial
		     * data is kept. - MH */
#ifdef VIOLAIO
#if 0
		    if (TUNNELFS_GLOBAL_MASTER < 0) TUNNELFS_GLOBAL_MASTER = MPIR_meta_cfg.npExtra_ranks[j];
#endif
#endif
		    j++;
		}
	    rankOffset += MPIR_meta_cfg.np_metahost[metaHost] +
		MPIR_meta_cfg.npExtra_metahost[metaHost];
	}
#endif
    
    if (MPIR_meta_cfg.npExtra_metahost[MPIR_meta_cfg.my_metahost_rank] > 0)
	MPIR_meta_cfg.npExtra_local_ranks = 
	    (int*) malloc( sizeof(int) *
			   MPIR_meta_cfg.npExtra_metahost[MPIR_meta_cfg.my_metahost_rank]);
    
#if 0
    /* on my local metahost ... */
    for (i=0; i<MPIR_meta_cfg.npExtra_metahost[MPIR_meta_cfg.my_metahost_rank]; i++) {
	/* ... get all local ranks of extra procs */
	MPIR_meta_cfg.npExtra_local_ranks[i] =
	    MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] -
	    MPIR_meta_cfg.npExtra_metahost[MPIR_meta_cfg.my_metahost_rank]+i;
    }
#endif
    /* ----------------------------------------------- */
    
    /* prepare the MPID_Config entries */
    *dev_cfg = &primary_device;

    /* set the pointer to the device initialization function for the primary device */
    switch( MPID_selected_primary_device ) {
#ifdef CH_SMI_PRESENT
    case DEVICE_ch_smi_nbr:
#endif
#ifdef CH_SHMEM_PRESENT
    case DEVICE_ch_shmem_nbr:
#endif
#ifdef CH_P4_PRESENT	    
    case DEVICE_ch_p4_nbr:
#endif
#ifdef CH_USOCK_PRESENT	    
    case DEVICE_ch_usock_nbr:
#endif
#ifdef CH_MPX_PRESENT	    
    case DEVICE_ch_mpx_nbr:
#endif
	/* look up initialization function */
	InitMsgPassPt =
	    ( MPID_Device *(*)(int *, char ***, int, int) ) MPID_GetInitMsgPassPt( MPID_selected_primary_device,
										   &error );
	if( error == MPI_ERR_INTERN ) {
	    TR_POP;
	    return -1;
	}
	else
	    primary_device.device_init = *InitMsgPassPt;
	break;
#ifdef CH_GM_PRESENT	    
    case DEVICE_ch_gm_nbr:
	break;
#endif
    default:
	fprintf( stderr, "Selected device was not built into the library\n" );
	fflush( stderr );
	TR_POP;
	return -1;
	break;
    } /* end of case statement */

    primary_device.num_served = 0;

    /* if we will use a secondary device, we put it between the primary device
       and the gateway device */
    if( MPIR_meta_cfg.useSecondaryDevice ) {

	/* get device number for secondary device as in mpichconf.h */
	secondaryDeviceNbr = MPID_GetDeviceNbr( MPIR_meta_cfg.secondaryDevice, &error );
	if( error != MPI_SUCCESS ) {
	    TR_POP;
	    return MPI_ERR_INTERN;
	}

	/* set the pointer to the device initialization function for the secondary device */
	secondary_device.device_init
	    = ( MPID_Device *(*)(int *, char ***, int, int) )MPID_GetInitMsgPassPt( secondaryDeviceNbr, &error );
	if( error != MPI_SUCCESS ) {
	    TR_POP;
	    return MPI_ERR_INTERN;
	}

	secondary_device.num_served = 0;
	primary_device.next = &secondary_device;

	if( MPIR_meta_cfg.useRouters )
	    secondary_device.next = &gw_device;
	else
	    secondary_device.next = (MPID_Config *)NULL;
    }
    else {
	if( MPIR_meta_cfg.useRouters )
	    primary_device.next = &gw_device;
	else
	    primary_device.next = (MPID_Config *)NULL;
    }


    /* Meta-Devices: ch_gateway for all MPI-procs,
       ch_tunnel for the routing procs */
    gw_device.device_init = MPID_Gateway_InitMsgPass;
    gw_device.num_served = 0;
    gw_device.next = (MPID_Config *) NULL;
    
    tn_device.device_init = MPID_Tunnel_InitMsgPass;
    tn_device.num_served = 0;
    tn_device.next = (MPID_Config *) NULL;
    

    if (MPIR_meta_cfg.nbr_metahosts == 1) {
	/* no META-functionality required */
	dev_cfg = NULL;
    }

    /* allocate space for rank-mappings (to simplify things a bit, we allocate an upper bound of what is needed for each device) */
    primary_device.granks_served = (int *)MALLOC( sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) );
    if( MPIR_meta_cfg.useSecondaryDevice )
	secondary_device.granks_served = (int *)MALLOC( sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) );
    gw_device.granks_served = (int *)MALLOC( sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) );
    tn_device.granks_served = (int *)MALLOC( sizeof(int) * (MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) );
	
    /* initialize mappings with dummy values */
    for( i=0; i<(MPIR_meta_cfg.np + MPIR_meta_cfg.nrp); i++ ) {
	primary_device.granks_served[i] = -1;
	if( MPIR_meta_cfg.useSecondaryDevice )
	    secondary_device.granks_served[i] = -1;
	gw_device.granks_served[i] = -1;
	tn_device.granks_served[i] = -1;
    }
	

    /* now we calculate which process can be reached via which device and how 
       many processes each device serves */

    /* walk through all global ranks by arranging all processes of all metahosts in one line */
    for ( metaHost=0; metaHost < MPIR_meta_cfg.nbr_metahosts; metaHost++){

	if ( metaHost == MPIR_meta_cfg.my_metahost_rank ) { /* its me! */
	    MPIR_meta_cfg.metahost_firstrank = nall;
		
	    for (i = 0; i < MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost]; i++) { 
		/*  mappings for me  */ 
		primary_device.granks_served[i] = nall + i; /* add linear mapping */ 
	    }
	    primary_device.num_served += MPIR_meta_cfg.np_metahost[metaHost] + 
		MPIR_meta_cfg.nrp_metahost[metaHost]; 
	} 
	else {/* it's a remote metahost */
	    if( MPIR_meta_cfg.useRouterToMetahost[metaHost] ) {
		    
		/* use gateway-device */
		for( i = 0; i < MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost]; i++ ) 
		    gw_device.granks_served[gw_device.num_served + i] = nall + i;
		gw_device.num_served += MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost];
	    }
	    else {
		
		/* use secondary device */
		for( i = 0; i < MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost]; i++ )
		    secondary_device.granks_served[secondary_device.num_served + i] = nall + i;
		secondary_device.num_served += MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost];
	    }
	    
	}

	/* add all local processes to rank counter */
	nall += MPIR_meta_cfg.np_metahost[metaHost] + MPIR_meta_cfg.nrp_metahost[metaHost];

    } /* end of loop over all metahosts */
	    
    /* this was for the primary, secondary and gateway device, now comes ch_tunnel ->
       mapping of the tunnel device for all MPI procs on this metahost (msgs with other global
       IDs shouldn't arrive here); if this metahost never imports messages via router, the following
       is a bit useless, but it shouldn't harm */
    global_rank = MPIR_meta_cfg.metahost_firstrank;
    rank_array_index = 0;
    /* loop over all nodes on my metahost */
    node = metahostlist[MPIR_meta_cfg.my_metahost_rank].nodeList;
    while( node ) {
	/* routers are those with the lower ranks on the node */
	global_rank += node->numRouters;
	/* put global ranks of app processes on this node in array */
	for( i = 0; i < node->np; i++ ) {
	    tn_device.granks_served[rank_array_index] = global_rank;
	    global_rank++;
	    rank_array_index++;
	}
	node = node->next;
    }
    
    tn_device.num_served =  MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank];
    
    TR_POP;
    return MPI_SUCCESS;
}
#endif /* /META */

int MPIR_Copy_collops (comm)
struct MPIR_COMMUNICATOR *comm;
{
	MPIR_COLLOPS new_collops;

	if ((new_collops = (MPIR_COLLOPS ) MALLOC (sizeof (*new_collops))) == NULL) {
		return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
			"Out of space in MPIR_Copy_collops" );
	}
	memcpy ((void *)new_collops, (void *)comm->collops, sizeof (*new_collops));

	comm->collops = new_collops;
	comm->comm_coll->collops = new_collops;

	return MPI_SUCCESS;
}

#ifdef META
EXPORT_MPI_API int get_COMM_HOST(void)
{
	return MPI_COMM_HOST;
}
EXPORT_MPI_API int get_COMM_LOCAL(void)
{
	return MPI_COMM_LOCAL;
}
#endif
/* /META */


/*
MPIR_Init - Initialize the MPI execution environment

Input Parameters:
+  argc - Pointer to the number of arguments 
-  argv - Pointer to the argument vector

See MPI_Init for the description of the input to this routine.

This routine is in a separate file from MPI_Init to allow profiling 
libraries to not replace MPI_Init; without this, you can get errors
from the linker about multiply defined libraries.

*/
int MPIR_Init(argc,argv)
int  *argc;
char ***argv;
{
	int            size, mpi_errno, i, j;
	void           *ADIctx = 0;
	static char myname[] = "MPIR_INIT";
	/* META */
#ifdef META
	char meta_config_file[255];
	int metacon = 0;
#endif
	int dev;

#ifdef _WIN32
	if(!argc || ! argv) {
		argc = &__argc;
		argv= &__argv;
	}
#endif
	/* /META */
	TR_PUSH("MPIR_Init");
	if (MPIR_Has_been_initialized){ 
		TR_MSG("MPIR_Has_been_initialized");
		TR_POP;
		return 
			MPIR_ERROR( (struct MPIR_COMMUNICATOR *)0, 
			MPIR_ERRCLASS_TO_CODE(MPI_ERR_OTHER,MPIR_ERR_INIT), myname);
	}

	/* Sanity check.  If this program is being run with MPIRUN, check that
	we have the expected information.  That is, make sure that we
	are not trying to use mpirun.ch_p4 to start mpirun.ch_shmem.
	This has a fall through in that if there is no information, the test
	is ignored
	*/


	/* this has to be revisited, temporarily disabled */
#if 0
	/* in a library that has multiple devices built in, we can not check the device
	that is chosen at runtime against a device that was chosen at configure time;
	maybe later we can check it against a list of devices that are built into the lib */

#if defined(MPIRUN_DEVICE) && defined(MPIRUN_MACHINE)
	{char *p1, *p2;
#ifdef HAVE_NO_C_CONST
	extern char *getenv (char *);
#else
	extern char *getenv (const char *);
#endif

	mpi_errno = MPI_SUCCESS;
	p1 = getenv( "MPIRUN_DEVICE" );   
	p2 = getenv( "MPIRUN_MACHINE" );   
	if (p1 && strcmp( p1, MPIRUN_DEVICE ) != 0) {
		mpi_errno = MPIR_Err_setmsg( MPI_ERR_OTHER, MPIR_ERR_MPIRUN, myname,
			(char *)0,(char *)0, p1, MPIRUN_DEVICE );
	}
	else if (p2 && strcmp( p2, MPIRUN_MACHINE ) != 0) {
		mpi_errno = MPIR_Err_setmsg( MPI_ERR_OTHER, MPIR_ERR_MPIRUN_MACHINE, 
			myname,
			(char *)0,(char *)0, p2, MPIRUN_MACHINE );
	}

	if (mpi_errno) {
		MPIR_Errors_are_fatal( (MPI_Comm*)0, &mpi_errno, myname,
			__FILE__, (int *)0 );
	}
	}
#endif

#endif  /* 0 */
	/* save name of the executable for use in debug and verbose output of meta mpich*/
	MPIR_process_name = (char*) malloc(strlen((*argv)[0])+1);
	strcpy(MPIR_process_name,(*argv)[0]);

	/* get number of selected primary device;
	   by default, device 0 is selected */
	MPID_selected_primary_device = 0;

	for( i = 0; i < *argc; i++ )
	    if((*argv)[i] ) {
		/* number of selected device */
		if( strcmp( (*argv)[i], "-usedevice" ) == 0 ) {
		    (*argv)[i] = 0;
		    i++;
		    
		    MPID_selected_primary_device = atoi( (*argv)[i] );
		    
		    (*argv)[i] = 0;
		}
	    }

	
	/* META */
#ifdef META
	/* Parsing of MetaMPICH-relevant args */
	TR_MSG("META defined");
	MPIR_meta_cfg.isMeta = 0; 
	MPIR_meta_cfg.router = 0;
	MPIR_meta_cfg.routerAutoCfg = 1;
	MPIR_meta_cfg.nbr_metahosts = 1;
	MPIR_meta_cfg.dedicated_rp = 0; 
	MPIR_meta_cfg.my_routingid = -1;
	MPIR_meta_cfg.my_localrank = -1;
	MPIR_meta_cfg.my_metahostname[0] = '\0';
	MPIR_meta_cfg.is_hetero = 0;
	MPIR_meta_cfg.secondaryDevice = DEVICE_NULL;
	MPIR_meta_cfg.useSecondaryDevice = 0;
	MPIR_meta_cfg.useRouters = 0;
	
	/* The META-MPICH needs multiple device support which seems not
	 * to be fully functioning yet. We have to search for the -meta
	 * cmdLineArg to determine if we are to be META-configured and
	 * if yes, get the according configuration from the config file
	 */
	if (argv && *argv) {
	    /* check for metahost argument and delete it */
	    for (i = 0; i < *argc; i++) {
		if ((*argv)[i]) {
		    /* get the name of the metahost */ 
		    if (strcmp( (*argv)[i], "-metahost" ) == 0) {
			(*argv)[i] = 0;
			i++;
			strcpy(MPIR_meta_cfg.my_metahostname,(*argv)[i]);
			(*argv)[i] = 0;
		    }
		}
	    }	
	    MPIR_meta_cfg.np_override=0;
	    /* check for metaconfiguation parameters */
	    for (i = 0; i < *argc; i++) {
		if ((*argv)[i]) {
		    /* get the parameter string */ 
		    if (strcmp( (*argv)[i], "-metaparam" ) == 0) {
			(*argv)[i] = 0;
			i++;
			sscanf((*argv)[i],"%d",&MPIR_meta_cfg.np_override);
			(*argv)[i] = 0;
		    }
		}
	    }	

	    /* check for a "magic meta key": */
	    for (i = 0; i < *argc; i++) {
		if ((*argv)[i]) {
		    /* get the parameter string */ 
		    if (strcmp( (*argv)[i], "-metakey" ) == 0) {
			(*argv)[i] = 0;
			i++;
			MPIR_meta_cfg.metakey=(char*)malloc(strnlen((*argv)[i],8)*sizeof(char));
			strncpy(MPIR_meta_cfg.metakey, (*argv)[i], 8);
			(*argv)[i] = 0;
		    }
		}
	    }

#ifndef WIN32
	    TR_MSG("WIN32 not defined");
	    if( strlen(MPIR_meta_cfg.my_metahostname) == 0) {
		/* fprintf(stderr, "Missing metahostargument - choosing localhost\n"); */ /* <- obsolete? */
		if ( gethostname(MPIR_meta_cfg.my_metahostname , MPI_MAX_PROCESSOR_NAME-1 ) < 0 ) {
		    fprintf(stderr, "no name for localhost!\n");
		    exit(98);
		}
	    }
#endif
	    
	    for (i = 0; i < *argc; i++) 
		if ((*argv)[i]) {
		    /* if found, stop at barrier for allowing attaching
		       with the debugger */
		    if (strcmp( (*argv)[i], "-barrier" ) == 0) {
			meta_barrier = 1;
			(*argv)[i] = 0;
		    } 
		}
	    for (i = 0; i < *argc; i++) 
		if ((*argv)[i]) {
		    /* router shall attach to a meta console  */
		    if (strcmp( (*argv)[i], "-metacon" ) == 0) {
			metacon = 1;
			(*argv)[i] = 0;
		    } 
		}
	    for (i = 0; i < *argc; i++) 
		if ((*argv)[i]) {
		    /* router shall attach to a meta console  */
		    if (strcmp( (*argv)[i], "-router" ) == 0) {
			(*argv)[i] = 0;
			i++;
			sscanf((*argv)[i],"%d",&MPIR_meta_cfg.my_routingid);
			(*argv)[i] = 0;
			MPIR_meta_cfg.routerAutoCfg=0;
			MPIR_meta_cfg.router=1;
		    } 
		}
	    for (i = 0; i < *argc; i++) 
		if ((*argv)[i]) {
		    /* router shall attach to a meta console  */
		    if (strcmp( (*argv)[i], "-app" ) == 0) {
			(*argv)[i] = 0;
			MPIR_meta_cfg.routerAutoCfg=0;
			MPIR_meta_cfg.router=0;
		    } 
		}
	    for (i = 0; i < *argc; i++) 
		if ((*argv)[i]) {
		    /* get the name of the meta config file */ 
		    if (strcmp( (*argv)[i], "-metarun" ) == 0) {		    
			MPIR_meta_cfg.isMeta=1;
			if (i+1 <*argc) {
			    strcpy (meta_config_file, (*argv)[i+1]);
			    (*argv)[i]   = 0;
			    (*argv)[i+1] = 0;
			    MPIR_init_systemconfig (meta_config_file, &MPID_multidev_cfg);
#ifdef FOO
			    /* (currently removed to avoid clashes with devices that do NOT expect the -np option in the arg list) */
			    
			    /* eventually adjust the number of MPI processes given via -np or -n option
			       to the real number of processes in the system. the devices need this ! */
			    if (MPID_multidev_cfg != NULL) {
				int j;
				int npFound=0;
				int nFound=0;
				for (j = 0; j < *argc; j++) 
				    if ((*argv)[j]) {
					if (strcmp( (*argv)[j], "-np" ) == 0) {
					    npFound=1;
					    sprintf ((*argv)[j+1], "%d", 
						     MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] 
						     + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank]);
					}
					if (strcmp( (*argv)[j], "-n" ) == 0) {
					    nFound=1;
					    sprintf ((*argv)[j+1], "%d", 
						     MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] 
						     + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank]);
					}
				    }
				if (!npFound && !nFound) { /* we have no -np arg : set it! */
				    sprintf((*argv)[i],"-np");
				    sprintf((*argv)[i+1],"%d", MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] 
					    + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank]);
				} else { /* delete arg */
				    (*argv)[i] = 0;
				    (*argv)[i+1] = 0;
				}
			    }
#endif
			} else {
			    fprintf(stderr, "Missing filename for -metarun\n");
			    exit(99);
			    /*			return MPI_ERR_PRE_INIT;*/
			}
			/* two args done */
			i += 1;
		    }
		}
	    
	}
#endif
	/* /META */
	
	MPID_ArgSqueeze( argc, *argv );
	
	/* If we wanted to be able to check if we're being debugged,
	 * (so that we could explicitly request that the other processes
	 * come up stopped), this would be a good place to do it.
	 * That information should be available by looking at a global.
	 *
	 * For now we don't bother, but assume that we're cheating and using
	 * an extra argument to mpirun which 
	 * 1) starts a debugger on the host process
	 * 2) causes the other processes to stop in mpi_init (see below).
	 */
	
	/* META */
#ifdef META
	TR_MSG("META defined");
	MPID_Init( argc, argv, (void *)MPID_multidev_cfg, &mpi_errno );
#else /* /META */
	TR_MSG("META not defined");
	MPID_Init( argc, argv, (void *)0, &mpi_errno );
#endif
	
	/* need to do this again, there may have been new cmdline-args created (e.g. ch_p4) */
	/* MPID_ArgSqueeze( argc, *argv ); */
	
	if (mpi_errno) {
	    MPIR_Errors_are_fatal( (MPI_Comm*)0, &mpi_errno, myname, 
				   __FILE__, (int *)0 );
	}
	DEBUG(MPIR_tid=MPID_MyWorldRank;)
	    
#ifdef MPID_HAS_PROC_INFO
	    TR_MSG("MPID_HAS_PROC_INFO defined");
	if (MPID_MyWorldRank == 0) {
	    /* We're the master process, so we need to grab the info
	     * about where and who all the other processes are 
	     * and flatten it in case the debugger wants it.
	     */
	    int i;
	    MPIR_proctable = (MPIR_PROCDESC *)MALLOC(MPID_MyWorldSize*sizeof(MPIR_PROCDESC));
	    
	    /* Cause extra state to be remembered */
	    MPIR_being_debugged = 1;
	    
	    /* Link in the routine that contains info on the location of
	       the message queue DLL */
	    MPIR_Msg_queue_export();
	    
	    if (MPIR_proctable)
		{
		    for (i=0; i<MPID_MyWorldSize; i++)
			{
			    MPIR_PROCDESC *this = &MPIR_proctable[i];
			    
			    this->pid = MPID_getpid(i, &this->host_name, &this->executable_name);
			    DEBUG(PRINTF("[%d] %s :: %s %d\n", i, 
					 this->host_name ? this->hostname : "local",
					 this->executable_name ? this->executable_name : "", 
					 this->pid);)
				}
		    
		    MPIR_proctable_size = MPID_MyWorldSize;
		    /* Let the debugger know that the info is now valid */
		    MPIR_debug_state    = MPIR_DEBUG_SPAWNED;
		    MPIR_Breakpoint();  
		}
	} 
#endif
	
	/* Indicate that any pointer conversions are permanent */
	MPIR_PointerPerm( 1 );
	
	DEBUG(PRINTF("[%d] About to do allocations\n", MPIR_tid);)
	    
	    /* initialize topology code */
	    MPIR_Topology_init();
	
	/* initialize memory allocation data structures */
	MPIR_errhandlers= MPID_SBinit( sizeof( struct MPIR_Errhandler ), 10, 10 );
	
	MPIR_SENDQ_INIT();
#ifdef FOO
	MPIR_fdtels     = MPIR_SBinit( sizeof( MPIR_FDTEL ), 100, 100 );
#endif
	MPIR_HBT_Init();
	MPIR_Topology_Init();
	
	/* This handles ALL datatype initialization */
	MPIR_Init_dtes();
	
	/* Predefined combination functions */
	DEBUG(PRINTF("[%d] About to create combination functions\n", MPIR_tid);)
	    MPIR_Setup_Reduce_Ops();
	
	/* Create Error handlers */
	/* Must create at preassigned values */
	MPIR_Errhandler_create( MPIR_Errors_are_fatal, MPI_ERRORS_ARE_FATAL );
	MPIR_Errhandler_create( MPIR_Errors_return,    MPI_ERRORS_RETURN );
	MPIR_Errhandler_create( MPIR_Errors_warn,      MPIR_ERRORS_WARN );
	
	/* GROUP_EMPTY is a valid empty group */
	DEBUG(PRINTF("[%d] About to create groups and communicators\n", MPIR_tid);)
	    MPIR_GROUP_EMPTY     = MPIR_CreateGroup(0);
	MPIR_GROUP_EMPTY->self = MPI_GROUP_EMPTY;
	MPIR_RegPointerIdx( MPI_GROUP_EMPTY, MPIR_GROUP_EMPTY );
	MPIR_GROUP_EMPTY->permanent = 1;
	
	/* META */
#ifdef META
	if (MPIR_meta_cfg.nbr_metahosts == 1) {
#endif
	    /* /META */
	    
	    MPIR_ALLOC(MPIR_COMM_WORLD,NEW(struct MPIR_COMMUNICATOR),
		       (struct MPIR_COMMUNICATOR *)0,
		       MPI_ERR_EXHAUSTED,myname);
	    MPIR_SET_COOKIE(MPIR_COMM_WORLD,MPIR_COMM_COOKIE)
		MPIR_RegPointerIdx( MPI_COMM_WORLD, MPIR_COMM_WORLD );
	    MPIR_COMM_WORLD->self = MPI_COMM_WORLD;
	    
	    MPIR_COMM_WORLD->comm_type	   = MPIR_INTRA;
	    MPIR_COMM_WORLD->ADIctx	   = ADIctx;
	    size     = MPID_MyWorldSize;
	    MPIR_tid = MPID_MyWorldRank;
	    MPIR_COMM_WORLD->group	   = MPIR_CreateGroup( size );
	    MPIR_COMM_WORLD->group->self   = 
		(MPI_Group) MPIR_FromPointer( MPIR_COMM_WORLD->group );
#if defined(MPID_DEVICE_SETS_LRANKS)
	    TR_MSG("MPID_DEVICE_SETS_LRANKS defined");
	    MPID_Set_lranks ( MPIR_COMM_WORLD->group );
#else
	    MPIR_SetToIdentity( MPIR_COMM_WORLD->group );
#endif
	    MPIR_Group_dup ( MPIR_COMM_WORLD->group, 
			     &(MPIR_COMM_WORLD->local_group) );
	    MPIR_COMM_WORLD->local_rank	   = MPIR_COMM_WORLD->local_group->local_rank;
	    MPIR_COMM_WORLD->lrank_to_grank = MPIR_COMM_WORLD->group->lrank_to_grank;
	    MPIR_COMM_WORLD->np		   = MPIR_COMM_WORLD->group->np;
	    MPIR_COMM_WORLD->send_context   = MPIR_WORLD_PT2PT_CONTEXT;
	    MPIR_COMM_WORLD->recv_context   = MPIR_WORLD_PT2PT_CONTEXT;
	    MPIR_COMM_WORLD->error_handler  = MPI_ERRORS_ARE_FATAL;
	    MPIR_COMM_WORLD->use_return_handler = 0;
	    MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
	    MPIR_COMM_WORLD->ref_count	   = 1;
	    MPIR_COMM_WORLD->permanent	   = 1;
	    (void)MPID_CommInit( (struct MPIR_COMMUNICATOR *)0, MPIR_COMM_WORLD );
	    
	    MPIR_Attr_create_tree ( MPIR_COMM_WORLD );
	    MPIR_COMM_WORLD->comm_cache	   = 0;
	    MPIR_Comm_make_coll ( MPIR_COMM_WORLD, MPIR_INTRA );
	    
	    MPIR_COMM_WORLD->comm_name      = 0;
	    MPI_Comm_set_name ( MPI_COMM_WORLD, "MPI_COMM_WORLD");
	    
	    /* Predefined attributes for MPI_COMM_WORLD */
	    DEBUG(PRINTF("[%d] About to create keyvals\n", MPIR_tid);)
#define NULL_COPY (MPI_Copy_function *)0
#define NULL_DEL  (MPI_Delete_function*)0
#ifdef WIN32 
#define WINEXT ,0,0
#else 
#define WINEXT	
#endif
		i = MPI_TAG_UB;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_HOST;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_IO;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_WTIME_IS_GLOBAL;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    
	    /* Initialize any device-specific keyvals */
	    MPID_KEYVAL_INIT();
	    MPI_TAG_UB_VAL = MPID_TAG_UB;
#ifndef MPID_HOST
#define MPID_HOST MPI_PROC_NULL
#endif    
	    MPI_HOST_VAL   = MPID_HOST;
	    
	    /* The following isn't strictly correct, but I'm going to leave it
	       in for now.  I've tried to make this correct for a few systems
	       for which I know the answer.  
	    */
	    /* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
	       perhaps for some other systems */
	    /*     MPI_IO_VAL = MPI_PROC_NULL; */
#ifndef MPID_IO
#define MPID_IO MPI_ANY_SOURCE
#endif
	    MPI_IO_VAL = MPID_IO;
	    /* The C versions - pass the address of the variable containing the 
	       value */
	    MPI_Attr_put( MPI_COMM_WORLD, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
	    MPI_Attr_put( MPI_COMM_WORLD, MPI_HOST,   (void*)&MPI_HOST_VAL );
	    MPI_Attr_put( MPI_COMM_WORLD, MPI_IO,     (void*)&MPI_IO_VAL );
	    
	    /* Do the Fortran versions - Pass the actual value.  Note that these
	       use MPIR_Keyval_create with the "is_fortran" flag set. 
	       If you change these; change the removal in finalize.c. */
	    i = MPIR_TAG_UB;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_HOST;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_IO;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_WTIME_IS_GLOBAL;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    MPI_Attr_put( MPI_COMM_WORLD, MPIR_TAG_UB, (void*)MPI_TAG_UB_VAL );
	    MPI_Attr_put( MPI_COMM_WORLD, MPIR_HOST,   (void*)MPI_HOST_VAL );
	    MPI_Attr_put( MPI_COMM_WORLD, MPIR_IO,     (void*)MPI_IO_VAL );
	    
	    /* Add the flag on whether the timer is global */
#ifdef MPID_Wtime_is_global
	    TR_MSG("MPID_Wtime_is_global defined");
	    MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
	    MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
	    MPI_Attr_put( MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, 
			  (void *)&MPI_WTIME_IS_GLOBAL_VAL );
	    MPI_Attr_put( MPI_COMM_WORLD, MPIR_WTIME_IS_GLOBAL, 
			  (void *)MPI_WTIME_IS_GLOBAL_VAL );
	    /* Make these permanent.  Must do this AFTER the values are set (because
	       changing a value of a permanent attribute is an error) */
	    MPIR_Attr_make_perm( MPI_TAG_UB );
	    MPIR_Attr_make_perm( MPI_HOST );
	    MPIR_Attr_make_perm( MPI_IO );
	    MPIR_Attr_make_perm( MPI_WTIME_IS_GLOBAL );
	    MPIR_Attr_make_perm( MPIR_TAG_UB );
	    MPIR_Attr_make_perm( MPIR_HOST );
	    MPIR_Attr_make_perm( MPIR_IO );
	    MPIR_Attr_make_perm( MPIR_WTIME_IS_GLOBAL );
	    
	    /* Remember COMM_WORLD for the debugger */
	    MPIR_Comm_remember ( MPIR_COMM_WORLD );
	    
	    /* META */
#ifdef META
	}
	else {
	    
	    /* create  MPI_COMM_ALL including ALL MPI- and routing procs */
	    MPIR_ALLOC(MPIR_COMM_ALL,NEW(struct MPIR_COMMUNICATOR),
		       (struct MPIR_COMMUNICATOR *)0,
		       MPI_ERR_EXHAUSTED,myname);
	    MPIR_SET_COOKIE(MPIR_COMM_ALL,MPIR_COMM_COOKIE)
		MPIR_RegPointerIdx( MPI_COMM_ALL, MPIR_COMM_ALL );
	    MPIR_COMM_ALL->self = MPI_COMM_ALL;
	    
	    MPIR_COMM_ALL->comm_type	   = MPIR_INTRA;
	    MPIR_COMM_ALL->ADIctx	   = ADIctx;
	    size     = MPID_MyAllSize;
	    MPIR_tid = MPID_MyAllRank;
	    MPIR_COMM_ALL->group	   = MPIR_CreateGroup( size );
	    MPIR_COMM_ALL->group->self   = (MPI_Group) MPIR_FromPointer( MPIR_COMM_ALL->group );
	    /* what's that for ? */
#if defined(MPID_DEVICE_SETS_LRANKS)
	    MPID_Set_lranks ( MPIR_COMM_ALL->group );
#else
	    MPIR_SetToIdentity( MPIR_COMM_ALL->group );
	    /* XXX boris */
	    /*	MPIR_COMM_ALL->group->local_rank = MPID_MyAllRank; */
#endif
	    MPIR_Group_dup ( MPIR_COMM_ALL->group, &(MPIR_COMM_ALL->local_group) );
	    MPIR_COMM_ALL->local_rank	   = MPIR_COMM_ALL->local_group->local_rank;
	    MPIR_COMM_ALL->lrank_to_grank = MPIR_COMM_ALL->group->lrank_to_grank;
	    MPIR_COMM_ALL->np		   = MPIR_COMM_ALL->group->np;
	    MPIR_COMM_ALL->send_context   = MPIR_ALL_PT2PT_CONTEXT;
	    MPIR_COMM_ALL->recv_context   = MPIR_ALL_PT2PT_CONTEXT;
	    MPIR_COMM_ALL->error_handler  = MPI_ERRORS_ARE_FATAL;
	    MPIR_COMM_ALL->use_return_handler = 0;
	    MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
	    MPIR_COMM_ALL->ref_count	   = 1;
	    MPIR_COMM_ALL->permanent	   = 1;
	    MPID_CommInit( (struct MPIR_COMMUNICATOR *)0, MPIR_COMM_ALL );
	    
	    /* default msg represensation does not work for heterogenous cluster */
	    if (MPIR_meta_cfg.is_hetero)
		MPIR_COMM_ALL->msgform = MPID_MSG_XDR;
	    
	    MPIR_Attr_create_tree ( MPIR_COMM_ALL );
	    MPIR_COMM_ALL->comm_cache	   = 0;
	    
	    MPID_Set_mapping( MPIR_COMM_ALL, "MPI_COMM_ALL" );
	    
	    MPIR_Comm_make_coll ( MPIR_COMM_ALL, MPIR_INTRA );
	    
	    MPIR_COMM_ALL->comm_name      = 0;
	    MPI_Comm_set_name ( MPI_COMM_ALL, "MPI_COMM_ALL");
	    
	    /* Predefined attributes for MPI_COMM_ALL */
	    DEBUG(PRINTF("[%d] About to create keyvals\n", MPIR_tid);)
		
#define NULL_COPY (MPI_Copy_function *)0
#define NULL_DEL  (MPI_Delete_function*)0
		
		i = MPI_TAG_UB;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_HOST;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_IO;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    i = MPI_WTIME_IS_GLOBAL;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 0 WINEXT);
	    MPI_TAG_UB_VAL = MPID_TAG_UB;

#ifndef MPID_HOST
#define MPID_HOST MPI_PROC_NULL
#endif    
	    
	    MPI_HOST_VAL   = MPID_HOST;
	    
	    /* The following isn't strictly correct, but I'm going to leave it
	       in for now.  I've tried to make this correct for a few systems
	       for which I know the answer.  
	    */
	    /* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
	       perhaps for some other systems */
	    /*     MPI_IO_VAL = MPI_PROC_NULL; */
#ifndef MPID_IO
#define MPID_IO MPI_ANY_SOURCE
#endif
	    
	    MPI_IO_VAL = MPID_IO;
	    /* The C versions - pass the address of the variable containing the 
	       value */
	    MPI_Attr_put( MPI_COMM_ALL, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
	    MPI_Attr_put( MPI_COMM_ALL, MPI_HOST,   (void*)&MPI_HOST_VAL );
	    MPI_Attr_put( MPI_COMM_ALL, MPI_IO,     (void*)&MPI_IO_VAL );
	    
	    /* Do the Fortran versions - Pass the actual value.  Note that these
	       use MPIR_Keyval_create with the "is_fortran" flag set. 
	       If you change these; change the removal in finalize.c. */
	    i = MPIR_TAG_UB;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_HOST;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_IO;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    i = MPIR_WTIME_IS_GLOBAL;
	    MPIR_Keyval_create( NULL_COPY, NULL_DEL, &i, (void *)0, 1 WINEXT);
	    MPI_Attr_put( MPI_COMM_ALL, MPIR_TAG_UB, (void*)MPI_TAG_UB_VAL );
	    MPI_Attr_put( MPI_COMM_ALL, MPIR_HOST,   (void*)MPI_HOST_VAL );
	    MPI_Attr_put( MPI_COMM_ALL, MPIR_IO,     (void*)MPI_IO_VAL );
	    
	    /* Add the flag on whether the timer is global */
#ifdef MPID_Wtime_is_global
	    MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
	    MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
	    MPI_Attr_put( MPI_COMM_ALL, MPI_WTIME_IS_GLOBAL, (void *)&MPI_WTIME_IS_GLOBAL_VAL );
	    MPI_Attr_put( MPI_COMM_ALL, MPIR_WTIME_IS_GLOBAL, (void *)MPI_WTIME_IS_GLOBAL_VAL );
	    
	    /* permanent'ing of the attributes is done after 
	       creating the last communicator */
	    
	    /* Remember COMM_ALL for the debugger */
	    MPIR_Comm_remember ( MPIR_COMM_ALL );
	    
	    /* The Router uses COMM_ALL for COMM_WOLRD */
	    if (MPIR_meta_cfg.router)
		MPIR_COMM_WORLD = MPIR_COMM_ALL;
	    
	    if (!MPIR_meta_cfg.router) {
		/* create COMM_LOCAL which includes all local MPI-Processes without 
		   the routing processes */
		MPIR_ALLOC(MPIR_COMM_LOCAL,NEW(struct MPIR_COMMUNICATOR),
			   (struct MPIR_COMMUNICATOR *)0,
			   MPI_ERR_EXHAUSTED,myname);
		MPIR_SET_COOKIE(MPIR_COMM_LOCAL,MPIR_COMM_COOKIE);
		/* this is kind of a trick to give different communicators with
		   identical names on the involved hosts */
		/* MPI_COMM_LOCAL = MPI_COMM_LOCAL_BASE + MPIR_meta_cfg.my_metahost_rank; */
		MPIR_RegPointerIdx( MPI_COMM_LOCAL, MPIR_COMM_LOCAL );
		MPIR_COMM_LOCAL->self = MPI_COMM_LOCAL;
		
		MPIR_COMM_LOCAL->comm_type = MPIR_INTRA;
		MPIR_COMM_LOCAL->ADIctx	   = ADIctx;
		size     = MPID_MyLocalSize;
		MPIR_tid = MPID_MyLocalRank;
		MPIR_COMM_LOCAL->group	   = MPIR_CreateGroup( size );
		MPIR_COMM_LOCAL->group->self   = 
		    (MPI_Group) MPIR_FromPointer( MPIR_COMM_LOCAL->group );
#if defined(MPID_DEVICE_SETS_LRANKS)
		MPID_Set_lranks ( MPIR_COMM_LOCAL->group );
#else
		MPIR_SetToIdentity( MPIR_COMM_LOCAL->group );
#endif
		MPIR_Group_dup ( MPIR_COMM_LOCAL->group, 
				 &(MPIR_COMM_LOCAL->local_group) );
		
		MPIR_COMM_LOCAL->local_rank	   = MPIR_COMM_LOCAL->local_group->local_rank;
		MPIR_COMM_LOCAL->lrank_to_grank    = MPIR_COMM_LOCAL->group->lrank_to_grank;
		MPIR_COMM_LOCAL->np		   = MPIR_COMM_LOCAL->group->np;
		MPIR_COMM_LOCAL->send_context   = MPIR_LOCAL_PT2PT_CONTEXT;
		MPIR_COMM_LOCAL->recv_context   = MPIR_LOCAL_PT2PT_CONTEXT;
		MPIR_COMM_LOCAL->error_handler  = MPI_ERRORS_ARE_FATAL;
		MPIR_COMM_LOCAL->use_return_handler = 0;
		MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
		MPIR_COMM_LOCAL->ref_count	   = 1;
		MPIR_COMM_LOCAL->permanent	   = 1;
		MPID_CommInit( (struct MPIR_COMMUNICATOR *)0, MPIR_COMM_LOCAL );
		
		MPIR_Attr_create_tree ( MPIR_COMM_LOCAL );
		MPIR_COMM_LOCAL->comm_cache	   = 0;
		
		/* set the specific mapping from lranks to granks */
		MPID_Set_mapping (MPIR_COMM_LOCAL, "MPI_COMM_LOCAL");
		
		MPIR_Comm_make_coll ( MPIR_COMM_LOCAL, MPIR_INTRA );
		
		MPIR_COMM_LOCAL->comm_name      = 0;
		MPI_Comm_set_name ( MPI_COMM_LOCAL, "MPI_COMM_LOCAL");
		
		/* Predefined attributes for MPI_COMM_LOCAL */
		DEBUG(PRINTF("[%d] About to create keyvals\n", MPIR_tid);)
		    
#define NULL_COPY (MPI_Copy_function *)0
#define NULL_DEL  (MPI_Delete_function*)0
		    
		    /* keyvalues are to be created only once  (when initalizing COMM_WORLD) */
		    
#ifndef MPID_HOST
#define MPID_HOST MPI_PROC_NULL
#endif    
		    
		    MPI_HOST_VAL   = MPID_HOST;
		
		/* The following isn't strictly correct, but I'm going to leave it
		   in for now.  I've tried to make this correct for a few systems
		   for which I know the answer.  
		*/
		/* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
		   perhaps for some other systems */
		/*     MPI_IO_VAL = MPI_PROC_NULL; */
		
#ifndef MPID_IO
#define MPID_IO MPI_ANY_SOURCE
#endif
		
		MPI_IO_VAL = MPID_IO;
		
		/* The C versions - pass the address of the variable containing the 
		   value */
		MPI_Attr_put( MPI_COMM_LOCAL, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
		MPI_Attr_put( MPI_COMM_LOCAL, MPI_HOST,   (void*)&MPI_HOST_VAL );
		MPI_Attr_put( MPI_COMM_LOCAL, MPI_IO,     (void*)&MPI_IO_VAL );
		
		
		/* Do the Fortran versions - Pass the actual value.  Note that these
		   use MPIR_Keyval_create with the "is_fortran" flag set. 
		   If you change these; change the removal in finalize.c. */
		/* keyvalues are to be created only once  (when initalizing COMM_WORLD) */
		
		/* Add the flag on whether the timer is global */
		
#ifdef MPID_Wtime_is_global
		MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
		MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
		
		MPI_Attr_put( MPI_COMM_LOCAL, MPI_WTIME_IS_GLOBAL, (void *)&MPI_WTIME_IS_GLOBAL_VAL );
		MPI_Attr_put( MPI_COMM_LOCAL, MPIR_WTIME_IS_GLOBAL,(void *)MPI_WTIME_IS_GLOBAL_VAL );
		/* Make these permanent.  Must do this AFTER the values are set (because
		   changing a value of a permanent attribute is an error) 
		   MPIR_Attr_make_perm( MPI_TAG_UB );
		   MPIR_Attr_make_perm( MPI_HOST );
		   MPIR_Attr_make_perm( MPI_IO );
		   MPIR_Attr_make_perm( MPI_WTIME_IS_GLOBAL );
		   MPIR_Attr_make_perm( MPIR_TAG_UB );
		   MPIR_Attr_make_perm( MPIR_HOST );
		   MPIR_Attr_make_perm( MPIR_IO );
		   MPIR_Attr_make_perm( MPIR_WTIME_IS_GLOBAL );
		*/
		/* Remember COMM_LOCAL for the debugger */
		MPIR_Comm_remember ( MPIR_COMM_LOCAL );
		
		/* create COMMM_META which includes all MPI-Processes and is used
		   like COMM_WORLD by the application (in fact, a #define replaces
		   all occurences of MPI_COMM_WORLD by MPI_COMM_META) */
		MPIR_ALLOC(MPIR_COMM_META,NEW(struct MPIR_COMMUNICATOR), (struct MPIR_COMMUNICATOR *)0,
			   MPI_ERR_EXHAUSTED,myname);
		MPIR_SET_COOKIE(MPIR_COMM_META,MPIR_COMM_COOKIE);
		/* this is kind of a trick to give different communicators with
		   identical names on the involved hosts */
		MPIR_RegPointerIdx( MPI_COMM_META, MPIR_COMM_META );
		MPIR_COMM_META->self = MPI_COMM_META;
		
		MPIR_COMM_META->comm_type = MPIR_INTRA;
		MPIR_COMM_META->ADIctx	   = ADIctx;
		size     = MPID_MyMetaSize;
		MPIR_tid = MPID_MyMetaRank;
		MPIR_COMM_META->group	   = MPIR_CreateGroup( size );
		MPIR_COMM_META->group->self   =  (MPI_Group) MPIR_FromPointer( MPIR_COMM_META->group );
		
#if defined(MPID_DEVICE_SETS_LRANKS)
		MPID_Set_lranks ( MPIR_COMM_META->group );
#else
		MPIR_SetToIdentity( MPIR_COMM_META->group );
#endif
		MPIR_Group_dup ( MPIR_COMM_META->group, &(MPIR_COMM_META->local_group) );

		MPIR_COMM_META->local_rank     = MPIR_COMM_META->local_group->local_rank;
		MPIR_COMM_META->lrank_to_grank = MPIR_COMM_META->group->lrank_to_grank;
		MPIR_COMM_META->np	       = MPIR_COMM_META->group->np;
		MPIR_COMM_META->send_context   = MPIR_WORLD_PT2PT_CONTEXT;
		MPIR_COMM_META->recv_context   = MPIR_WORLD_PT2PT_CONTEXT;
		MPIR_COMM_META->error_handler  = MPI_ERRORS_ARE_FATAL;
		MPIR_COMM_META->use_return_handler = 0;
		MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
		MPIR_COMM_META->ref_count	   = 1;
		MPIR_COMM_META->permanent	   = 1;
		MPID_CommInit( (struct MPIR_COMMUNICATOR *)0, MPIR_COMM_META );
		/* default msg represensation does not work for heterogenous cluster */
		if (MPIR_meta_cfg.is_hetero)
		    MPIR_COMM_META->msgform = MPID_MSG_XDR;
		
		MPIR_Attr_create_tree ( MPIR_COMM_META );
		MPIR_COMM_META->comm_cache	   = 0;
		
		/* set the specific mapping from lranks to granks */
		MPID_Set_mapping (MPIR_COMM_META, "MPI_COMM_META");
		
		/* use special meta-barrier 
		   XXX is there a better (cleaner) way to install this new barrier ? */
		MPIR_Comm_make_coll ( MPIR_COMM_META, MPIR_INTRA );
		/* copy old collops pointer, then set pointer 
		   MPIR_Copy_collops (MPIR_COMM_META);
		   MPIR_REF_INCR (MPIR_COMM_META->collops);
		   MPIR_COMM_META->collops->Barrier = MPID_Gateway_Barrier;*/
		
		MPIR_COMM_META->comm_name      = 0;
		MPI_Comm_set_name ( MPI_COMM_META, "MPI_COMM_META");
		
		/* Predefined attributes for MPI_COMM_META */
		DEBUG(PRINTF("[%d] About to create keyvals\n", MPIR_tid);)
		    
#define NULL_COPY (MPI_Copy_function *)0
#define NULL_DEL  (MPI_Delete_function*)0
		    
		    /* keyvalues are to be created only once  (when initalizing COMM_WORLD) */
		    
#ifndef MPID_HOST
#define MPID_HOST MPI_PROC_NULL
#endif    
		    
		    MPI_HOST_VAL   = MPID_HOST;
		
		/* The following isn't strictly correct, but I'm going to leave it
		   in for now.  I've tried to make this correct for a few systems
		   for which I know the answer.  
		*/
		/* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
		   perhaps for some other systems */
		/*     MPI_IO_VAL = MPI_PROC_NULL; */
#ifndef MPID_IO
#define MPID_IO MPI_ANY_SOURCE
#endif
		
		MPI_IO_VAL = MPID_IO;
		/* The C versions - pass the address of the variable containing the 
		   value */
		MPI_Attr_put( MPI_COMM_META, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
		MPI_Attr_put( MPI_COMM_META, MPI_HOST,   (void*)&MPI_HOST_VAL );
		MPI_Attr_put( MPI_COMM_META, MPI_IO,     (void*)&MPI_IO_VAL );
		
		/* Do the Fortran versions - Pass the actual value.  Note that these
		   use MPIR_Keyval_create with the "is_fortran" flag set. 
		   If you change these; change the removal in finalize.c. */
		/* keyvalues are to be created only once  (when initalizing COMM_WORLD) */
		
		/* Add the flag on whether the timer is global */
#ifdef MPID_Wtime_is_global
		MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
		MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
		/* XXX - sind die Attrib notwendig ?
		   MPI_Attr_put( MPI_COMM_META, MPI_WTIME_IS_GLOBAL, 
		   (void *)&MPI_WTIME_IS_GLOBAL_VAL );
		   MPI_Attr_put( MPI_COMM_META, MPIR_WTIME_IS_GLOBAL, 
		   (void *)MPI_WTIME_IS_GLOBAL_VAL );
		*/
		
		/* Remember COMM_META for the debugger */
		MPIR_Comm_remember ( MPIR_COMM_META );
		
		/* The normal MPI procs use COMM_META for COMM_WOLRD */
		MPIR_COMM_WORLD = MPIR_COMM_META;
	    }
	    
	    /* create COMM_HOST which includes all local MPI-Processes and
	       the routing processes. It is used for transmitting messages
	       between the gateway- and the tunnel-ADI */
	    MPIR_ALLOC(MPIR_COMM_HOST,NEW(struct MPIR_COMMUNICATOR), (struct MPIR_COMMUNICATOR *)0,
		       MPI_ERR_EXHAUSTED,myname);
	    MPIR_SET_COOKIE(MPIR_COMM_HOST,MPIR_COMM_COOKIE)
		/* this is kind of a trick to give different communicators with
		   identical names on the involved hosts */
		/*	  	MPI_COMM_HOST = MPI_COMM_HOST_BASE + MPIR_meta_cfg.my_metahost_rank;*/
		MPIR_RegPointerIdx( MPI_COMM_HOST, MPIR_COMM_HOST );
	    MPIR_COMM_HOST->self = MPI_COMM_HOST;
	    
	    MPIR_COMM_HOST->comm_type	   = MPIR_INTRA;
	    MPIR_COMM_HOST->ADIctx	   = ADIctx;
	    size     = MPID_MyHostSize;
	    /* MPIR_tid = MPID_MyHostRank;*/
	    MPIR_COMM_HOST->group	   = MPIR_CreateGroup( size );
	    MPIR_COMM_HOST->group->self   = (MPI_Group) MPIR_FromPointer( MPIR_COMM_HOST->group );
#if defined(MPID_DEVICE_SETS_LRANKS)
	    MPID_Set_lranks ( MPIR_COMM_HOST->group ); /* XXX evtl. muss hier eingegriffen werden! */
#else
	    MPIR_SetToIdentity( MPIR_COMM_HOST->group );
#endif
	    MPIR_Group_dup ( MPIR_COMM_HOST->group, &(MPIR_COMM_HOST->local_group) );
	    
	    MPIR_COMM_HOST->local_rank	   = MPIR_COMM_HOST->local_group->local_rank;
	    MPIR_COMM_HOST->lrank_to_grank = MPIR_COMM_HOST->group->lrank_to_grank;
	    MPIR_COMM_HOST->np		   = MPIR_COMM_HOST->group->np;
	    MPIR_COMM_HOST->send_context   = MPIR_HOST_PT2PT_CONTEXT;
	    MPIR_COMM_HOST->recv_context   = MPIR_HOST_PT2PT_CONTEXT;
	    MPIR_COMM_HOST->error_handler  = MPI_ERRORS_ARE_FATAL;
	    MPIR_COMM_HOST->use_return_handler = 0;
	    MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
	    MPIR_COMM_HOST->ref_count	   = 1;
	    MPIR_COMM_HOST->permanent	   = 1;
	    MPID_CommInit( (struct MPIR_COMMUNICATOR *)0, MPIR_COMM_HOST );
	    
	    MPIR_Attr_create_tree ( MPIR_COMM_HOST );
	    MPIR_COMM_HOST->comm_cache	   = 0;
	    
	    /* set the specific mapping from lranks to granks */
	    MPID_Set_mapping (MPIR_COMM_HOST, "MPI_COMM_HOST");
	    
	    MPIR_Comm_make_coll ( MPIR_COMM_HOST, MPIR_INTRA );
	    
	    MPIR_COMM_HOST->comm_name      = 0;
	    MPI_Comm_set_name ( MPI_COMM_HOST, "MPI_COMM_HOST");
	    
	    /* Predefined attributes for MPI_COMM_HOST */
	    DEBUG(PRINTF("[%d] About to create keyvals\n", MPIR_tid);)
#define NULL_COPY (MPI_Copy_function *)0
#define NULL_DEL  (MPI_Delete_function*)0
		
		/* keyvalues are to be created only once  (when initalizing COMM_WORLD) */
		
#ifndef MPID_HOST
#define MPID_HOST MPI_PROC_NULL
#endif    
		
		MPI_HOST_VAL   = MPID_HOST;
	    
	    /* The following isn't strictly correct, but I'm going to leave it
	       in for now.  I've tried to make this correct for a few systems
	       for which I know the answer.  
	    */
	    /* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
	       perhaps for some other systems */
	    /*     MPI_IO_VAL = MPI_PROC_NULL; */
#ifndef MPID_IO
#define MPID_IO MPI_ANY_SOURCE
#endif
	    
	    MPI_IO_VAL = MPID_IO;
	    /* The C versions - pass the address of the variable containing the 
	       value */
	    MPI_Attr_put( MPI_COMM_HOST, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
	    MPI_Attr_put( MPI_COMM_HOST, MPI_HOST,   (void*)&MPI_HOST_VAL );
	    MPI_Attr_put( MPI_COMM_HOST, MPI_IO,     (void*)&MPI_IO_VAL );
	    
	    /* Do the Fortran versions - Pass the actual value.  Note that these
	       use MPIR_Keyval_create with the "is_fortran" flag set. 
	       If you change these; change the removal in finalize.c. */
	    /* keyvalues are to be created only once (when initalizing COMM_WORLD) */
	    
	    /* Add the flag on whether the timer is global */
	    
#ifdef MPID_Wtime_is_global
	    MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
	    MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
	    MPI_Attr_put( MPI_COMM_HOST, MPI_WTIME_IS_GLOBAL, (void *)&MPI_WTIME_IS_GLOBAL_VAL );
	    MPI_Attr_put( MPI_COMM_HOST, MPIR_WTIME_IS_GLOBAL, (void *)MPI_WTIME_IS_GLOBAL_VAL );
	    
	    /* Make these permanent.  Must do this AFTER the values are set (because
	       changing a value of a permanent attribute is an error) */
	    MPIR_Attr_make_perm( MPI_TAG_UB );
	    MPIR_Attr_make_perm( MPI_HOST );
	    MPIR_Attr_make_perm( MPI_IO );
	    MPIR_Attr_make_perm( MPI_WTIME_IS_GLOBAL );
	    MPIR_Attr_make_perm( MPIR_TAG_UB );
	    MPIR_Attr_make_perm( MPIR_HOST );
	    MPIR_Attr_make_perm( MPIR_IO );
	    MPIR_Attr_make_perm( MPIR_WTIME_IS_GLOBAL );
	    
	    /* Remember COMM_HOST for the debugger */
	    MPIR_Comm_remember ( MPIR_COMM_HOST );
	    
	}
#endif
	/* /META */
	
	/* COMM_SELF is the communicator consisting only of myself */
	MPIR_ALLOC(MPIR_COMM_SELF,NEW(struct MPIR_COMMUNICATOR),
		   (struct MPIR_COMMUNICATOR *)0,
		   MPI_ERR_EXHAUSTED,myname);
	MPIR_SET_COOKIE(MPIR_COMM_SELF,MPIR_COMM_COOKIE)
	    MPIR_RegPointerIdx( MPI_COMM_SELF, MPIR_COMM_SELF );
	MPIR_COMM_SELF->self = MPI_COMM_SELF;
	
	MPIR_COMM_SELF->comm_type		    = MPIR_INTRA;
	MPIR_COMM_SELF->group		    = MPIR_CreateGroup( 1 );
	MPIR_COMM_SELF->group->self   = 
	    (MPI_Group) MPIR_FromPointer( MPIR_COMM_SELF->group );
	MPIR_COMM_SELF->group->local_rank	    = 0;
	MPIR_COMM_SELF->group->lrank_to_grank[0] = MPIR_tid;
	MPIR_Group_dup ( MPIR_COMM_SELF->group, 
			 &(MPIR_COMM_SELF->local_group) );
	MPIR_COMM_SELF->local_rank	      = 
	    MPIR_COMM_SELF->local_group->local_rank;
	MPIR_COMM_SELF->lrank_to_grank     = 
	    MPIR_COMM_SELF->group->lrank_to_grank;
	MPIR_COMM_SELF->np		      = MPIR_COMM_SELF->group->np;
	MPIR_COMM_SELF->send_context	      = MPIR_SELF_PT2PT_CONTEXT;
	MPIR_COMM_SELF->recv_context	      = MPIR_SELF_PT2PT_CONTEXT;
	MPIR_COMM_SELF->error_handler      = MPI_ERRORS_ARE_FATAL;
	MPIR_COMM_SELF->use_return_handler = 0;
	MPIR_Errhandler_mark( MPI_ERRORS_ARE_FATAL, 1 );
	MPIR_COMM_SELF->ref_count	      = 1;
	MPIR_COMM_SELF->permanent	      = 1;
	(void)MPID_CommInit( MPIR_COMM_WORLD, MPIR_COMM_SELF );
	MPIR_Attr_create_tree ( MPIR_COMM_SELF );
	MPIR_COMM_SELF->comm_cache	      = 0;
	MPIR_Comm_make_coll ( MPIR_COMM_SELF, MPIR_INTRA );
	/* Remember COMM_SELF for the debugger */
	MPIR_COMM_SELF->comm_name          = 0;
	MPI_Comm_set_name ( MPI_COMM_SELF, "MPI_COMM_SELF");
	MPIR_Comm_remember ( MPIR_COMM_SELF );
	
#if !defined(MPID_NO_FORTRAN)
	/* fcm sets MPI_BOTTOM */
#if  !defined(WIN32)
	mpir_init_flog_( &MPIR_F_TRUE, &MPIR_F_FALSE );
	mpir_init_fcm_( );
#endif
#endif
	
	MPIR_PointerPerm( 0 );
	
	DEBUG(PRINTF("[%d] About to search for argument list options\n",MPIR_tid);)
	    
	    /* Search for "-mpi debug" options etc.  We need a better interface.... */
	    if (argv && *argv) {
		int i;
		for (i=1; i<*argc; i++) {
		    if ((*argv)[i]) {
			if (strcmp( (*argv)[i], "-mpiqueue" ) == 0) {
			    MPIR_Print_queues = 1;
			    (*argv)[i] = 0;
			}
			else if (strcmp((*argv)[i],"-metaverbose" ) == 0) {
#ifdef META
			    RDEBUG_verbose = 1;
#endif /* META */
			    (*argv)[i] = 0;
			    
			}
			else if (strcmp((*argv)[i],"-mpiversion" ) == 0) {
			    char ADIname[MPID_MAX_VERSION_NAME];
			    PRINTF( "MP-MPICH version %3.1f.%d%s\n", PATCHLEVEL, PATCHLEVEL_SUBMINOR, 
				    PATCHLEVEL_RELEASE_KIND);
			    PRINTF ("  %s\n", PATCHLEVEL_RELEASE_DATE);
			    PRINTF ("  build of %s\n", __DATE__);
			    MPID_Version_name( ADIname );
			    PRINTF( "%s\n", ADIname);
			    PRINTF( "Configured with %s\n", CONFIGURE_ARGS_CLEAN );
			    (*argv)[i] = 0;
			}
#ifdef HAVE_NICE
			else if (strcmp((*argv)[i],"-mpinice" ) == 0) {
			    int niceincr;
			    (*argv)[i] = 0;
			    i++;
			    if (i <*argc) {
				niceincr = atoi( (*argv)[i] );
				nice(niceincr);
				(*argv)[i] = 0;
			    }
			    else {
				printf( "Missing argument for -mpinice\n" );
			    }
			}
#endif
#ifdef FOO
#if defined(MPE_USE_EXTENSIONS) && !defined(MPI_NO_MPEDBG)
			else if (strcmp((*argv)[i],"-mpedbg" ) == 0) {
			    MPE_Errors_call_dbx_in_xterm( (*argv)[0], (char *)0 ); 
			    MPE_Signals_call_debugger();
			    (*argv)[i] = 0;
			}
#endif
#if defined(MPE_USE_EXTENSIONS) && !defined(MPI_NO_MPEDBG)
			else if (strcmp((*argv)[i],"-mpegdb" ) == 0) {
			    MPE_Errors_call_gdb_in_xterm( (*argv)[0], (char *)0 ); 
			    MPE_Signals_call_debugger();
			    (*argv)[i] = 0;
			}
#endif
#endif
			else if (strcmp((*argv)[i],"-mpichtv" ) == 0) {
			    (*argv)[i] = 0; /* Eat it up so the user doesn't see it */
			    
			    /* Cause extra state to be remembered */
			    MPIR_being_debugged = 1;
			    
			    /* As per Jim Cownie's request #3683; allows debugging even if this startup
			       code should not be used. */
			    /* The real answer is to use a different definition for this, since
			       stop-when-starting-for-debugger is different from HAS_PROC_INFO */
#ifdef MPID_HAS_PROC_INFO
			    /* Check to see if we're not the master,
			     * and wait for the debugger to attach if we're 
			     * a slave. The debugger will reset the debug_gate.
			     * There is no code in the library which will do it !
			     */
			    if (MPID_MyWorldRank != 0) {
				while (MPIR_debug_gate == 0) {
				    /* Wait to be attached to, select avoids 
				     * signaling and allows a smaller timeout than 
				     * sleep(1)
				     */
#ifndef WIN32
				    struct timeval timeout;
				    timeout.tv_sec  = 0;
				    timeout.tv_usec = 250000;
				    select( 0, (void *)0, (void *)0, (void *)0,
					    &timeout );
#else
				    Sleep(250);
#endif
				}
			    }
#endif
			}
			else if (strcmp((*argv)[i],"-mpichksq") == 0) {
			    /* This tells us to Keep Send Queues so that we 
			     * can look at them if we're attached to.
			     */
			    (*argv)[i] = 0; /* Eat it up so the user doesn't see it */
			    MPIR_being_debugged = 1;
			}
			
#ifdef MPIR_PTRDEBUG
			else if (strcmp((*argv)[i],"-mpiptrs") == 0) {
			    MPIR_Dump_Mem = 1;
			}
#endif
#ifdef MPIR_MEMDEBUG
			else if (strcmp((*argv)[i],"-mpimem" ) == 0) {
			    MPID_trDebugLevel( 1 );
			}
#endif
		    }
		}
		/* Remove the null arguments */
		MPID_ArgSqueeze( argc, *argv );
	    }
	
	/* barrier */
	MPIR_Has_been_initialized = 1;
	
	/* META */
#ifdef META
	/* XXX debug barrier 
	   while (meta_barrier) {
	   usleep (1000);
	   }
	*/
	
	/* print some debugging information  */
	/* debug_print_communicator_setup(); */
	
	
	if (MPIR_meta_cfg.router) {
	    /* this process never returns from MPI_Init() but becomes a  routing process */
	    MPIR_Router(meta_config_file, MPIR_meta_cfg.metahost_firstrank,
			metacon,
			MPIR_meta_cfg.my_metahostname);      
	    exit (0);
	}
	/* wait until the routing procs are synchronized */
	if (MPIR_meta_cfg.nbr_metahosts > 1) {
	    MPI_Barrier (MPI_COMM_HOST);
	}
#endif
	/* /META */
#ifdef VIOLAIO
	/* TODO: Here, we have only user Processes and Extra Processes */
	
	/* first let's determine if this is an extra process */
	{
	    int my_meta_rank;
	    int i;
	    
	    if (MPI_COMM_META != MPI_COMM_NULL)
		MPI_Comm_rank(MPI_COMM_META, &my_meta_rank);
	    
	    /*
	      fprintf(stderr, "[%i] running binary: %s\n", my_meta_rank, (*argv)[0]);
	    */
	    if (strstr((*argv)[0], "tunnelfs_io_server") != NULL)
		MPIR_meta_cfg.extra = 1;
	    
	    /*
	      for (i=0; i<MPIR_meta_cfg.npExtra; i++)
	      if (my_meta_rank == MPIR_meta_cfg.npExtra_ranks[i]) 
	      MPIR_meta_cfg.extra = 1;
	    */
	}
	
	/* distribute ranks of extra procs to all client procs */
	{
	    int i,j;
	    void *extras = NULL;
	    int count;
	    int master_set = 0;
	    
	    
	    count = MPIR_meta_cfg.np + MPIR_meta_cfg.npExtra;
	    
	    if (count > 0)
		{
		    extras = malloc(count * sizeof(int));
		    
		    if (extras == NULL)
			{
			    fprintf(stderr, "Memory allocation failed for list of extra processes! (%i)\n", count); 
			    exit(-1);
			}
		    
		    MPI_Allgather(&(MPIR_meta_cfg.extra), 1, MPI_INT, extras, 1, MPI_INT,
				  MPI_COMM_META);
		    
		    j=0;
		    for (i=0; i < MPIR_meta_cfg.np + MPIR_meta_cfg.npExtra; i++)
			{
			    if (((int*)extras)[i] != 0)
				{
				    if (!master_set) 
					{
					    /* first discovered IO Server will be master */
					    TUNNELFS_GLOBAL_MASTER = i;
					    master_set = 1;
					}
				    MPIR_meta_cfg.npExtra_ranks[j++] = i;
				    break;
				}
			}
		    if (extras != NULL) free(extras);
		}
	}
	
	/* we have at least ONE io server */
	if (MPI_COMM_LOCAL != MPI_COMM_NULL)
	    MPI_Comm_split(MPI_COMM_LOCAL, MPIR_meta_cfg.extra, 0,
			   &MPI_COMM_LOCAL_REDUCED);
	
	if (MPI_COMM_META != MPI_COMM_NULL)
	    {
		int rank;

                /* seperate servers from clients */
                MPI_Comm_split(MPI_COMM_META, MPIR_meta_cfg.extra, 0,
                               &MPI_COMM_META_REDUCED);
		
                /* create new singleton communicator */
                MPI_Comm_rank(MPI_COMM_META, &rank);
                MPI_Comm_split(MPI_COMM_META, rank, 0, &MPI_COMM_TUNNELFS_SELF);

                fprintf(stderr, "Duplicating comm\n");
                /* create global communicator for tunnelfs messages */
                MPI_Comm_dup(MPI_COMM_META, &MPI_COMM_TUNNELFS_WORLD);
	    }
	
	if (MPI_COMM_META_REDUCED != 143) 
	    fprintf(stderr, "MPI_COMM_META_REDUCED has wrong value: %i\n", 
		    MPI_COMM_META_REDUCED);
	if (MPI_COMM_LOCAL_REDUCED != 140) 
	    fprintf(stderr, "MPI_COMM_LOCAL_REDUCED has wrong value: %i\n", 
		    MPI_COMM_LOCAL_REDUCED);
	if (MPI_COMM_TUNNELFS_SELF != 146) 
	    fprintf(stderr, "MPI_COMM_TUNNELFS_SELF has wrong value: %i\n", 
		    MPI_COMM_TUNNELFS_SELF);
        fprintf(stderr, "MPI_COMM_TUNNELFS_WORLD has value of %i\n",
                MPI_COMM_TUNNELFS_WORLD);
	
	if (!MPIR_meta_cfg.extra && (TUNNELFS_GLOBAL_MASTER >= 0))
	    {
		void *buf = NULL;
		int buf_size = 0;
		int recvd = 0;
		int msg_id = TUNNELFS_NEXT_MSG_ID;
		
		/* I am an io client and have to contact the master io server */
		
		/* sending init request */
		tunnelfs_msg_send_init(&buf, &buf_size, *argc, *argv);
		
		/* receiving reply */
		tunnelfs_msg_get_reply(&buf, &buf_size, &recvd, TUNNELFS_GLOBAL_MASTER, msg_id);
		
		free(buf);
	    }
#endif
	
	DEBUG(PRINTF("[%d] About to exit from MPI_Init\n", MPIR_tid);)
	    TR_POP;
	return MPI_SUCCESS;
}


#ifndef MPID_NO_FORTRAN

#ifdef WIN32

void __cdecl mpir_init_bottom_win32( void *p ) {
    MPIR_F_MPI_BOTTOM	   = p;
    MPIR_F_STATUS_IGNORE   = ((MPI_Fint*)p) + 1;
    MPIR_F_STATUSES_IGNORE = ((MPI_Fint*)p) + 2;
    
    
}

#else
/* 
   This routine is CALLED by MPIR_init_fcm to provide the address of 
   the Fortran MPI_BOTTOM to C 
*/ 
void FORTRAN_API mpir_init_bottom_( p )
     void *p;
{
    MPIR_F_MPI_BOTTOM	   = p;
    MPIR_F_STATUS_IGNORE   = ((MPI_Fint*)p) + 1;
    MPIR_F_STATUSES_IGNORE = ((MPI_Fint*)p) + 2;
}

#endif


#endif /* MPID_NO_FORTRAN */

/****************************************************************************/
/* The various MPI objects (MPI_Errhandler, MPI_Op, ... ) require some      */
/* special routines to initialize and manipulate them.  For the "smaller"   */
/* objects, that code is here.  The larger objects (e.g., MPI_Comm)         */
/* have their own xxx_util.c or initxxx.c files that contain the needed     */
/* code.                                                                    */
/****************************************************************************/
/* Utility code for Errhandlers                                             */
/****************************************************************************/
#define MPIR_SBalloc MPID_SBalloc
int MPIR_Errhandler_create( function, errhandler )
     MPI_Handler_function *function;
     MPI_Errhandler       errhandler;
{
	struct MPIR_Errhandler *new;
	
	MPIR_ALLOC(new,(struct MPIR_Errhandler*) MPIR_SBalloc( MPIR_errhandlers ),
		   MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		   "MPI_ERRHANDLER_CREATE" );
	
	MPIR_SET_COOKIE(new,MPIR_ERRHANDLER_COOKIE);
	new->routine   = function;
	new->ref_count = 1;
	new->type = MPIR_PREDEFINED_HANDLER;
	MPIR_RegPointerIdx( errhandler, new );
	return MPI_SUCCESS;
}

/* Change the reference count of errhandler by incr */
#ifdef MPIR_ToPointer
#undef MPIR_ToPointer
#endif
void MPIR_Errhandler_mark( errhandler, incr )
     MPI_Errhandler errhandler;
     int            incr;
{
    struct MPIR_Errhandler *new = (struct MPIR_Errhandler *) 
	MPIR_ToPointer( errhandler );
    if (new) {
	if (incr == 1) {
	    MPIR_REF_INCR(new);
	}
	else {
	    MPIR_REF_DECR(new);
	}
    }
}


#ifdef META

/* this function is meant for debugging; it prints out all information in the communicators that
   are relevant for rank mapping, so one can see if the commuinicators are properly set up in
   a meta configuration */
void debug_print_communicator_setup()
{
    struct MPIR_COMMUNICATOR *comm;
    int n;
    
    printf( "\n--------------------------------------------------\n" );
    printf( "Communicator Setup:\n" );
    if( MPIR_meta_cfg.router ) {
	/* in router processes, only MPI_COMM_ALL and MPIR_COMM_HOST are relevant, because
	   routers do not belong to the other two communicators */
	printf( "  router process:\n" );
	
	comm = MPIR_COMM_ALL;
	printf( "    MPI_COMM_ALL: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
	
	comm = MPIR_COMM_HOST;
	printf( "    MPI_COMM_HOST: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
    }
    else {
	printf( "  application process:\n" );
	
	comm = MPIR_COMM_ALL;
	printf( "    MPI_COMM_ALL: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
	
	comm = MPIR_COMM_WORLD;
	printf( "    MPI_COMM_WORLD: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
	
	comm = MPIR_COMM_HOST;
	printf( "    MPI_COMM_HOST: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
	
	comm = MPIR_COMM_LOCAL;
	printf( "    MPI_COMM_LOCAL: size = %d, rank = %d, lrank_to_grank = ", comm->np, comm->local_rank );
	for( n = 0; n < comm->np; n++ )
	    printf( "%d ", comm->lrank_to_grank[n] );
	printf( "\n" );
    }
    
    printf( "--------------------------------------------------\n" );
}

#endif
