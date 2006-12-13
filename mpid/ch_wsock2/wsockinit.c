/*
 *  $Id$
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

#ifdef _WIN32
#define _WIN32_WINNT 0x0400

#if (_MSC_VER < 1100)
#define NOMINMAX 
#include <winsock2.h>
#endif

#include <wtypes.h>
#include <winbase.h>
#include <mpidefs.h>
/*#define HAVE_STDLIB_H*/
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
#include "mpiddev.h"
#include "mpimem.h"
#include "flow.h"
#include "wsock2debug.h"
#include <stdio.h>


/*#define DEBUG(a) {a}*/

#define DEBUG(a)


static int MPID_CH_long_len_value;
static int MPID_CH_vlong_len_value;


/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_CH_End ANSI_ARGS(( MPID_Device * ));
int MPID_CH_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));
int MPID_CH_long_len( int );
int MPID_CH_vlong_len( int );
void MPID_CH_Version_name ANSI_ARGS(( char * ));
MPID_Device *MPID_SHMEM_InitMsgPass ANSI_ARGS(( int *, char ***, int, int ));
int MPID_WSOCK_Collops_init(struct MPIR_COMMUNICATOR *, MPIR_COMM_TYPE);
extern int MPID_numids;
extern int mixed;

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */

/*
WINBASEAPI
DWORD
WINAPI
SetThreadIdealProcessor(
    HANDLE hThread,
    DWORD dwIdealProcessor
);
*/
/*
short_len: threshold value between short and eager protocols 
		default + max. MPID_PKT_MAX_DATA_SIZE
long_len: threshold value between eager and rndv protocols 
		default 128000
*/

MPID_Device *MPID_CH_WSOCK_InitMsgPass( argc, argv, short_len, long_len )
/*MPID_Device *MPID_CH_InitMsgPass( argc, argv, short_len, long_len )*/

int  *argc;
char ***argv;
int  short_len, long_len;
{
    MPID_Device *dev,*help;
	SYSTEM_INFO SysInfo;


    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev) return 0;
    /* The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet */
    if (short_len < 0) short_len = MPID_PKT_MAX_DATA_SIZE;
    if (long_len < 0)  long_len  = 128000;
    MPID_CH_long_len_value = short_len;
    dev->long_len     = &MPID_CH_long_len;
    MPID_CH_vlong_len_value = long_len;
    dev->vlong_len    = &MPID_CH_vlong_len;

    dev->short_msg    = MPID_CH_Short_setup();
    dev->long_msg     = MPID_WSOCK_Eagern_setup();
	dev->vlong_msg    = MPID_WSOCK_Eagern_setup();
	/*  SI 13.12.2005: deadlock at rndv occures, switch to eager! 
	dev->vlong_msg    = MPID_WSOCK_Rndvb_setup();*/    
    dev->eager        = dev->long_msg;
	dev->rndv         = dev->long_msg;
/*  SI 13.12.2005: deadlock at rndv occures, switch to eager! 
	dev->rndv         = dev->vlong_msg;*/
    dev->ready        = NULL;
    dev->check_device = MPID_CH_Check_incoming;
    dev->terminate    = MPID_CH_End;
    dev->abort	      = MPID_CH_Abort;
    dev->cancel	      = MPID_WSOCK_SendCancelPacket;
    dev->wtime        = 0;
    dev->collops_init = MPID_WSOCK_Collops_init;
    dev->comm_init    = 0;
    dev->comm_free    = 0;
    dev->persistent_init = 0;
    dev->persistent_free = 0;
    dev->nc_enable = 0;
	dev->alloc_mem = 0;
	dev->free_mem = 0;
	dev->get_version  = MPID_CH_Version_name;



    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif

	mixed = 0;
    MPID_WSOCK_Init( argc, argv );
	if(MPID_numids) {
		dev->next = MPID_SHMEM_InitMsgPass(argc,argv,-1,-1);
		GetSystemInfo(&SysInfo);
		mixed = (SysInfo.dwNumberOfProcessors < MPID_numids);
	} else {
		dev->next	      = 0;
		SetThreadAffinityMask(GetCurrentThread(),1);
	}
	
	if(MPID_numids == MPID_MyWorldSize) {
		help = dev;
		dev=dev->next;
		dev->next = 0;
		free(help);
	} 
	

    DEBUG_PRINT_MSG("Finished init");
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

    DEBUG_PRINT_MSG("Leaving MPID_CH_InitMsgPass");

    return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_CH_Abort( comm_ptr, code, msg )
struct MPIR_COMMUNICATOR *comm_ptr;
int      code;
char     *msg;
{
    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );
    /* Some systems (e.g., p4) can't accept a (char *)0 message argument. */
    SYexitall( "", code );
    return 0;
}

int MPID_CH_End( dev )
MPID_Device *dev;
{
    DEBUG_PRINT_MSG("Entering MPID_CH_End\n");
    /* Finish off any pending transactions */
    /* MPID_CH_Complete_pending(); */

    MPID_FinishCancelPackets( dev );

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
    
    MPID_WSOCK_End();
    return 0;
}

void MPID_CH_Version_name( name )
char *name;
{
    sprintf( name, "%s v %4.2f\n", MPIDTRANSPORT,MPIDPATCHLEVEL);
}

int MPID_CH_long_len( int partner_devlrank )
{
    return MPID_CH_long_len_value;
}

int MPID_CH_vlong_len( int partner_devlrank )
{
    return MPID_CH_vlong_len_value;
}
