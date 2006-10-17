/*
 *  $Id: usockinit.c 3252 2005-03-21 09:28:13Z boris $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

/* We put these include FIRST incase we are building the memory debugging
   version; since these includes may define malloc etc., we need to include 
   them before mpid.h 
 */

#define _DEBUG_EXTERN_REC
#include "mydebug.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0400

#if (_MSC_VER < 1100)
#define NOMINMAX 
#include <winsock2.h>
#endif

#include <wtypes.h>
#include <winbase.h>
#endif

#ifndef HAVE_STDLIB_H
extern char *getenv();
#else
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "flow.h"
#include <stdio.h>

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int MPID_CH_USOCK_long_len_value;
static int MPID_CH_USOCK_vlong_len_value;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_USOCK_End ANSI_ARGS(( MPID_Device * ));
int MPID_USOCK_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));
void MPID_CH_USOCK_Version_name ANSI_ARGS(( char * ));

int MPID_CH_USOCK_long_len( int );
int MPID_CH_USOCK_vlong_len( int );

MPID_Device *MPID_CH_USOCK_InitMsgPass( argc, argv, short_len, long_len )
int  *argc;
char ***argv;
int  short_len, long_len;
{
    DSECTION("MPID_CH_USOCK_InitMsgPass");
    MPID_Device *dev;

    DSECTENTRYPOINT;
    
    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev)  {
	DSECTLEAVE
	    return 0;
    }
    /* The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet */
    if (short_len < 0) short_len = MPID_PKT_MAX_DATA_SIZE; /* <- defined in usockpackets.h */
    if (long_len < 0)  long_len  = 128000;
    MPID_CH_USOCK_long_len_value = short_len;
    dev->long_len     = &MPID_CH_USOCK_long_len;
    MPID_CH_USOCK_vlong_len_value = long_len;
    dev->vlong_len    = &MPID_CH_USOCK_vlong_len;

    dev->short_msg    = MPID_USOCK_Short_setup();
    dev->long_msg     = MPID_USOCK_Eagern_setup();   
    
 /* dev->vlong_msg    = MPID_USOCK_Rndvb_setup();  */
    dev->vlong_msg    = MPID_USOCK_Eagern_setup(); /* <- Since the Rendezvous-Protocol leads no good performance and
						    |    shows a trend to deadlocks, it is temporarily disabled.
						    */
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->check_device = MPID_USOCK_Check_incoming;
    dev->terminate    = MPID_USOCK_End;
    dev->abort	      = MPID_USOCK_Abort;
    dev->cancel	      = MPID_USOCK_SendCancelPacket;
    dev->wtime        = 0;
    dev->collops_init = 0;
    dev->comm_init    = 0;
    dev->comm_free    = 0;
    
    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
    /* mixed = 0; */
#endif
    MPID_USOCK_Init( argc, argv );
    dev->next	      = 0;
    SetThreadAffinityMask(GetCurrentThread(),1);

    MPID_USOCK_DEBUG_PRINT_MSG("Finished init");
    MPID_DO_HETERO(MPID_CH_Init_hetero( argc, argv ));
    
#ifdef MPID_FLOW_CONTROL
/* Try to get values for thresholds.  Note that everyone MUST have
   the same values for this to work */
    {int buf_thresh = 0, mem_thresh = 0;
    char *val;
    val = getenv( "MPI_BUF_THRESH" );
    if (val) buf_thresh = atoi(val);
    val = getenv( "MPI_MEM_THRESH" );
    if (val) mem_thresh = atoi(val);
    MPID_FlowSetup( buf_thresh, mem_thresh );
    }
#endif
    
    MPID_USOCK_DEBUG_PRINT_MSG("Leaving MPID_CH_USOCK_InitMsgPass");

    DSECTLEAVE
	return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_USOCK_Abort( comm_ptr, code, msg )
struct MPIR_COMMUNICATOR *comm_ptr;
int      code;
char     *msg;
{
    DSECTION("MPID_USOCK_Abort");
    
    DSECTENTRYPOINT;

    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_USOCK_Data_global.MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_USOCK_Data_global.MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );

    MPID_USOCK_SysError(msg,code);

    DSECTLEAVE
	return 0;
}

int MPID_USOCK_End( dev )
MPID_Device *dev;
{
    DSECTION("MPID_USOCK_End");
   
    DSECTENTRYPOINT;
    
    MPID_USOCK_DEBUG_PRINT_MSG("Entering MPID_USOCK_End\n");
    /* Finish off any pending transactions */
    /* MPID_CH_Complete_pending(); */
    /*
      MPID_FinishCancelPackets( dev );
    */
    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
#ifdef MPID_HAS_HETERO /* #HETERO_START# */
    MPID_CH_Hetero_free();
#endif                 /* #HETERO_END# */
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );

#ifdef MPID_FLOW_CONTROL
    MPID_FlowDelete();
#endif

    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    
    MPID_USOCK_Finalize();
    
    DSECTLEAVE
	return 0;
}

void MPID_CH_USOCK_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}

int MPID_CH_USOCK_long_len( int partner_devlrank )
{
    return MPID_CH_USOCK_long_len_value;
}

int MPID_CH_USOCK_vlong_len( int partner_devlrank )
{
    return MPID_CH_USOCK_vlong_len_value;
}

