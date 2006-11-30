/* $Id$ */

#include "timer.h"

//------------------------------------------------------------------------
//
// timer's constructor
//
timer::timer()
{
	startTime.tv_sec=0;
	stopTime.tv_sec=0;
	startTime.tv_usec=0;
	stopTime.tv_usec=0;
}
//------------------------------------------------------------------------
// timer's function elapsedTime:
// calculates the time elapsed between the calls of start() and stop() 
//
double timer::elapsedTime()
{
	double	tmp;

   tmp=(double)(stopTime.tv_sec-startTime.tv_sec)
			+0.000001*(double)(stopTime.tv_usec-startTime.tv_usec);
	startTime.tv_sec=0;
	stopTime.tv_sec=0;
	startTime.tv_usec=0;
	stopTime.tv_usec=0;
   if(tmp<0) 
      tmp=0;
	return(tmp);	
}

