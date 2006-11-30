//
// $Id$
//
//

SCI_barrier::SCI_barrier (SCI_macc macc)
{
    
  return;
}

SCI_barrier::~SCI_barrier ()
{
    
  return;
}

void SCI_barrier::wait ()
{
  // orig. call:  sync_barrier(atomic_t atomic, int threadCount, errCode_t (*hook) (void))
  sci_error_t     sisci_error = 0;
  errCode_t	err = OK_SIMPLESYNC, err2;
  uint		ownSync,baseBar,sumBar,oldSync,getVal,i;
  uint            firstLoop=1;
  uint            lastLoop=0;
  volatile uint   *locSearch;
  uint            modThreadCount;
  
  
  modThreadCount = threadCount + 1;

#ifdef VERBOSE
  printf("Start SCI barrier at slot %i\n",atomic);
#endif

  // compute the base value for the barrier
  baseBar = ((volatile uint) atomLocal[atomic+1]);
  
#ifdef VERBOSE
  printf("Base %i started\n", baseBar);
#endif
  
  // try to enter the critical section for the barrier 
  // if fail, than another thread is already in there 
  // and doing the barrier for us 
  
  if (pthread_mutex_trylock(&(localBarrier[atomic])) == 0) {
    // we are in the critical section 
    // i.e. the first local thread at the barrier -> do everything as normal 
    
#ifdef VERBOSE
    printf("Number of activities running: %i\n",threadCount);
#endif
    // reset, then barrier loop 
    oldSync = 0; 
    do {
      // status of local barrier - did something change ?
      ownSync = ((volatile uint) atomLocal[atomic]) + 1;
      if (ownSync != oldSync) {
#ifdef VERBOSE
	printf("Own barrier changed, now at %i / sending to slots %i - %i\n",ownSync,atomic,atomic+locNodeCount-1);
#endif

	// inform all nodes, where we are 
	
	for (i = 0; i < locNodeCount; i++) {
	  (volatile uint)(syncMem[i][atomic+locNodeNum]) = (uint)baseBar + ownSync;

	  // the SCIStoreBarrier below should suffice !
#if 0
	  /*	WCFLUSH */
	  /*	CPUID         Using this leads to segfaults  */
	  _smi_MPIFlushWriteBuffers(); /* using smi-version instead */
#endif
	}

	SCIStoreBarrier(atomicIdent.seq, NO_FLAGS, &sisci_error);
	if (sisci_error != SCI_ERR_OK) {
	  // throw exception
	}
	oldSync = ownSync;
      }

      // count where we are globally 
      sumBar = 0;
      for (i = 0; i < locNodeCount; i++) {
	(volatile uint)getVal = (volatile uint)(syncMem[locNodeNum][atomic+i]);
	if (getVal >= baseBar)	{
	  getVal = (getVal - baseBar) % modThreadCount;
	  sumBar += getVal;
	}
      }
    } while (sumBar < threadCount);

#ifdef VERBOSE
    printf("New base %i reached\n", baseBar+threadCount);
    fflush(stdout);
#endif
    
    pthread_mutex_unlock(&(localBarrier[atomic]));
    // barrier reached, let's reset the local counter 
    
    ownSync = (volatile uint)(atomLocal[atomic]);
    if (ownSync > 0) {
      // we have other threads waiting 
      ((volatile uint) atomLocal[atomic]) = 0;
    }
	    
    (volatile uint)(atomLocal[atomic+1]) = baseBar + modThreadCount;
    WCFLUSH;
  } else {
    volatile uint locbarwait;
    volatile uint *locbarpoint;
	  
    // another thread already does the barrier -> let's just wait for it
#ifdef VERBOSE
    printf("Waiting for another thread\n");
#endif
    
    // get local ticket
    myatomic_inc(&(atomLocal[atomic]));

#ifdef VERBOSE
    if (ownSync > 1)
      printf("BIG OOPS %d\n",ownSync);
#endif

    // wait until main barrier is done 
#ifdef VERBOSE
    cout << "Secondary thread waiting on slot" << ownSync-1 << eol;
#endif
    
    locbarpoint = &((volatile uint)(atomLocal[atomic+1]));
    do {
      locbarwait = ((volatile uint)(*locbarpoint));
    } while (locbarwait < baseBar + modThreadCount);
    
#ifdef VERBOSE
    printf("Secondary thread released on slot %i\n",ownSync-1);
#endif
  }
  
  return;
}

void SCI_barrier::enter ()
{
  // not yet implemented

  return;
}

void SCI_barrier::leave ()
{
  // not yet implemented
    
  return;
}




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
