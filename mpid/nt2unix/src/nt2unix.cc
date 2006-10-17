/* 

 nt2unix is Copyright (c) 1997, 1998 by

 |  _     :  Sven M. Paas, Lehrstuhl fuer Betriebssysteme (LfBS) 
 |_|_`__  :  RWTH Aachen,  Kopernikusstr. 16,  D-52056 Aachen 
   | |__) :  Tel.  :  +49-241-80-5162,  Fax: +49-241-8888-339
     |__) :  e-mail:  sven@lfbs.rwth-aachen.de  [PGP-public-key] 

*/

#ifdef __POSIX_THREADS__
#include <pthread.h>
#else
#include <thread.h>
#endif

#ifndef linux
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

#ifndef linux
#include <sys/systeminfo.h>
#include <stropts.h>
#include <poll.h>
#include <sys/mman.h>
#else 
#include <sys/ioctl.h>
#include <fstream.h>
#endif

#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <map>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

#include "nt2unix.h"

#ifdef __POSIX_THREADS__
#define thr_self pthread_self
#define thread_t pthread_t
#ifndef linux
#define SIGUNUSED SIGUSR1
#endif

// Mutex-functions and types
#define mutex_t pthread_mutex_t
#define mutex_destroy pthread_mutex_destroy
#define mutex_unlock pthread_mutex_unlock
#define mutex_trylock pthread_mutex_trylock
#define mutex_lock pthread_mutex_lock

// Local storage
#define thread_key_t pthread_key_t
#endif



#ifdef _DEBUG
CRITICAL_SECTION _DBGCS;
#define DBG(m) { cerr<<"[nt2unix:"<<thr_self()<<": "<<m<<"]"<<endl<<flush; }
#else
#define DBG(m) {}
#endif

// #define _DEBUGSYNC

#ifdef _DEBUGSYNC
//CRITICAL_SECTION _DBGCS;
#define DBGS(m) { cerr<<"[nt2unix:"<<thr_self()<<": "<<m<<"]"<<endl<<flush; }
#else
#define DBGS(m) {}
#endif

#ifdef sparc
#define PATH_PREFIX "/global/tmp"
#else
#define PATH_PREFIX 0
#endif

// A comfortable critical section object. 
class CriticalSection {
  public:
    CriticalSection::CriticalSection() {
      InitializeCriticalSection(&cs);
    }
    CriticalSection::~CriticalSection() {
      DeleteCriticalSection(&cs);
    }    
    inline void CriticalSection::enter() {
      EnterCriticalSection(&cs);       
    }  
    inline void CriticalSection::leave() {
      LeaveCriticalSection(&cs);
    }
  protected: 
    CRITICAL_SECTION cs; 
};

#define SYSVSEGMENT 0
#define FILESEGMENT 1

// We manage a list (a vector) of created file mappings.
struct FileMapping {
  LPVOID lpBaseAddress;		// base address of mapping
  DWORD dwNumberOfBytesToMap;	// mapping size
  HANDLE hFileMappingObject;	// file handle
  char FileName[MAX_PATH]; 	// file name
  DWORD refcnt;			// number of references to the mapping
  DWORD type;
};
static vector<FileMapping> FileMappings;

// The NT kernel stores various information along with each thread.
// We try to emulate this by our ThreadInfo struct for each thread. 
#define THREAD_RUNNING		0
#define THREAD_SUSPENDED	1
#define THREAD_TERMINATED	2

//Threads----------------------------------------------------------------------------------------------
struct ThreadInfo {

    ThreadInfo::ThreadInfo() {
      ThreadInfo::init(THREAD_RUNNING);
    }
    
    ThreadInfo::ThreadInfo(DWORD aState) {
      ThreadInfo::init(aState);
    }
    
#ifdef __POSIX_THREADS__
    // We need a copy constructor because the map copies
    // elements. We have to avoid destruction of the
    // mutex and the condition when struct is copied.
    // And creation of unused synch. objects.
    ThreadInfo::ThreadInfo(const ThreadInfo& other) {
       state=other.state;
       suspendCount=other.suspendCount;
       exitCode=other.exitCode;
       mutex=other.mutex;
       cond=other.cond;
//       other.cond=0;
//       other.mutex=0;
       counter=other.counter;
       (*counter)++;
       
    }
    
    
    ThreadInfo::~ThreadInfo() {
       (*counter)--;
       if(*counter) return;
       if(cond) {
          pthread_cond_destroy(cond);
          delete cond;
          cond=0;
       }
       if(mutex) {
          pthread_mutex_destroy(mutex);
          delete mutex;
          mutex=0;
       }
       delete counter;
       counter=0;
    }
    
private:
   // Hide assignment operator to avoid dangerous copies.
   bool ThreadInfo::operator=(ThreadInfo& other);
public:
#endif
    inline void ThreadInfo::init(DWORD aState) {
      state = aState;
      suspendCount=0;
      exitCode = 0;
#ifdef __POSIX_THREADS__
      cond=new pthread_cond_t;
      mutex= new pthread_mutex_t;
      pthread_cond_init(cond,0);
      pthread_mutex_init(mutex,0);
      counter=new DWORD;
      *counter=1;
#else
      threadHasBeenResumed = FALSE;      
#endif
    }
    
    volatile DWORD suspendCount;	 // suspend count, see ThreadResume(), ThreadSuspend()
    volatile DWORD state;         // state: THREAD_*
    DWORD exitCode; 	 // the thread's exit code
#ifdef __POSIX_THREADS__
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
    DWORD *counter;
#else
    volatile BOOL  threadHasBeenResumed; 
    // A special flag to synchronize SuspendThread() / ResumeThread()
#endif
};
typedef map<HANDLE, ThreadInfo, less<HANDLE> > ThreadInfoMap;
static ThreadInfoMap ThreadInfos;
// We must protect the access to the ThreadInfoMap: 
static CriticalSection ThreadInfoLock;

/*Si*/
/*
// not used yet
struct HandleInfo {
  DWORD handleType; 	// handletype of this handle (see below)
  void *obj; 		// pointer to handled object
  DWORD refcnt; 
}; 
*/
// not used yet
#define HANDLETYPE_CONSOLE	0	// not supported
#define HANDLETYPE_EVENTFILE	1	// not supported
#define HANDLETYPE_FILEMAPPING	2
#define HANDLETYPE_MUTEX	3
#define HANDLETYPE_NAMEDPIPE	4
#define HANDLETYPE_PROCESS	5
#define HANDLETYPE_SEMAPHORE	6
#define HANDLETYPE_THREAD	7
#define HANDLETYPE_TOKEN	8


