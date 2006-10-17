#include <stdlib.h>
#include <stdio.h>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

#include "nt2unix.h"

#ifdef __cplusplus
extern "C" {
#endif

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
  HandleInfo* Process;
  Process = (HandleInfo*)malloc(sizeof(HandleInfo));

  Process->obj = (HANDLE) getpid();
  Process->handleType = HANDLETYPE_PROCESS;
  Process->refcnt++;

  return (HANDLE)Process;
}

WINBASEAPI
HANDLE
WINAPI
OpenProcess(DWORD dwDesiredAccess, // access flag 
	    BOOL bInheritHandle,   // handle inheritance flag 
	    DWORD dwProcessId)     // process identifier
{
  HandleInfo* Process;
  Process = (HandleInfo*)malloc(sizeof(HandleInfo));
  printf ("! OpenProcess only returns given ProcessID !");

  Process->obj = (HANDLE) dwProcessId;
  Process->handleType = HANDLETYPE_PROCESS;
  Process->refcnt++;

  return (HANDLE)Process;
}
  

#ifdef __cplusplus 
}
#endif
