#ifndef CHCONFIG
#define CHCONFIG

/* Later we will make this work. */
/*#define MPID_HAS_HETERO*/

/* This makes chbrndv.c use memcpy for rendezvous messages to self */
#define MPID_RNDV_SELF

/* define this to make the device thread-safe
   NOTE: This isn't fully implemented yet.*/
/*#define THREAD_SAFE*/


#define MPID_END_NEEDS_BARRIER

/* Turn off flow control */
#define MPID_NO_FLOW_CONTROL 
#ifndef MPID_NO_FLOW_CONTROL
#define MPID_FLOW_CONTROL
#endif

/* Currently no nonblocking sends are available*/
/* #define MPID_USE_SEND_BLOCK */


/* Communicator initialization routines */
/* Comm_msgrep determines the common representation format for 
   members of the new communicator */
#define MPID_CommInit(oldcomm,newcomm) MPID_CH_Comm_msgrep( newcomm )
#define MPID_CommFree(comm)            MPI_SUCCESS

#ifdef THREAD_SAFE
#include <wtypes.h>
#include <winbase.h>
#endif

#endif
