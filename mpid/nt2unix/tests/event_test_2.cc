#define _DEBUG_MAIN_REC
#include "../src/mydebug.h"
#include "nt2unix.h"
#include "insocket.h"
#include <iostream>
#include <sys/time.h>
using namespace std;

#define min(a,b)  (((a) < (b)) ? (a) : (b))

  int nCount = 3;
  HANDLE Event[3];

unsigned long EventSignaler(void*)
{
  cout << "Signaler : SLEEPING\n";
  Sleep (2200);

  for (unsigned int i = 0; i < 3; i++)
    {
      cout << "Setting..." << endl;
      Sleep (1000);
      if (!SetEvent (Event[i]))
	cout << "SetEvent FAILED !" << endl;
      else
	cout << "Event is SET !" << endl;
    }

  return 0;
}


unsigned long wait(void*)
{
  unsigned int i = 0;
  BOOL WaitAll = 0;
#ifdef MEASURE_TIME
  hrtime_t Start, End;
#endif
  cout << "WAIT : warte..." << endl;
 
#ifdef MEASURE_TIME
  Start = gethrtime();
#endif
  if (WaitForMultipleObjects (nCount, Event, WaitAll, INFINITE) == WAIT_TIMEOUT)
    cout << "WaitTimeout" << endl;
  else
    cout << "GOT EVENT !!!" << endl;
#ifdef MEASURE_TIME
  End = gethrtime();
  cout << "WaitForMultipleObjects needs " << (End - Start) << " nSecs !" << endl;
#endif
 
  return 0;
}


int main()
{
  DWORD Id1;
  HANDLE SignalerHandle;
  
  DWORD Id2;
  HANDLE WaitHandle;

  BOOL InitialState = 0;
  BOOL Manual = 0;
  unsigned int i = 0,c;
  int z;

  i = c;

  i = min (z, c);


  for (i = 0; i < 3; i++)
    Event[i] = CreateEvent (0, Manual, InitialState, 0);

  SignalerHandle = CreateThread (0, 0, EventSignaler, 0, 0, &Id1);
  WaitHandle = CreateThread (0, 0, wait, 0, 0, &Id2);

  cout << "waiting for Threads to terminate..." << endl;
  WaitForSingleObject (WaitHandle, INFINITE);

  for (i = 0; i < 3; i++)
    {
      if (!CloseHandle(Event[i]))
	cout << "CloseHandle (Event) FAILED !" << endl;
      else
	cout << "EventHandle closed !" << endl;
    }
	
  CloseHandle (SignalerHandle);
	
  CloseHandle (WaitHandle);

  cout << "un wech....\n";
	
  return 0;
}
	
	