#ifdef __cplusplus
extern "C" {
#endif


// Try to map a NT-style PTHREAD_START_ROUTINE to a Solaris-style
// start_func() (see "man -s 3T thr_create()").  
typedef void * (*solaris_PTHREAD_START_ROUTINE)(void *);
#ifdef __POSIX_THREADS__


struct pthread_start_struct {
    solaris_PTHREAD_START_ROUTINE routine;
    void *arg;
    DWORD suspended;
};

void *posix_start_routine(pthread_start_struct * s) {
   DSECTION("posix_start_routine");
   void *erg; 
   
   DSECTENTRYPOINT;

   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
   if(s->suspended==CREATE_SUSPENDED) SuspendThread((HANDLE)pthread_self());
   erg=s->routine(s->arg);
   delete s;

   DSECTLEAVE
       return erg;
}

#endif
        
// Threading functions ---------------------------------------------------
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
  DSECTION("CreateThread");
  ThreadInfoLock.enter();
  DWORD state = THREAD_RUNNING; // the new thread's default state.
#ifdef __POSIX_THREADS__
  pthread_attr_t attr;
  pthread_start_struct *start_struct;
#endif
  
  DSECTENTRYPOINT;

  if (lpThreadAttributes)
    DBG("CreateThread(): LPSECURITY_ATTRIBUTES not supported.");
#ifdef __POSIX_THREADS__
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
      DSECTLEAVE
	  return 0; 
  }
  
#else
  if (thr_create(0, (size_t)dwStackSize,
     (solaris_PTHREAD_START_ROUTINE) lpStartAddress,
     lpParameter,
     (long)dwCreationFlags | THR_BOUND | THR_NEW_LWP,
     (thread_t *)lpThreadId)) {
      DSECTLEAVE
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
  // This is incompatible to NT, but should work to some extend.

  DSECTLEAVE
      return (HANDLE)*lpThreadId;
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
#ifdef __POSIX_THREADS__
  return (HANDLE)pthread_self();
#else
  return (HANDLE)thr_self();
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
  DSECTION("ExitThread");
  DSECTENTRYPOINT;
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
  DSECTLEAVE;
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
  DSECTION("TerminateThread");
  DSECTENTRYPOINT;
  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo =
    ThreadInfos.find(hThread);

  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    (*thisThreadInfo).second.exitCode = dwExitCode;
    (*thisThreadInfo).second.state = THREAD_TERMINATED;
    ThreadInfoLock.leave();
#ifdef __POSIX_THREADS__
    if (pthread_cancel((pthread_t)hThread))
#else
    if (thr_kill((thread_t)hThread, SIGKILL))
#endif
      // failed.
	DSECTLEAVE
	    return FALSE;
  }

  ThreadInfoLock.leave();
  DSECTLEAVE
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
  DSECTION("GetExitCodeThread");
  
  if (!lpExitCode) 
    return FALSE;

  DSECTENTRYPOINT;
  
  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo =
    ThreadInfos.find(hThread);

  
  if (thisThreadInfo != ThreadInfos.end()) {
    // found it. 
    if ((*thisThreadInfo).second.state != THREAD_TERMINATED)
      *lpExitCode = (*thisThreadInfo).second.exitCode;
    else
      *lpExitCode = STILL_ACTIVE;
    ThreadInfoLock.leave();
    DSECTLEAVE
	return TRUE;
  }
  ThreadInfoLock.leave();
  DSECTLEAVE
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
  DSECTION("ResumeThread");
  DWORD oldSuspendCount ;

  DSECTENTRYPOINT;

  ThreadInfoLock.enter();
  DBG("Entering: ResumeThread, hThread ==" <<hThread);
  ThreadInfoMap::iterator thisThreadInfo = ThreadInfos.find(hThread);

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
          if (thr_continue((thread_t)hThread)) {

            perror("thr_continue()");
            ThreadInfoLock.leave();
	    DSECTLEAVE
		return 0xFFFFFFFF;
          }
          // Give up the CPU so that the resumed thread has a chance to
          // update the associated threadHasBeenResumed flag.
          thr_yield(); 
        } while (!(*thisThreadInfo).second.threadHasBeenResumed); 
        (*thisThreadInfo).second.state = THREAD_RUNNING;
        DBG("ResumeThread(): thread really resumed, handle == "<<hThread);
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
	 DBG("returning "<<oldSuspendCount<<flush);
    ThreadInfoLock.leave();
    DSECTLEAVE
	return oldSuspendCount;
  }
  // thread not found.
  DBG("ResumeThread(): thread handle not found, hThread == " << hThread); 
  ThreadInfoLock.leave();    
  DSECTLEAVE
      return 0xFFFFFFFF;
}

DWORD SuspendThread(HANDLE hThread)
{
  DSECTION("SuspendThread");
  BOOL same = FALSE; 
  // this flag indicates whether a thread suspends itself.
  // If same == TRUE, we must avoid a "lost signal" problem, see below. 

  DSECTENTRYPOINT;
  
  ThreadInfoLock.enter();
  ThreadInfoMap::iterator thisThreadInfo = ThreadInfos.find(hThread);

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
      if (same = (thr_self() == (thread_t)hThread)) { 
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
	DSECTLEAVE
	    return 0;
        
      // DANGER!!! If at this point, another thread 
      // is scheduled in ResumeThread(), the resume "signal" may get lost.
      // To avoid this, ResumeThread() polls until the thread is
      // really resumed, i.e. until threadHasBeenResumed == TRUE.  
      } else if(pthread_kill(pthread_t(hThread),SIGUNUSED)) {
#else
      DBG("Suspending thread "<<hThread<<flush);		
        (*thisThreadInfo).second.threadHasBeenResumed = FALSE; 
        ThreadInfoLock.leave();      
      }
//      (*thisThreadInfo).second.state = THREAD_SUSPENDED;
      if (thr_suspend((thread_t)hThread)) {
#endif
        perror("thr_suspend()");
	DSECTLEAVE
	    return 0xFFFFFFFF;
      }
#ifdef __POSIX_THREADS__
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
    
    DSECTLEAVE
	return oldSuspendCount;
  }
  // Thread not found.
  DBG("SuspendThread(): Thread handle not found, hThread == "<<hThread);
  ThreadInfoLock.leave();
  DSECTLEAVE
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
 
#ifdef __POSIX_THREADS__
  pthread_key_delete((pthread_key_t)dwTlsIndex);
#else
  DBG("TlsFree() not supported.");  
#endif
  return TRUE;
}


