#include <map>
#include <threadsync.h>

#ifndef THREADS_HEADER
#define THREADS_HEADER

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

// Try to map a NT-style PTHREAD_START_ROUTINE to a Solaris-style
// start_func() (see "man -s 3T thr_create()").  
typedef void * (*solaris_PTHREAD_START_ROUTINE)(void *);

#define THREAD_RUNNING		0
#define THREAD_SUSPENDED	1
#define THREAD_TERMINATED	2

struct ThreadInfo {

    ThreadInfo() {
      init(THREAD_RUNNING);
    }
    
    ThreadInfo(DWORD aState) {
      init(aState);
    }
    
#ifdef __POSIX_THREADS__
    // We need a copy constructor because the map copies
    // elements. We have to avoid destruction of the
    // mutex and the condition when struct is copied.
    // And creation of unused synch. objects.
    ThreadInfo(const ThreadInfo& other) {
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
    
    
    ~ThreadInfo() {
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
   void operator=(ThreadInfo& other);
public:
#endif
    void init(DWORD aState) {
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
    DWORD exitCode; 	 // the threads exit code
#ifdef __POSIX_THREADS__
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
    DWORD *counter;
#else
    volatile BOOL  threadHasBeenResumed; 
    // A special flag to synchronize SuspendThread() / ResumeThread()
#endif
};

typedef std::map<HANDLE, ThreadInfo, std::less<HANDLE> > ThreadInfoMap;
extern ThreadInfoMap ThreadInfos;
// We must protect the access to the ThreadInfoMap:
extern CriticalSection ThreadInfoLock;
#endif //THREADS_HEADER
