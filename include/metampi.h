/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 *
 * metampi.h - contains the basic data structures for the meta-computing
 *             setup
 */

#ifndef __metampi
#define __metampi

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#include "mpid.h"
#include "dev.h"
#include "mpi.h"
#include "metaconfig.h"


/* special Meta-Barrier (see gatewaypriv.c) */
int MPID_Gateway_Barrier ANSI_ARGS((struct MPIR_COMMUNICATOR *));

/* from src/env/initutil.c */
/* XXX debug: -barrier option enables halts at barriers for
   being able to attach with the debugger */
extern int meta_barrier;
/* mapping for the devices */
extern MPID_Config primary_device, secondary_device, tn_device, gw_device;

extern MPIR_COLLOPS MPIR_meta_collops;



#endif
