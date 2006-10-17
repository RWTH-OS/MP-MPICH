/*
    These are the definitions particular to the p4 implementation.
 */

#ifndef __commWSOCK
#define __commWSOCK

#include "wsockpriv.h"

#include "packets.h"

#define PI_NO_NSEND
#define PI_NO_NRECV

/*
#define MPID_USE_SEND_BLOCK
*/



/* Initialization routines */
/*#define PIiInit   MPID_WSOCK_Init*/
/*#define PIiFinish MPID_WSOCK_End*/
#define SYexitall(msg,code) wsock_syserror(msg,code)

#endif
