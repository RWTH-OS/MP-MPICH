#include <stdio.h>
#include "prof_timer.h"

#include <sys/time.h>
int myTimer;

/* timer for profiling */
void start_time(double *t)
{
    struct timeval* t1;
    gettimeofday(t1,0);
    *t=(double)(t1->tv_sec)*1000000.0 +  (double)(t1->tv_usec);
}
void stop_time(double *t)
{
    struct timeval* t1;
    gettimeofday(t1,0);
    *t=*t-(double)(t1->tv_sec)*1000000.0 - (double)(t1->tv_usec);
}

void show_global_time(char * who) {
    struct timeval t1;
    gettimeofday(&t1,0);
    fprintf(stderr, "time on %s %7.2f\n",who, t1.tv_sec * 1000000.0+(double) t1.tv_usec);
}
