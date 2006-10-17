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

#if !defined (linux)
#include <sys/systeminfo.h>
#include <stropts.h>
#include <poll.h>
#include <sys/mman.h>
#else 
#include <sys/ioctl.h>
#include <fstream>
#endif

#include <dlfcn.h>
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

//---------------------------------------------------------------------------------------------
#define _DEBUG_EXTERN_REC
#include "mydebug.h"
#include "nt2unix.h"
#include "debugnt2u.h"

#ifdef __cplusplus
extern "C" {
#endif
using namespace std;
    
// #define MAKELANGID(a, b) 0
    
DWORD FormatMessage (DWORD dwFlags,        // IGNORED
		     LPVOID lpSource,      // Source
		     DWORD dwMessageId,    // ErrorNumber
		     DWORD dwLanguageId,   // IGNORED
		     LPTSTR lpBuffer,      // Target Buffer
		     DWORD nSize,          // IGNORED
		     va_list *Arguments)   // IGNORED
{
    DSECTION("FormatMessage");
    
    DSECTENTRYPOINT;
    
    if (dwFlags == FORMAT_MESSAGE_FROM_STRING)
	memcpy (lpBuffer, lpSource, nSize);
    else
	lpBuffer = strerror (dwMessageId);
    
    DSECTLEAVE
	return((*lpBuffer > 0)?(sizeof (lpBuffer)):0);
}
    
void FreeLibrary(HANDLE Lib)
{
  dlclose((HANDLE)Lib);
}

HANDLE LoadLibrary(char *LibraryName)
{
    void *handle;

    if(!(handle=dlopen(LibraryName,RTLD_NOW)))
	std::cerr << dlerror() << endl;
    return handle;
    
}

HANDLE GetModuleHandle(char *ModuleName)
{
    void *handle;

    if(!(handle=dlopen(ModuleName,RTLD_NOLOAD)))
	std::cerr << dlerror() << endl;

    return handle;
}

void* GetProcAddress(HANDLE Library,char *FuncName)
{
  void *handle;

  /*
    beware: dlsym() may return NULL as a valid pointer, the only safe way to detect an error
    is by calling dlerror(); the problem is, what to do in case we get a NULL pointer:
    we can't return it because under Windows this means an error condition; this problem
    is currently unsolved, we just print out a warning message if dlsym() returns NULL 
  */ 
  if( !(handle = dlsym(Library,FuncName)) )
    std::cerr << "Got NULL pointer: This may be a valid pointer!" << endl;
  
  return handle;
}

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
#if defined(linux)
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
    DWORD i = 0;
    DWORD procset = 0;  
 
    DSECTENTRYPOINT;
    
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

int stricmp (const char* string1, const char* string2)
{
    return strcasecmp (string1, string2);
}



#define MPID_PKT_READY_CLR(x)    *(x) = 0


#ifdef __cplusplus 
}
#endif
