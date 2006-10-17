/* Definitions for the device only 
   This is an example that can can be used by channel codes
 */
#ifndef MPID_DEV_H
#define MPID_DEV_H

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#include "dev.h"
#include "metampi.h"
#include "packets.h"
#include "mpid_debug.h"

#include "metampi.h"
#include "../../src/routing/mpi_router.h"

/* Globals - For the device */
extern MPID_DevSet *MPID_devset;
extern MPIR_MetaConfig MPIR_meta_cfg;

extern MPID_Device **MPID_Tunnel_native_dev;

/* Function prototypes for routines known only to the device */

extern int MPID_Tunnel_Check_incoming ANSI_ARGS(( MPID_Device *, 
						  MPID_BLOCKING_TYPE));
  
#endif
