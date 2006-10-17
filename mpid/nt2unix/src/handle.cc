#if !defined (linux)
#include <synch.h>

#endif

#include <pthread.h>
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
#include <string.h>

#include <vector>
#include <map>
#include <iostream>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

//----------------------------------------------------
#define _DEBUG_EXTERN_REC
#include "mydebug.h"
#include "nt2unix.h"
#include "filemap.h"
#include "debugnt2u.h"

using namespace std;

#ifdef linux
typedef long long  hrtime_t;
#endif
 

/* not used yet
typedef struct HandleInfo {
  DWORD handleType; 	// handletype of this handle (see below)
  void *obj; 		// pointer to handled object
  DWORD refcnt; 
};


// not used yet
#define HANDLETYPE_CONSOLE	0	// not supported
#define HANDLETYPE_EVENTFILE	1	// not supported
#define HANDLETYPE_FILEMAPPING	2
#define HANDLETYPE_MUTEX	3
#define HANDLETYPE_NAMEDPIPE	4
#define HANDLETYPE_PROCESS	5
#define HANDLETYPE_SEMAPHORE	6
#define HANDLETYPE_THREAD	7
#define HANDLETYPE_TOKEN	8 */

#ifdef __POSIX_THREADS__
#define MUTEX_LOCK(mtx) pthread_mutex_lock((pthread_mutex_t*) mtx)
#define THREAD_JOIN(thrd) pthread_join((pthread_t) thrd, 0)
#else
#define MUTEX_LOCK(mtx) mutex_lock((mutex_t*) mtx)
#define THREAD_JOIN(thrd) thr_join((thread_t) thrd, 0, 0) 
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void DeleteFilemapping(std::vector<FileMapping>::iterator &i);

WINBASEAPI
BOOL
WINAPI
CloseHandle(HANDLE hObject)
{
    DSECTION("CloseHandle");

    DSECTENTRYPOINT;
    
    if (!hObject) {
	DSECTLEAVE
	    return 0;
    }
    HandleInfo* Object;
    Object = (HandleInfo*)hObject;
    std::vector<FileMapping>::iterator i=FileMappings.begin();
    Object->refcnt--;
    
    switch (Object->handleType)
    {
    case HANDLETYPE_FILEMAPPING:
	while(i != FileMappings.end() && i->hFileMappingObject != Object->obj)
	    i++;
	if (i != FileMappings.end() && !i->Closed) i->Closed = TRUE;
	else
	    return FALSE;
     DeleteFilemapping(i);
     break;
     
    case HANDLETYPE_MUTEX:
#ifdef __POSIX_THREADS__
	if (Object->refcnt <= 0)
	{
	    pthread_mutex_destroy ((pthread_mutex_t*)Object->obj);
	    free ((pthread_mutex_t*)Object->obj);
	}
#else
	if (Object->refcnt <= 0)
	{
	    mutex_destroy((mutex_t*)Object->obj);
	    free ((mutex_t*)Object->obj);
	}
#endif
	break;
	
    case HANDLETYPE_SEMAPHORE:
	if (Object->refcnt <= 0)
#ifdef __POSIX_THREADS__
	{
	    Event_Struct_Typ* Sema = (Event_Struct_Typ*) Object->obj;
	    pthread_cond_destroy (&Sema->condition);
	    pthread_mutex_destroy (&Sema->mutex);
	    free (Sema);
	}
#else
	{
	    sema_destroy ((sema_t*)Object->obj);
	    free ((sema_t*)Object->obj);
	}
#endif
	break;
	
    case HANDLETYPE_EVENTFILE:
	Event_Struct_Typ* Event_Typ;
	Event_Typ = (Event_Struct_Typ*)Object->obj;
	if (Object->refcnt <= 0)
	{
#ifdef __POSIX_THREADS__
	    pthread_cond_destroy (&Event_Typ->condition);
	    pthread_mutex_destroy (&Event_Typ->mutex);
#else
	    cond_destroy (&Event_Typ->condition);
	    mutex_destroy (&Event_Typ->mutex);
#endif
	    free (Object->obj);
	}
	break;
	
	/*  case HANDLETYPE_THREAD:
	    if (Object->refcnt <= 0)
	    if (pthread_cancel ((pthread_t)Object->obj)){
	    perror ("CloseHandle : pthread_cancel");
	    return 0;
	    } 
	    break;*/
    }
    
    free (Object);
 
    DSECTLEAVE
	return TRUE;
}
    
    
WINBASEAPI
BOOL
WINAPI
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    )
{
    DSECTION("DuplicateHandle");

    DSECTENTRYPOINT;
    
    HandleInfo* SourceInfo;
    HandleInfo* TargetInfo;
    
    // we ignore handles. Just copy the "handle".
    // We should use dup() or dup2() for file handles, however. 
    //DBG("DuplicateHandle() not supported. Use dup(2)/dup2(2) if appropriate.");
    
    DBG("Src is: "<<hSourceHandle<<" Taget is: "<<lpTargetHandle<<endl);
    SourceInfo = (HandleInfo*) hSourceHandle;
    TargetInfo = SourceInfo;
    
    TargetInfo->refcnt++;
    
    *lpTargetHandle = (HANDLE)TargetInfo;
    
    DSECTLEAVE
	return TRUE;
}
    
    
WINBASEAPI
BOOL
WINAPI
GetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
    )
{
    // we ignore handles. 
    DBG("GetHandleInformation() not supported.");
    return FALSE;
}

