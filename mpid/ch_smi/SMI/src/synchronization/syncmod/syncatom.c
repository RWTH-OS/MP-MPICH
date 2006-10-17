/*--------------------------------------------------------------------------*/
/* Basic functionality to manage the atomic segment */

/*.......................................................................*/
/* read out an atomic counter (change byte order) */

errCode_t atomic_getAtomic(atomic_t atomic, volatile uint *val)
{
	uint between;

	between = (volatile uint) (atomStart[atomic]);
	
	*val = ((between >> 24) & 0x000000FF) | ((between >>  8) & 0x0000FF00) |
		   ((between <<  8) & 0x00FF0000) | ((between << 24) & 0xFF000000);

	return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* read out an atomic counter directly (change byte order) */

errCode_t atomic_getAtomicDirect(atomic_t atomic, volatile uint *val)
{
	uint between;

	between = (volatile uint) (atomStartDirect[atomic]);
	*val = ((between >> 24) & 0x000000FF) | ((between >>  8) & 0x0000FF00) |
		   ((between <<  8) & 0x00FF0000) | ((between << 24) & 0xFF000000);
	
	return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* set an atomic counter directly (change byte order) */

errCode_t atomic_setAtomicDirect(atomic_t atomic, uint val)
{
	uint between;

	between=((val >> 24) & 0x000000FF) | ((val >>  8) & 0x0000FF00) |
		    ((val <<  8) & 0x00FF0000) | ((val << 24) & 0xFF000000);
	(volatile uint) (atomStartDirect[atomic])=between;

	WCFLUSH;

	return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* allocate counters from the atomic address range */
/* global operation */

errCode_t sync_allocAtomic(uint size, atomic_t *atomic)
{
	atomic_t	locManager,between;
	errCode_t	err;

#ifdef SYNCMOD_STANDALONE_LIBRARY
	sci_error_t     sisci_error = 0;
#endif

	/* aquire the global lock */

#ifdef VERBOSE
	printf("sync_allocAtomic: do lock\n");
#endif

	err=simpleSync_lock(ATOMIC_MAINLOCK);
	
	/* save old index of max. counter */

#ifdef VERBOSE
	printf("sync_allocAtomic: lock done, reading manager\n");
#endif

	between=atomStartDirect[ATOMIC_MANAGER];

	/* inkrementiere max. index */

#ifdef VERBOSE
	printf("sync_allocAtomic: got old value %i from slot %i\n",between,ATOMIC_MANAGER);
#endif

	locManager = between + size;

	/* check if space is left */

#ifdef VERBOSE
	printf("sync_allocAtomic: new locManager %i, comparing with end %i\n",locManager,atomEnd);
#endif

	if ((locManager<0) || (locManager>atomEnd))
	{
		simpleSync_unlock(ATOMIC_MAINLOCK);
		return ERR_SIMPLESYNC_OUTOFATOMIC;
	}

	/* save new counter value */

#ifdef VERBOSE
	printf("sync_allocAtomic: assigning locManager %i\n",locManager);
#endif

	atomStartDirect[ATOMIC_MANAGER]=locManager;

	/* release the lock */

#ifdef VERBOSE
	printf("sync_allocAtomic: locManager assigned %i at slot %i\n",locManager,ATOMIC_MANAGER);
#endif

	*atomic=between;

#ifdef VERBOSE
	printf("sync_allocAtomic: new value %i, doing sync\n",between);
#endif

#ifdef SYNCMOD_STANDALONE_LIBRARY
	SMI_SCIStoreBarrier(atomicIdent.seq,NO_FLAGS,&sisci_error);
	if (sisci_error!=SCI_ERR_OK)
	  return CONVERT_SISCI_HAMSTER(sisci_error);
#else
	err=simpleSync_sync();
	if (err!=OK_SIMPLESYNC) return err;
#endif

#ifdef VERBOSE
	printf("sync_allocAtomic: doing unlock at slot %i\n",ATOMIC_MAINLOCK);
#endif

	err=simpleSync_unlock(ATOMIC_MAINLOCK);

#ifdef VERBOSE
	printf("sync_allocAtomic: done\n");
#endif

	return err;
}


