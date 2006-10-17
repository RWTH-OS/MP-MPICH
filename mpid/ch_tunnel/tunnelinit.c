/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it

    Special version for the Tunnel-Device which is a wrapper for the
    real CH2-device on this host.
 */

#include <stdio.h>

#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "../../src/routing/rhlist.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)


/* pointers to native devices on metahost */
MPID_Device **MPID_Tunnel_native_dev;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_Tunnel_End ANSI_ARGS(( MPID_Device * ));
int MPID_Tunnel_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));
int MPID_Tunnel_long_len( int );
int MPID_Tunnel_vlong_len( int );

extern MPID_Protocol *MPID_Tunnel_Short_setup ANSI_ARGS(( void ));
extern MPID_Protocol *MPID_Tunnel_Eagern_setup ANSI_ARGS(( void ));
extern MPID_Protocol *MPID_Tunnel_Rndvn_setup ANSI_ARGS(( void ));


/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *MPID_Tunnel_InitMsgPass( argc, argv, short_len, long_len )
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

    dev->long_len  = &MPID_Tunnel_long_len;
    dev->vlong_len = &MPID_Tunnel_vlong_len;

    dev->short_msg    = MPID_Tunnel_Short_setup();
    dev->long_msg     = MPID_Tunnel_Eagern_setup();
    dev->vlong_msg    = MPID_Tunnel_Rndvn_setup();
    
    /* same as above */
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;

    dev->check_device = MPID_Tunnel_Check_incoming;
    dev->terminate    = MPID_Tunnel_End;
    dev->cancel       = 0;
    dev->abort	      = MPID_Tunnel_Abort;
    dev->wtime	      = 0;
    dev->collops_init = 0;
    dev->comm_init    = 0;
    dev->comm_free    = 0;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif

    MPID_DO_HETERO(MPID_CH_Init_hetero( argc, argv ));

    DEBUG_PRINT_MSG("Finished init");

    DEBUG_PRINT_MSG("Leaving MPID_Tunnel_InitMsgPass");

    return dev;
}


/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_Tunnel_Abort( comm, code, msg )
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

    /* This needs to try and kill any connected routing processes 
       (XXX to be implemented) */
    /* kill_routing_procs(); */

    exit(code);
    return 0;
}

int MPID_Tunnel_End( dev )
MPID_Device *dev;
{
    DEBUG_PRINT_MSG("Entering MPID_Tunnel_End\n");
    /* Finish off any pending transactions */
    /* MPID_Tunnel_Complete_pending(); */

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );
    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    MPID_Tunnel_finalize();
    return 0;
}

void MPID_Tunnel_Version_name( name )
char *name;
{
    sprintf( name, "Tunnel ADI version %4.2f", MPIDPATCHLEVEL );
}


int MPID_Tunnel_long_len( int partner_devlrank )
{
    int partner_grank;
    MPID_Device *dev;

    /* device relative ranks for ch_tunnel are ranks in MPI_COMM_HOST;
       get global rank of partner process */
    partner_grank = MPIR_COMM_HOST->lrank_to_grank[partner_devlrank];

    /* get device via which to communicate with partner process */
    dev = MPID_Tunnel_native_dev[partner_grank];

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
      int temp = MPID_Device_call_long_len ( partner_devlrank, dev );

      MPID_devset->active_dev = me;
      return temp;
    }	
#else
    return dev->long_len( partner_devlrank );
#endif
}

int MPID_Tunnel_vlong_len( int partner_devlrank )
{
    int partner_grank;
    MPID_Device *dev;

    /* device relative ranks for ch_tunnel are ranks in MPI_COMM_HOST;
       get global rank of partner process */
    partner_grank = MPIR_COMM_HOST->lrank_to_grank[partner_devlrank];

    /* get device via which to communicate with partner process */
    dev = MPID_Tunnel_native_dev[partner_grank];

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
      int temp = MPID_Device_call_vlong_len ( partner_devlrank, dev );

      MPID_devset->active_dev = me;
      return temp;
    }	
#else
    return dev->vlong_len( partner_devlrank );
#endif
}
