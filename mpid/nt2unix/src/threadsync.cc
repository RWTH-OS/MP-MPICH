/* $Id$ */

#if !defined (linux)
#include <synch.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if !defined(linux)
#include <sys/systeminfo.h>
#include <stropts.h>
#include <poll.h>
#include <sys/mman.h>
#else
#include <sys/ioctl.h>
#include <fstream>
#endif

#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

//-----------------------------------------------------------------------------------------
#include "nt2unix.h"
#include "threadsync.h"
#include "threads.h"
#include "debugnt2u.h"

using namespace std;
  
#ifdef __cplusplus
extern "C" {
#endif

// Thread-Synchronization Functions -----------------------------------------

/*
#ifdef DBG
#undef DBG
#endif
#define DBG(x) {printf(x); printf("\n"); fflush(stdout);}

#ifdef DNOTICE
#undef DNOTICE
#endif
#define DNOTICE(x) {printf(x); printf("\n"); fflush(stdout);}

#ifdef DNOTICEI
#undef DNOTICEI
#endif
#define DNOTICEI(x,i) {printf(x); printf(": %d\n",i); fflush(stdout);}
*/

WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  assert(lpCriticalSection); 
  
  lpCriticalSection->LockCount = 0;
  lpCriticalSection->RecursionCount = 0;
  lpCriticalSection->OwningThread = (HANDLE)0xFFFFFFFF;

  lpCriticalSection->LockSemaphore = malloc(sizeof(mutex_t));   
  if (!lpCriticalSection->LockSemaphore)
    perror("InitializeCriticalSection(): malloc()");
#ifdef __POSIX_THREADS__
  if (pthread_mutex_init((mutex_t *)(lpCriticalSection->LockSemaphore),0))
    perror("InitializeCriticalSection() : pthread_mutex_init()");
#else  
  if (mutex_init((mutex_t *)(lpCriticalSection->LockSemaphore), USYNC_THREAD, 0))
    perror("InitializeCriticalSection() : mutex_init()"); 
#endif                    
  return;
}
 

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  assert(lpCriticalSection); 

  mutex_destroy((mutex_t *)(lpCriticalSection->LockSemaphore));
  free(lpCriticalSection->LockSemaphore);
  lpCriticalSection->LockSemaphore = 0; 
  
  return; 
}

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  thread_t me; 
  me = thr_self(); 
  assert(lpCriticalSection);
  if(mutex_trylock((mutex_t *)(lpCriticalSection->LockSemaphore))) {
      // The mutex is not free...
		if (lpCriticalSection->OwningThread == (HANDLE)me) {
    		// I have the lock. This cannot be a race condition. 
    		lpCriticalSection->RecursionCount++; 
    		DBGS("New Recursion count of mutex is: "<<lpCriticalSection->RecursionCount++);
    		return; 
  		} 
  		DBGS("Trying to lock, LockSema =="<<lpCriticalSection->LockSemaphore);
  		if(mutex_lock((mutex_t *)(lpCriticalSection->LockSemaphore)))
    		cerr<<"EnterCriticalSection(): mutex_lock() failed, errno == "<<GetLastError()<<endl<<flush;
	}    		
  // got it. I must be the first thread: 
  if (lpCriticalSection->RecursionCount) {
    cerr<<"EnterCriticalSection(): RecursionCount != 0"<<endl<<flush; 
    return; 
  }
  lpCriticalSection->RecursionCount = 1;   
  lpCriticalSection->OwningThread = (HANDLE)me; 
  DBGS("Mutex Acquired, Owner == "<<me<<" LockSema == "<< lpCriticalSection->LockSemaphore); 
  return;
}


WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  thread_t me; 
  
  assert(lpCriticalSection);
  
  DBGS("Trying to unlock, LockSema =="<<lpCriticalSection->LockSemaphore);
  me = thr_self(); 
  if (lpCriticalSection->OwningThread == (HANDLE)me) {
    lpCriticalSection->RecursionCount--; 
    if (lpCriticalSection->RecursionCount < 1) {
      lpCriticalSection->OwningThread = (HANDLE)0xFFFFFFFF;
      DBGS("Mutex release, me == "<<me<<" LockSema == "<<lpCriticalSection->LockSemaphore);
      if(mutex_unlock((mutex_t *)lpCriticalSection->LockSemaphore))
        cerr<<"LeaveCriticalSection(): mutex_unlock() failed, errno == "<<GetLastError()<<endl<<flush; 
    }
  } else {
    cerr<<"LeaveCriticalSection(): not lock owner, me == "<<me<<" owner == "<<lpCriticalSection->OwningThread<<" LockSema =="<<lpCriticalSection->LockSemaphore<<endl<<flush; 
  }

  return;
}


