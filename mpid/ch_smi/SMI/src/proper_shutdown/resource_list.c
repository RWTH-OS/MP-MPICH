/* $Id$ */

/* the resource list keeps track of all _smi_allocated SCI resources and 
   frees them on exit (normal or abort by signal) */

#include <stdio.h>
#include <pthread.h>

#include "resource_list.h"
#include "env/smidebug.h"

#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static rs_node_t rs_node_root;
static int rs_initialized = FALSE;
static pthread_mutex_t rsMutex;

int rs_is_equal(rs_node_t* pNode,rs_resource_t Resource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_is_equal");

  if (pNode->ResourceType != ResourceType)
    return(FALSE);
  
  switch(ResourceType) {
#ifndef NO_SISCI
  case rtSciDesc:
    return(Resource.SciDesc == pNode->Resource.SciDesc);
    break;
  case rtDmaQueue:
    return(Resource.DmaQueue == pNode->Resource.DmaQueue);
    break;
  case rtLocInterrupt:
    return(Resource.LocInterrupt == pNode->Resource.LocInterrupt);
    break;
  case rtRmtInterrupt:
    return(Resource.RmtInterrupt == pNode->Resource.RmtInterrupt);
    break;
  case rtSequence:
    return(Resource.Sequence == pNode->Resource.Sequence);
    break;
  case rtSegment:
    return(Resource.Segment == pNode->Resource.Segment);
    break;
  case rtMap:
    return(Resource.Map == pNode->Resource.Map);
    break;
  case rtConnection:
    return(Resource.Connection == pNode->Resource.Connection);
    break;
#endif
  case rtSharedMemory:
    return(Resource.ShmId == pNode->Resource.ShmId);
    break;
  case rtTempFile:
    return(strcmp(Resource.TempFile.szName, pNode->Resource.TempFile.szName)==0);
    break;
  case rtTempStream:
    return(strcmp(Resource.TempStream.szName, pNode->Resource.TempStream.szName)==0);
    break;
#ifndef DISABLE_THREADS
  case rtThread:
      return (Resource.thread_id == pNode->Resource.thread_id);
    break;
#endif
  case rtNone:
    return(Resource.None == pNode->Resource.None);
    break;
  default:
    return(FALSE);
    break;
  }  
  /* return(FALSE); */
}

void rs_mk_resource(rs_resource_t* newResource, void *pResource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_mk_resource");

  switch(ResourceType) {
#ifndef NO_SISCI
  case rtSciDesc:
    newResource->SciDesc = (*((sci_desc_t*) pResource));
    break;
  case rtDmaQueue:
    newResource->DmaQueue = (*((sci_dma_queue_t*) pResource));
    break;
  case rtLocInterrupt:
    newResource->LocInterrupt = (*((sci_local_interrupt_t*) pResource));
    break;
  case rtRmtInterrupt:
    newResource->RmtInterrupt = (*((sci_remote_interrupt_t*) pResource));
    break;
  case rtSequence:
    newResource->Sequence = (*((sci_sequence_t*) pResource));
    break;
  case rtSegment:
    newResource->Segment = (*((sci_local_segment_t*) pResource));
    break;
  case rtMap:
    newResource->Map = (*((sci_map_t*) pResource));
    break;
  case rtConnection:
    newResource->Connection = (*((sci_remote_segment_t*) pResource));
    break;
#endif
  case rtSharedMemory:
    newResource->ShmId = *((int*)pResource);
    break;
  case rtTempFile:
    memcpy((void*)&(newResource->TempFile), pResource, sizeof(rs_tempfile_t));
    break;
  case rtTempStream:
    memcpy((void*)&(newResource->TempStream), pResource, sizeof(rs_tempstream_t));
    break;
#ifndef DISABLE_THREADS
  case rtThread:
      newResource->thread_id = *(pthread_t *)pResource;
    break;
#endif
  case rtNone:
    newResource->None = *((void**)pResource);
    break;
  default:
    newResource->None = NULL;
    break;
  }
}

rs_node_t* rs_node_search_pre(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_node_search_pre");
  rs_node_t* pTemp=pRoot;
  rs_resource_t Resource;

  rs_mk_resource(&Resource, pResource, ResourceType);

  while(pTemp->pNext!=NULL){
    if (rs_is_equal(pTemp->pNext, Resource, ResourceType))
      break;
    pTemp = pTemp->pNext;
  }
   
  return((pTemp->pNext != NULL)? pTemp : NULL);
}

