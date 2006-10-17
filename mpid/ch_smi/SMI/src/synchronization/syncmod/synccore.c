/*--------------------------------------------------------------------------*/
/* Basic routines to perform synchronization */

/*#define DEBUG_SYNCMOD */

#ifdef VERBOSE
#define VERBOSE2
#endif

#ifdef DEBUG_SYNCMOD
uint oldBaseSave1=0,oldBaseSave2 = 0;
#endif

/*.......................................................................*/
/* execute an barrier */

#ifdef SIMPLESYNC_NEWBARRIER

errCode_t sync_barrier(atomic_t atomic, int threadCount, errCode_t (*hook) (void))
{
        errCode_t	err = OK_SIMPLESYNC, err2;
	uint		ownSync,baseBar,sumBar,oldSync,getVal,i;
	uint            firstLoop=1;
	uint            lastLoop=0;
	volatile uint   *locSearch;
	uint            modThreadCount;

#ifdef SYNCMOD_STANDALONE_LIBRARY
	sci_error_t     sisci_error = 0;
#endif

#ifdef VERBOSE2
	int             debug_counter = 999999;
#endif

#ifdef COLLECT_STATS
	hamster_clock_t  startTime,interTime;

	startTime=hamster_time();
#endif

	    modThreadCount=threadCount+1;

#ifdef VERBOSE2
	printf("Start SCI barrier at slot %i\n",atomic);
#endif

	/* compute the base value for the barrier */

#ifdef GLOBAL_BARRIER_BASE	
	err=atomic_getAtomicDirect(atomic,&baseBar);
	if (err!=OK_SIMPLESYNC)
	  return err;
#else
	baseBar=((volatile uint) atomLocal[atomic+1]);
#endif

#ifdef DEBUG_SYNCMOD
	if (oldBaseSave1>=baseBar)
	  printf("JUMP BACK : new %d, old %d\n",baseBar, oldBaseSave1);
	oldBaseSave2=oldBaseSave1;
	oldBaseSave1=baseBar;
	#undef DEBUG_SYNCMOD
#endif

#ifdef VERBOSE
	printf("Base %i started\n", baseBar);
#endif

	/* try to enter the critical section for the barrier */
	/* if fail, than another thread is already in there */
	/* and doing the barrier for us */

#ifdef WIN32
	if (TryEnterCriticalSection(&(localBarrier[atomic])))
#endif
#ifdef LINUX
	if (pthread_mutex_trylock(&(localBarrier[atomic]))==0)
#endif
	{
		/* we are in the critical section */
		/* i.e. the first local thread at the barrier */
		/* do everything es normal */
	
		#ifdef VERBOSE
		printf("Number of activities running: %i\n",threadCount);
		#endif

		/* reset */

		oldSync=0;

		/* barrier loop */

		do
		{
#ifdef COLLECT_STATS	      
		  lastLoop=0;
#endif

			/* status of local barrier */

			ownSync=((volatile uint) atomLocal[atomic])+1;

			/* did something change ? */

			if (ownSync!=oldSync)
			{
#ifdef VERBOSE
				printf("Own barrier changed, now at %i / sending to slots %i - %i\n",ownSync,atomic,atomic+locNodeCount-1);
#endif

#ifdef COLLECT_STATS
				lastLoop=1;
#endif
				
				/* inform all nodes, where we are */

				for (i=0; i<locNodeCount; i++)
				{
					(((volatile uint) ((syncMem[i])[atomic+locNodeNum])))=((uint) baseBar+ownSync);
				/*	WCFLUSH */
				/*	CPUID         Using this leads to segfaults  */
				_smi_flush_write_buffers(); /* using smi-version instead */
				}

#ifdef SYNCMOD_STANDALONE_LIBRARY
				SMI_SCIStoreBarrier(atomicIdent.seq,NO_FLAGS,&sisci_error);
				if (sisci_error!=SCI_ERR_OK)
				  return CONVERT_SISCI_HAMSTER(sisci_error);
#else
				err2=scial_writeBarrier();
#endif
				oldSync=ownSync;
			}

			/*simpleSync_sync(); */
			/*Sleep(0); */

			/* count where we are globally */

			sumBar=0;
			for (i=0; i<locNodeCount; i++)
			{
				((volatile uint) getVal)=((volatile uint) (((syncMem[locNodeNum])[atomic+i])));
				if (getVal>=baseBar)
				{
					getVal = (getVal - baseBar) % modThreadCount;
					sumBar += getVal;
				}
#ifdef VERBOSE2
				if ((debug_counter % 1000000) == 0)
				  {
				    printf("%i: %i, ",i,getVal);
				    fflush(stdout);
				  }
#endif
			}

#ifdef VERBOSE2
			if ((debug_counter % 1000000) == 0)
				  {
				    printf("\nSum at %i, with own part at %i (%i threads) / base %d / slot %d\n",sumBar,ownSync,threadCount,baseBar,atomic);
				    fflush(stdout);
				    debug_counter=1000000;
				  }
				debug_counter--;
#endif

#ifdef WIN32XX
			if (sumBar<threadCount)
				Sleep(0);
#endif

#ifdef COLLECT_STATS
			if ((firstLoop) && (sumBar==1))
			  {
			    syncMod_globalStat.numBarFirst++;
			    firstLoop=0;
			  }
#endif

		}
		while (sumBar<threadCount);

#ifdef COLLECT_STATS
		if (lastLoop)
		  {
		    syncMod_globalStat.numBarLast++;
		  }
#endif
#ifdef VERBOSE
		printf("New base %i reached\n", baseBar+threadCount);
		fflush(stdout);
#endif

		/* execute user function at 1 thread/node barrier point */

		if (hook!=NULL)
		  {
		    err=hook();
		    if (err!=OK_SIMPLESYNC)
		      return err;
		  }

		/* compute the base value for the barrier */

#ifdef GLOBAL_BARRIER_BASE
		err=atomic_setAtomicDirect(atomic,baseBar+modThreadCount);
		if (err!=OK_SIMPLESYNC)
			return err;

		err=scial_writeBarrier();
		if (err!=OK_SIMPLESYNC)
			return err;		
#endif

		/* we are done, let's leave the critical section */

#ifdef WIN32
		LeaveCriticalSection(&(localBarrier[atomic]));
#endif
#ifdef LINUX
		pthread_mutex_unlock(&(localBarrier[atomic]));
#endif
		/* barrier reached, let's reset the local counter */

		ownSync=(volatile uint) (atomLocal[atomic]);
		if (ownSync>0)
		  {
		    /* we have other threads waiting */
#ifdef LINUX
		    /*  pthread_mutex_lock(&localBarrier[atomic+1]); */
#endif
		    ((volatile uint) atomLocal[atomic])=0;

		    /* release other local threads (if any) */
	
#ifndef GLOBAL_BARRIER_BASE
		  }
#endif
	    
		    (volatile uint) (atomLocal[atomic+1])=baseBar+modThreadCount;
		    WCFLUSH;

#ifdef SECONDARY_BLOCK
#ifndef GLOBAL_BARRIER_BASE
		if (ownSync>0)
		  {
#endif

#ifdef VERBOSE
		    printf("Releasing the other %i threads\n",ownSync);
#endif

#ifdef WIN32
		    for (i=0; i<ownSync; i++)
		      SetEvent(localThreadBarrier[i]);
#endif
#ifdef LINUX
		    /*  pthread_mutex_unlock(&localBarrier[atomic+1]); */
		    pthread_cond_broadcast(&localCond[atomic]);
#endif
		  }
#endif /* SECONDARY BLOCK */
	}
	else
	{
	  volatile uint locbarwait;
	  volatile uint *locbarpoint;
	  
		/* another thread already does the barrier */
		/* let's just wait for it */

#ifdef VERBOSE
		printf("Waiting for another thread\n");
#endif

		/* get local ticket */

#ifdef WIN32
		ownSync=InterlockedIncrement((long*) &(atomLocal[atomic]));
#endif
#ifdef LINUX
#ifdef SECONDARY_BLOCK
		pthread_mutex_lock(&localBarrier[atomic+1]);
		ownSync=(long) (atomLocal[atomic]);
		(volatile uint) atomLocal[atomic] = ownSync+1;
		WCFLUSH;
#else /* SECONDARY_BLOCK */
		myatomic_inc(&(atomLocal[atomic]));
#endif /* SECONDARY_BLOCK */

#endif /* LINUX */

#ifdef VERBOSE
		  if (ownSync>1)
		    printf("BIG OOPS %d\n",ownSync);
#endif

		/* wait until main barrier is done */

#ifdef VERBOSE
		printf("Secondary thread waiting on slot %i\n",ownSync-1);
#endif

		locbarpoint = &((volatile uint) (atomLocal[atomic+1]));
		do
		  {
		    locbarwait=((volatile uint) (*locbarpoint));
#ifdef SECONDARY_BLOCK
#ifdef WIN32
		    WaitForSingleObject(localThreadBarrier[ownSync-1],INFINITE);
#endif
#ifdef LINUX
		    pthread_cond_wait(&localCond[atomic],&localBarrier[atomic+1]);
#endif
#endif
		  }
		while (locbarwait< baseBar+modThreadCount);
		
#ifdef SECONDARY_BLOCK
		pthread_mutex_unlock(&localBarrier[atomic+1]);
#endif

#ifdef VERBOSE
		printf("Secondary thread released on slot %i\n",ownSync-1);
#endif
	}

	#ifdef VERBOSE
	printf("Done with barrier\n");
	#endif

#ifdef COLLECT_STATS
	interTime=hamster_time()-startTime;
	if (syncMod_globalStat.maxBarrier<interTime)
	  syncMod_globalStat.maxBarrier=interTime;
	syncMod_globalStat.timeBarrier+=interTime;
	syncMod_globalStat.numBarrier++;
#endif	

	return OK_SIMPLESYNC;
}


