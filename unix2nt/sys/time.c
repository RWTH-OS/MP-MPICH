/* $Id$ */

#include <sys/time.h>




int gettimeofday(struct timeval *tp, void *tz)
{
	/*
	double i;
	clock_t system_time;

	system_time = clock();
	i = ((double) system_time) / ((double) CLOCKS_PER_SEC);
	tp->tv_sec = (int) i;
	tp->tv_usec = (int) ((i - tp->tv_sec) * 1000000.0);
	*/

	LARGE_INTEGER clocks;
	double secs, usecs;
	static LARGE_INTEGER freq;
    double dfreq;



	QueryPerformanceCounter(&clocks);
	QueryPerformanceFrequency(&freq);
	dfreq = (double)freq.QuadPart;

	secs = (double)clocks.QuadPart/dfreq;	
	usecs = ((double)clocks.QuadPart - ((double)((int)secs) * dfreq)) / (dfreq/1000000.0);

	tp->tv_sec = (long int)secs;
	tp->tv_usec	= (long int)usecs;

	  
	return(0);
}