//EndThreads---------------------------------------------------------------------------------------------

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    HANDLE hObject
    )
{
  DSECTION("CloseHandle");
  DSECTENTRYPOINT;
  // Handles are somewhat NT-specific. At the moment, we ignore them.
   HandleInfo* Object; // *******
   Object = new HandleInfo; // *******
   Object = (HandleInfo*)hObject; // *******
 
   vector<FileMapping>::iterator i=FileMappings.begin();
     while(i != FileMappings.end() && i->hFileMappingObject != Object->obj)  // *******
         i++;
   if (i != FileMappings.end()) {
      if(i->type==SYSVSEGMENT) {
       	if(shmctl((int)Object->obj,IPC_RMID,0)<0) {
       		perror("shmctl");
       	}
       } else {
          close((int)Object->obj);
       }
   }

   DSCECTLEAVE
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
  HandleInfo* SourceInfo;

  DSECTENTRYPOINT;

  // we ignore handles. Just copy the "handle".
  // We should use dup() or dup2() for file handles, however. 
  DBG("DuplicateHandle() not supported. Use dup(2)/dup2(2) if appropriate.");

  SourceInfo = (HandleInfo*) hSourceHandle;
  SourceInfo->refcnt++;
  hSourceHandle = (HANDLE) SourceInfo;

  *lpTargetHandle = hSourceHandle;
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

DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD dwMilliseconds
  )
{
  DSECTION("WaitForSingleObject");
  DSECTENTRYPOINT;

  if (dwMilliseconds != INFINITE)
    DBG("WaitForSingleObject(): dwMilliseconds != INFINITE not supported.");
    DBG("Calling thr_join");
#ifdef __POSIX_THREADS__
    DNOTICE("calling pthread_join");
  if(pthread_join((pthread_t)hHandle,0)) {
#else  
    DNOTICE("calling thr_join");
  if (thr_join((thread_t)hHandle, 0, 0)) {
#endif
    DBG("WaitForSingleObject(): thr_join() failed\n");
    DSECTLEAVE
	return WAIT_FAILED;
  }
  DBG("Back from join");
  DSECTLEAVE
      return WAIT_OBJECT_0;
}
//EndHandles---------------------------------------------------------------------------------------------------------------
  //Begin Process-------------------------------------------------------------------------------------------------------

// Process Functions --------------------------------------------------------
WINBASEAPI
DWORD
WINAPI
GetCurrentProcessId(
    VOID
    )
{
  return (DWORD)getpid();
}
        
WINBASEAPI
HANDLE
WINAPI
GetCurrentProcess(
    VOID
    )
{
  return (HANDLE)getpid();
}

//End Process----------------------------------------------------------------------------------------
// Thread-Synchronization Functions -----------------------------------------
WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  DSECTION("InitializeCriticalSection");
  assert(lpCriticalSection); 

  DSECTENTRYPOINT;
  
  lpCriticalSection->LockCount = 0;
  lpCriticalSection->RecursionCount = 0;
  lpCriticalSection->OwningThread = (HANDLE)0xFFFFFFFF;

  lpCriticalSection->LockSemaphore = malloc(sizeof(mutex_t));   
  if (!lpCriticalSection->LockSemaphore)
    perror("InitializeCriticalSection(): malloc()");
#ifdef __POSIX_THREADS__
  pthread_mutex_init((mutex_t *)(lpCriticalSection->LockSemaphore),0);
#else  
  mutex_init((mutex_t *)(lpCriticalSection->LockSemaphore),
                 USYNC_THREAD, 0); 
#endif
  DSECTLEAVE
      return;
}
 

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  DSECTION("DeleteCriticalSection");
  assert(lpCriticalSection); 

  DSECTENTRYPOINT;

  mutex_destroy((mutex_t *)(lpCriticalSection->LockSemaphore));
  free(lpCriticalSection->LockSemaphore);
  lpCriticalSection->LockSemaphore = 0; 
  
  DSECTLEAVE
      return; 
}

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  DSECTION("EnterCriticalSection");
  thread_t me; 

  DSECTENTRYPOINT;

  me = thr_self(); 
  assert(lpCriticalSection);
  if(mutex_trylock((mutex_t *)(lpCriticalSection->LockSemaphore))) {
      // The mutex is not free...
		if (lpCriticalSection->OwningThread == (HANDLE)me) {
    		// I have the lock. This cannot be a race condition. 
    		lpCriticalSection->RecursionCount++; 
    		DBGS("New Recursion count of mutex is: "<<lpCriticalSection->RecursionCount++);
		DSECTLEAVE
		    return; 
  		} 
  		DBGS("Trying to lock, LockSema =="<<lpCriticalSection->LockSemaphore);
  		if(mutex_lock((mutex_t *)(lpCriticalSection->LockSemaphore)))
    		cerr<<"EnterCriticalSection(): mutex_lock() failed, errno == "<<GetLastError()<<endl<<flush;
	}    		
  // got it. I must be the first thread: 
  if (lpCriticalSection->RecursionCount) {
    cerr<<"EnterCriticalSection(): RecursionCount != 0"<<endl<<flush; 
    DSECTLEAVE
	return; 
  }
  lpCriticalSection->RecursionCount = 1;   
  lpCriticalSection->OwningThread = (HANDLE)me; 
  DBGS("Mutex Acquired, Owner == "<<me<<" LockSema == " <<
       lpCriticalSection->LockSemaphore); 
  DSECTLEAVE
      return;
}


WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
  DSECTION("LeaveCriticalSection");
  thread_t me; 

  DSECTENTRYPOINT;
  
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

  DSECTLEAVE
      return;
}

//EndThreadSync---------------------------------------------------------------------------------------------------






// Virtual Memory Management Funtions -----------------------------------------------------------------------------
BOOL VirtualLock(LPVOID lpAddress, DWORD dwSize)
{
  if (mlock((caddr_t)lpAddress, (size_t)dwSize))
    // failed
    return FALSE;
  return TRUE;
}

BOOL VirtualUnlock(LPVOID lpAddress, DWORD dwSize)
{
  if (munlock((caddr_t)lpAddress, (size_t)dwSize))
    // failed
    return FALSE;
  return TRUE;
}

