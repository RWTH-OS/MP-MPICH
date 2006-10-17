#include "nt2unix.h"
#include <iostream>

  HANDLE Event;
  int Data;

unsigned long Ausgabe(void*)
{
        for (int i = 0; i <= 2; i++)
	{
	  cout << "Ausgabe : waiting for Event......" << endl;
	  WaitForSingleObject(Mutex,INFINITE);
	  cout << "Wert " << i << " = " << Data << endl;
	  if (!ResetEvent (Event))
	    cout << "Ausgabe : ResetEvent !!!" << endl;
	  Sleep (1000);
	}

	return 0;
}

unsigned long prepare(void*)
{
        int count = 0;

	while (count <= 2)
	{
	  cout << "prepare : waiting for Event....." << endl;
	  WaitForSingleObject (Event,INFINITE);
	  cout << "prepare : ....Event is SIGNALED.....resetting !" << endl;
	  Data++;
	  ResetEvent (Event);
	  Sleep (1000);
	}

	return 0;
}

int main() 
{
	BOOL InitialState = 0;
	BOOL Manual = 1;
	DWORD PrepareId, AusgabeId;
        unsigned long(*prepare_pointer)(void*) = prepare;
	unsigned long(*Ausgabe_pointer)(void*) = Ausgabe;
	HANDLE AusgabeHandle, PrepareHandle;
	
        Event = CreateEvent (0, Manual, InitialState, 0);
	cout << "Event created. HANDLE : " << Mutex << endl;
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
	
	
