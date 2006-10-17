#define _DEBUG_MAIN_REC
#include "../src/mydebug.h"
#include "nt2unix.h"
#include <iostream>
using namespace std;

  HANDLE Event;
  int Data;

unsigned long Ausgabe(void*)
{
        for (int i = 0; i <= 2; i++)
	{
	  cout << "Ausgabe : waiting for Mutex Ownership......" << endl;
	  WaitForSingleObject(Event,INFINITE);
	  cout << "Wert " << i << " = " << Data << endl;
	  if (!ResetEvent (Event))
	    cout << "Ausgabe : ResetEvent failed !!!" << endl;
	  Sleep (1000);
	}

	return 0;
}

unsigned long prepare(void*)
{
        int count = 0;

	while (count <= 2)
	{
	  cout << "prepare : waiting ...." << endl;
	  WaitForSingleObject (Event,INFINITE);
	  cout << "prepare : ....Event is signaled.....adding..... !" << endl;
	  Data++;
	  cout << "prepare : ...Resetting..." << endl;
	  ResetEvent (Event);
	  Sleep (1000);
	}

	return 0;
}

int main() 
{
	BOOL ManualReset = 1;
	BOOL InitialState = 0;
	DWORD PrepareId, AusgabeId;
        unsigned long(*prepare_pointer)(void*) = prepare;
	unsigned long(*Ausgabe_pointer)(void*) = Ausgabe;
	HANDLE AusgabeHandle, PrepareHandle;
	
        Event = CreateEvent (0, ManualReset, InitialState, 0);
	cout << "Event created. HANDLE : " << Event << endl;
	cout << "Pointer to function : " << prepare_pointer << endl;

	cout << "MAIN : Creating Thread for prepare function" << endl;
        PrepareHandle = CreateThread (0, 0, prepare_pointer, 0, 0, &PrepareId);
	cout << "MAIN : Creating Thread for Ausgabe function" << endl;
	AusgabeHandle = CreateThread (0, 0, Ausgabe_pointer, 0, 0, &AusgabeId);

	cout << "MAIN : Waiting for Threads to terminate......." << endl;
	WaitForSingleObject (AusgabeHandle, INFINITE);

	ResetEvent (Event);
	if (!CloseHandle(Event))
	  cout << "CloseHandle (Event) FAILED !" << endl;
	if (!CloseHandle(PrepareHandle) || !CloseHandle(AusgabeHandle))
	  cout << "CloseHandle (Thread) FAILED" << endl;
	cout << "un wech....\n";
	
	return 0;
}
	
	