WINBASEAPI
LPVOID
WINAPI
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{
  DSECTION("VirtualAlloc"); 
  int flags = 0;

  DSECTENTRYPOINT;
 
  /*
  cerr<<"VirtualAlloc() "<<lpAddress<<" Size "<<dwSize;
  if (flAllocationType & MEM_COMMIT)
    cerr<<" MEM_COMMIT ";
  if (flAllocationType & MEM_RESERVE)
    cerr<<" MEM_RESERVE ";
  if (flProtect & PROT_READ)
    cerr<<" READ ";
  if (flProtect & PROT_WRITE)
    cerr<<" WRITE ";
  cerr<<endl;   
  */
       
  if ( ((flProtect & PAGE_GUARD)   == PAGE_GUARD) ||
       ((flProtect & PAGE_NOCACHE) == PAGE_NOCACHE) )
    DBG("VirtualAlloc(): PAGE_GUARD, PAGE_NOCACHE not supported.");

  if ( ((flAllocationType & MEM_TOP_DOWN) == MEM_TOP_DOWN) )
    DBG("VirtualAlloc(): MEM_TOP_DOWN not supported.");
    
  // We have to emulate the MEM_RESERVE and MEM_COMMIT options.
  // Under Unix, we reserve an address range by allocating
  // it via valloc() but protecting in totally via mprotect(). 
  // To commit an address range, the same is done -- but with
  // user defined access rights. 
  // MEM_RESERVE
  /* 
  if ( ((flAllocationType & MEM_RESERVE) == MEM_RESERVE) ) {
     flags = MAP_PRIVATE; 
     if (lpAddress)
       flags |= MAP_FIXED; 
     lpAddress = mmap(lpAddress, (size_t)dwSize, PROT_NONE, flags, -1, 0);                       
     if (!lpAddress) {
       perror("VirtualAlloc(): mmap()");
       return 0; 
     }      
  }
  */
  
  // First of all, allocate the region, if necessary.     
  if (!lpAddress) {
    lpAddress = valloc((size_t)dwSize); 
    if (!lpAddress) {
      perror("VirtualAlloc(): valloc()");
      DSECTLEAVE
	  return 0; 
    }
  }

  // Note that, in this implementation, if both flags
  // are specified, the range is just commited. 
  // (Because the memory is implicitely reserved.)  
  if ( ((flAllocationType & MEM_COMMIT) == MEM_COMMIT) ) {
    // Just set the protection flags on the memory range.
    if (mprotect((caddr_t)lpAddress, (size_t)dwSize, (int)flProtect) == -1) {
      perror("VirtualAlloc(): mprotect()"); 
      DSECTLEAVE
	  return 0;
    }
  } else {
    if ( ((flAllocationType & MEM_RESERVE) == MEM_RESERVE) ) {
      // Just protect the region totally. 
      if (mprotect((caddr_t)lpAddress, (size_t)dwSize, PROT_NONE) == -1) {
        perror("VirtualAlloc(): mprotect()");
	DSECTLEAVE
	    return 0; 
      }
    }  
  }  
  DSECTLEAVE
      return lpAddress;
}


WINBASEAPI
BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    )
{
  DSECTION("VirtualFree");
  DSECTENTRYPOINT;

  if ( (dwFreeType & MEM_DECOMMIT) == MEM_DECOMMIT ) {
    // Hmm, this is not supported under UNIX.
    // We just protect the pages totally, however. 
    if (mprotect((caddr_t)lpAddress, (size_t)dwSize, PROT_NONE) == -1) {
      perror("VirtualFree(): mprotect()");
      DSECTLEAVE
	  return FALSE;
    }
    //if (memcntl((caddr_t)lpAddress, (size_t)dwSize, MC_SYNC, MS_INVALIDATE, 0, 0) == -1)
    //    perror("memcntl()");
  } else if ( (dwFreeType & MEM_RELEASE) == MEM_RELEASE ) {
      if (dwSize) {
	  DSECTLEAVE
	      return FALSE; // man page says that dwSize must be zero in this case. 
      }
    // Just free the memory range. 
    free(lpAddress);
  } else{
      // unknown parameter. 
      DSECTLEAVE
	  return FALSE;
  }
  // Everything ok. 
  DSECTLEAVE
      return TRUE;
}


WINBASEAPI
BOOL
WINAPI
VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{
  DSECTION("VirtualProtect");
  DSECTENTRYPOINT;
  //sigset_t newSigset; 
  /*
  cerr<<"VirtualProtect "<<lpAddress<< " Size "<<dwSize;
  
  if (flNewProtect & PROT_READ)
    cerr<<" READ ";
  if (flNewProtect & PROT_WRITE)
    cerr<<" WRITE ";
  cerr<<endl; 
  */  
  // DUMMY
  DBG("VirtualProtect("<<lpAddress<<","<<flNewProtect<<"): returning lpflOldProtect not supported."); 
  if (lpflOldProtect)
    *lpflOldProtect = 0; 

  //sigemptyset(&newSigset);
  //sigaddset(&newSigset, SIGSEGV);
  //sigprocmask(SIG_BLOCK, &newSigset, 0); 
  if (mprotect((caddr_t)lpAddress, (size_t)dwSize, (int)flNewProtect) == -1) {
    perror("VirtualProtect(): mprotect()");
    DSECTLEAVE
	return FALSE; 
  }
  //if (memcntl((caddr_t)lpAddress, (size_t)dwSize, MC_SYNC, MS_INVALIDATE | MS_SYNC, (int)flNewProtect, 0) == -1)
  //  perror("memcntl()");
  //sigprocmask(SIG_UNBLOCK, &newSigset, 0); 
  DSECTLEAVE
      return TRUE;
}


//Files------------------------------------------------------------------------------------------------------------------
WINBASEAPI
LPVOID
WINAPI
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    )
{
  return MapViewOfFileEx(hFileMappingObject,
           dwDesiredAccess,
           dwFileOffsetHigh,
           dwFileOffsetLow,
           dwNumberOfBytesToMap,
           0);
}

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif 

