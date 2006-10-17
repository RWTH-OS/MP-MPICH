/*--------------------------------------------------------------------------*/
/*                                                                          */
/* User-level Module: SYNCMOD                                               */
/*                                                                          */
/* (c) 1998-2001 Martin Schulz, LRR-TUM                                     */
/*                                                                          */
/* Contains the HAMSTER routines for synchronization                        */
/* Standalone SISCI Version                                                 */
/*                                                                          */
/* Headerfile with definitions to emulate HAMSTER environment               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*-----------------------------------------------------------*/
/* a few HAMSTER constants */

#define MAX_CLUSTER_NODES 64


/*-----------------------------------------------------------*/
/* a few HAMSTER variables */

typedef uint nodeNum_t;


/*-----------------------------------------------------------*/
/* replace simpleSync routines by syncMod routines */

#define simpleSync_barrier syncMod_barrier
#define simpleSync_lock    syncMod_lock
#define simpleSync_unlock  syncMod_unlock


/*-----------------------------------------------------------*/
/* resolve unfortunate type conflict with Linux kernel */

typedef syncMod_atomic_t atomic_t;


/*-----------------------------------------------------------*/
/* only allow new types of locks and barriers */

#ifndef SIMPLESYNC_NEWBARRIER
#error Only new type of barrier supported by syncMod
#endif

#ifndef SIMPLESYNC_NEWLOCK
#error Only new type of lock supported by syncMod
#endif


/*-----------------------------------------------------------*/
/* up to now only for Linux */

#ifndef LINUX
#error Only LINUX supported
#endif


/*-----------------------------------------------------------*/
/* distinguish between the syncMod and the HAMSTER version */

#define SYNCMOD_STANDALONE_LIBRARY


/*-----------------------------------------------------------*/
/* HAMSTER timing support */

/* right now not supported */

#define hamster_time() (0)

/*-----------------------------------------------------------*/
/* a few more definitions (that don't make any sense without */
/* knowing internal details of HAMSTER) */

#define OLD_SCIAL


/*-----------------------------------------------------------*/
/* The End. */