rs_node_t* rs_node_search(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_node_search");
  rs_node_t* pTemp = pRoot;
  rs_resource_t Resource;

  rs_mk_resource(&Resource, pResource, ResourceType);

  while(pTemp->pNext != NULL){
    if (rs_is_equal(pTemp->pNext, Resource, ResourceType))
      break;
    pTemp = pTemp->pNext;
  }
   
  return(pTemp->pNext);
}

int rs_node_push(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_node_push");
  rs_node_t* pTemp;

  SMI_LOCK(&rsMutex);
  pTemp = pRoot->pNext;

  ALLOCATE (pRoot->pNext,  rs_node_t*,  sizeof (rs_node_t));
  if(pRoot->pNext == NULL) {
    SMI_UNLOCK(&rsMutex);
    return(-1);
  }
  pRoot->pNext->pNext = pTemp;
  pTemp = pRoot->pNext; 
  rs_mk_resource(&(pTemp->Resource), pResource, ResourceType);
  pTemp->ResourceType = ResourceType;
  
  SMI_UNLOCK(&rsMutex);
  return(0);
}

int rs_node_pop(rs_node_t* pRoot, rs_resource_t* pResource, rs_resource_type_t* pResourceType)
{
  DSECTION("rs_node_pop");
  rs_node_t* pTemp;

  SMI_LOCK(&rsMutex);
  pTemp = pRoot->pNext;

  if (pTemp == NULL) {
    SMI_UNLOCK(&rsMutex);
    return(-1);
  }

  *pResource = pTemp->Resource;
  *pResourceType = pTemp->ResourceType;
  
  pRoot->pNext = pTemp->pNext;
  free(pTemp);

  SMI_UNLOCK(&rsMutex);

  return(0);
}

int rs_node_remove(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType)
{
  DSECTION("rs_node_remove");
  rs_node_t* pTemp;
  rs_node_t* pTemp2;

  SMI_LOCK(&rsMutex);

  pTemp = rs_node_search_pre(pRoot,pResource,ResourceType);

  if (pTemp == NULL) {
    SMI_UNLOCK(&rsMutex);
    return(-1);
  }
  
  pTemp2 = pTemp->pNext;
  pTemp->pNext = pTemp2->pNext;
  free(pTemp2);

  SMI_UNLOCK(&rsMutex);

  return(0);
}

int _smi_init_resource_list()
{
    DSECTION("_smi_init_resource_list");
    
    DSECTENTRYPOINT;

    if(rs_initialized == FALSE) {
	rs_node_root.pNext = NULL;
#ifndef DISABLE_THREADS
	SMI_INIT_LOCK(&rsMutex);
#endif
	rs_initialized = TRUE;
	
	/* initialize extensions for sci-descriptor management */
	_smi_sci_desc_init();
    }
    else
	DWARNING("resourcelist already initialized; nothing done");
    
    DSECTLEAVE;
    return(0);
}

int _smi_clear_all_resources()
{
    DSECTION("_smi_clear_all_resources");
    rs_resource_t Resource;
    rs_resource_type_t ResourceType;
    sci_error_t sciError;
    
    DSECTENTRYPOINT;

    if (rs_initialized == TRUE) {
	/* finalize extensions for sci-descriptor management */
	/* smi_sci_desc_finalize(); */
	
	while(rs_node_pop(&rs_node_root, &Resource, &ResourceType) == 0) {
	    rs_FreeResource(Resource,ResourceType,&sciError);
	}
	
#ifndef DISABLE_THREADS
	SMI_DESTROY_LOCK(&rsMutex);
#endif
    }
    else
	DWARNING("resourcelist is not initialized; nothing done");
    
    DSECTLEAVE;
    return(0);
}

