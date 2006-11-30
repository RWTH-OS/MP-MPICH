/* $Id$ */

#include <unistd.h>
#include <windows.h>

#ifdef WIN32
int errno;
#endif

long sysconf(int name)
{
	SYSTEM_INFO system_info;

	if (name == _SC_PAGESIZE)
	{
		GetSystemInfo(&system_info);
		return(system_info.dwPageSize);
	}

	errno = GetLastError();
	return(-1);
}

int usleep(unsigned useconds)
{
  Sleep(useconds/1000);
  return(0);
}

int sleep(unsigned seconds)
{
	Sleep(seconds*1000);
	return 0;
}
/*
int gethostname(char *name, int namelen)
{

    return 0;
}
*/