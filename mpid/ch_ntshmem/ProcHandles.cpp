/*
 * $Id$
 *
 */


#pragma warning(disable: 4786)

#include <wtypes.h>
#include <winbase.h>
#include <map>
#include <deque>

typedef std::map<DWORD,HANDLE> CHandleMap;
typedef std::deque<volatile int*> CFlagQueue;

static CHandleMap Handles;
static CFlagQueue Flags;

#ifdef __cplusplus
extern "C" {
#endif

#include "mpid.h"
#include "p2p_shmalloc.h"
#include "p2p_special.h"

void StoreProcHandle(DWORD pid,HANDLE hProcess) {
	Handles.insert(CHandleMap::value_type(pid,hProcess));
}

HANDLE GetProcessHandle(DWORD pid) {
	CHandleMap::iterator i;
	HANDLE hProcess;
	i=Handles.find(pid);
	if(i==Handles.end()) {
		hProcess = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE,FALSE,pid);
		Handles.insert(CHandleMap::value_type(pid,hProcess));
	} else hProcess = (*i).second;
	return hProcess;
}

void CleanupHandles(void) {
	CHandleMap::iterator i;
	for(i=Handles.begin();i!=Handles.end();i++) 
		CloseHandle((*i).second);
	Handles.clear();
}

/* This should result in exactly one memory-block*/
#define NUMFLAGS (MPID_CACHE_LINE_SIZE*2/sizeof(int*))

void CreateFlags(void) {
	volatile int *f;
	int i;
	f = (volatile int*)p2p_shmalloc(NUMFLAGS*sizeof(int));
	if(!f) return;
	for(i=0;i<NUMFLAGS;i++)
		Flags.push_back(f+i);
}


volatile int* GetFlag(void) {
	volatile int *f;
	if(!Flags.size()) CreateFlags();
	if(Flags.size()) {
		f=Flags.front();
		Flags.pop_front();
	} else f=0;
	return f;
}

void FreeFlag(volatile int *flag) {
	Flags.push_back(flag);
}


#ifdef __cplusplus
}
#endif
