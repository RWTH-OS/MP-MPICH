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

#if !defined (linux)
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

#ifndef __cplusplus
#error "threads.cc: this file is C++."
#endif

//----------------------------------------------------------------------
#include "nt2unix.h"
#include "threads.h"
#include "debugnt2u.h"
//ThreadInfoMap ThreadInfos;
// We must protect the access to the ThreadInfoMap: 
//CriticalSection ThreadInfoLock;
#ifdef __cplusplus
extern "C" {
#endif

// Threading functions ---------------------------------------------------



#ifdef __POSIX_THREADS__


struct pthread_start_struct {
    solaris_PTHREAD_START_ROUTINE routine;
    void *arg;
    DWORD suspended;
};

void *posix_start_routine(pthread_start_struct * s) {
   void *erg; 
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
   if(s->suspended==CREATE_SUSPENDED) SuspendThread((HANDLE)pthread_self());
   erg=s->routine(s->arg);
   delete s;
   return erg;
}

#endif

WINBASEAPI
HANDLE
WINAPI
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
  ThreadInfoLock.enter();
  DWORD state = THREAD_RUNNING; // the new thread's default state.
  HandleInfo* ThreadStruct;
  ThreadStruct = (HandleInfo*) malloc(sizeof(HandleInfo));

#if defined (__POSIX_THREADS__) 
  pthread_attr_t attr;
  pthread_start_struct *start_struct;
#endif  
  if (lpThreadAttributes)
    DBG("CreateThread(): LPSECURITY_ATTRIBUTES not supported.");
#if defined (__POSIX_THREADS__) 
  pthread_attr_init(&attr);
  /* system-wide contention */
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  start_struct=new pthread_start_struct;
  start_struct->arg=lpParameter;
  start_struct->suspended=dwCreationFlags;
  start_struct->routine=(solaris_PTHREAD_START_ROUTINE)lpStartAddress;
  if (pthread_create((thread_t *)lpThreadId,&attr,
     (solaris_PTHREAD_START_ROUTINE) posix_start_routine,
     start_struct)) {
    return 0; 
  }
#else /* __POSIX_THREADS__*/
  if (thr_create(0, (size_t)dwStackSize,
                 (solaris_PTHREAD_START_ROUTINE) lpStartAddress,
                 lpParameter,
		 (long)dwCreationFlags | THR_BOUND | THR_NEW_LWP,
		 (thread_t *)lpThreadId)){
        perror ("Error thr_create");
	return 0;
     }
	

#endif
  if ((dwCreationFlags & CREATE_SUSPENDED) == CREATE_SUSPENDED) {
    DBG("Creating suspended thread");
    state = THREAD_SUSPENDED;
  }
  
  // Create a ThreadInfo struct for the new thread. 
  ThreadInfo thisThreadInfo(state);
#ifndef __POSIX_THREADS__
    if (state==THREAD_SUSPENDED) {
       thisThreadInfo.suspendCount=1;
       thisThreadInfo.threadHasBeenResumed = TRUE;
    }
#endif
  // add it to the global Map of ThreadInfos with the handle as key.
  DBG("CreateThread(): inserting handle "<< *lpThreadId);
  ThreadInfos.insert(ThreadInfoMap::value_type((HANDLE)*lpThreadId, thisThreadInfo));
  ThreadInfoLock.leave();
    
  // Under UNIX, we define the thread id as the handle.
  // This is incompatible to NT, but should work to some extent

  ThreadStruct->handleType = HANDLETYPE_THREAD;
  ThreadStruct->obj = (HANDLE)*lpThreadId;
  ThreadStruct->refcnt++;

  return (HANDLE)ThreadStruct;
}


WINBASEAPI
HANDLE
WINAPI
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
  DBG("CreateRemoteThread() not supported.");
  return 0;
}



