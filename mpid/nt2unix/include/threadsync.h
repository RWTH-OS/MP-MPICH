#ifndef THREADSYNC_HEADER
#define THREADSYNC_HEADER

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
#endif
