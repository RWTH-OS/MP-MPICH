
#include "memlog.h"
#include "psapi.h"


DWORD GetProcessSize()
{
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	DWORD retval=0;

	if (NULL == hProcess)
		return 0;

	if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
	{
		retval=pmc.PeakWorkingSetSize;
	}

	CloseHandle( hProcess );
	return retval;
}