WINBASEAPI
HANDLE
WINAPI
GetCurrentThread(
    VOID
)    
{
  HandleInfo* Thread;
  Thread = (HandleInfo*) malloc(sizeof(HandleInfo));
  Thread->refcnt++;

#ifdef __POSIX_THREADS__
  Thread->obj = (HANDLE)pthread_self();
  Thread->handleType = HANDLETYPE_THREAD;
  return (HANDLE)Thread;
#else
  Thread->obj = (HANDLE)thr_self();
  Thread->handleType = HANDLETYPE_THREAD;
  return (HANDLE)Thread;
#endif
}



WINBASEAPI
DWORD
WINAPI
GetCurrentThreadId(
    VOID
    )
{
  return (DWORD)thr_self();
}



WINBASEAPI
DWORD
WINAPI
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    )
{
  DBG("SetThreadAffinityMask() not supported."); 
  return 0;
}


WINBASEAPI
BOOL
WINAPI
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    )
{
  DBG("SetThreadPriority() not supported.");
  
  return 0;
}



WINBASEAPI
int
WINAPI
GetThreadPriority(
    HANDLE hThread
    )
{
  DBG("GetThreadPriority() not supported.");
  return 0;
}


WINBASEAPI
BOOL
WINAPI
GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    )
{
  DBG("GetThreadTimes() not supported.");
  return FALSE;
}


WINBASEAPI
VOID
WINAPI
ExitThread(
    DWORD dwExitCode
    )
{
  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo =
    ThreadInfos.find(GetCurrentThread());

  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    (*thisThreadInfo).second.exitCode = dwExitCode;
    (*thisThreadInfo).second.state = THREAD_TERMINATED;
    ThreadInfoLock.leave();
#ifdef __POSIX_THREADS__
    pthread_exit((void *)dwExitCode);
#else
    thr_exit((void *)dwExitCode);
#endif
  }
  ThreadInfoLock.leave();
  DBG("ExitThread() failed.");
}

VOID ExitProcess(UINT uExitCode)
{
  exit((int)uExitCode);
}


WINBASEAPI
BOOL
WINAPI
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    )
{

  HandleInfo* ThreadStruct;
  ThreadStruct = (HandleInfo*)hThread;

  if (!ThreadStruct->refcnt)
  {
    ThreadStruct->handleType = HANDLETYPE_THREAD;
    ThreadStruct->obj = hThread;
  }

  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo =
    ThreadInfos.find(ThreadStruct->obj);

  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    (*thisThreadInfo).second.exitCode = dwExitCode;
    (*thisThreadInfo).second.state = THREAD_TERMINATED;
    ThreadInfoLock.leave();
#ifdef __POSIX_THREADS__
    if (pthread_cancel((pthread_t)ThreadStruct->obj))
#else
    if (thr_kill((thread_t)ThreadStruct->obj, SIGKILL))
#endif
      // failed.
      ThreadStruct->refcnt--;
      if (ThreadStruct->refcnt == 0)
	free (ThreadStruct);
      return FALSE;
  }
  ThreadStruct->refcnt--;
  if (ThreadStruct->refcnt == 0)
    free (ThreadStruct);
  ThreadInfoLock.leave();  
  return FALSE; 
}



WINBASEAPI
BOOL
WINAPI
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    )
{
  HandleInfo* ThreadStruct;
  ThreadStruct = (HandleInfo*)hThread;

  if (!ThreadStruct->refcnt)
  {
    ThreadStruct->handleType = HANDLETYPE_THREAD;
    ThreadStruct->obj = hThread;
  }

  if (!lpExitCode) 
    return FALSE;

  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo =
    ThreadInfos.find(ThreadStruct->obj);

  
  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    if ((*thisThreadInfo).second.state == THREAD_TERMINATED)
      *lpExitCode = (*thisThreadInfo).second.exitCode;
    else
      *lpExitCode = STILL_ACTIVE;
    ThreadInfoLock.leave();
    return TRUE;
  }
  ThreadInfoLock.leave();
  return FALSE;
}



WINBASEAPI
BOOL
WINAPI
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    )
{
  DBG("GetThreadSelectorEntry not supported.");
  return FALSE;
}

