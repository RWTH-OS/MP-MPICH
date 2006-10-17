/*--------------------------------------------------------------------------*/
/* Public routines for syncMod Barrier */

/*.......................................................................*/
/* Allocate a barrier */

DLLEXPORT errCode_t DLLDECL syncMod_allocBarrier(atomic_t *atomic)
{
    SIMPLESYNC_ASSERT(1);

#ifdef SIMPLESYNC_NEWBARRIER
    if (locNodeCount<2)
	return sync_allocAtomic(2,atomic);
    else
	return sync_allocAtomic(locNodeCount,atomic);
#else
	return sync_allocAtomic(1,atomic);
#endif
}


/*.......................................................................*/
/* Execute a barrier over all registered activities */

DLLEXPORT errCode_t DLLDECL syncMod_fixedBarrier(atomic_t atomic, int threadCount)
{
	errCode_t	err;
    
	SIMPLESYNC_ASSERT(11);

    /* barrier over a fixed number of activities */
    /* used to synchronize user threads */

	return sync_barrier(atomic,threadCount,NULL);
}



/*--------------------------------------------------------------------------*/
/* Public routines for locks */

/*.......................................................................*/
/* Allocate a lock */

DLLEXPORT errCode_t DLLDECL syncMod_allocLock(atomic_t *atomic)
{
        SIMPLESYNC_ASSERT(3);

#ifdef SIMLPESYNC_NEWLOCK
	return sync_allocAtomic(2+MAX_CLUSTER_NODES,atomic);
#else
	return sync_allocAtomic(2,atomic);
#endif
}


/*.......................................................................*/
/* aquire a lock */

DLLEXPORT errCode_t DLLDECL syncMod_lock(atomic_t atomic)
{
    SIMPLESYNC_ASSERT(4);

	return sync_lock(atomic);
}


/*.......................................................................*/
/* release a lock */

DLLEXPORT errCode_t DLLDECL syncMod_unlock(atomic_t atomic)
{
        SIMPLESYNC_ASSERT(4);

	return sync_unlock(atomic);
}


/*--------------------------------------------------------------------------*/
/* Public routines for atomic counters */

/*.......................................................................*/
/* Allocate a counter */

DLLEXPORT errCode_t DLLDECL syncMod_allocCounter(atomic_t *atomic)
{
        SIMPLESYNC_ASSERT(5);

#ifdef COLLECT_STATS
      syncMod_globalStat.numCountAlloc++;
#endif	

	return sync_allocAtomic(1,atomic);
}


/*.......................................................................*/
/* Allocate a counter block */

DLLEXPORT errCode_t DLLDECL syncMod_allocCounterBlock(atomic_t *atomic, uint count)
{
        SIMPLESYNC_ASSERT(6);

#ifdef COLLECT_STATS
      syncMod_globalStat.numCountAlloc += count;
#endif	

	return sync_allocAtomic(count,atomic);
}


/*.......................................................................*/
/* increment a counter */

DLLEXPORT errCode_t DLLDECL syncMod_incCounter(atomic_t atomic, countVal_t *val)
{
        SIMPLESYNC_ASSERT(7);

#ifdef COLLECT_STATS
      syncMod_globalStat.numCountInc++;
#endif	

	return atomic_getAtomic(atomic,val);
}


/*.......................................................................*/
/* decrement a counter */

DLLEXPORT errCode_t DLLDECL syncMod_getCounter(atomic_t atomic, countVal_t *val)
{
        SIMPLESYNC_ASSERT(8);

#ifdef COLLECT_STATS
      syncMod_globalStat.numCountGet++;
#endif	

	return atomic_getAtomicDirect(atomic,val);
}


/*.......................................................................*/
/* decrement a counter */
/* CAREFUL: CAN CAUSE RACES !!!! */

DLLEXPORT errCode_t DLLDECL syncMod_setCounter(atomic_t atomic, countVal_t val)
{
        SIMPLESYNC_ASSERT(9);

#ifdef COLLECT_STATS
      syncMod_globalStat.numCountSet++;
#endif	

	return atomic_setAtomicDirect(atomic,val);
}


/*--------------------------------------------------------------------------*/
/* Statistics syncMod */

/*.......................................................................*/
/* Synchronization Module - get data */

DLLEXPORT errCode_t DLLDECL syncMod_getStatistics(syncMod_stat_t *stat)
{
  *stat = syncMod_globalStat;

  #ifdef COLLECT_STATS
  return OK_SIMPLESYNC;
  #else
  return ERR_SIMPLESYNC_NOTSUP;
  #endif
}


/*.......................................................................*/
/* Synchronization Module - get and reset data */
DLLEXPORT errCode_t DLLDECL syncMod_resetStatistics(syncMod_stat_t *stat)
{
  errCode_t err;

  /* get the old data */

  err=syncMod_getStatistics(stat);

  /* reset the data */

  syncMod_globalStat.numLock     = 0;
  syncMod_globalStat.numUnlock   = 0;
  syncMod_globalStat.numBarrier  = 0;
  
  syncMod_globalStat.numBarFirst = 0;
  syncMod_globalStat.numBarLast  = 0;
  syncMod_globalStat.numLockWait = 0;
  syncMod_globalStat.numLockFail = 0;

  syncMod_globalStat.timeLock    = 0;
  syncMod_globalStat.timeBarrier = 0;
  syncMod_globalStat.maxLock     = 0;
  syncMod_globalStat.maxBarrier  = 0;

  syncMod_globalStat.numCountAlloc = 0;
  syncMod_globalStat.numCountSet   = 0;
  syncMod_globalStat.numCountGet   = 0;
  syncMod_globalStat.numCountInc   = 0;

  /* return */

  return err;
}