timespec TimeAdder (DWORD MilliSeconds)
{
    DSECTION("TimeAdder");
    timespec NanoTime;
    timeval MikroTime;
    DWORD AbsMikros;
    
    DSECTENTRYPOINT;
    
    gettimeofday (&MikroTime, NULL);
    
    AbsMikros = ((MilliSeconds % 1000) * 1000) + MikroTime.tv_usec;
    
    if (AbsMikros >= 1000000 )
    {
	MikroTime.tv_sec++;
	NanoTime.tv_sec = MikroTime.tv_sec + (MilliSeconds / 1000);
	NanoTime.tv_nsec = (AbsMikros - 1000000) * 1000;
    } 
    else
    {
	NanoTime.tv_nsec = AbsMikros * 1000;
	NanoTime.tv_sec = MikroTime.tv_sec + (MilliSeconds / 1000);
    }

    DSECTLEAVE
	return NanoTime;
}

DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD dwMilliseconds)
{
    DSECTION("WaitForSingleObject");
    HandleInfo* Handle;
    Handle = (HandleInfo*) hHandle;
    Event_Struct_Typ *EventStruct;
    DWORD ret = 0;
    
    DSECTENTRYPOINT;
    
    if (!hHandle) {
	DSECTLEAVE
	    return WAIT_FAILED;
    }
    switch (Handle->handleType) {
    case HANDLETYPE_MUTEX:
     	DBG("Calling mutex_lock");
	if(MUTEX_LOCK(Handle->obj)){
	    DBG("mutex_lock failed");
	    DSECTLEAVE
		return WAIT_FAILED;
	}
	DSECTLEAVE
	    return WAIT_OBJECT_0;
	break;
	
    case HANDLETYPE_THREAD:
	DBG("Calling thr_join");
	if(THREAD_JOIN(Handle->obj)) {
	    DBG("WaitForSingleObject(): thr_join() failed\n");
	    DSECTLEAVE
		return WAIT_FAILED;
	}
	DBG("Back from join");
	DSECTLEAVE
	    return WAIT_OBJECT_0;
	break;
	
    case HANDLETYPE_SEMAPHORE: 
#ifdef __POSIX_THREADS__
	{
	Event_Struct_Typ* Semaphore = (Event_Struct_Typ*)Handle->obj;

        pthread_mutex_lock (&(Semaphore->mutex));

        if((signed int)dwMilliseconds != 0)
	{
          while (Semaphore->count <= 0) {
	    if (pthread_cond_wait (&(Semaphore->condition), &(Semaphore->mutex)))  {
		DBG ("pthread_cond_wait FAILED !!!");
		DSECTLEAVE
		    return WAIT_FAILED;
	    }
	  }
        }

        if(Semaphore->count == 0) ret=WAIT_TIMEOUT;
        else      
        {
           ret= WAIT_OBJECT_0;
          (Semaphore->count)--;
        }
	
	pthread_mutex_unlock (&(Semaphore->mutex));  
       }
       DSECTLEAVE
	    return ret;
	
	break;
#else
	if((signed int)dwMilliseconds != 0)
	{
	  DBG("Calling sema_wait");
	  if(sema_wait((sema_t*)Handle->obj)){
	    DBG("sema_wait failed");
	    DSECTLEAVE
	      return WAIT_FAILED;
	  }
	  DSECTLEAVE
	    return WAIT_OBJECT_0;
	}
	else
	{
	  DBG("Calling sema_trywait");
	  if(sema_trywait((sema_t*)Handle->obj)){
	    DBG("sema_trywait failed");
	    DSECTLEAVE
	      return WAIT_TIMEOUT;
	  }
	  DSECTLEAVE
	    return WAIT_OBJECT_0;
	}
	
	break;
#endif	

    case HANDLETYPE_EVENTFILE:
	EventStruct = (Event_Struct_Typ*) Handle->obj;
	if (EventStruct->Manual)
	    EventStruct->count++;
	timespec Time;
	hrtime_t Start, End;
	
	
	if (EventStruct->State == 1)
	{
	    if (!EventStruct->Manual)
		EventStruct->State = 0;
	    DSECTLEAVE
		return WAIT_OBJECT_0;
	}
	
#ifdef __POSIX_THREADS__	    
	if (dwMilliseconds != INFINITE)
	{
	    if (pthread_mutex_lock (&EventStruct->mutex))
		DBG ("pthread_mutex_lock FAILED");
	    
	    DBG ("Calling pthread_cond_timedwait");
	    Time = TimeAdder(dwMilliseconds);
	    ret = pthread_cond_timedwait (&EventStruct->condition, &EventStruct->mutex, &Time);
	    if (ret == EFAULT || ret == EINVAL)
	    {
		DBG("pthread_cond_timedwait returned Error !");
		pthread_mutex_unlock (&EventStruct->mutex);
		DSECTLEAVE
		    return WAIT_FAILED;
	    }
	    
	    pthread_mutex_unlock (&EventStruct->mutex);
	}
	else
	{
	    while (EventStruct->State == 0)
	    {
		if (pthread_mutex_lock (&EventStruct->mutex))
		    DBG ("pthread_mutex_lock FAILED");
		DBG("Calling pthread_cond_wait");
		if (pthread_cond_wait(&EventStruct->condition, &EventStruct->mutex))
		{
		    DBG ("pthread_cond_wait failed");
		    pthread_mutex_unlock (&EventStruct->mutex);
		    DSECTLEAVE
			return WAIT_FAILED;
		}
		pthread_mutex_unlock (&EventStruct->mutex);
	    }
	}
	if (ret == ETIMEDOUT)
	{
	    EventStruct->count--;
	    DSECTLEAVE
		return WAIT_TIMEOUT;
	}
#else
	if (dwMilliseconds != INFINITE)
	{
	    if (mutex_lock (&EventStruct->mutex))
		cout << "mutex_lock failed !" << endl;
	    
	    Time = TimeAdder(dwMilliseconds);
	    Start = gethrtime();
	    ret = cond_timedwait (&EventStruct->condition, &EventStruct->mutex, &Time);
	    End = gethrtime();
	    cout << "cond_timedwait needs " << (End - Start) << " nSecs !" << endl;
	    
	    if (ret == EFAULT || ret == EINVAL)
	    {
		cout << "cond_timedwait returned Error !" << endl;
		mutex_unlock (&EventStruct->mutex);
		DSECTLEAVE
		    return WAIT_FAILED;
	    }
	    mutex_unlock (&EventStruct->mutex);
	}
	else
	{
	    while (EventStruct->State == 0)
	    {
		if(mutex_lock (&EventStruct->mutex))
		    DBG ("mutex_lock FAILED");
		ret = cond_wait(&EventStruct->condition, &EventStruct->mutex);
		if (ret)
		{
		    DBG ("cond_wait failed");
		    mutex_unlock (&EventStruct->mutex);
		    DSECTLEAVE
			return WAIT_FAILED;
		}
		mutex_unlock (&EventStruct->mutex);
	    }
	}
	
	if (ret == ETIME)
	{
	    EventStruct->count--;
	    DSECTLEAVE
		return WAIT_TIMEOUT;
	}
#endif
	if (!EventStruct->Manual)
	    EventStruct->State = 0;
	if (ret == 0)
	{
	    EventStruct->count--;
	    DSECTLEAVE
		return WAIT_OBJECT_0;
	}
	DSECTLEAVE
	    return WAIT_FAILED;
	break;
	
    default:
	DSECTLEAVE
	    return WAIT_FAILED;
	break;
    }
}