WINBASEAPI
LPVOID
WINAPI
MapViewOfFileEx(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap,
    LPVOID lpBaseAddress
    )
{
  DSECTION("MapViewOfFileEx");
  int prot = 0, flags = 0; 
  LPVOID ret;
 
  HandleInfo* Map;  // *******
  
  DSECTENTRYPOINT;

  // Map = new HandleInfo;  // *******
  Map = (HandleInfo*)hFileMappingObject;  // *******

  DBG("Entering MapViewOfFileEx(). DesiredAccess== "<<dwDesiredAccess<<" FILE_MAP_COPY=="<<FILE_MAP_COPY);  
  if (dwFileOffsetHigh > 0)
    DBG("MapViewOfFileEx(): ignoring dwFileOffsetHigh");

  // Filter the protection bits ...
  prot = dwDesiredAccess & FILE_MAP_ALL_ACCESS; 
  // ... and mapping flags: 
  flags = ((dwDesiredAccess & FILE_MAP_COPY)==FILE_MAP_COPY) ? MAP_PRIVATE : MAP_SHARED;
  DBG("mapping "<<(flags==MAP_PRIVATE?"private":"shared"));
  /* MAP_SHARED doesnot work correctly under Solaris SPARC 2.5.1 */
  if (lpBaseAddress)
     flags |= MAP_FIXED; 
      
  // Search and update the mapping in the vector.
  vector<FileMapping>::iterator i=FileMappings.begin();
  while(i != FileMappings.end() && i->hFileMappingObject != Map->obj)  // *******
    i++;
  if (i != FileMappings.end()) {
    if (dwNumberOfBytesToMap) {
      DBG("Setting dwNumberOfBytesToMap to "<<dwNumberOfBytesToMap);
      i->dwNumberOfBytesToMap = dwNumberOfBytesToMap;
    }
  } else {
    DBG("MapViewOfFileEx(): mapping not found.");
    DSECTLEAVE
	return 0; 
  } 
#if defined(_DEBUG) && !defined(SYSVSHM) 
  fprintf(stderr, "mmap(): lpBaseAddress == %x, prot == %d, flags == %d\n",
                   lpBaseAddress, prot, flags);
  fprintf(stderr, "  dwNumberOfBytesToMap == %d, dwFileOffsetLow == %d\n",
           i->dwNumberOfBytesToMap, dwFileOffsetLow); 
#endif 
  if(i->type==FILESEGMENT) {
  	if ((ret = (LPVOID)mmap((caddr_t)lpBaseAddress, 
              (size_t)i->dwNumberOfBytesToMap,    
	       prot, flags, (int)Map->obj,  // *******
              (off_t)dwFileOffsetLow)) == (LPVOID)MAP_FAILED) {             
    	perror("MapViewOfFileEx(): mmap() failed. \n");
	DSECTLEAVE
	    return 0;
  	}
  } else {
          ret=shmat((int)Map->obj,(const void *)lpBaseAddress,0);  // *******
  	  if((int)ret==-1) {
  	     perror("shmat");
	     DSECTLEAVE
		 return 0;
  	  }
 }

  // Initialize the mapping.
  DBG("Initializing mapping ...");
  if (mprotect((caddr_t)ret, (size_t)i->dwNumberOfBytesToMap, PROT_WRITE) == -1)
    perror("mprotect()"); 
  /*
  memset(ret, 0x00, (size_t)i->dwNumberOfBytesToMap);
  */
  /* The following memset() is absolutely necessary,
     even for NFS file systems !!!! */
  memset(ret, 0x00, 4);
  DBG("... done. prot is now "<< prot);
  if (mprotect((caddr_t)ret, (size_t)i->dwNumberOfBytesToMap, prot) == -1)
    perror("mprotect()"); 
       
  DBG("MapViewOfFileEx(): set lpBaseAddress to "<<ret);
  i->lpBaseAddress = ret;  
      
  DSECTLEAVE
      return ret;  
}
                            
                            
WINBASEAPI
BOOL
WINAPI
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    )
{
    DSECTION("UnmapViewOfFile");
    // Search the mapping according to the lpBaseAddress.
    DBG("UnmapViewOfFile(): searching for "<<lpBaseAddress);
    vector<FileMapping>::iterator i=FileMappings.begin();
    while(i != FileMappings.end() && i->lpBaseAddress != (LPVOID)lpBaseAddress)
	i++;
    if (i != FileMappings.end()) {
	// found
	if(i->type==FILESEGMENT) {
	    if (munmap((caddr_t)lpBaseAddress, (size_t)(i->dwNumberOfBytesToMap))<0) {
		DBG("UnmapViewOfFile(): munmap() failed.");
		DSECTLEAVE
		    return FALSE;
	    }
	} else {
	    shmdt((char*)lpBaseAddress);
	}
	i->refcnt--;
	/*
	  if (i->refcnt < 1) 
	  FileMappings.erase(i);
	  bullshit
	*/
	DSECTLEAVE
	    return TRUE;     
    } else {
	DBG("UnmapViewOfFile(): mapping not found.");
    } 
    DSECTLEAVE
	return FALSE;
}


WINBASEAPI
HANDLE
WINAPI
CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    )
{
  DSECTION("CreateFileMappingA");
  DSECTENTRYPOINT;
  DBG("Entering CreateFileMappingA(), hFile == "<<hFile);

  HandleInfo* Map;
  Map = new HandleInfo;
  Map->handleType = HANDLETYPE_FILEMAPPING;

  int fildes;
  struct FileMapping thisMapping;   

  // Initialize FileMapping structure
  thisMapping.lpBaseAddress = 0;
  thisMapping.dwNumberOfBytesToMap = 0;
  thisMapping.hFileMappingObject = 0; 
  thisMapping.FileName[0] = 0; 
  thisMapping.refcnt = 1; 
     
  if (lpFileMappingAttributes)
    DBG("CreateFileMappingA(): lpFileMappingAttributes not supported.");
 
  if (hFile == (HANDLE)0xFFFFFFFF) {
    // user must specify file size
    if ((dwMaximumSizeHigh < 1) && (dwMaximumSizeLow < 1))
      DBG("CreateFileMappingA(): dwMaximumSize == 0");
    DBG("Creating segment of size "<<dwMaximumSizeLow);
    // Open a file of size dwMaximumSizeLow.
    if (dwMaximumSizeHigh > 0)
      DBG("CreateFileMappingA(): ignoring dwMaximumSizeHigh");
#ifdef SYSVSHM    
     fildes=shmget(IPC_PRIVATE,dwMaximumSizeLow,0600);
      if(fildes==-1) {
        perror("shmget()");
        DBG("Cannot SHMOpen\n");
        DSECTLEAVE
	    return 0;
      }
      thisMapping.type=SYSVSEGMENT;
#else
     thisMapping.type=FILESEGMENT;
#endif
    if (!lpName) {
#ifndef SYSVSHM
      // We dont use tmpfile(), since it obviously generates only
      // one file per process.
      // NOTE: Mapping with MAP_SHARED doesnot
      // work correctly on a non-NFS file system under Solaris SPARC. 
      lpName = tempnam(PATH_PREFIX,"svm__"); 
      if (!lpName) {
	  DBG("CreateFileMappingA: tempnam() failed.");
	  DSECTLEAVE
	      return 0; 
      }      
      DBG("CreateFileMappingA(): creating temporary file: "<<lpName<<" Prefix is: "<<PATH_PREFIX);
      fildes = open((const char*)lpName,
                     O_CREAT | O_RDWR, 0600); 
      if (!fildes) {
        DBG("CreateFileMappingA(): cannot create file.");
	DSECTLEAVE
	    return 0;
      }
      // make sure the file is really of the right size. 
      if(ftruncate(fildes, (off_t)((dwMaximumSizeLow/8192+1)*8192) ) == -1)
        perror("CreateFileMappingA(): ftruncate()");
      unlink((const char *)lpName);
#endif
   } else {
    #ifndef SYSVSHM
      fildes = open((const char*)lpName, O_RDWR);
    #endif       
      strcpy(thisMapping.FileName, lpName); 
   }
    
   if (!fildes) {
      DBG("CreateFileMappingA(): cannot create / open file.");
      DSECTLEAVE
	  return 0;
    }
    
    thisMapping.hFileMappingObject = (HANDLE)fildes; 
    thisMapping.dwNumberOfBytesToMap = dwMaximumSizeLow;   
    FileMappings.push_back(thisMapping); 
    Map->obj = (HANDLE)fildes;    
 
    DSECTLEAVE
	return (HANDLE)Map;
  }
  fildes = (int)hFile; 
  thisMapping.hFileMappingObject = hFile;
  thisMapping.type=FILESEGMENT;
  if(!dwMaximumSizeLow) {
    // In this case, we must take the size of the specified file
    // as the size of the mapping. 
    if ( (off_t)(dwMaximumSizeLow = lseek(fildes, 0, SEEK_END)) == -1) 
      perror("CreateFileMappingA(): lseek()"); 
    if (dwMaximumSizeLow < 1) {
      DBG("CreateFileMappingA(): cannot create empty file mapping.");
      DSECTLEAVE
	  return 0;
    }
  }
  
  thisMapping.dwNumberOfBytesToMap = dwMaximumSizeLow; 
  FileMappings.push_back(thisMapping);

  Map->obj = hFile; /*Si*/
        
  DSECTLEAVE
      return (HANDLE)Map;
}