#else

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/* old style barrier */

errCode_t sync_barrier(atomic_t atomic, int threadCount)
{
	uint		oldCounter,getVal,top,bot,atomTLS,errCount;
	errCode_t	err = OK_SIMPLESYNC;


#ifdef VERBOSE
	printf("Start SCI barrier at slot %i\n",atomic);
#endif

	/* try to enter the critical section for the barrier */
	/* if fail, than another thread is already in there */
	/* and doing the barrier for us */

#ifdef WIN32
	if (TryEnterCriticalSection(&(localBarrier[atomic])))
#endif
#ifdef LINUX
	if (pthread_mutex_trylock(&(localBarrier[atomic]))==0)
#endif
	{
		/* we are in the critical section */
		/* i.e. the first local thread at the barrier */
		/* do everything es normal */
	
		#ifdef VERBOSE
		printf("Number of activities running: %i\n",threadCount);
		#endif

		/* increment global counter */

		if (atomLocal[atomic]<40)
			bot=0;
		else
			bot=atomLocal[atomic]-40;

		top=atomLocal[atomic]+40;

		errCount=0;
		do
		{
	        #ifdef WIN32
			if (errCount!=0)
				Sleep(errCount*50);
			#endif

			errCount++;

			#ifdef WIN32
			Sleep(0);
			#endif
			#ifdef
			sleep(0);
			#endif

			#ifdef VERBOSE
			printf("Get ticket\n");
			#endif

			err=atomic_getAtomic(atomic,&oldCounter);
			#ifdef VERBOSEXX
			printf("Get value from atomic segment: %8x\n",oldCounter);
			#endif
			if (err!=OK_SIMPLESYNC) 
			{
			        #ifdef WIN32
				LeaveCriticalSection(&(localBarrier[atomic]));
                                #endif
                                #ifdef LINUX
				pthread_mutex_unlock(&(localBarrier[atomic]));
			        #endif
				return err;
			}
		
			#ifdef VERBOSE
			printf("Ticket is %i\n",oldCounter);
			#endif
		}
		while ((oldCounter<bot) || (oldCounter>top));

		/* flush local memory state */

		err=simpleSync_sync();
		if (err!=OK_SIMPLESYNC)
			return err;

		/*		err=sci_clearMemory(); */
		if (err!=OK_SIMPLESYNC) 
		{
		        #ifdef WIN32
			LeaveCriticalSection(&(localBarrier[atomic]));
			#endif
			#ifdef LINUX
			pthread_mutex_unlock(&(localBarrier[atomic]));
			#endif
			return err;
		}
	
		/* increment local counter */

		atomTLS=atomLocal[atomic] + threadCount;
		atomLocal[atomic]=atomTLS;

		/* wait until all nodes have reached the barrier */

		top=atomTLS+threadCount;
		bot=atomTLS;
		#ifdef VERBOSE
		printf("Before wait\n");
		#endif
		errCount=0;
		do
		{
			errCount++;

	        #ifdef WIN32
			if (errCount % 20 == 0)
				Sleep(1);
			#endif

			err=atomic_getAtomicDirect(atomic,&getVal);
			if (err!=OK_SIMPLESYNC)
			{
			        #ifdef WIN32
				LeaveCriticalSection(&(localBarrier[atomic]));
          			#endif
		        	#ifdef LINUX
				pthread_mutex_unlock(&(localBarrier[atomic]));
                                #endif
				return err;
			}
		}
		while ((getVal<bot) || (getVal>=top));

		#ifdef WIN32
		LeaveCriticalSection(&(localBarrier[atomic]));
		/* hack for second thread on the same node */
		SetEvent(secondaryBarrier);
		#endif
		#ifdef LINUX
		pthread_mutex_unlock(&(localBarrier[atomic]));
		#endif
	}
	else
	{
		/* another thread already does the barrier */
		/* let's just wait for it */

#ifdef VERBOSE
		printf("Waiting for another thread\n");
#endif

		/* increment global counter */

		if (atomLocal[atomic]<40)
			bot=0;
		else
			bot=atomLocal[atomic]-40;

		top=atomLocal[atomic]+40;

		errCount=0;
		do
		{
		        #ifdef WIN32
			if (errCount!=0)
				Sleep(errCount*50);
			#endif
			errCount++;

			#ifdef VERBOSE
			printf("Get ticket\n");
			#endif

			err=atomic_getAtomic(atomic,&oldCounter);
			#ifdef VERBOSEXX
			printf("Get value from atomic segment: %8x\n",oldCounter);
			#endif
			if (err!=OK_SIMPLESYNC)
				return err;

			#ifdef VERBOSE
			printf("Ticket is %i\n",oldCounter);
			#endif
		}
		while ((oldCounter<bot) || (oldCounter>top));

		/* flush local memory state */

		err=simpleSync_sync();
		if (err!=OK_SIMPLESYNC)
			return err;

		/*		err=sci_clearMemory(); */
		/*		if (err!=OK_SIMPLESYNC) */
		/*			return err; */

		#ifdef WIN32
		/* Quick hack for Win32 only and for 2 threads per node only */
		/* has to be fixed later !!! */
		WaitForSingleObject(secondaryBarrier,INFINITE);
		#endif
		#ifdef LINUX
		pthread_mutex_lock(&(localBarrier[atomic]));
		pthread_mutex_unlock(&(localBarrier[atomic]));
		#endif
	}

	#ifdef VERBOSE
	printf("Done with barrier\n");
	#endif

	return OK_SIMPLESYNC;
}

