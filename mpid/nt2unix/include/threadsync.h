#ifndef THREADSYNC_HEADER
#define THREADSYNC_HEADER

// A comfortable critical section object. 
class CriticalSection {
  public:
    CriticalSection() {
      InitializeCriticalSection(&cs);
    }
    ~CriticalSection() {
      DeleteCriticalSection(&cs);
    }    
    inline void enter() {
      EnterCriticalSection(&cs);       
    }  
    inline void leave() {
      LeaveCriticalSection(&cs);
    }
  protected: 
    CRITICAL_SECTION cs; 
};
#endif