void rs_FreeResource(rs_resource_t Resource, rs_resource_type_t ResourceType, sci_error_t* error) 
{
  int loops;
  DSECTION("rs_FreeResource");

  /* DSECTENTRYPOINT; */

  switch(ResourceType) {
#ifndef NO_SISCI
  case rtSciDesc:
    DNOTICEP("Closing SciDesc",Resource.SciDesc);
    SCIClose(Resource.SciDesc, 0, error);
    break;
  case rtDmaQueue:
    DNOTICEP("Removing DMAQueue",Resource.DmaQueue);
#ifdef DOLPHIN_SISCI    
    SCIRemoveDMAQueue(Resource.DmaQueue, 0, error);
#else
    DPROBLEM ("DMA only supported by Dolphin's SISCI");
#endif
    break;
  case rtLocInterrupt:
    DNOTICEP("Removing local Interrupt",Resource.LocInterrupt);
    SCIRemoveInterrupt(Resource.LocInterrupt, 0, error);
    break;
  case rtRmtInterrupt:
    DNOTICEP("Disconnection from remote Interrupt",Resource.RmtInterrupt);
    SCIDisconnectInterrupt(Resource.RmtInterrupt, 0, error);
    break;
  case rtSequence:
    DNOTICEP("Removing Sequence",Resource.Sequence);
    SCIRemoveSequence(Resource.Sequence, 0, error);
    break;
  case rtSegment:
    /* XXX Problem: SetSegmentUnavailable takes the AdapterNo of the adapter which has 
       created the segment while RemoveSegment does not need it. We assume adapter 0 
       as creator because we do not have the information at this point */
    DNOTICEP("Setting Segment unavailable",Resource.Segment);
    SCISetSegmentUnavailable(Resource.Segment, 0, 0, error);
    DNOTICEP("Removing Segment",Resource.Segment);
    /* only wait for a limited time for the other processes to disconnect*/
    for (loops = 0; loops < SCI_REMOVE_TRIES; loops++) {
	SCIRemoveSegment(Resource.Segment, 0, error);
	if (*error == SCI_ERR_OK)
	    break;
	else
	    usleep (SCI_REMOVE_DELAY);
    }
    break;
  case rtMap:
      DNOTICEP("Unmapping Segment", Resource.Map);
      SCIUnmapSegment(Resource.Map, 0, error);
      break;
  case rtConnection:
    DNOTICEP("Disconnecting Segment",Resource.Connection);
    SCIDisconnectSegment(Resource.Connection, 0, error);
    break;
#endif
  case rtSharedMemory:
    DNOTICEI("Removing Shared Memory",Resource.ShmId);
    shmctl(Resource.ShmId, IPC_RMID, NULL);
    *error = SMI_SUCCESS;
    break;
  case rtTempFile:
    DNOTICES("Removing TempFile",Resource.TempFile.szName);
    close(Resource.TempFile.fd);
    remove(Resource.TempFile.szName);
    break;
  case rtTempStream:
      DNOTICES("Removing TempStream",Resource.TempStream.szName);
      /* re-open in case the user had closed it in the meantime */
      Resource.TempStream.fd = fopen (Resource.TempStream.szName, "r");
      if (Resource.TempStream.fd != NULL) {
	  fclose (Resource.TempStream.fd);
	  remove(Resource.TempStream.szName);
      } else
	  DWARNING("Couldn't open the stream to be removed");
      break;
#ifndef DISABLE_THREADS
  case rtThread:
      pthread_cancel (Resource.thread_id);
    break;
#endif
  case rtNone:
      DNOTICE("Freeing rtNone, this funktion is not implemented!");
      /* free(Resource.None);
       *error = SCI_ERR_OK; */
      *error = SMI_ERR_NOTIMPL; 
    break;
  default:
    DPROBLEM("illegal parameter!");
    *error = SMI_ERR_PARAM;
    break;
  }
  
  /* DSECTLEAVE; */
}

#ifndef NO_SISCI
void rs_SCIOpen(sci_desc_t   *sd,
		unsigned int flags,
		sci_error_t  *error)
{
  DSECTION("rs_SCIOpen");

  SCIOpen(sd,flags,error);
  if (*error == SCI_ERR_OK) {
    rs_node_push(&rs_node_root, sd, rtSciDesc); 
    DNOTICEP("Opened SciDesc",*sd);
  }
}

void rs_SCIClose(sci_desc_t sd,
                  unsigned int flags,
                  sci_error_t *error)
{ 
  DSECTION("rs_SCIClose");
  
  DNOTICEP("Closing SciDesc",sd);
  rs_node_remove(&rs_node_root, &sd, rtSciDesc);
  SCIClose(sd,flags,error);
}

void rs_SCICreateSegment(sci_desc_t             sd,
		      sci_local_segment_t    *segment,
		      unsigned int           segmentId,
		      size_t                 size,
		      sci_cb_local_segment_t callback,
		      void                   *callbackArg, 
		      unsigned int           flags,
		      sci_error_t            *error)
{
  DSECTION("rs_SCICreateSegment");

  SCICreateSegment(sd,segment,segmentId,(unsigned int)size,callback,callbackArg,flags,error);
  if (*error == SCI_ERR_OK) {
    rs_node_push(&rs_node_root, segment, rtSegment); 
    DNOTICEP("Created Segment",*segment);
  }
}



