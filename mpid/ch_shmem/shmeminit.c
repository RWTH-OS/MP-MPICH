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

#include <stdio.h>

#include "mpid.h"
#include "shmemdev.h"
#include "mpimem.h"
#include "flow.h"
#include "chpackflow.h"
#include "shmemdebug.h"
#include "shmemcommon.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int MPID_SHMEM_long_len_value;
static int MPID_SHMEM_vlong_len_value;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_SHMEM_End ( MPID_Device * );
int MPID_SHMEM_Abort ( struct MPIR_COMMUNICATOR *, int, char * );
void MPID_SHMEM_Version_name(char * name );

int MPID_SHMEM_long_len( int );
int MPID_SHMEM_vlong_len( int );
/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */

MPID_Device *MPID_CH_SHMEM_InitMsgPass( argc, argv, short_len, long_len )
int  *argc;
char ***argv;
int  short_len, long_len;
{
    MPID_Device *dev;

    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev) return 0;
    /* The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet */
    if (short_len < 0) short_len = MPID_PKT_MAX_DATA_SIZE;
    if (long_len < 0)  long_len  = 128000;
    MPID_SHMEM_long_len_value = short_len;
    dev->long_len     = &MPID_SHMEM_long_len;
    MPID_SHMEM_vlong_len_value = long_len;
    dev->vlong_len    = &MPID_SHMEM_vlong_len;

    dev->short_msg    = MPID_SHMEM_Short_setup();
    dev->long_msg     = MPID_SHMEM_Eagern_setup();
    dev->vlong_msg    = MPID_SHMEM_Rndvn_setup();
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->ready        = NULL;
    dev->check_device = MPID_SHMEM_Check_incoming;
    dev->terminate    = MPID_SHMEM_End;
    dev->cancel       = MPID_SHMEM_SendCancelPacket;
    dev->abort	      = MPID_SHMEM_Abort;
    dev->wtime        = 0;
    dev->collops_init = 0;
    dev->comm_init    = 0;
    dev->comm_free    = 0;
    dev->get_version  = MPID_SHMEM_Version_name;
    dev->persistent_init = 0;
    dev->persistent_free = 0;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif

    MPID_SHMEM_init( argc, *argv );
    MPID_SHMEM_DEBUG_PRINT_MSG("Finished init");

    dev->lrank = MPID_SHMEM_rank;
    dev->lsize = MPID_SHMEM_size;
    
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
#ifdef MPID_PACK_CONTROL
    MPID_PacketFlowSetup( );
#endif
    MPID_SHMEM_DEBUG_PRINT_MSG("Leaving MPID_SHMEM_InitMsgPass");

    return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_SHMEM_Abort( comm_ptr, code, msg )
struct MPIR_COMMUNICATOR *comm_ptr;
int      code;
char     *msg;
{
    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_SHMEM_rank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_SHMEM_rank );
    }

    /* This needs to try and kill any generated processes */
    p2p_kill_procs();
    /* Cleanup any SYSV IPC facilities (message queues, semaphore arrays, shared memory segments) */
    MPID_SHMEM_sysv_shmem_cleanup();

    exit(code);
    return 0;
}

int MPID_SHMEM_End( dev )
MPID_Device *dev;
{
    MPID_SHMEM_DEBUG_PRINT_MSG("Entering MPID_SHMEM_End\n");
    /* Finish off any pending transactions */
    /* MPID_SHMEM_Complete_pending(); */
    MPID_SHMEM_FlushPkts();

#ifdef MPID_PACK_CONTROL
    MPID_PackDelete();
#endif

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );

#ifdef MPID_FLOW_CONTROL
    MPID_FlowDelete();
#endif

    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    MPID_SHMEM_finalize();
    return 0;
}

/*
 * Currently, this is inactive because adi2init contains MPID_Version_name .
 */
void MPID_SHMEM_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s, locks %s", 
	     MPIDPATCHLEVEL, MPIDTRANSPORT, p2p_lock_name );
}

int MPID_SHMEM_long_len( int partner_devlrank )
{
    return MPID_SHMEM_long_len_value;
}

int MPID_SHMEM_vlong_len( int partner_devlrank )
{
    return MPID_SHMEM_vlong_len_value;
}