#endif


/*.......................................................................*/
/* acquire a lock */

#ifdef SIMPLESYNC_NEWLOCK

errCode_t sync_lock(int atomic)
{
  volatile uint	ticket,access,test;
  uint            minimum; 
  int             boundary,i;
  errCode_t		err = OK_SIMPLESYNC;
  volatile uint	valid[MAX_CLUSTER_NODES];
  
#ifdef COLLECT_STATS
  hamster_clock_t  startTime,interTime;
  
  startTime=hamster_time();
#endif

  /* do local lock first */

  pthread_mutex_lock(&localBarrier[atomic]);
  
  /* mark wish */
  
#ifdef VERBOSE
  printf("Start lock on node %i / slot %i\n",locNodeNum,atomic);
#endif
  err=atomic_setAtomicDirect(atomic+2+locNodeNum,1);
  if (err!=OK_SIMPLESYNC) return err;
  
  /* flush the write buffers */
  
#ifdef VERBOSE
  printf("Start write Barrier 1\n");
#endif
  /*err=scial_writeBarrier(); */
  /*if (err!=OK_SCIAL) return err; */
  
  /* atomically increment ticket counter */
  
#ifdef VERBOSE
  printf("Get ticket\n");
#endif
  err=atomic_getAtomic(atomic,&ticket);
  if (err!=OK_SIMPLESYNC) return err;
#ifdef VERBOSE
  printf("Ticket = %i\n",ticket);
#endif
  
  /* mark the ticket in the slot */
  
  err=atomic_setAtomicDirect(atomic+2+locNodeNum,ticket+2);
  if (err!=OK_SIMPLESYNC) return err;
#ifdef VERBOSE
  printf("Ticket set in wish list\n");
#endif
  
  /* Test whether we have direct access to the lock (shortcut) */
  
  err=atomic_getAtomicDirect(atomic+1,&access);
#ifdef VERBOSE
  printf("Access token : %i\n",access);
#endif
  if (ticket==access)
    {
      /*err=sci_acquire(); */
#ifdef VERBOSE
      printf("We got the lock the fast way\n");
#endif
    }
  else
    {
      /* lock is not free or error during inc, do longer version */
      
#ifdef COLLECT_STATS
      syncMod_globalStat.numLockWait++;
#endif	

      /* flush the write buffers again (paranoia hack) */
      
#ifdef VERBOSE
      printf("Start write Barrier 2\n");
#endif
      /*      err=scial_writeBarrier(); */
      /*      if (err!=OK_SCIAL) return err; */
      
      /* first run, check valid entries */
      
      boundary=-1;
      
      for (i=0; i<locNodeCount; i++)
	{
	  err=atomic_getAtomicDirect(atomic+2+i,&(valid[i]));
	  if (err!=OK_SIMPLESYNC) return err;
	  if (valid[i]) boundary=i;
	}			
      
#ifdef VERBOSE
      printf("Created valid field, boundary = %i\n",boundary);
#endif
      
      /* now loop until we have the lock */
      
      do
	{
#ifdef VERBOSE
	  printf("Test field: ");
#endif
	  minimum=ticket+2;
	  for (i=0; i<=boundary; i++)
	    {
	      if (valid[i])
		{
		  err=atomic_getAtomicDirect(atomic+2+i,&test);
		  if (err!=OK_SIMPLESYNC) return err;
		  if (test==0)
		    valid[i]=0;
		  else
		    if (test<minimum) minimum=test;
#ifdef VERBOSE
		  printf(" %i",test);
#endif
		}
#ifdef VERBOSE
	      else 
		printf(" -");
#endif
	    }
#ifdef VERBOSE
	  printf(" : Wait loop end - minimum %i, ticket %i\n",minimum,ticket);
#endif
#ifdef WIN32
	  Sleep(0);
#endif
	}
      while (minimum!=ticket+2);
      
      /* we got the lock, everything is fine, do acquire */
      
#ifdef VERBOSE
      printf("We got the lock the slow way\n");
#endif
      
      /*	err=sci_acquire(); */
    } /* else, i.e. lock the slow way */

	    
#ifdef COLLECT_STATS
  interTime=hamster_time()-startTime;
  if (syncMod_globalStat.maxLock<interTime)
    syncMod_globalStat.maxLock=interTime;
  syncMod_globalStat.timeLock+=interTime;
  syncMod_globalStat.numLock++;
#endif	

  return err;
}