//----------------------------------------------------------------------------------------------------------
DWORD GetLastError(VOID)
{
  perror("GetLastError()");
  return (DWORD)errno; 
}

VOID SetLastError(DWORD dwErrCode)
{
  errno = (DWORD)dwErrCode;
}

UINT SetErrorMode(UINT uMode)
{
  DBG("SetErrorMode() not supported.\n"); 
  return 0; 
}

VOID Sleep(DWORD dwMilliseconds)
{
  switch(dwMilliseconds) {
    case 0:
       thr_yield();
       break;
       
    case INFINITE:
#ifdef __POSIX_THREADS__
//       pthread_kill(pthread_self(),SIGSTOP);
       select(0,0,0,0,0);
#else
       thr_suspend(thr_self()); 
#endif
       break;
        
    default:
// We could also use the POSIX call nanosleep(2) here
#ifdef linux
       usleep(dwMilliseconds*1000);
#else
       poll(0, 0, (int)dwMilliseconds); 
#endif
       break;
  }
  return;
}
#ifdef linux


int calcNumProcs() {
   ::ifstream cpuinfo("/proc/cpuinfo");
   int count=0;
   char s[255];
   if(!cpuinfo) {
     DBG("Sorry, cannot open /proc/cpuinfo");
     return 1;
   }
   while(!cpuinfo.eof()) {
    cpuinfo>>s;
    if(!strncmp("processor",s,9)) count++;
   }
   return count;
}

// This is just a quick hack. 
// We should improve this!!


VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
    DSECTION("GetSystemInfo");
    DSECTENTRYPOINT;

    DWORD i = 0;
    DWORD procset = 0;  
    
    lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
    // This should not be hardcoded:
    lpSystemInfo->wProcessorLevel = 5; 
    lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
    
    
    lpSystemInfo->dwNumberOfProcessors = calcNumProcs();
    
    lpSystemInfo->dwActiveProcessorMask = 0xFFFF;
    
    lpSystemInfo->dwPageSize =
	lpSystemInfo->dwAllocationGranularity = 4096;
    
    // We just use 1 GB above the maximum address on the heap. 
    // This implies that we now have only 1 GB for the process stack. 
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)0x88000000;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)0xa0000000;
    
    DSECTLEAVE;
}

#else
#define SYSINFOBUFMAX 256
VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
  DSECTION("GetSystemInfo");
  char buf[SYSINFOBUFMAX]; 
  long buflen = 0; 
  DWORD i = 0;
  DWORD procset = 0;  
 
  DSECTENTRYPOINT;
  
  assert(lpSystemInfo);
  memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
  
  buflen = sysinfo(SI_ARCHITECTURE, buf, SYSINFOBUFMAX);
  if (strcmp(buf, "sparc") == 0) {
    lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_SPARC;
  } else if (strcmp(buf, "i386") == 0) {
    lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
    // This should not be hardcoded:
    lpSystemInfo->wProcessorLevel = 5; 
    lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
  } else
    lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN; 
  
  lpSystemInfo->dwNumberOfProcessors = sysconf(_SC_NPROCESSORS_ONLN);
  
  procset = sysconf(_SC_NPROCESSORS_CONF); 
  for (i = 0; i < procset; i++)
    lpSystemInfo->dwActiveProcessorMask |= ((DWORD)1 << i);
  
  lpSystemInfo->dwPageSize =
    lpSystemInfo->dwAllocationGranularity = (DWORD)sysconf(_SC_PAGESIZE);
  
  // A Solaris SPARC process has 4 GB of virtual address space.
  // It is partioned as follows:
  // 
  // +-------+ 0xFFFFFFFF
  // | STACK |
  // |   |   | Stack grows from top, max. 2 GB (RLIMIT_STACK) size
  // |   V   | See getrlimit(2). 
  // |       |
  // |       |
  // |       |
  // |       | 
  // |       | 
  // +-------+ 0x80000000 (maximum brk(2) value, e.g. &_etext+RLIMIT_DATA)
  // |   ^   |
  // |   |   | Heap grows from bottom after data segment to brk value. 
  // | HEAP  |
  // +-------+ &_end       See end(3C)
  // |       | &_edata      
  // | DATA  |
  // +-------+ &_etext
  // | TEXT  | 
  // +-------+ 0x00000000
  //   
  // We just use 1 GB above the maximum address on the heap. 
  // This implies that we now have only 1 GB for the process stack. 
  lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)0x88000000;
  lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)0xc8000000;

  DSECTLEAVE;
}
#endif
WINBASEAPI
BOOL
WINAPI
GetComputerName(LPTSTR lpBuffer, LPDWORD nSize)
{
  if (gethostname(lpBuffer, *nSize))
    return FALSE;
  return TRUE; 
}

//EndGeneralStuff-----------------------------------------------------------------------------------------------------




// WinSock API -------------------------------------------------------

int PASCAL FAR WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
  if(!lpWSAData)
    return WSAEFAULT;
    
  // pretend to support the WinSock 2.0 API. 
  lpWSAData->wVersion =
    lpWSAData->wHighVersion = 0x0002;
  
  strcpy(lpWSAData->szDescription, "This is WinFake 0.1\n");
  strcpy(lpWSAData->szSystemStatus, "[still buggy]");
    
  return 0;
}

