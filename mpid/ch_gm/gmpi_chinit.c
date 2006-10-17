/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#include "mpid.h"
#include "mpiddev.h"
/* We put stdlib ahead of mpimem.h in case we are building the memory debugging
   version; since these includes may define malloc etc., we need to include 
   them before mpimem.h 
 */
#ifndef HAVE_STDLIB_H
extern char *getenv();
#else
#include <stdlib.h>
#endif
#include "gmpi.h"
#include "mpimem.h"
#include "flow.h"
#include "chpackflow.h"
#include "gmpi_packets.h"
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int MPID_CH_GM_long_len_value;
static int MPID_CH_GM_vlong_len_value;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_CH_GM_End ( MPID_Device * );
int MPID_CH_GM_Abort ( struct MPIR_COMMUNICATOR *, int, char * );
int MPID_CH_GM_long_len( int );
int MPID_CH_GM_vlong_len( int );
void MPID_CH_GM_Version_name ( char * );

/* 
   In addition, Chameleon processes many command-line arguments 

   This should return a structure that contains any relavent context
   (for use in the multiprotocol version)
   
   Returns a device.  
   This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *MPID_CH_GM_InitMsgPass( int *argc, char ***argv, int short_len, int long_len )
{
    MPID_Device *dev;
    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev) return 0;

  if (short_len < 0) short_len = gmpi.eager_size;
  if (long_len < 0) long_len = 0;

  MPID_CH_GM_long_len_value = short_len;
  dev->long_len = &MPID_CH_GM_long_len;
  MPID_CH_GM_vlong_len_value = short_len;
  dev->vlong_len = &MPID_CH_GM_vlong_len;

  dev->short_msg = MPID_CH_Short_setup();
  dev->long_msg = 0;
  dev->vlong_msg = MPID_CH_Rndvn_setup();
  dev->eager = 0;
  dev->rndv = dev->vlong_msg;
  dev->check_device = MPID_CH_Check_incoming;
  dev->terminate = MPID_CH_GM_End;
  dev->abort = MPID_CH_GM_Abort;
  dev->next = 0;

  /* Set the file for Debugging output.  The actual output is controlled
     by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif
    
  /* If requested, setup a separate process group before creating the
     other MPI processes */
  (void) MPID_Process_group_init();

  gmpi_init(argc, argv);
  DEBUG_PRINT_MSG("Finished init");
  
  MPID_DO_HETERO(MPID_CH_Init_hetero( argc, argv ));
  
  DEBUG_PRINT_MSG("Leaving MPID_CH_GM_InitMsgPass");
  
  return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_CH_GM_Abort( struct MPIR_COMMUNICATOR *comm_ptr, int code, char *msg )
{
    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_GM_rank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_GM_rank );
    }
    fflush( stderr );
    fflush( stdout );
  
#ifdef USE_PRINT_LAST_ON_ERROR
  MPID_Ch_dprint_last();
#endif
  
  /* Some systems (e.g., p4) can't accept a (char *)0 message argument. */
    SYexitall( "", code );
  return 0;
}

int MPID_CH_GM_End( 
	MPID_Device *dev )
{
  DEBUG_PRINT_MSG("Entering MPID_CH_GM_End\n");
  /* Finish off any pending transactions */
  
  GMPI_PROGRESSION_LOCK();
  MPID_FinishCancelPackets(dev);   
    if (MPID_GetMsgDebugFlag()) {
      MPID_PrintMsgDebug();
    }
#ifdef MPID_HAS_HETERO /* #HETERO_START# */
  MPID_CH_Hetero_free();
#endif                 /* #HETERO_END# */
    (dev->short_msg->delete)( dev->short_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    GMPI_PROGRESSION_UNLOCK();
    gmpi_finish(dev);
    FREE( dev );

  /* We should really generate an error or warning message if there 
     are uncompleted operations... */
  
  DEBUG_PRINT_MSG("Leaving MPID_CH_GM_End");      
  return 0;
}

void MPID_CH_GM_Version_name( char *name )
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}


int MPID_CH_GM_long_len( int partner_devlrank )
{
    return MPID_CH_GM_long_len_value;
}

int MPID_CH_GM_vlong_len( int partner_devlrank )
{
    return MPID_CH_GM_vlong_len_value;
}




