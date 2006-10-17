/*--------------------------------------------------------------------------*/
/*                                                                          */
/* User-level Module: SYNCMOD                                               */
/*                                                                          */
/* (c) 1998-2001 Martin Schulz, LRR-TUM                                     */
/*                                                                          */
/* Contains the HAMSTER routines for synchronization                        */
/* Standalone SISCI Version                                                 */
/*                                                                          */
/* Implementation file for initialization and clean-up routines             */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Low-level SCI routines */

/*.......................................................................*/
/* type definitions */

typedef struct syncModScial_ident_d
{
  int                   used;
  int                   local;
  int                   connected;
  int                   mapped;
  int                   open;
  int                   seqdone;
  sci_desc_t            sci_desc;
  sci_remote_segment_t  remoteSegment;
  sci_local_segment_t   localSegment;
  sci_map_t             map;
  sci_sequence_t        seq;
} syncModScial_ident_t;


/*.......................................................................*/
/* global variables */

syncModScial_ident_t syncMemIdent[MAX_CLUSTER_NODES];
syncModScial_ident_t masterIdent;
syncModScial_ident_t atomicIdent;


/*.......................................................................*/
/* create and map a local segment */

errCode_t syncModScial_createSegment(uint segID, void **addr, syncModScial_ident_t *ident)
{
  sci_error_t error;
  int         flags;

  /* preset ident structure */

  ident->local=1;
  ident->connected=0;
  ident->mapped=0;
  ident->open=0;
  ident->seqdone;
  ident->used=1;

  /* open SCI descriptor */

  SCIOpen(&(ident->sci_desc), NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->open = 1;

  /* create segment */
  
  SCICreateSegment(ident->sci_desc,&(ident->localSegment), segID,
		   SIZE_ATOMICMEM, NO_CALLBACK, NULL, NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->connected = 1;

  /* perparations for the new segment */

  SCIPrepareSegment(ident->localSegment, 0, NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  SCISetSegmentAvailable(ident->localSegment, 0, NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  /* map the local segment */

  *addr=SCIMapLocalSegment(ident->localSegment,&(ident->map),0,SIZE_ATOMICMEM,
			   NULL,NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->mapped = 1;

  /* make the segment available for remote connections */
  
  SCISetSegmentAvailable(ident->localSegment, 0, NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  /* create the appropriate sequence */

  /*  SCICreateMapSequence(ident->map, &(ident->seq), NO_FLAGS, &error); */
  /*  if (error != SCI_ERR_OK) */
  /*    return CONVERT_SISCI_HAMSTER(error); */

  ident->seqdone = 1;

  return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* delete and unmap a local segment */

errCode_t syncModScial_deleteSegment(syncModScial_ident_t *ident)
{
  sci_error_t error;

  if (ident->seqdone)
    {
      /* delete the sequence */

      /* SCIRemoveSequence(ident->seq, NO_FLAGS, &error); */
      /*      if (error != SCI_ERR_OK) */
      /*	return CONVERT_SISCI_HAMSTER(error); */
      
      ident->seqdone = 0;
    }

  /* set the segment unavailable */

  if (ident->connected)
    {
      SCISetSegmentUnavailable(ident->localSegment,0,NO_FLAGS,&error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
    }

  if (ident->mapped)
    {
      /* unmap the segment */

      SCIUnmapSegment(ident->map, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->mapped = 0;
    }

  if (ident->connected)
    {
      /* delete segment */

      SCIRemoveSegment(ident->localSegment, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->connected = 0;
    }

  if (ident->open)
    {
      /* close descriptor */

      SCIClose(ident->sci_desc, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->open=0;
    }

  ident->used=0;
  return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* connect to and map a remote segment */

errCode_t syncModScial_connectSegment(uint segID, uint nodeID, int atomic, 
				      void **addr, syncModScial_ident_t *ident)
{
  sci_error_t error;
  int         flags;

  /* preset ident structure */

  ident->local=0;
  ident->connected=0;
  ident->mapped=0;
  ident->open=0;
  ident->seqdone=1;
  ident->used=1;

  /* open SCI descriptor */

  SCIOpen(&(ident->sci_desc), NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->open = 1;

  /* compose flags */

  if (atomic)
    flags = SCI_FLAG_LOCK_OPERATION;
  else
    flags = 0;

  /* try to connect */

  do
    {
      SCIConnectSegment(ident->sci_desc, &(ident->remoteSegment),nodeID,segID,0,
			NO_CALLBACK,NULL,SCI_INFINITE_TIMEOUT,NO_FLAGS,&error);
    }
  while (error != SCI_ERR_OK);

  ident->connected=1;

  /* map the segment */

  *addr=(void*) SCIMapRemoteSegment(ident->remoteSegment,&(ident->map),0,
				    SIZE_ATOMICMEM, NULL, flags, &error);

  if (error!=SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->mapped=1;

  /* create the appropriate sequence */

  SCICreateMapSequence(ident->map, &(ident->seq), NO_FLAGS, &error);
  if (error != SCI_ERR_OK)
    return CONVERT_SISCI_HAMSTER(error);

  ident->seqdone = 1;

  return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* disconnect to and unmap a remote segment */

errCode_t syncModScial_disconnectSegment(syncModScial_ident_t *ident)
{
  sci_error_t error;

  if (ident->seqdone)
    {
      /* delete the sequence */

      SCIRemoveSequence(ident->seq, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->seqdone = 0;
    }

  if (ident->mapped)
    {
      /* unmap the segment */

      SCIUnmapSegment(ident->map, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->mapped = 0;
    }

  if (ident->connected)
    {
      /* delete segment */

      SCIDisconnectSegment(ident->remoteSegment, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->connected = 0;
    }

  if (ident->open)
    {
      /* close descriptor */

      SCIClose(ident->sci_desc, NO_FLAGS, &error);
      if (error != SCI_ERR_OK)
	return CONVERT_SISCI_HAMSTER(error);
      
      ident->open=0;
    }

  ident->used=0;
  return OK_SIMPLESYNC;
}


/*.......................................................................*/
/* remove a segment (either local or remote) */

errCode_t syncModScial_removeSegment(syncModScial_ident_t *ident)
{
  if (ident->used)
    {
      if (ident->local)
	{
	  return syncModScial_deleteSegment(ident);
	}
      else
	{
	  return syncModScial_disconnectSegment(ident);
	}
    }
  else
    return OK_SIMPLESYNC;
}


/*--------------------------------------------------------------------------*/
/* Connection routines */

/*.......................................................................*/
/* create the initial mappings */

errCode_t syncModInit_start(uint *nodeIDs, uint masterID, uint syncMemID)
{
  errCode_t err;
  uint      i;

  /* create atomic segment */

  if (locNodeNum==0)
    {
      err=syncModScial_createSegment(masterID, (void**) (&atomStartDirect), &masterIdent);
    }
  else
    {
      err=syncModScial_connectSegment(masterID, nodeIDs[0],0,(void**) (&atomStartDirect), &masterIdent);
    }

  if (err!=OK_SIMPLESYNC)
    return err;


  /* atomic map to segment */

  err=syncModScial_connectSegment(masterID, nodeIDs[0],1,(void**) (&atomStart), &atomicIdent);
  if (err!=OK_SIMPLESYNC)
    return err;


  /* create own syncmem */

  err=syncModScial_createSegment(syncMemID+locNodeNum,(void**) (&(syncMem[locNodeNum])), &syncMemIdent[locNodeNum]);
  if (err!=OK_SIMPLESYNC)
    return err;


  /* map other syncmems */

  for (i=0; i<locNodeCount; i++)
    {
      if (i!=locNodeNum)
        {
          err=syncModScial_connectSegment(syncMemID+i,nodeIDs[i],0,(void**) (&(syncMem[i])), &syncMemIdent[i]);
          if (err!=OK_SIMPLESYNC)
            return err;
        }
    }    

  return OK_SIMPLESYNC;
}

#ifdef USED_WITHIN_SMI
#define BCAST(data, sender) _smi_ll_bcast(&(data), 1, (sender), _smi_my_proc_rank);
errCode_t syncModInit_start_dynamic(uint *nodeIDs, uint masterID, uint syncMemID)
{
  errCode_t err;
  uint      i;
  uint      iTemp;


  /* create atomic segment */

  if (locNodeNum==0)
    {
        do {
	    err=syncModScial_createSegment(masterID, (void**)
					   (&atomStartDirect), &masterIdent);
	    masterID++;  
	} while (err == CONVERT_SISCI_HAMSTER(SCI_ERR_SEGMENTID_USED));
	masterID--;
	BCAST(masterID, 0);
    }
  else
    {
	BCAST(masterID, 0);
	err=syncModScial_connectSegment(masterID, nodeIDs[0],0,(void**) (&atomStartDirect), &masterIdent);
    }

  if (err!=OK_SIMPLESYNC)
    return err;


  /* atomic map to segment */

  err=syncModScial_connectSegment(masterID, nodeIDs[0],1,(void**) (&atomStart), &atomicIdent);
  if (err!=OK_SIMPLESYNC)
    return err;


  /* create own syncmem */

  syncMemID += locNodeNum;
  do {
      err=syncModScial_createSegment(syncMemID,(void**)(&(syncMem[locNodeNum])), &syncMemIdent[locNodeNum]);
      syncMemID++;
  } while (err == CONVERT_SISCI_HAMSTER(SCI_ERR_SEGMENTID_USED));	
  syncMemID--;

  if (err!=OK_SIMPLESYNC)
	  return err;

  /* map other syncmems */

  for (i=0; i<locNodeCount; i++) {
      iTemp = syncMemID;
      BCAST(iTemp, i);
      if (i!=locNodeNum)
      {
	  err=syncModScial_connectSegment(iTemp,nodeIDs[i],0,(void**) (&(syncMem[i])), &syncMemIdent[i]);
	  if (err!=OK_SIMPLESYNC)
	      return err;
      }
  }    
  
  return OK_SIMPLESYNC;
}
#endif /* USED_WITHIN_SMI */ */

/*.......................................................................*/
/* delete the initial mappings */

errCode_t syncModInit_end()
{
  errCode_t err,err2;
  uint      i;

  err=syncModScial_removeSegment(&atomicIdent);
  err2=syncModScial_removeSegment(&masterIdent);
  if (err==OK_SIMPLESYNC)
    err=err2;
  
  for (i=0; i<locNodeCount; i++)
    {
      err2=syncModScial_removeSegment(&syncMemIdent[i]);
      if (err==OK_SIMPLESYNC)
	err=err2;
    }

  return err2;
}


/*--------------------------------------------------------------------------*/
/* Library initialization and cleanup */

/*.......................................................................*/
/* library entrance */

DLLEXPORT errCode_t DLLDECL syncMod_start(uint nodeNum, uint nodeCount, uint *nodeIds, 
					  uint syncMemSegId, uint masterSegId)
{
  if (!simpleSync_started)
    {
      errCode_t	err;
      uint		i;
      

      /* sanity checks */

      if (nodeCount>MAX_CLUSTER_NODES)
	return ERR_SIMPLESYNC_STARTUP;

      if (nodeNum>=nodeCount)
	return ERR_SIMPLESYNC_STARTUP;


      /* Intialize connection identifier */

      masterIdent.used = 0;
      atomicIdent.used = 0;
      for (i=0; i<MAX_CLUSTER_NODES; i++)
	{
	  syncMemIdent[i].used=0;
	}


      /* reset statistics */

      syncMod_resetStatistics(&syncMod_globalStat);


      /* Initialize the critical section required for local barriers */
      
      for (i=0; i<(SIZE_ATOMICMEM / sizeof(uint)); i++)
	{
	  #ifdef WIN32
	  InitializeCriticalSection(&(localBarrier[i]));
	  #endif
	  #ifdef LINUX
	  pthread_mutex_init(&(localBarrier[i]), NULL);
	  pthread_cond_init(&(localCond[i]), NULL);
	  #endif
	}
				

      /* query the local node number */

      locNodeNum=nodeNum;

      /* query the number of nodes in the cluster */
      
      locNodeCount=nodeCount;

      /* query the local SCI node ID */
      
      locNodeID=nodeIds[locNodeNum];


      /* reset the array that stores the pointers to the syncMem */

#ifdef SIMPLESYNC_NEWBARRIER
      for (i=0; i<MAX_CLUSTER_NODES; i++)
	{
	  syncMem[i]=NULL;
	}
#endif


      /* initialize secondary barrier */
      
#ifdef SIMPLESYNC_NEWBARRIER
#else
      secondaryBarrier=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif

      
      /* more difficult task from former extra init routine */

      /* prepare the atomic memory segment and the syncMem */
#ifndef USED_WITHIN_SMI
      err=syncModInit_start(nodeIds,masterSegId,syncMemSegId);
#else
      err=syncModInit_start_dynamic(nodeIds,masterSegId,syncMemSegId);
#endif 
      if (err!=OK_SIMPLESYNC)
	{
	  syncModInit_end();
	  return err;
	}

      /* create event variables for all potential local threads */

#ifdef SIMPLESYNC_NEWBARRIER
#ifdef WIN32
	for (i=0; i<MAX_THREADS_PER_NODE; i++)
	{
		localThreadBarrier[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
	}
#endif
#endif

	/* done with all the initialization */
      
	simpleSync_started=TRUE;

	if (locNodeNum==0)
	  (volatile uint) atomStartDirect[ATOMIC_MANAGER]=ATOMIC_USERSTART;
	atomEnd=(SIZE_ATOMICMEM / sizeof(uint))-1;

	/* test the barrier:-) */
	
	syncMod_fixedBarrier(ATOMIC_BARRIER,locNodeCount);
    }

  return OK_SIMPLESYNC;
}

/*.......................................................................*/
/* manual lib exit */

errCode_t syncMod_stop()
{
  if (simpleSync_started)
    {
      int i;
      errCode_t err;
      
      /* execute a last barrier */

      syncMod_fixedBarrier(ATOMIC_BARRIER,locNodeCount);
      
      /* Delete the critical section required for local barriers */
      
#ifdef WIN32
      for (i=0; i<(SIZE_ATOMICMEM / sizeof(uint)); i++)
	DeleteCriticalSection(&(localBarrier[i]));
#endif
      
      /* remove SCI mapping */
      
      syncModInit_end(); 
      
      simpleSync_started=FALSE;
    }

  return OK_SIMPLESYNC;
}


/*--------------------------------------------------------------------------*/
/* The End. */
