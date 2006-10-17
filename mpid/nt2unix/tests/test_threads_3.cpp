/* $id$ */
#include <windows.h>
#include <iostream>
#include <time.h>

using namespace std;

DWORD WINAPI fnsven(LPVOID p) {

	cerr << "Thread created\n";
	while (1) { Sleep(0);
	}
	cerr << "Thread done" << endl;

	return 0;
}

#define LOOP 1000

int main(void) {

	DWORD id, i = 0; 
	HANDLE h = (HANDLE)0;

	clock_t t1, t2; 

	h = CreateThread(0, 0,  fnsven, 0, CREATE_SUSPENDED, &id);
//	h = CreateThread(0, 0,  fnsven, 0, 0, &id);
	if(!h) {
	  cerr<<"CreateThread failed\n";
	  return 0;
	 }

	t1 = clock(); 
	while (i++ < LOOP) {
		
	if (ResumeThread(h) == 0xFFFFFFFF) {
			cerr << "ResumeThread() failed" << endl;
			return 0;
		}
	if (SuspendThread(h) == 0xFFFFFFFF) {
			cerr << "SuspendThread() failed" << endl;
			return 0;
		}	
		
	}
	t2 = clock(); 

	cerr << "Done." << endl;

	cerr << "Time: " <<
		(double)((double)t2 - (double)t1) / CLOCKS_PER_SEC / LOOP << endl;

	// ResumeThread(GetCurrentThread());
	return 0;
}
