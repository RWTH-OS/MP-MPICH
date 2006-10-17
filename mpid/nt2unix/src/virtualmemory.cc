#include "nt2unix.h"

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
#include <string.h>
#include <vector>
#include <map>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

//-----------------------------------------------------------------------
#include <debugnt2u.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  int flags = 0; 
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

  // First of all, allocate the region, if necessary.     
  if (!lpAddress) {
    lpAddress = valloc((size_t)dwSize); 
    if (!lpAddress) {
      perror("VirtualAlloc(): valloc()");
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
      return 0;
    }
  } else {
    if ( ((flAllocationType & MEM_RESERVE) == MEM_RESERVE) ) {
      // Just protect the region totally. 
      if (mprotect((caddr_t)lpAddress, (size_t)dwSize, PROT_NONE) == -1) {
        perror("VirtualAlloc(): mprotect()");
        return 0; 
      }
    }  
  }  
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
  if ( (dwFreeType & MEM_DECOMMIT) == MEM_DECOMMIT ) {
    // Hmm, this is not supported under UNIX.
    // We just protect the pages totally, however. 
    if (mprotect((caddr_t)lpAddress, (size_t)dwSize, PROT_NONE) == -1) {
      perror("VirtualFree(): mprotect()");
      return FALSE;
    }
    //if (memcntl((caddr_t)lpAddress, (size_t)dwSize, MC_SYNC, MS_INVALIDATE, 0, 0) == -1)
    //    perror("memcntl()");
  } else if ( (dwFreeType & MEM_RELEASE) == MEM_RELEASE ) {
    if (dwSize)
      return FALSE; // man page says that dwSize must be zero in this case. 
  
    // Just free the memory range. 
    free(lpAddress);
  } else
    // unknown parameter. 
    return FALSE;
     
  // Everything ok. 
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
    return FALSE; 
  }
  //if (memcntl((caddr_t)lpAddress, (size_t)dwSize, MC_SYNC, MS_INVALIDATE | MS_SYNC, (int)flNewProtect, 0) == -1)
  //  perror("memcntl()");
  //sigprocmask(SIG_UNBLOCK, &newSigset, 0); 
  return TRUE;
}
#ifdef __cplusplus 
 }
#endif