DWORD ResumeThread(HANDLE hThread)
{
  HandleInfo* ThreadStruct;
  ThreadStruct = (HandleInfo*) hThread;

  /*if (!ThreadStruct->refcnt)
  {
    ThreadStruct->handleType = HANDLETYPE_THREAD;
    ThreadStruct->obj = hThread;
    ThreadStruct->refcnt++;
    }*/

  DWORD oldSuspendCount ;
  ThreadInfoLock.enter();
  DBG("Entering: ResumeThread, hThread ==" <<ThreadStruct->obj);
  ThreadInfoMap::iterator thisThreadInfo = ThreadInfos.find(ThreadStruct->obj);

  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
#ifdef __POSIX_THREADS__
    pthread_mutex_lock((*thisThreadInfo).second.mutex);
#endif
    oldSuspendCount = (*thisThreadInfo).second.suspendCount;
    if ((oldSuspendCount > 0)&&((*thisThreadInfo).second.state==THREAD_SUSPENDED)) {
      (*thisThreadInfo).second.suspendCount--;     
      if (oldSuspendCount < 2) {
        // oldSuspendCount == 1 -> new value is 0 -> really resume thread
//        (*thisThreadInfo).second.state = THREAD_RUNNING;
#ifdef __POSIX_THREADS__
   	  pthread_cond_signal((*thisThreadInfo).second.cond);
          pthread_cond_wait((*thisThreadInfo).second.cond,(*thisThreadInfo).second.mutex);
          DBG("Thread is now resumed...");
#else
        do {
          // Loop until the target thread is really resumed. 
          if (thr_continue((thread_t)ThreadStruct->obj)) {

            perror("thr_continue()");
            ThreadInfoLock.leave();
            return 0xFFFFFFFF;
          }
          // Give up the CPU so that the resumed thread has a chance to
          // update the associated threadHasBeenResumed flag.
          thr_yield(); 
        } while (!(*thisThreadInfo).second.threadHasBeenResumed); 
        (*thisThreadInfo).second.state = THREAD_RUNNING;
        DBG("ResumeThread(): thread really resumed, handle == "<<ThreadStruct->obj);
#endif
      } else {
        DBG("ResumeThread(): thread stays suspended, cnt =="<<oldSuspendCount-1);
      }  
    } else {
      DBG("ResumeThread(): thread is not suspended, oldSuspendCount == "<<oldSuspendCount);
      // oldSuspendCount == 0 -> thread is not currently suspended.
    }
#ifdef __POSIX_THREADS__
    pthread_mutex_unlock((*thisThreadInfo).second.mutex);
#endif
	 DBG("returning "<<oldSuspendCount<<std::flush);
    ThreadInfoLock.leave();
    return oldSuspendCount;
  }
  // thread not found.
  DBG("ResumeThread(): thread handle not found, hThread == " << ThreadStruct->obj); 
  ThreadInfoLock.leave();    
  return 0xFFFFFFFF;
}