WINBASEAPI
HANDLE
WINAPI
CreateMutex(LPSECURITY_ATTRIBUTES lpMutex_Attributes,
	    BOOL bInitialOwner,
	    LPCTSTR lpName)
{
  HandleInfo *MutexInfo = (HandleInfo*)malloc(sizeof(HandleInfo));

	MutexInfo->refcnt = 0;
#ifdef __POSIX_THREADS__
  pthread_mutex_t* mutex_pointer = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  if(pthread_mutex_init (mutex_pointer, NULL))
    DBG ("pthread_mutex_init FAILED !");
  if (bInitialOwner)
    if (pthread_mutex_lock (mutex_pointer))
      DBG ("pthread_mutex_lock failed");
#else
  mutex_t* mutex_pointer = (mutex_t*)malloc(sizeof(mutex_t));
  if (mutex_init (mutex_pointer, 0, NULL))
    DBG ("mutex_init FAILED !");
  if (bInitialOwner)
    if (mutex_lock (mutex_pointer))
      DBG ("mutex_lock failed");
#endif

  MutexInfo->obj = (HANDLE)mutex_pointer;
  MutexInfo->refcnt++;
  MutexInfo->handleType = HANDLETYPE_MUTEX;

  return (HANDLE)MutexInfo;
}


WINBASEAPI
BOOL
WINAPI
ReleaseMutex(HANDLE hMutex)
{
  if (!hMutex){
    DBG("No HANDLE !!!");
    return 0;
  }
  HandleInfo *MutexInfo = (HandleInfo*)hMutex;
#ifdef __POSIX_THREADS__
  if (!pthread_mutex_unlock ((pthread_mutex_t*) MutexInfo->obj))
#else
  DBG("Calling mutex_unlock...");
  if (!mutex_unlock ((mutex_t*)MutexInfo->obj))
#endif
    return 1;
  else
    return 0;
}


WINBASEAPI
HANDLE
WINAPI
CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, 
	    BOOL bManualReset, 
	    BOOL bInitialState,
	    LPCTSTR lpName)
{
  HandleInfo *EventInfo = (HandleInfo*)malloc(sizeof(HandleInfo));

  Event_Struct_Typ *Event_Struct = (Event_Struct_Typ*)malloc(sizeof(Event_Struct_Typ));
    
#ifdef __POSIX_THREADS__
  if (pthread_cond_init(&Event_Struct->condition, NULL))
    {
      DBG ("pthread_cond_init FAILED !");
      return WSA_INVALID_EVENT;
    }
  if (pthread_mutex_init(&Event_Struct->mutex, NULL))
    {
      DBG ("pthread_mutex_init FAILED !");
      return WSA_INVALID_EVENT;
    }
  if (bInitialState)
  {
    pthread_mutex_lock (&Event_Struct->mutex);
    Event_Struct->State = 1;
    DBG ("calling pthread_cond_signal");
    pthread_cond_signal (&Event_Struct->condition);
    pthread_mutex_unlock (&Event_Struct->mutex);
  }
#else
  if (cond_init(&Event_Struct->condition, 0, NULL))
    {
      DBG ("cond_init FAILED !");
      return WSA_INVALID_EVENT;
    }      
  if (mutex_init(&Event_Struct->mutex, 0, NULL))
    {
      DBG ("mutex_init FAILED !");
      return WSA_INVALID_EVENT;
    }
  if (bInitialState)
  {
    mutex_lock (&Event_Struct->mutex);
    Event_Struct->State = 1;
    cond_signal (&Event_Struct->condition);
    mutex_unlock (&Event_Struct->mutex);
  }
#endif
  Event_Struct->Manual = bManualReset;
  
  EventInfo->handleType = HANDLETYPE_EVENTFILE;
  EventInfo->obj        = (HANDLE)Event_Struct;
  EventInfo->refcnt++;

  return (HANDLE)EventInfo;
}