void rs_SCIConnectSegment( sci_desc_t sd,
			   sci_remote_segment_t* segment,
			   unsigned int nodeId,
			   unsigned int segmentId,
			   unsigned int localAdapterNo,
			   sci_cb_remote_segment_t callback,
			   void* callbackArg,
			   unsigned int timeout,
			   unsigned int flags,
			   sci_error_t* error)
{
  DSECTION("rs_SCIConnectSegment");

  SCIConnectSegment(sd,segment,nodeId,segmentId,localAdapterNo,callback,callbackArg,timeout,flags,error);
  if (*error == SCI_ERR_OK) {
      rs_node_push(&rs_node_root, segment, rtConnection); 
      DNOTICEP("Connected Segment",*segment);
  } else {
      DPROBLEMP("Failed to connect to remote segment, SISCI error", *error);
  }
}

volatile void *rs_SCIMapLocalSegment (sci_local_segment_t segment,
                             sci_map_t           *map,
                             size_t        offset,
                             size_t        size,
                             void                *addr,
                             unsigned int        flags,
                             sci_error_t         *error)
{
    volatile void *map_addr;
    DSECTION("rs_MapLocalSegment");
    
    map_addr = SCIMapLocalSegment (segment, map, (unsigned int)offset, (unsigned int)size, addr, flags, error);
    if (*error == SCI_ERR_OK) {
	rs_node_push(&rs_node_root, map, rtMap); 
	DNOTICEP("Mapped local segment", segment);
	DNOTICEP("Used sci_map_t structure", map);
    } else {
	DWARNING("Failed to map local segment");
    }
    return (map_addr);
}

volatile void *rs_SCIMapRemoteSegment (sci_remote_segment_t segment,
                              sci_map_t            *map,
                              size_t               offset,
                              size_t               size,
                              void                *addr,
                              unsigned int         flags,
                              sci_error_t          *error)
{
    volatile void *map_addr;
    DSECTION("rs_MapRemoteSegment");

    map_addr = SCIMapRemoteSegment (segment, map, (unsigned int)offset, (unsigned int)size, addr, flags, error);
    if (*error == SCI_ERR_OK) {
	rs_node_push(&rs_node_root, map, rtMap); 
	DNOTICEP("Mapped remote segment", segment);
	DNOTICEP("Used sci_map_t structure", map);
    } else {
	DWARNING("Failed to map remote segment");
    }
    return (map_addr);
}

void rs_SCIUnmapSegment(sci_map_t    map,
                         unsigned int flags,
                         sci_error_t  *error)
{
    DSECTION("rs_SCIUnmapSegment");
    
    DNOTICEP("Unmapping Segment, map structure is", map);
    
    rs_node_remove(&rs_node_root, &map, rtMap);
    SCIUnmapSegment (map, flags, error);
}


void rs_SCIDisconnectSegment( sci_remote_segment_t segment,
			      unsigned int flags,
			      sci_error_t* error)
{ 
  DSECTION("rs_SCIDisconnectSegment");
  
  DNOTICEP("Disconnecting Segment",segment);
  
  rs_node_remove(&rs_node_root, &segment, rtConnection);
  SCIDisconnectSegment(segment,flags,error);
}

void rs_SCIRemoveSegment(sci_local_segment_t segment,
                          unsigned int        flags, 
                          sci_error_t         *error)
{
  DSECTION("rs_SCIRemoveSegment");
  
  DNOTICEP("Removing Segment",segment);
  rs_node_remove(&rs_node_root, &segment, rtSegment);
  SCIRemoveSegment(segment,flags,error);
}

void rs_SCICreateMapSequence(sci_map_t   map, 
                           sci_sequence_t *sequence, 
                           unsigned int   flags, 
                           sci_error_t    *error)
{
  DSECTION("rs_SCICreateMapSequence");

  SCICreateMapSequence(map, sequence, flags, error);
  if (*error == SCI_ERR_OK) {
    rs_node_push(&rs_node_root, sequence, rtSequence);
    DNOTICEP("Created CheckSequence",*sequence);
  }
}

void rs_SCIRemoveSequence(sci_sequence_t sequence, 
                           unsigned int   flags, 
                           sci_error_t    *error)
{
  DSECTION("rs_SCIRemoveSequence");

  DNOTICEP("Removing Sequence",sequence);
  rs_node_remove(&rs_node_root, &sequence, rtSequence);
  SCIRemoveSequence(sequence,flags,error);
}

