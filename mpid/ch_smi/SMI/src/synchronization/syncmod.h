/*--------------------------------------------------------------------------*/
/*                                                                          */
/* User-level Module: SYNCMOD                                               */
/*                                                                          */
/* (c) 1998-2001 Martin Schulz, LRR-TUM                                     */
/*                                                                          */
/* Contains the HAMSTER routines for synchronization                        */
/* Standalone SISCI Version                                                 */
/*                                                                          */
/* Headerfile                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#include "syncmod/allglob.h"

#ifndef _LOC_SIMPLESYNC_HEADER
#define _LOC_SIMPLESYNC_HEADER


/*--------------------------------------------------------------------------*/
/* Global type exported by this module */

typedef uint syncMod_atomic_t;
typedef uint countVal_t;


/*--------------------------------------------------------------------------*/
/* global types for the general HAMSTER timing */

typedef long long hamster_clock_t;


/*--------------------------------------------------------------------------*/
/* statistics data structures */

typedef struct syncMod_stat_d
{
  int             numLock,numUnlock,numBarrier;
  hamster_clock_t timeLock,timeBarrier, maxLock, maxBarrier;
  int             numBarFirst,numBarLast,numLockWait,numLockFail;
  int             numCountAlloc,numCountSet,numCountGet,numCountInc;
} syncMod_stat_t;


/*--------------------------------------------------------------------------*/
/* Declaration of exported routines */

#if defined(__cplusplus)
extern "C"
{
#endif

  /*............................................................*/
  /* Library Management routines */

  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* Initialization of the SyncMod, needs to be called first                */
  /* Parameters: nodeNum        = rank of the local node (0-n)              */
  /*             nodeCount      = total number of nodes/processes involved  */
  /*             nodeIds        = pointer to an array with SCI nodes        */
  /*                              size of array: nodeCount                  */
  /*             syncMemSegId   = Segment ID to be used for SISCI segments  */
  /*                              <nodeCount> segments used                 */
  /*             masterSegId    = Segment ID to be used for SISCI segment   */
  /*                              only one ID used, allocated on rank 0     */

  DLLEXPORT errCode_t DLLDECL syncMod_start(uint nodeNum, uint nodeCount, 
					    uint *nodeIds, uint syncMemSegId, uint masterSegId);


  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* stop SyncMod and clean up, should be called at the end */
  /* includes an implicit barrier */

  DLLEXPORT errCode_t DLLDECL syncMod_stop();


  /*............................................................*/
  /* routine to distribute an identifier */
  /* has to be called by all nodes/processes with equal parameters */
  /* includes implicit barriers                                    */
  /* as a result the value *atomic from node master to *atomic     */
  /* of all other nodes                                            */

  DLLEXPORT    errCode_t       DLLDECL syncMod_distribute(uint master, syncMod_atomic_t *atomic);


  /*............................................................*/
  /* routines for barriers */

  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* allocates a new barrier, to be called by one thread */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_allocBarrier(syncMod_atomic_t *atomic);


  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* executes a barrier across threadCount global activities            */
  /* Allows also several threads on one node participate                */
  /* The distribution of threads across nodes does not have to be known */
  /* WARNING: Once a barrier has been executed once on a set of threads */
  /*          or nodes, subsequent calls to the same barrier have to    */
  /*          include the same set of activities                        */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_fixedBarrier(syncMod_atomic_t atomic, int threadCount);


  /*............................................................*/
  /* routines for locks */
  
  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* allocate a new lock, to be called by one thread */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_allocLock(syncMod_atomic_t *atomic);

  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* acquire a lock and block if lock is taken           */
  /* Allows also several threads per node to participate */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_lock(syncMod_atomic_t atomic);


  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* Release a lock */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_unlock(syncMod_atomic_t atomic);


  /*............................................................*/
  /* routines to utilize atmoic counters */
  /* WARNING: directly mapped to SCI atomic transaction with all */
  /*          their problems */

  DLLEXPORT	errCode_t	DLLDECL	syncMod_allocCounter(syncMod_atomic_t *atomic);
  DLLEXPORT	errCode_t	DLLDECL	syncMod_allocCounterBlock(syncMod_atomic_t *atomic, uint count);
  DLLEXPORT	errCode_t	DLLDECL	syncMod_incCounter(syncMod_atomic_t atomic, countVal_t *val);
  DLLEXPORT	errCode_t	DLLDECL	syncMod_getCounter(syncMod_atomic_t atomic, countVal_t *val);
  DLLEXPORT	errCode_t	DLLDECL	syncMod_setCounter(syncMod_atomic_t atomic, countVal_t val);
  
  /*............................................................*/
  /* statistics */

  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* query the current statistics, return in parameter stat */

  DLLEXPORT  errCode_t  DLLDECL syncMod_getStatistics(syncMod_stat_t *stat);

  /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .*/
  /* query the current statistics and then reset the counters */

  DLLEXPORT  errCode_t  DLLDECL syncMod_resetStatistics(syncMod_stat_t *stat);


  /*............................................................*/

#if defined(__cplusplus)
}
#endif

/*--------------------------------------------------------------------------*/
/* The End. */

#endif
