/*
   This file contains routines that are private and unique to the ch_tunnel
   implementation
 */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mpid.h"
#include "mpiddev.h"
#include "sbcnst.h"

/* XXX for accessing MPID_myid and MPID_incoming
  only necessary for shared memory device
#include "shdef.h" */

#include "metampi.h"

/* XXX Get an integer from the environment; otherwise, return defval. 
int MPID_GetIntParameter ANSI_ARGS(( char *, int )); */


void MPID_Tunnel_finalize()
{
    fflush(stdout);
    fflush(stderr);

/* There is a potential race condition here if we want to catch
   exiting children.  We should probably have each child indicate a successful
   termination rather than this simple count.  To reduce this race condition,
   we'd like to perform an MPI barrier before clearing the signal handler.

   However, in the current code, MPID_xxx_End is called after most of the
   MPI system is deactivated.  Thus, we use a simple count-down barrier.
   Eventually, we the fast barrier routines.
 */
/* 
   This version assumes that the packets are dynamically allocated (not off of
   the stack).  This lets us use packets that live in shared memory.

   NOTE THE DIFFERENCES IN BINDINGS from the usual versions.
 */
}