void rs_SCICreateDMAQueue(sci_desc_t      sd,
                           sci_dma_queue_t *dq,
                           unsigned int    localAdapterNo,
                           size_t          maxEntries,
                           unsigned int    flags,
                           sci_error_t     *error)
{
  REMDSECTION("rs_SCICreateDMAQueue");

#ifdef DOLPHIN_SISCI    
  SCICreateDMAQueue(sd, dq, localAdapterNo, (unsigned int)maxEntries, flags, error);
#else
  DPROBLEM ("DMA only supported by Dolphin's SISCI");
  *error = SCI_ERR_NOT_IMPLEMENTED;
#endif
  if (*error == SCI_ERR_OK) {
    rs_node_push(&rs_node_root, dq, rtDmaQueue);
    DNOTICEP("Created DmaQueue",*dq);
  }
}
  

void rs_SCIRemoveDMAQueue(sci_dma_queue_t dq,
			  unsigned int    flags,
			  sci_error_t     *error)
{
  REMDSECTION("rs_SCIRemoveDMAQueue");
 
#ifndef DOLPHIN_SISCI
  DPROBLEM ("DMA only supported by Dolphin's SISCI");
  *error = SCI_ERR_NOT_IMPLEMENTED;
#else
  DNOTICEP("Removing DmaQueue",dq);
  rs_node_remove(&rs_node_root, &dq, rtDmaQueue);
  SCIRemoveDMAQueue(dq,flags,error);
#endif
}

void rs_SCICreateInterrupt(sci_desc_t            sd,
                            sci_local_interrupt_t *interrupt,
                            unsigned int          localAdapterNo,
                            unsigned int          *interruptNo,
                            sci_cb_interrupt_t    callback,
                            void                  *callbackArg,
                            unsigned int          flags,
                            sci_error_t           *error)
{
  DSECTION("rs_SCICreateInterrupt");

  SCICreateInterrupt(sd, interrupt, localAdapterNo, interruptNo, callback, callbackArg, flags, error);
  if (*error == SCI_ERR_OK) {
    rs_node_push(&rs_node_root, interrupt, rtLocInterrupt);
    DNOTICEP("Created local Interrupt",*interrupt);
  }
}

void rs_SCIRemoveInterrupt(sci_local_interrupt_t interrupt,
                            unsigned int          flags,
                            sci_error_t           *error)
{
  DSECTION("rs_SCIRemoveInterrupt");

  DNOTICEP("Removing local Interrupt",interrupt);
  rs_node_remove(&rs_node_root, &interrupt, rtLocInterrupt);
  SCIRemoveInterrupt(interrupt,flags,error);
}

void rs_SCIConnectInterrupt(sci_desc_t            sd,
                            sci_remote_interrupt_t *interrupt,
			    unsigned int           nodeId,
			    unsigned int           localAdapterNo,
			    unsigned int           interruptNo,
			    unsigned int           timeout,
			    unsigned int           flags,
			    sci_error_t            *error)
{
  DSECTION("rs_SCIConnectInterrupt");

  SCIConnectInterrupt(sd, interrupt, nodeId, localAdapterNo, interruptNo, timeout, flags, error);
  if (*error == SCI_ERR_OK) {
      rs_node_push(&rs_node_root, interrupt, rtRmtInterrupt);
      DNOTICEP("Connected to remote Interrupt",*interrupt);
  }
}

void rs_SCIDisconnectInterrupt(sci_remote_interrupt_t interrupt,
			       unsigned int          flags,
			       sci_error_t           *error)
{
  DSECTION("rs_SCIDisconnectInterrupt");

  DNOTICEP("Disconnected from remote Interrupt",interrupt);
  rs_node_remove(&rs_node_root, &interrupt, rtRmtInterrupt);
  SCIDisconnectInterrupt(interrupt,flags,error);
}
#endif

int rs_shmget(key_t key, size_t size, int shmflg)
{
  int RetVal;
  DSECTION("rs_shmget");
  DSECTENTRYPOINT;
  
  DNOTICEI("desired memory size:", size);

  RetVal = shmget(key, (int)size, shmflg);
  if (RetVal != -1) {
    rs_node_push(&rs_node_root, &RetVal, rtSharedMemory);
    DNOTICEI("Got SharedMemory",RetVal);
  } 
  else {
      DPROBLEMI("shmget failed, errorcode:", errno);
  }

  DSECTLEAVE
  return(RetVal);
}