WINBASEAPI
BOOL
WINAPI
SetEvent (HANDLE hEvent)
{
  HandleInfo *EventInfo = (HandleInfo*)hEvent;
  Event_Struct_Typ *EventStruct = (Event_Struct_Typ*) EventInfo->obj;

#ifdef __POSIX_THREADS__

  if (pthread_mutex_lock (&EventStruct->mutex))
  {
    DBG ("pthread_mutex_lock FAILED");
    return 0;
  }
  EventStruct->State = 1;

  if (!EventStruct->Manual)
  {
    DBG ("calling pthread_cond_signal");
    if (pthread_cond_signal (&EventStruct->condition))
    {
      DBG ("pthread_cond_signal FAILED");
      pthread_mutex_unlock (&EventStruct->mutex);
      return 0;
    }
  }
  else
  {
    while (EventStruct->count > 0)
    {
      DBG ("calling pthread_cond_signal");
      if (pthread_cond_signal (&EventStruct->condition))
      {
        DBG ("pthread_cond_signal FAILED");
        pthread_mutex_unlock (&EventStruct->mutex);
	return 0;
      }
    }
  }
  pthread_mutex_unlock (&EventStruct->mutex);
#else
  if (mutex_lock (&EventStruct->mutex))
  {
    DBG ("mutex_lock FAILED");
    return 0;
  }
  EventStruct->State = 1;
  mutex_unlock (&EventStruct->mutex);
  if (!EventStruct->Manual)
  {
    if (cond_signal (&EventStruct->condition))
    {
      DBG ("cond_signal FAILED");
      return 0;
    }
  }
  else
  {
    while (EventStruct->count > 0)
    {
      if (cond_signal (&EventStruct->condition))
      {
        DBG ("cond_signal FAILED");
        return 0;
      }
    }
  }
#endif

  return 1;
}


WINBASEAPI
BOOL
ResetEvent (HANDLE hEvent)
{
  HandleInfo *EventInfo = (HandleInfo*)hEvent;
  Event_Struct_Typ *EventStruct = (Event_Struct_Typ*) EventInfo->obj;

#ifdef __POSIX_THREADS__
  if (pthread_mutex_lock (&EventStruct->mutex))
  {
    DBG ("pthread_mutex_lock FAILED");
    return 0;
  }
  if (EventStruct->Manual)
  {
    EventStruct->State = 0;
    EventStruct->count = 0;
  }
  pthread_mutex_unlock (&EventStruct->mutex);
#else
  if (mutex_lock (&EventStruct->mutex))
  {
    DBG ("mutex_lock FAILED");
    return 0;
  }
  if (EventStruct->Manual)
  {
    EventStruct->State = 0;
    EventStruct->count = 0;
  }
  mutex_unlock (&EventStruct->mutex);
#endif

  return 1;
}


WINBASEAPI
HANDLE
CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCTSTR lpName)
{
  HandleInfo* SemaInfo = (HandleInfo*)malloc(sizeof(HandleInfo));
#ifdef __POSIX_THREADS__
  Event_Struct_Typ* Semaphore_Pointer = (Event_Struct_Typ*)malloc(sizeof(Event_Struct_Typ));
  Semaphore_Pointer->count = lInitialCount;
  pthread_mutex_init (&(Semaphore_Pointer->mutex), NULL /* default*/);
  pthread_cond_init (&(Semaphore_Pointer->condition), NULL /* default*/);
#else
  sema_t* Semaphore_Pointer = (sema_t*)malloc(sizeof(sema_t));
  if (sema_init (Semaphore_Pointer, lInitialCount, 0, NULL))
  {	
    DBG ("sema_init failed");
    return 0;
  }
#endif
   SemaInfo->handleType = HANDLETYPE_SEMAPHORE;
   SemaInfo->obj = (HANDLE)Semaphore_Pointer;
   SemaInfo->refcnt++;

   return (HANDLE)SemaInfo;
}


WINBASEAPI
BOOL
ReleaseSemaphore(HANDLE hSemaphore, 
		 LONG lReleaseCount, 
		 LPLONG lpPreviousCount)
{
  if (lReleaseCount > 1)
  {
    DBG("ReleaseCount > 1 NOT supported !");
    return FALSE;
  }
  HandleInfo *SemaInfo = (HandleInfo*)hSemaphore;
#ifdef __POSIX_THREADS__
  Event_Struct_Typ* Semaphore = (Event_Struct_Typ*)SemaInfo->obj;

  pthread_mutex_lock (&(Semaphore->mutex));

  (Semaphore->count)++;

  if (pthread_mutex_unlock(&(Semaphore->mutex)))
  {
    DBG ("pthread_mutex_unlock FAILED !!!");
    return FALSE;
  }
  if (pthread_cond_signal (&(Semaphore->condition)))
  {
    DBG("pthread_cond_signal FAILED !!!");
    return FALSE;
  }
#else
  if (sema_post ((sema_t*)SemaInfo->obj)){
    DBG("sema_post failed");
    return FALSE;
  }
#endif
  return TRUE;
}


#ifdef __cplusplus 
}
#endif

