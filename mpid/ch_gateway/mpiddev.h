/* $Id$ */
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

extern MPID_DevSet *MPID_devset;
extern MPIR_MetaConfig MPIR_meta_cfg;

/*
 * Function prototypes for routines known only to the device 
 */

Meta_Header *MPID_Gateway_Wrap_Msg ANSI_ARGS((void *, int, int, int, 
						    int, int, MPI_Sendmode, int, unsigned int));

extern int MPID_Gateway_Check_incoming ANSI_ARGS(( MPID_Device *, 
						 MPID_BLOCKING_TYPE));
int MPID_Gateway_Unified_send( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * );

int MPID_Gateway_Unified_isend( void *, int, int, int, int, int, MPID_Msgrep_t,
				       MPIR_SHANDLE *, struct MPIR_DATATYPE * );

unsigned int MPID_Gateway_get_global_msgid( void );

#endif