#else

errCode_t sync_lock(int atomic)
{
	volatile uint	ticket,access;
	errCode_t		err = OK_SIMPLESYNC;

	/* atomically increment ticket counter */

	err=atomic_getAtomic(atomic,&ticket);
	if (err!=OK_SIMPLESYNC) return err;

	/* wait until access counter is high enough to proceed */

	do
	{
		err=atomic_getAtomicDirect(atomic+1,&access);
		if (err!=OK_SIMPLESYNC) return err;
	}
	while (access!=ticket);

	/* synchronize */

/*	err=sci_acquire(); */
	return err;
}

#endif

/*.......................................................................*/
/* release a lock */

#ifdef SIMPLESYNC_NEWLOCK

errCode_t sync_unlock(int atomic)
{
	volatile uint	oldCounter,test;
	errCode_t		err;

#ifdef COLLECT_STATS
      syncMod_globalStat.numUnlock++;
#endif	

/*	err=sci_clearMemory(); */
/*	if (err!=OK_SIMPLESYNC) return err; */

	/* as we still have the lock, we can modify the access counter non atomically */

	err=atomic_getAtomicDirect(atomic+1,&test);
	if (err!=OK_SIMPLESYNC) return err;
	err=atomic_setAtomicDirect(atomic+1,test+1);
	if (err!=OK_SIMPLESYNC) return err;
	
	/* now the lock is released, but we still have to undo the reservation */

	err=atomic_setAtomicDirect(atomic+2+locNodeNum,0);

	pthread_mutex_unlock(&localBarrier[atomic]);

	return err;
}

#else

errCode_t sync_unlock(int atomic)
{
	volatile uint	oldCounter,test;
	errCode_t		err;

/*	WCFLUSH; */

/*	err=sci_release(); */
/*  if (err!=OK_SIMPLESYNC) return err; */

	/* as we still have the lock, we can modify the access counter non atomically */

	err=atomic_getAtomicDirect(atomic+1,&test);
	if (err!=OK_SIMPLESYNC) return err;
	err=atomic_setAtomicDirect(atomic+1,test+1);
	if (err!=OK_SIMPLESYNC) return err;

	WCFLUSH;
	
	/* increment ticket counter */

	/*err=atomic_getAtomic(atomic+1,&oldCounter); */

#ifdef VERBOSE
	printf("sync_allocAtomic: did unlock for Slot %i\n",atomic);
#endif

	/*if (err==OK_SIMPLESYNC) */
	/*	err=consMod_syncRead(); */

	return err;
}

#endif


