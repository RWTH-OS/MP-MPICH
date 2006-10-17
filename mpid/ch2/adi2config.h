/* $Id: adi2config.h,v 1.4.14.1 2004/12/16 12:59:17 boris Exp $ */

#ifndef _ADI2CONFIG_INC
#define _ADI2CONFIG_INC

/* maximum length of ADI2 version string */
#define MPID_MAX_VERSION_NAME 256

/* fro now, this contains the default configuration switches for *all* devices -
   we need to let the devices decide if they need flow control/hetero/etc. */

#define MPID_NO_FLOW_CONTROL
#ifndef MPID_NO_FLOW_CONTROL
#define MPID_FLOW_CONTROL
#endif

#ifdef MPID_HAS_HETERO
#define MPID_DO_HETERO(a) a
/* Communicator initialization routines */
/* Comm_msgrep determines the common representation format for 
   members of the new communicator */
#else
#define MPID_DO_HETERO(a) 
#endif

#endif