int rs_shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
  int RetVal;
  DSECTION("rs_shmctl");
  
  DSECTENTRYPOINT;

  if (cmd == IPC_RMID) {
    DNOTICEI("Removing SharedMemory",shmid);
    rs_node_remove(&rs_node_root, &shmid, rtSharedMemory); 
  }
  RetVal = shmctl(shmid, cmd, buf);
    
  DSECTLEAVE
    return(RetVal);
}

int rs_CreateTempfile(const char* szName, int oflag) 
{
  DSECTION("rs_CreateTempfile");
  int RetVal;
  rs_tempfile_t tfTemp;

  strcpy(tfTemp.szName, szName);
#ifdef WIN32
  RetVal=open(szName,oflag, S_IREAD | S_IWRITE);	
#else
  RetVal=open(szName,oflag, S_IRUSR | S_IWUSR);
#endif
  tfTemp.fd = RetVal;

  if (RetVal != -1)
    rs_node_push(&rs_node_root, &tfTemp, rtTempFile); 

  DNOTICES("Created Tempfile",tfTemp.szName);

  return(RetVal);
}

void rs_RemoveTempfile(char* szName)
{ 
  DSECTION("rs_RemoveTempfile");
  sci_error_t error;
  rs_resource_t tfTemp;

  strcpy(tfTemp.TempFile.szName,szName);

  DNOTICES("Removing Tempfile",tfTemp.TempFile.szName);
  
  rs_node_remove(&rs_node_root, &tfTemp, rtTempFile);
  rs_FreeResource(tfTemp, rtTempFile, &error);
}

FILE *rs_CreateTempstream(const char* szName, const char *mode)
{
  DSECTION("rs_CreateTempstream");
  FILE *RetVal;
  rs_tempstream_t tsTemp;

  strcpy(tsTemp.szName, szName);
  RetVal = fopen(szName, mode);
  tsTemp.fd = RetVal;

  if (RetVal != NULL) 
    rs_node_push(&rs_node_root, &tsTemp, rtTempStream); 

  DNOTICES("Created Tempstream",tsTemp.szName);

  return(RetVal);
}

void rs_RemoveTempstream(char* szName)
{ 
  DSECTION("rs_RemoveTempstream");
  sci_error_t error;
  rs_resource_t tsTemp;

  strcpy(tsTemp.TempFile.szName,szName);

  DNOTICES("Removing Tempstream",tsTemp.TempFile.szName);
  
  rs_node_remove(&rs_node_root, &tsTemp, rtTempStream);
  rs_FreeResource(tsTemp, rtTempStream, &error);
}

#ifndef DISABLE_THREADS

static void *rs_pthread_startup (void *arg)
{
    pthread_t myid;
    rs_thread_create_t *thread_info = (rs_thread_create_t *)arg;
    void *(*jump_to)(void *) = thread_info->start_routine;
    void *jump_arg = thread_info->arg;
    void *retval;

    free (arg);
    
    /* We required the threads to be async. cancable. */
    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    retval = jump_to(jump_arg);
    myid = pthread_self();
    rs_node_remove(&rs_node_root, &myid, rtThread); 

    return retval;
}

/* Create a thread and store it's thread-id in the resource list. */
int rs_pthread_create (pthread_t  *thread, pthread_attr_t *attr, 
		       void * (*start_routine)(void *), void *arg)
{
    rs_thread_create_t *thread_info;
    int retval; 

    ALLOCATE (thread_info, rs_thread_create_t *, sizeof(rs_thread_create_t));
    thread_info->start_routine = start_routine;
    thread_info->arg = arg;

    retval = pthread_create (thread, attr, rs_pthread_startup, thread_info);
    if (retval == 0) 
	rs_node_push(&rs_node_root, thread, rtThread); 
    
    return retval;
}


void rs_pthread_exit(void *retval)
{
    pthread_t myid = pthread_self();

    rs_node_remove(&rs_node_root, &myid, rtThread); 
    
    return;
}

int rs_pthread_cancel(pthread_t thread)
{
    int retval; 

    retval = pthread_cancel(thread);

    if (retval == 0) 
	rs_node_remove(&rs_node_root, &thread, rtThread); 
    
    return retval;
}

int rs_pthread_join(pthread_t thread, void **thread_return)
{
    int retval; 

    retval = pthread_join(thread, thread_return);

    if (retval == 0) 
	rs_node_remove(&rs_node_root, &thread, rtThread); 

    return retval;
}

#endif
