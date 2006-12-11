/* $Id$ */

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

#define MPID_HAS_PROC_INFO
/* This is needed if you want TotalView to acquire all of the processes
 * automagically.
 * If it is defined you must also define 
 *      int MPID_getpid(int index, char **hostname, char **imagename);
 * which takes an index in COMM_WORLD and returns the pid of that process as a result,
 * and also fills in the pointers to the two strings hostname (something which we can
 * pass to inet_addr, and image_name which is the name of the executable running
 * that this process is running.
 * You can fill in either (or both) pointers as (char *)0 which means
 * "the same as the master process".
 * 
 * this function lies in src/env/initutil.c */
extern int MPID_getpid(int, char**, char**);

#ifdef MPID_HAS_HETERO
#define MPID_DO_HETERO(a) a
/* Communicator initialization routines */
/* Comm_msgrep determines the common representation format for 
   members of the new communicator */
#else
#define MPID_DO_HETERO(a) 
#endif

#endif
