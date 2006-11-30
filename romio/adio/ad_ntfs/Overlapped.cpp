/*
 * $Id$
 *
 */


#if (_MSC_VER<1100)
//avoid the definition of min and max in winbase.h
#define NOMINMAX
#endif
#include <wtypes.h>
#include <winbase.h>

#if (_MSC_VER<1100)
//avoid the definition of min and max in winbase.h
#define NOMINMAX
#include <new.h>
#include <iostream>
namespace std {
#include <deque>
}
#else
#include <deque>
#endif



typedef std::deque<OVERLAPPED*> COverQueue;
static COverQueue OverQueue;

#ifdef _MSC_VER
#pragma comment(exestr,"ROMIO NTFS ADIO, LFBS 1999")
#endif

extern "C" {
	#include "ad_ntfs.h"
	
	OVERLAPPED *GetOverlappedStruct(void) {
		OVERLAPPED *Over;
		DWORD err;
		if(!OverQueue.size()) {
			Over  = (OVERLAPPED *) ADIOI_Malloc(sizeof(OVERLAPPED));
			Over->hEvent=CreateEvent(0,TRUE,FALSE,0);
			if(!Over->hEvent) {
				err = GetLastError();
				fprintf(stderr,"CreateEvent failed\n");
				MPI_Abort(MPI_COMM_WORLD,err);
			}
		} else {
			Over = OverQueue.front();
			OverQueue.pop_front();
		}
		return Over;
	}
	
	void FreeOverlappedStruct(OVERLAPPED *Over) {
		if(!Over) return;
		if(OverQueue.size() >= MAX_BUFFERED_OVERS) {
			CloseHandle(Over->hEvent);
			ADIOI_Free(Over);
		} else {
			OverQueue.push_back(Over);
		}
	}


}