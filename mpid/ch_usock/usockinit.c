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
#include "usockpriv.h"
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
    MPID_USOCK_Data_global_type *global_data;
  
    DSECTION("MPID_CH_USOCK_InitMsgPass");

    MPID_Device *dev;
   
    DSECTENTRYPOINT;

    /* allocate memory for the global data of this device: */
    global_data = (MPID_USOCK_Data_global_type*)MALLOC( sizeof(MPID_USOCK_Data_global_type) );

    DNOTICE("data allocated");
    
    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    DNOTICE("device allocated");
    if (!dev)  {
	DSECTLEAVE
	    return 0;
    }

    DNOTICE("set the active device");

    /* now, this is the currently active device: */
    MPID_devset->active_dev = dev;

    DNOTICE("testing device");

    MPID_USOCK_Test_device(MPID_devset->active_dev, "InitMsgPass");

    DNOTICE("device tested");

    if (short_len < 0) {
      short_len = MPID_PKT_MAX_DATA_SIZE; /* <- defined in usockpackets.h */
      if(getenv("USOCK_SHORT_SIZE")!=NULL) {
      	short_len = atoi(getenv("USOCK_SHORT_SIZE"));
      }
    }
    
    if (long_len < 0) {
      long_len = short_len;
      if(getenv("USOCK_LONG_SIZE")!=NULL) {
      	long_len = atoi(getenv("USOCK_LONG_SIZE"));
      }
    } 

    global_data->long_len_value = short_len;
    dev->long_len     = &MPID_CH_USOCK_long_len;
    global_data->vlong_len_value = long_len;
    dev->vlong_len    = &MPID_CH_USOCK_long_len;

 /* dev->short_msg    = MPID_USOCK_Short_setup();  */
    dev->short_msg    = MPID_USOCK_Eagern_setup(); /* <- We do not realy need to differ between protocols at all when
						    |    just using socket communication --> always Eager mode!
						    |    (See also MPID_USOCK_Eagern_isend() in usockneager.c)
						    */

    dev->long_msg     = MPID_USOCK_Eagern_setup();
    
 /* dev->vlong_msg    = MPID_USOCK_Rndvb_setup();  */
    dev->vlong_msg    = MPID_USOCK_Eagern_setup(); /* <- Since the Rendezvous-Protocol leads no good performance and
						    |    shows a trend to deadlocks, it is disabled...
						    |    Thus, USOCK always useses Eager mode! (s.a.)
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

    /* store the pointer to the global data in the device struct: */
    dev->global_data = (void*)global_data;
    
    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
    /* mixed = 0; */
#endif
  
    MPID_USOCK_Init( argc, argv );

    dev->lrank = global_data->MyWorldRank;
    dev->lsize = global_data->MyWorldSize;
    dev->next  = 0;

    if( (getenv("USOCK_VERBOSE")!=NULL) )
    {
      if(atoi(getenv("USOCK_VERBOSE"))>1)
      {
	 printf("USOCK SHORT SIZE: %d\n", short_len);
	 printf("USOCK LONG SIZE: %d\n", long_len);
      }
      printf("USOCK device initialized.\n");
      fflush(stdout);
    }

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

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Abort");

    if (msg) {
      fprintf( stderr, "[%d] %s\n", ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg );
    }
    else {
      fprintf( stderr, "[%d] Aborting program!\n", ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank );
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

    MPID_devset->active_dev = dev;
    MPID_USOCK_Test_device(MPID_devset->active_dev, "End");
    
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

    MPID_Protocol_call_delete( dev->short_msg->delete, dev->short_msg, dev );
    MPID_Protocol_call_delete( dev->long_msg->delete, dev->long_msg, dev );
    MPID_Protocol_call_delete( dev->vlong_msg->delete, dev->vlong_msg, dev );
    
/*
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
*/
    

#ifdef MPID_FLOW_CONTROL
    MPID_FlowDelete();
#endif

    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    
    MPID_USOCK_Finalize();

    FREE( dev->global_data );
  
    FREE( dev );

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

  MPID_USOCK_Test_device(MPID_devset->active_dev, "long_len");

  return ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->long_len_value;
}


int MPID_CH_USOCK_vlong_len( int partner_devlrank )
{
  MPID_USOCK_Test_device(MPID_devset->active_dev, "vlong_len");

  return ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->vlong_len_value;
}