int PASCAL FAR WSACleanup(void)
{
  return 0;
}

int PASCAL FAR ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
  // This is untested. 
    
  if (ioctl(s, cmd, argp) == -1)
    return SOCKET_ERROR; 
    
  return 0; 
}








// Exception handling. ----------------------------------------------

static LPTOP_LEVEL_EXCEPTION_FILTER topFilter = 0; 
#ifdef __POSIX_THREADS__
void posixResumeHandler(int sig) {
 DSECTION("posixResumeHandler");
 DSECTENTRYPOINT;
 DBG("Entering posixSleepHandler");
 ThreadInfoMap::iterator thisThreadInfo = ThreadInfos.find((HANDLE)pthread_self());
 // We dn't need synchonization here because the list is locked
 // in SuspendThread by the signal sender.
  if (thisThreadInfo != ThreadInfos.end()) {
    pthread_cond_t *cond=(*thisThreadInfo).second.cond;
    pthread_mutex_t *mutex=(*thisThreadInfo).second.mutex;
    // I will block here until Suspender has called cond_wait
    pthread_mutex_lock(mutex);
      DBG("Suspending thread");
      (*thisThreadInfo).second.state=THREAD_SUSPENDED;
    pthread_cond_signal(cond);
    pthread_cond_wait(cond,mutex);
    (*thisThreadInfo).second.state=THREAD_RUNNING;      
    pthread_cond_signal(cond);
    pthread_mutex_unlock(mutex);
  } else {
     fprintf(stderr,"posixSleepHandler(): cannot find thread %x\n",pthread_self());
     ThreadInfoLock.leave();
  }
  DBG("Leaving posixSleepHandler");
  DSECTLEAVE
      return;
}
#endif
#ifdef linux
inline void globalSignalHandler(int sig);
#else
inline void globalSignalHandler(int sig, siginfo_t *sip, void *uap);
#endif  
#define MAX_SIGNALS (SIGUSR2+1)
// the maximum catchable signal number is defined by MAX_SIGNALS-1.

// The following class translates Solaris signal codes into
// Windows NT exception codes.
// Note that this is not possible for all codes.
// The following codes may be translated in a meaningful way:
//
// Unix signal code	Windows NT exception code
// --------------------------------------------------------
// SIGSEGV		EXCEPTION_ACCESS_VIOLATION
// SIGFPE		EXCEPTION_FLT_INVALID_OPERATION 
// SIGILL               EXCEPTION_ILLEGAL_INSTRUCTION
// SIGBUS		EXCEPTION_IN_PAGE_ERROR
// SIGTRAP              EXCEPTION_SINGLE_STEP

struct _UnixException {
    _UnixException::_UnixException() {    
      DBG("Entering ::_UnixException() ...");
      globalSignalMask = (1 << SIGSEGV) |
                                (1 << SIGFPE) |
                                (1 << SIGILL) |
                                (1 << SIGTRAP)|
                                (1 << SIGINT) |
                                (1 << SIGBUS) |
                                (1 << SIGABRT);
                                ;
      if (MAX_SIGNALS > (sizeof(DWORD)*8)) {
        fprintf(stderr, "_UnixException: MAX_SIGNALS to big.\n");
        exit(1);
      }
      memset(oact, 0, sizeof(oact));
      handlerInstalled = FALSE; 
     
      ExceptionInfo.ExceptionRecord =
        (EXCEPTION_RECORD*)malloc(sizeof(EXCEPTION_RECORD));   
      ExceptionInfo.ContextRecord = 0;

      // Create a ThreadInfo struct for the first thread. 
      ThreadInfo thisThreadInfo(THREAD_RUNNING);
      // add it to the global Map of ThreadInfos with the handle as key.
      ThreadInfos.insert(ThreadInfoMap::value_type((HANDLE)thr_self(), thisThreadInfo));
#ifdef __POSIX_THREADS__
     struct sigaction act;
     memset(&act,0,sizeof(act)); 
     DBG("Installing Handler for SIGUNUSED");
     act.sa_flags = SA_RESTART;
     act.sa_handler = posixResumeHandler; 
#ifdef linux
     act.sa_restorer= NULL;
#endif
     if (sigaction(SIGUNUSED, &act,  0)) {
          perror("sigaction(): posixResumeHandler()");
     } 
#endif
      DBG("Leaving ::_UnixException()");
    }
    
    _UnixException::~_UnixException() {
      // The process is terminating.
      if (handlerInstalled)
        uninstallHandler();
      free(ExceptionInfo.ExceptionRecord);
      
      // we should clean up ThreadInfos and FileMappings ...
      CleanUpMappings();
    }
    
    void _UnixException::installHandler() {
      struct sigaction act;
      DWORD i, bit; 
    
      DBG("Entering _UnixException::installHandler() ...");
      // Install a global signal handler for all signals defined
      // in globalSignalMask.
     memset(&act,0,sizeof(act)); 
#ifdef linux
     act.sa_flags = SA_RESTART;
     act.sa_handler = globalSignalHandler; 
     act.sa_restorer= NULL;
#else  
      act.sa_flags = SA_RESTART | SA_SIGINFO;
      act.sa_sigaction = globalSignalHandler;
#endif
      for (i = 1; i < MAX_SIGNALS; i++) {
        DBG("Trying bit i ==" << i << " globalSignalMask == "<<globalSignalMask);
        bit = 1 << i;
        if ((globalSignalMask & bit) == bit)  {
          DBG("::installHandler(): catching signal " << i);
          if (sigaction(i, &act,  &(oact[i]))) {
             perror("sigaction()");
          }
        }
      }
      handlerInstalled = TRUE; 
    }
    
