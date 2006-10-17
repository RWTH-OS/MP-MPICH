/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/


/* We put these include FIRST in case we are building the memory debugging
   version; since these includes may define malloc etc., we need to include 
   them before mpid.h 
 */
#ifndef HAVE_STDLIB_H
extern char *getenv();
#else
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "flow.h"
#include "gmpi.h"
#include "gmpi_smpi.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int MPID_SMP_long_len_value;
static int MPID_SMP_vlong_len_value;

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_SMP_End(MPID_Device *);
int MPID_SMP_Abort(struct MPIR_COMMUNICATOR *, int, char *);
void MPID_SMP_Version_name(char *);

int MPID_SMP_long_len( int );
int MPID_SMP_vlong_len( int );
/* 
   In addition, Chameleon processes many command-line arguments 
   
   This should return a structure that contains any relavent context
   (for use in the multiprotocol version)
   
   Returns a device.  
   This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *
MPID_SMP_InitMsgPass(int * argc, char *** argv, int short_len, int long_len)
{
  MPID_Device *dev;

  dev = (MPID_Device *)malloc(sizeof(MPID_Device));
  smpi_malloc_assert(dev, "MPID_SMP_InitMsgPass",
		     "malloc: SMP MPID device");
  if (!dev) 
    {
      return 0;
    }
  
  if (short_len < 0)
    {
      short_len = gmpi.eager_size;
    }
  
  MPID_SMP_long_len_value = short_len;
  dev->long_len = &MPID_SMP_long_len;
  MPID_SMP_vlong_len_value = short_len;
  dev->vlong_len = &MPID_SMP_vlong_len;

  dev->short_msg = MPID_SMP_Short_setup();
  dev->long_msg = MPID_SMP_Rndv_setup();
  dev->vlong_msg = dev->long_msg;
  dev->eager = dev->long_msg;
  dev->rndv = MPID_SMP_Rndv_setup();
  dev->check_device = MPID_SMP_Check_incoming;
  dev->terminate = MPID_SMP_End;
  dev->abort = MPID_SMP_Abort;
  dev->next = 0;
  
  /* Set the file for Debugging output.  The actual output is controlled
     by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
  if (MPID_DEBUG_FILE == 0) 
    {
      MPID_DEBUG_FILE = stdout;
    }
#endif
  
  smpi_init();
  DEBUG_PRINT_MSG("Finished init");
  DEBUG_PRINT_MSG("Leaving MPID_SMP_InitMsgPass");
  
  return dev;
}


/* 
   Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
*/
int 
MPID_SMP_Abort(struct MPIR_COMMUNICATOR * comm_ptr, int code, char * msg)
{
  if (msg)
    {
      fprintf(stderr, "[%d] %s\n", MPID_GM_rank, msg);
    }
  else
    {
      fprintf(stderr, "[%d] Aborting program!\n", MPID_GM_rank);
    }
  
  /* Some systems (e.g., p4) can't accept a (char *)0 message argument. */
  SYexitall("", code);
  return 0;
}

int 
MPID_SMP_End(MPID_Device * dev)
{
  GMPI_PROGRESSION_LOCK();
  DEBUG_PRINT_MSG("Entering MPID_CH_End\n");
  
  /* Finish off any pending transactions */
  /* MPID_CH_Complete_pending(); */
  MPID_FinishCancelPackets(dev);
  
  if (MPID_GetMsgDebugFlag()) 
    {
      MPID_PrintMsgDebug();
    }
  
  (dev->short_msg->delete)(dev->short_msg);
  FREE(dev);
  
  /* We should really generate an error or warning message if there 
     are uncompleted operations... */
  
  smpi_finish();
  /* We should really generate an error or warning message if there 
     are uncompleted operations... */
  DEBUG_PRINT_MSG("Leaving MPID_SMP_End");

  GMPI_PROGRESSION_UNLOCK();
  return 0;
}

void 
MPID_SMP_Version_name(char * name)
{
  sprintf(name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	  MPIDTRANSPORT);
}

int MPID_SMP_long_len( int partner_devlrank )
{
    return MPID_SMP_long_len_value;
}

int MPID_SMP_vlong_len( int partner_devlrank )
{
    return MPID_SMP_vlong_len_value;
}

