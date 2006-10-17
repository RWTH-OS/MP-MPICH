#if !defined(linux)
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

#include <iostream>
#include <string>
#include <vector>
#include <map>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

//---------------------------------------------------------------------------------------
#define _DEBUG_EXTERN_REC
#include "mydebug.h"
#include "nt2unix.h"
#include "threads.h"
#include "filemap.h"
#include "debugnt2u.h"

using namespace std;

ThreadInfoMap ThreadInfos;
// We must protect the access to the ThreadInfoMap: 
CriticalSection ThreadInfoLock;
#ifdef __cplusplus
extern "C" {
#endif

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
#if defined(linux)
    inline void globalSignalHandler(int sig);
#else
inline void globalSignalHandler(int sig, siginfo_t *sip, void *uap);
#endif  

// the maximum catchable signal number is defined by MAX_SIGNALS-1.

// The following class translates Unix signal codes into
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


#include "unixexception.h"


void _UnixException::init(unsigned long mask) { 
    DSECTION("_UnixException::init");

    DSECTENTRYPOINT;
    
    DBG("Entering _UnixException::init ...");
    
    globalSignalMask = mask;
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
    DSECTLEAVE;
}
    
_UnixException::~_UnixException() {
    DSECTION("_UnixException::~_UnixException");
    DSECTENTRYPOINT;
    // The process is terminating.
    if (handlerInstalled)
	uninstallHandler();
    free(ExceptionInfo.ExceptionRecord);
    
    // we should clean up ThreadInfos and FileMappings ...
    CleanUpMappings();
    DSECTLEAVE;
    return;
}
    
void _UnixException::installHandler() {
    DSECTION("_UnixException::installHandler");
    struct sigaction act;
    DWORD i, bit; 
    
    DSECTENTRYPOINT;

    if(handlerInstalled) {
	DSECTLEAVE
	    return;
    }
    
    DBG("Entering _UnixException::installHandler() ...");
    // Install a global signal handler for all signals defined
    // in globalSignalMask.
    memset(&act,0,sizeof(act)); 
#if defined(linux)
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
    DSECTLEAVE;
}
    
void _UnixException::uninstallHandler() {
    DSECTION("_UnixException::uninstallHandler");
    DWORD i, bit; 

    DSECTENTRYPOINT;
    
    for (i = 1; i < MAX_SIGNALS; i++) {
        bit = 1 << i;
        if ((globalSignalMask & bit) == bit) {
	    if (sigaction(i, &(oact[i]), 0)) {
		perror("sigaction()");
	    }
        }
    }
    handlerInstalled = FALSE; 

    DSECTLEAVE;
}
    
    
#if defined(linux)
  inline void _UnixException::signalHandler(int sig,unsigned long * stack) {
#else
  inline void _UnixException::signalHandler(int sig, siginfo_t *sip, void *uap) {
#endif
      DSECTION("_UnixException::signalHandler");
      // The global signal handler method.
      // It converts the UNIX-style siginfo_t and ucontext_t
      // to a NT-Stye struct EXCEPTION_POINTERS. The result is stored
      // in the ExceptionInfo instance variable. 
      
      DSECTENTRYPOINT;
      
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
#if defined(linux)
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
          char hostname[16];
          gethostname(hostname,16);          
	  fprintf(stderr, "[%s] _UnixException::signalHandler(): Unhandled signal %d. Trying old handler ...\n", hostname, sig);
	  CleanUpMappings();
#if defined(linux)
          if (oact[sig].sa_handler) {
	      oact[sig].sa_handler(sig);
	  }
#else
	  if (oact[sig].sa_sigaction) {
	      oact[sig].sa_sigaction(sig, sip, uap);
	  }
#endif
	  else { 
	      sigaction(sig, &(oact[sig]), 0);
	      kill(getpid(),sig);
	  }
      }
   
      DSECTLEAVE
	  return;
  }   
  
  
  
void _UnixException::CleanUpMappings() {
    DSECTION("_UnixException::CleanUpMappings");
    std::vector<FileMapping>::iterator i;
    
    DSECTENTRYPOINT;
    
    i=FileMappings.begin();
    while(i!=FileMappings.end()) {
	if(i->type==SYSVSEGMENT) {
	    cerr<<"Removing SYSVSegment with id "<<i->hFileMappingObject<<endl;
	    shmctl((int)i->hFileMappingObject,IPC_RMID,0);
	}
	else close((int)i->hFileMappingObject);
	i++;
    }

    DSECTLEAVE;
}

_UnixException::_UnixException() {
    DWORD mask;
    init( (1 << SIGSEGV)|
	  (1 << SIGFPE) |
	  (1 << SIGILL) |
	  (1 << SIGTRAP)|
	  (1 << SIGINT) |
	  (1 << SIGBUS) |
	  (1 << SIGABRT));
    
}

_UnixException::_UnixException(DWORD mask) {
    init(mask);
}

_UnixException UnixException((1 << SIGSEGV)|
                (1 << SIGFPE) |
                (1 << SIGILL) |
                (1 << SIGTRAP)|
                (1 << SIGINT) |
                (1 << SIGBUS) |
                (1 << SIGABRT));

#if defined (linux)
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
    DSECTION("SetUnhandledExceptionFilter");
    LPTOP_LEVEL_EXCEPTION_FILTER ret;

    DSECTENTRYPOINT;
    
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
    
    DSECTLEAVE
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






