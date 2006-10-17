#ifndef __UNIX_EXCEPTION_H__
#define __UNIX_EXCEPTION_H__

#define MAX_SIGNALS (SIGUSR2+1)

class _UnixException {
public:    
    _UnixException(DWORD mask);
    _UnixException();
    void init(unsigned long mask);    
    ~_UnixException();
    void installHandler();
    void uninstallHandler();
#if defined(linux)
    void signalHandler(int sig,unsigned long * stack);
#else
    void signalHandler(int sig, siginfo_t *sip, void *uap);
#endif

    void CleanUpMappings();

    BOOL handlerInstalled; 
    EXCEPTION_POINTERS ExceptionInfo; 

	protected:
    // array of old signal handlers. 
    struct sigaction oact[MAX_SIGNALS];

    // for GetExceptionInformation(). 
    
    // is the handler for the signal(s) installed ?
    
    DWORD globalSignalMask;
    // When calling SetUnhandledExceptionFilter, a new signal handler
    // for all signals i with ((1<<i) & globalSignalMask) == (1<<i) is
    // installed.
    // See /usr/include/sys/signal.h for the signal codes.  
    // For example, if you want to catch SIGSEGV's only, simply set
    // globalSignalMask to (1 << SIGSEGV) above.
    // See also MAX_SIGNALS above. 
};


extern _UnixException UnixException;


#endif
