/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include "mpid.h"
#include "mpiddev.h"
#include "../util/queue.h"

/***************************************************************************/
/* This is one of the main routines.  It checks for incoming messages and  */
/* dispatches them.  There is another such look in MPID_CH_blocking_recv   */
/* which is optimized for the important case of blocking receives for a    */
/* particular message.                                                     */
/*                                                                         */
/* This is a special version for shared memory.  It moves addresses of     */
/* packets, not packets, from one processor to another.                    */
/***************************************************************************/

/* Check for incoming messages.
    Input Parameter:
.   is_blocking - true if this routine should block until a message is
    available

    Returns -1 if nonblocking and no messages pending

    This routine makes use of a single dispatch routine to handle all
    incoming messages.  This makes the code a little lengthy, but each
    piece is relatively simple.

    This is the message-passing version.  The shared-memory version is
    in chchkshdev.c .
 */    
int MPID_Tunnel_Check_incoming( dev, is_blocking )
MPID_Device        *dev;
MPID_BLOCKING_TYPE is_blocking;
{
    /*    DEBUG_PRINT_MSG("Tunnel: Entering check_incoming (no functionality)");

    DEBUG_PRINT_MSG("Tunnel: Leaving check_incoming"); 

    return MPI_SUCCESS;*/

    /* returning -1 to keep the check_incoming polling loop alive 
       (see MPID_RecvComplete() and MPID_SHMEM_Check_incoming() */
    return -1;

}