    void _UnixException::uninstallHandler() {
      DWORD i, bit; 
      for (i = 1; i < MAX_SIGNALS; i++) {
        bit = 1 << i;
        if ((globalSignalMask & bit) == bit) {
          if (sigaction(i, &(oact[i]), 0)) {
            perror("sigaction()");
          }
        }
      }
      handlerInstalled = FALSE; 
    }
#ifdef linux
    inline void _UnixException::signalHandler(int sig,unsigned long * stack) {
#else
    inline void _UnixException::signalHandler(int sig, siginfo_t *sip, void *uap) {
#endif
      // The global signal handler method.
      // It converts the UNIX-style siginfo_t and ucontext_t
      // to a NT-Stye struct EXCEPTION_POINTERS. The result is stored
      // in the ExceptionInfo instance variable. 
      
      LONG topFilterReturn = EXCEPTION_CONTINUE_SEARCH;
      // default return value from the top level filter
      
      switch (sig) {
      
        case SIGSEGV:
          // A segmentation violation. 
          ExceptionInfo.ExceptionRecord->ExceptionCode =
             EXCEPTION_ACCESS_VIOLATION;
          ExceptionInfo.ExceptionRecord->ExceptionInformation[0] = 
#if defined(sparc)
            (*(unsigned *)((ucontext_t*)uap)->uc_mcontext.gregs[REG_PC] & (1 << 21));
#elif defined(__X86)
            (((ucontext_t*)uap)->uc_mcontext.gregs[ERR] & 2);
            //for (DWORD i = 7; i < NGREG; i++)
            //cerr << (DWORD)((ucontext_t*)uap)->uc_mcontext.gregs[i] << endl;
#elif defined(linux)
            stack[14] & 2; 
#endif
          if (ExceptionInfo.ExceptionRecord->ExceptionInformation[0])
            ExceptionInfo.ExceptionRecord->ExceptionInformation[0] = 1; 
            // 1 == write access 

            ExceptionInfo.ExceptionRecord->ExceptionInformation[1] =
#ifdef linux
            stack[22]; 
#else          
            (DWORD)sip->si_addr;
#endif            
          topFilterReturn = topFilter(&ExceptionInfo);        
          break;
        
        case SIGFPE:
          // A floating-point exception. 
          fprintf(stderr, "SIGFPE ....\n");
          ExceptionInfo.ExceptionRecord->ExceptionCode =
            EXCEPTION_FLT_INVALID_OPERATION;
          topFilterReturn = topFilter(&ExceptionInfo);        
          break;
          
        case SIGILL:
          // An illegal instruction. 
          ExceptionInfo.ExceptionRecord->ExceptionCode =
            EXCEPTION_ILLEGAL_INSTRUCTION;
          topFilterReturn = topFilter(&ExceptionInfo);        
          break;
          
        case SIGBUS:
          // A bus error. 
          ExceptionInfo.ExceptionRecord->ExceptionCode =
            EXCEPTION_IN_PAGE_ERROR;
          topFilterReturn = topFilter(&ExceptionInfo);        
          break;
            
        case SIGTRAP:
          // A trace trap. 
          ExceptionInfo.ExceptionRecord->ExceptionCode =
            EXCEPTION_SINGLE_STEP;
          topFilterReturn = topFilter(&ExceptionInfo);        
          break;
#ifdef __POSIX_THREADS__
        case SIGUNUSED: topFilterReturn=EXCEPTION_CONTINUE_EXECUTION;
                	break;   
#endif
        default:
          fprintf(stderr, "_UnixException::signalHandler(): unexpected signal: %d\n", sig);
          break; 
      }
      
      // We now have a valid topFilterReturn value (at least the default).      
      // If necessary, recall the old handler for the last signal: 
      if (topFilterReturn != EXCEPTION_CONTINUE_EXECUTION) {
        fprintf(stderr, "_UnixException::signalHandler(): Unhandled signal %d. Trying old handler ...\n", sig);
        CleanUpMappings();
#ifdef linux
          if (oact[sig].sa_handler) {
          oact[sig].sa_handler(sig);
#else
        if (oact[sig].sa_sigaction) {
          oact[sig].sa_sigaction(sig, sip, uap);
#endif
        } else { 
          sigaction(sig, &(oact[sig]), 0);
          kill(getpid(),sig);
        }
      }
        
      return;
    }   
    bool _UnixException::CleanUpMappings() { /*Si*/ /*bool added*/
    	 vector<FileMapping>::iterator i;
    	 i=FileMappings.begin();
    	 while(i!=FileMappings.end()) {
    	 	if(i->type==SYSVSEGMENT) {
    	 	  cerr<<"Removing SYSVSegment with id "<<i->hFileMappingObject<<endl;
    	 	  shmctl((int)i->hFileMappingObject,IPC_RMID,0);
    	 	 }
    	 	else close((int)i->hFileMappingObject);
    	 	i++;
    	 }
    }
    // array of old signal handlers. 
    struct sigaction oact[MAX_SIGNALS];

    EXCEPTION_POINTERS ExceptionInfo; 
    // for GetExceptionInformation(). 
    
    BOOL handlerInstalled; 
    // is the handler for the signal(s) installed ?
    
    DWORD globalSignalMask;
    // When calling SetUnhandledExceptionFilter, a new signal handler
    // for all signals i with ((1<<i) & globalSignalMask) == (1<<i) is
    // installed.
    // See /usr/include/sys/signal.h for the signal codes.  
    // For example, if you want to catch SIGSEGV's only, simply set
    // globalSignalMask to (1 << SIGSEGV) above.
    // See also MAX_SIGNALS above. 
} UnixException;

#ifdef linux
inline void globalSignalHandler(int sig)
#else
inline void globalSignalHandler(int sig, siginfo_t *sip, void *uap)
#endif
{
  // This function is installed as signal handler for all signals after
  // the first call to SetUnhandledExceptionFilter() (see below) with a
  // non-NULL argument.
  if (topFilter)
#ifdef linux
    UnixException.signalHandler(sig, (unsigned long *)&sig);
#else  
    UnixException.signalHandler(sig, sip, uap);
#endif
}


WINBASEAPI
LONG
WINAPI
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
  return EXCEPTION_CONTINUE_SEARCH;    
}  

WINBASEAPI
LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    )
{
  LPTOP_LEVEL_EXCEPTION_FILTER ret;

  DBG("Entering SetUnhandledExceptionFilter() ...");  
  ret = topFilter;
  
  // Set global filter:    
  topFilter = lpTopLevelExceptionFilter;

  if (UnixException.handlerInstalled) {
    // The handler is already installed. Is the new filter NULL ?
    if (!topFilter)
      // Yes -> deinstall handler. 
      UnixException.uninstallHandler();
    // No -> ok.
  } else {
    // The handler is not (yet) installed. Is the new filter non-NULL ? 
    if (topFilter)
      // Yes -> install the handler. 
      UnixException.installHandler();
    // No -> ok. 
  }     
         
  return ret;
}

LPEXCEPTION_POINTERS GetExceptionInformation(VOID)
{
  return &(UnixException.ExceptionInfo); 
}

VOID QueryPerformanceCounter(LARGE_INTEGER *v)
{
  timeval t;

  gettimeofday(&t,0);
  
  if (v)
    v->QuadPart=(LONGLONG)t.tv_sec*1000000+(LONGLONG)t.tv_usec;
}
      
VOID QueryPerformanceFrequency(LARGE_INTEGER *f)
{
  if (f)
    f->QuadPart = 1000000;
}

#ifdef __cplusplus 
}
#endif
