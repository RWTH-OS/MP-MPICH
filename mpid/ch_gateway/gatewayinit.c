/*
 *  $Id$
 *
 *  (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 */

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it

    Special version for the Gateway-Device which is a wrapper for the
    real CH2-device on this host.
 */
#include <stdio.h>
#include <assert.h>

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "../../src/routing/rhlist.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)


/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
void MPID_Set_mapping ANSI_ARGS(( struct MPIR_COMMUNICATOR *, char *));
int MPID_Gateway_End ANSI_ARGS(( MPID_Device * ));
int MPID_Gateway_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));

int MPID_Gateway_long_len( int );
int MPID_Gateway_vlong_len( int );

extern MPID_Protocol *MPID_Gateway_Short_setup ANSI_ARGS(( void ));
extern MPID_Protocol *MPID_Gateway_Eagern_setup  ANSI_ARGS(( void ));
extern MPID_Protocol *MPID_Gateway_Rndvn_setup  ANSI_ARGS(( void ));

/* implemented in gatewaycancel.c */
extern int MPID_Gateway_SendCancelPacket ANSI_ARGS(( MPIR_SHANDLE * ) );

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *MPID_Gateway_InitMsgPass( argc, argv, short_len, long_len )
int  *argc;
char ***argv;
int  short_len, long_len;
{
    MPID_Device *dev;

    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );    

    /* the one and only device that is allready initialized is the real
       communication device that we will use */
    if (!(MPID_devset->ndev > 1)) {
      return (MPID_Device *)0;
    }  

    if( !dev ) 
	return (MPID_Device *)0;

    /* we use the same protocol-break-lenghts */
    dev->long_len     = &MPID_Gateway_long_len;
    dev->vlong_len    = &MPID_Gateway_vlong_len;
    dev->short_msg    = MPID_Gateway_Short_setup();
    dev->long_msg     = MPID_Gateway_Eagern_setup();
    dev->vlong_msg    = MPID_Gateway_Rndvn_setup();
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->check_device = MPID_Gateway_Check_incoming;
    dev->terminate    = MPID_Gateway_End;
    dev->cancel       = MPID_Gateway_SendCancelPacket;
    dev->abort	      = MPID_Gateway_Abort;
    dev->wtime        = 0;
    dev->collops_init = 0;
    dev->comm_init    = 0;
    dev->comm_free    = 0;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif
    MPID_Gateway_init( argc, *argv );
    MPID_DO_HETERO(MPID_CH_Init_hetero( argc, argv ));

    DEBUG_PRINT_MSG("Finished init");

    DEBUG_PRINT_MSG("Leaving MPID_Gateway_InitMsgPass");

    return dev;
}

void MPID_Set_mapping ANSI_ARGS((comm, comm_name))
struct MPIR_COMMUNICATOR *comm;
char *comm_name;
{
    struct MPIR_GROUP *grp;
    int *map;

    assert (comm != NULL);
    grp = comm->group;
    assert (grp != NULL);
    map = grp->lrank_to_grank;    /* should already be allocated */
    assert (map != NULL);
    assert (comm_name != NULL);
    
    /* COMM_HOST */
    if (!strcmp(comm_name, "MPI_COMM_HOST")) {
	int proc;

	for (proc = 0; proc < grp->np; proc++) 
	    map[proc] = MPIR_meta_cfg.metahost_firstrank + proc;
	grp->local_rank = MPID_MyHostRank;
	comm->local_rank = MPID_MyHostRank;

	return;
    }

    /* COMM_LOCAL */
    if (!strcmp(comm_name, "MPI_COMM_LOCAL")) {
	int app_proc, proc;

	app_proc = 0;
	proc = 0;

	while( proc < ( MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank]
			+ MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank] )  ) {
	    if( !MPIR_meta_cfg.isRouter[proc] ) {
		map[app_proc] = MPIR_meta_cfg.metahost_firstrank + proc;
		app_proc++;
	    }		
	    proc++;
	}

	grp->local_rank = MPID_MyLocalRank;
	comm->local_rank = MPID_MyLocalRank;
	
	return;
    }
    
#ifdef VIOLAIO
    /* COMM_LOCAL_EXTRA */
    if (!strcmp(comm_name, "MPI_COMM_LOCAL_EXTRA")) {
	int app_proc, proc;

	app_proc = 0;
	proc = 0;

	while( proc < ( MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank]
			+ MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank] )  ) {
	    if( !MPIR_meta_cfg.isRouter[proc] ) {
		map[app_proc] = MPIR_meta_cfg.metahost_firstrank + proc;
		app_proc++;
	    }		
	    proc++;
	}

	grp->local_rank = MPID_MyLocalRank;
	comm->local_rank = MPID_MyLocalRank;
	
	return;
    }
