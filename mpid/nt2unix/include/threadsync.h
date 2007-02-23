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
    void enter() {
      EnterCriticalSection(&cs);       
    }  
    void leave() {
      LeaveCriticalSection(&cs);
    }
  protected: 
    CRITICAL_SECTION cs; 
};
#endif