DWORD SuspendThread(HANDLE hThread)
{
  HandleInfo* ThreadStruct;
  ThreadStruct = (HandleInfo*) hThread;
  
  /*if (!ThreadStruct->refcnt)
  {
    ThreadStruct->handleType = HANDLETYPE_THREAD;
    ThreadStruct->obj = hThread;
    ThreadStruct->refcnt++;
    }*/

  BOOL same = FALSE; 
  // this flag indicates whether a thread suspends itself.
  // If same == TRUE, we must avoid a "lost signal" problem, see below. 
  
  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo = ThreadInfos.find(ThreadStruct->obj);

  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    DWORD oldSuspendCount = (*thisThreadInfo).second.suspendCount;
    if (oldSuspendCount < MAXIMUM_SUSPEND_COUNT) 
      (*thisThreadInfo).second.suspendCount++;
    if (oldSuspendCount < 1) {
#ifndef __POSIX_THREADS__
      (*thisThreadInfo).second.state = THREAD_SUSPENDED;
#else
      pthread_mutex_t *mutex=(*thisThreadInfo).second.mutex;
      pthread_cond_t *cond=(*thisThreadInfo).second.cond;
      pthread_mutex_lock(mutex);
#endif
      if (same = (thr_self() == (thread_t)ThreadStruct->obj)) { 
        // if the thread suspends itself, we must release the lock
        // before actually calling thr_suspend(). Otherwise, the
        // thread would hold the mutex while being suspended, resulting
        // in a deadlock. 
#ifdef __POSIX_THREADS__    
        (*thisThreadInfo).second.state = THREAD_SUSPENDED;
        ThreadInfoLock.leave();
        pthread_cond_wait(cond,mutex);
        (*thisThreadInfo).second.state=THREAD_RUNNING;
        pthread_cond_signal(cond);
        pthread_mutex_unlock(mutex);
        return 0;
        
      // DANGER!!! If at this point, another thread 
      // is scheduled in ResumeThread(), the resume "signal" may get lost.
      // To avoid this, ResumeThread() polls until the thread is
      // really resumed, i.e. until threadHasBeenResumed == TRUE.  
      } else 
      if(pthread_kill(pthread_t(ThreadStruct->obj),SIGUNUSED)) {
#else
      	DBG("Suspending thread "<<ThreadStruct->obj<<flush);		
			(*thisThreadInfo).second.threadHasBeenResumed = FALSE; 
        	ThreadInfoLock.leave();      
      }

//      (*thisThreadInfo).second.state = THREAD_SUSPENDED;
      if (thr_suspend((thread_t)ThreadStruct->obj)) {
#endif
        perror("thr_suspend()");
        return 0xFFFFFFFF;
      }
#if defined (__POSIX_THREADS__) 
      else { 
         pthread_cond_wait(cond,mutex);
      }
      pthread_mutex_unlock(mutex);
#else
      (*thisThreadInfo).second.threadHasBeenResumed = TRUE; 
#endif
    if (!same)
        ThreadInfoLock.leave(); 
    } else {
      DBG("SuspendThread(): thread already sleeping.");    
      ThreadInfoLock.leave();
    }
    
    return oldSuspendCount;
  }

  // Thread not found.
  DBG("SuspendThread(): Thread handle not found, hThread == "<<ThreadStruct->obj);

  ThreadInfoLock.leave();
  return 0xFFFFFFFF; 

}

#ifdef __POSIX_THREADS__

#define thr_keycreate pthread_key_create

#define thr_setspecific pthread_setspecific
#endif

WINBASEAPI
DWORD
WINAPI
TlsAlloc(
    VOID
    )
{
  DWORD key; 
  
  if(thr_keycreate( (thread_key_t *)&key, 0)) {
    // failed.
    perror("TlsAlloc(): thr_keycreate()");
    return TLS_OUT_OF_INDEXES;
  }
  
  return key;
}


WINBASEAPI
LPVOID
WINAPI
TlsGetValue(
    DWORD dwTlsIndex
    )
{
  LPVOID value; 
#ifdef __POSIX_THREADS__
  if ((value=pthread_getspecific((thread_key_t)dwTlsIndex))==0) {
#else
  if (thr_getspecific((thread_key_t)dwTlsIndex, &value)) {
#endif
    // failed.
    perror("TlsGetValue(): thr_getspecific()");
    return 0;
  }
  return value;
}


WINBASEAPI
BOOL
WINAPI
TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    )
{
  if (thr_setspecific((thread_key_t)dwTlsIndex, lpTlsValue)) {
    // failed.
    perror("TlsSetValue(): thr_setspecific()");
    return FALSE; 
  }
  return TRUE;
}


WINBASEAPI
BOOL
WINAPI
TlsFree(
    DWORD dwTlsIndex
    )
{
  // Solaris doesnot support key delete functions!
  // POSIX defines pthread_key_delete(), however. 
 
#if defined(__POSIX_THREADS__)
  pthread_key_delete((pthread_key_t)dwTlsIndex);
#else
  DBG("TlsFree() not supported.");  
#endif
  return TRUE;
}

#ifdef __cplusplus 
}
#endif

