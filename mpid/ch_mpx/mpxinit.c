/*
 *  $Id: mpxinit.c 4397 2006-01-30 10:41:47Z carsten $
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

#ifndef HAVE_STDLIB_H
extern char *getenv();
#else
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mpid.h"
#include "mpxdev.h"
#include "mpimem.h"
#include "flow.h"
#include <stdio.h>

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int MPID_CH_MPX_long_len_value;
static int MPID_CH_MPX_vlong_len_value;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_MPX_End ANSI_ARGS(( MPID_Device * ));
int MPID_MPX_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));
void MPID_CH_MPX_Version_name ANSI_ARGS(( char * ));

int MPID_CH_MPX_long_len( int );
int MPID_CH_MPX_vlong_len( int );

MPID_Device *MPID_CH_MPX_InitMsgPass( argc, argv, short_len, long_len )
int  *argc;
char ***argv;
int  short_len, long_len;
{
    MPID_MPX_Data_global_type *global_data;
  
    DSECTION("MPID_CH_MPX_InitMsgPass");

    MPID_Device *dev;
   
    DSECTENTRYPOINT;

    /* allocate memory for the global data of this device: */
    global_data = (MPID_MPX_Data_global_type*)MALLOC( sizeof(MPID_MPX_Data_global_type) );

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

    MPID_MPX_Test_device(MPID_devset->active_dev, "InitMsgPass");

    DNOTICE("device tested");

    /* The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet */
    if (short_len < 0) short_len = MPID_PKT_MAX_DATA_SIZE; /* <- defined in mpxpackets.h */
    short_len=1;
    if (long_len < 0)  long_len  = 128000;
    global_data->long_len_value = short_len;
    dev->long_len     = &MPID_CH_MPX_long_len;
    global_data->vlong_len_value = long_len;
    dev->vlong_len    = &MPID_CH_MPX_long_len;

    dev->short_msg    = MPID_MPX_Short_setup();

    dev->long_msg     = MPID_MPX_Eagern_setup();   
    
 /* dev->vlong_msg    = MPID_MPX_Rndvb_setup();  */
    dev->vlong_msg    = MPID_MPX_Eagern_setup(); /* <- Since the Rendezvous-Protocol leads no good performance and
						    |    shows a trend to deadlocks, it is temporarily disabled.
						    */
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->check_device = MPID_MPX_Check_incoming;
    dev->terminate    = MPID_MPX_End;
    dev->abort	      = MPID_MPX_Abort;
    dev->cancel	      = MPID_MPX_SendCancelPacket;
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
  
#if 0
/* -TRUNK:-------- */
    MPID_MPX_Init( argc, argv );

    dev->lrank = MPID_MPX_Data_global.MyWorldRank;
    dev->lsize = MPID_MPX_Data_global.MyWorldSize;
/* --------------- */

/* -MULTIDEVICE:-- */
    MPID_MPX_Init( argc, argv, &lrank, &size );
    dev->lrank = lrank;
    dev->size  = size;
    dev->next  = 0;
/* --------------- */
#else

    MPID_MPX_Init( argc, argv );

    dev->lrank = global_data->MyWorldRank;
    dev->lsize = global_data->MyWorldSize;

#endif

    MPID_MPX_DEBUG_PRINT_MSG("Finished init");
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
    
    MPID_MPX_DEBUG_PRINT_MSG("Leaving MPID_CH_MPX_InitMsgPass");

    DSECTLEAVE
	return dev;
}


/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_MPX_Abort( comm_ptr, code, msg )
struct MPIR_COMMUNICATOR *comm_ptr;
int      code;
char     *msg;
{
    DSECTION("MPID_MPX_Abort");
    
    DSECTENTRYPOINT;

    MPID_MPX_Test_device(MPID_devset->active_dev, "Abort");

    if (msg) {
      fprintf( stderr, "[%d] %s\n", ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg );
    }
    else {
      fprintf( stderr, "[%d] Aborting program!\n", ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );

    MPID_MPX_SysError(msg,code);

    DSECTLEAVE
	return 0;
}


int MPID_MPX_End( dev )
MPID_Device *dev;
{
    DSECTION("MPID_MPX_End");
   
    DSECTENTRYPOINT;

    MPID_devset->active_dev = dev;
    MPID_MPX_Test_device(MPID_devset->active_dev, "End");
    
    MPID_MPX_DEBUG_PRINT_MSG("Entering MPID_MPX_End\n");
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
    FREE( dev );

#ifdef MPID_FLOW_CONTROL
    MPID_FlowDelete();
#endif

    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    
    MPID_MPX_Finalize();
    
    DSECTLEAVE
	return 0;
}


void MPID_CH_MPX_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}


int MPID_CH_MPX_long_len( int partner_devlrank )
{

  MPID_MPX_Test_device(MPID_devset->active_dev, "long_len");

  return ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->long_len_value;
}


int MPID_CH_MPX_vlong_len( int partner_devlrank )
{
  MPID_MPX_Test_device(MPID_devset->active_dev, "vlong_len");

  return ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->vlong_len_value;
}
