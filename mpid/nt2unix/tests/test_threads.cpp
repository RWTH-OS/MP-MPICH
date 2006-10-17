/* $id$ */

#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define MAX_THREADS 10

CRITICAL_SECTION s; 

volatile int i = 0; 

DWORD __stdcall threadFunc(void *) {
  EnterCriticalSection(&s);
  fprintf(stderr, "Thread %d created\n", GetCurrentThreadId());
  fflush(stderr);
  i++; 
  LeaveCriticalSection(&s);
  
//  while(1) ;   
  return 0; 
}

HANDLE threads[MAX_THREADS]; 

int main (int argc, char **argv) {

  int j = 0; DWORD id; 
  
  InitializeCriticalSection(&s);
  EnterCriticalSection(&s);
  for (j=0; j < MAX_THREADS; j++) {
    threads[j] = CreateThread(0, 0, threadFunc, 0, CREATE_SUSPENDED, &id); 

    if (!threads[j])
      fprintf(stderr, "%d: CreateThread failed\n", GetLastError());
/*    else  
      SuspendThread(threads[j]);
 */ }
  cerr<<"All Threads are now created.\n";
  LeaveCriticalSection(&s);
  cerr<<"CS now free. Going asleep.\n";
  Sleep(1200);
  cerr<<"Waking up threads.\n";
  for (j=0; j < MAX_THREADS; j++) ResumeThread(threads[j]);

  cerr<<"Threads all running\n";  
  for (j=0; j < MAX_THREADS; j++)  WaitForSingleObject(threads[j],INFINITE);
  DeleteCriticalSection(&s);
  return 0; 
}