unsigned int NextIndex (unsigned int index, DWORD nCount)
{
		if (index < (nCount - 1))
		    index++;
		else
		    index = 0;
		
		return index;
	    }
	
DWORD WaitForMultipleObjects(DWORD nCount, 
			     CONST HANDLE *lpHandles, 
			     BOOL bWaitAll,
			     DWORD dwMilliseconds)
{
  unsigned int index = 0, WaitCounter = 0, ObjectsSet = 0, Count = 0;

//  printf ("Entering WaitForMultipleObjects !!!\n");

  if (!bWaitAll)                                  // *** Waiting for ONE Object ***
    {
      if (dwMilliseconds == INFINITE)
	while (WaitForSingleObject (lpHandles[index], 50) == WAIT_TIMEOUT)
	  index = NextIndex(index, nCount);
      else
	while (WaitForSingleObject (lpHandles[index], 50) == WAIT_TIMEOUT)
	  {
	    index = NextIndex(index, nCount);
	    if (WaitCounter < dwMilliseconds)
	      WaitCounter += 50;
	    else
	      break;
	  }
      if (WaitCounter >= dwMilliseconds)
	return WAIT_TIMEOUT;
      else
	return WAIT_OBJECT_0;
    }
  else                                            // *** Waiting for ALL Objects ***
    {
      DBG ("Waiting for ALL Events...\n");

      HandleInfo* Copies[nCount];
      DWORD ret;
      for (unsigned int i = 0; i < nCount; i++)
	Copies[i] = new HandleInfo;
      memcpy (Copies, lpHandles, nCount * sizeof (HandleInfo));
      Count = nCount;
             
      while (ObjectsSet < Count)
	{
	  ret = WaitForSingleObject (Copies[index], 50); // EVENT NOT SET => Increment ArrayIndex !
	  switch (ret)
	    {
	    case WAIT_TIMEOUT:
	      index = NextIndex(index, nCount);
	      break;
	      
	    case WAIT_OBJECT_0: 
	      if (index != (nCount - 1))
		Copies[index] = Copies[nCount - 1]; // OverWrite Object, that was SET with last Object of Array
	      ObjectsSet++;
	      nCount--;
	      index = NextIndex(index, nCount);
	      break;
	      
	    case WAIT_FAILED:
	      return WAIT_FAILED;
	    }

	  if (dwMilliseconds != INFINITE)
	    {
	      if (WaitCounter < dwMilliseconds)
		WaitCounter += 50;
	      else
		return WAIT_TIMEOUT;
	    }
	} 
      for (unsigned int i = 0; i < nCount; i++)
	free (Copies[i]);
      
      return WAIT_OBJECT_0;
    }
		
}

#ifdef __cplusplus 
}
#endif