#endif
  /* VIOLAIO */

    /* COMM_META */
    if (!strcmp(comm_name, "MPI_COMM_META")) {
	int local_rank, global_rank, metahost, app_proc;
	Snode *node;

	local_rank = 0;
	global_rank = 0;
	/* loop over all metahosts */
	for( metahost = 0; metahost < rh_getNumMetahosts(); metahost++ ) {
	    /* loop over all nodes on the metahost */
	    node = metahostlist[metahost].nodeList;
	    while( node ) {
		/* routers are the first processes on the node */
		global_rank += node->numRouters;
		/* the other procs on the node are procs in MPI_COMM_META -> set mapping for them */
		for( app_proc = 0; app_proc < node->np; app_proc++ ) {
		    map[local_rank] = global_rank;
		    local_rank++;
		    global_rank++;
		}
		node = node->next;
	    }
	}

	grp->local_rank = MPID_MyMetaRank;
	comm->local_rank = MPID_MyMetaRank;
	
	return;
    }

#ifdef VIOLAIO
    /* COMM_META */
    if (!strcmp(comm_name, "MPI_COMM_META_EXTRA")) {
	int local_rank, global_rank, metahost, app_proc;
	Snode *node;

	local_rank = 0;
	global_rank = 0;
	/* loop over all metahosts */
	for( metahost = 0; metahost < rh_getNumMetahosts(); metahost++ ) {
	    /* loop over all nodes on the metahost */
	    node = metahostlist[metahost].nodeList;
	    while( node ) {
		/* routers are the first processes on the node */
		global_rank += node->numRouters;
		/* the other procs on the node are procs in MPI_COMM_META -> set mapping for them */
		for( app_proc = 0; app_proc < node->np; app_proc++ ) {
		    map[local_rank] = global_rank;
		    local_rank++;
		    global_rank++;
		}
		node = node->next;
	    }
	}

	grp->local_rank = MPID_MyMetaRank;
	comm->local_rank = MPID_MyMetaRank;
	
	return;
    }
#endif
  /* VIOLAIO */
  
    /* COMM_ALL */
    /* for COMM_ALL, the mapping set by MPIR_SetToIdentity() is correct and we assume here,
       that this function has already been called for this communicator, but we must set here
       grp->local_rank and comm->local_rank, because they are not set corretly by MPIR_SetToIdentity() */
    if( !strcmp( comm_name, "MPI_COMM_ALL" ) ) {
	grp->local_rank = MPID_MyAllRank;
	comm->local_rank = MPID_MyAllRank;
	
	return;
    }
    
    /* if it is none of the known communicators, create the default mapping */
    MPIR_SetToIdentity( grp );
    return;
}

int MPID_Gateway_Abort( comm, code, msg )
struct MPIR_COMMUNICATOR * comm;
int      code;
char     *msg;
{
    static int in_abort = 0;

    if (in_abort) exit(code);
    in_abort = 1;

    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
    }
    
    fflush( stderr );
    fflush( stdout );

    exit(code);
    return 0;
}

int MPID_Gateway_End( dev )
MPID_Device *dev;
{
    DEBUG_PRINT_MSG("Entering MPID_Gateway_End\n");
    /* Finish off any pending transactions */
    /* MPID_Gateway_Complete_pending(); */

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );
    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    MPID_Gateway_finalize();
    return 0;
}

void MPID_Gateway_Version_name( name )
char *name;
{
    sprintf( name, "Gateway ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}

int MPID_Gateway_long_len( int partner_devlrank )
{
    int router_grank;
    MPID_Device *dev;

    /* device relative ranks of ch_gateway are global ranks; get global
       rank of router process */
    router_grank = MPIR_meta_cfg.granks_to_router[partner_devlrank];

    /* get device via which to send to router */
    dev = MPID_devset->dev[router_grank];

#if 1
    /*
     |   XXX
     |   Since the (multi-) device instanciability, always the adi2-wrapper functions
     |   must be called instead of the direct device function pointers!
     |
     |   (remove this comment and the obsolete part in future!)
     */
    {
      MPID_Device *me = MPID_devset->active_dev;
      int temp = MPID_Device_call_long_len ( dev->grank_to_devlrank[router_grank], dev );

      MPID_devset->active_dev = me;
      return temp;
    }	
#else
    /* return threshold value for sending to router proc */
    return dev->long_len( dev->grank_to_devlrank[router_grank] );
#endif
}

int MPID_Gateway_vlong_len( int partner_devlrank )
{
    int router_grank;
    MPID_Device *dev;

    /* device relative ranks of ch_gateway are global ranks; get MPI_COMM_HOST
       rank of router process */
    router_grank = MPIR_meta_cfg.granks_to_router[partner_devlrank];

    /* get device via which to send to router */
    dev = MPID_devset->dev[router_grank];

#if 1
    /*
     |   XXX
     |   Since the (multi-) device instanciability, always the adi2-wrapper functions
     |   must be called instead of the direct device function pointers!
     |
     |   (remove this comment and the obsolete part in future!)
     */
    {
      MPID_Device *me = MPID_devset->active_dev;
      int temp = MPID_Device_call_vlong_len ( dev->grank_to_devlrank[router_grank], dev );

      MPID_devset->active_dev = me;
      return temp;
    }	
#else
    /* return threshold value for sending to router proc */
    return dev->vlong_len( dev->grank_to_devlrank[router_grank] );
#endif
}

