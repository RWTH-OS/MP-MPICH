/* $Id$ */

#ifndef __TIMER_H
#define __TIMER_H

#include <sys/time.h>
#include <unistd.h>

//------------------------------------------------------------------------
/////////////////
// class timer //
/////////////////
//
// class timer to measure time in the program
//
class timer 
{
private:
	struct timeval startTime;
	struct timeval stopTime;
public:
	timer();
   // writes timeOfDay in startTime
	inline void start() {gettimeofday(&startTime,NULL);}
   // writes timeOfDay in stopTime
	inline void stop() {gettimeofday(&stopTime,NULL);}
	double elapsedTime();
};

#endif

