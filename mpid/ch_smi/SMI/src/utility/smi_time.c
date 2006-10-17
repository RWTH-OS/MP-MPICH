/* $Id$ */

#include <sys/time.h>
#ifdef X86_64
#include <time.h>
#endif
#include <limits.h>

#include "env/smidebug.h"
#include "smi_time.h"
#include "query.h"
#ifndef SPARC
#include "getus.h"
#endif

static double nsecs_per_tick;         /*  time period per tick */

static int smi_second, smi_microsecond;

#ifdef X86_64
/* The byte-sequence is the RDTSC instruction, which puts the high order 32bits into
   edx and the low order ones into eax. */
static inline void getticks(longlong_t *us)
{
   unsigned hi,lo;
   __asm __volatile (".byte 0x0f; .byte 0x31; movl %%edx,%0; movl %%eax,%1;" : "=g" (hi), "=g" (lo) :: "eax", "edx");
   *us = ((longlong_t)hi << 32) + lo;
}
#endif

/********************************************************************************/
/* returns the wall-clock time in seconds                                       */
/********************************************************************************/
double SMI_Wtime()
{
#ifdef SPARC
  int iTime;
  longlong_t ticks = gethrtime();
 
  iTime = ticks >> 16;
 
  return (((double)iTime) * ((double)256 * 256) / 1e+9);
#elif defined ALPHA
  long ticks;
  GETTICKS(&ticks);
  return (nsecs_per_tick*ticks/1e+9);
#elif defined IA64
  struct timeval tp;
  struct timezone tzp;

  gettimeofday(&tp,&tzp);
  return ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
 
#else
  longlong_t ticks;
  
  GETTICKS(&ticks);
 
  return (nsecs_per_tick*ticks/1e+9);
#endif
}

/********************************************************************************/
/* returns a wall-clock time-stamp (not seconds, but arbitrary 'ticks'!)        */
/********************************************************************************/
long SMI_Wticks()
{
#ifdef SPARC
  long iTime;
  longlong_t ticks = gethrtime();
 
  iTime = ticks >> 16;
 
  return iTime;
#elif defined ALPHA
  long ticks;
  GETTICKS(&ticks);
  return ticks;
#elif defined IA64
  static longlong_t dummy_ticks = 0;

  DWARNING("SMI_Wticks not yet implemented on IA-64");
  return dummy_ticks++;
#else
  longlong_t ticks;
  
  GETTICKS(&ticks);
 
  return (long)ticks;
#endif
}

/********************************************************************************/
/* for high resolution/low overhead time measurement, return only the CPU ticks */
/********************************************************************************/
void SMI_Get_ticks(void *ticks)
{
#ifdef SPARC
  *(hrtime_t *)ticks = gethrtime();
#elif defined IA64
  static longlong_t dummy_ticks = 0;

  DWARNING("SMI_Get_ticks not yet implemented on IA-64");
  *(longlong_t *)ticks = dummy_ticks++;
#else
  GETTICKS((longlong_t *)ticks);
#endif
}

/********************************************************************************/
/* returns the current system time in seconds and microseconds                  */
/********************************************************************************/
smi_error_t SMI_Get_timer(int* sec, int* microsec)
{
  double time = SMI_Wtime();

  *sec = (int) time;
  *microsec = (int)(time - ((int) time) * 1000000);
  
  smi_second = *sec;
  smi_microsecond = *microsec;

  return(SMI_SUCCESS);
}

/********************************************************************************/
/* returns the elapsed time span since SMI_Get_timer or SMI_Get_timespan has    */
/* been call the last time                                                      */
/********************************************************************************/
smi_error_t SMI_Get_timespan(int* sec, int* microsec)
{
  int smi_sec_last, smi_microsec_last, dummy0, dummy1;

  smi_sec_last      = smi_second;
  smi_microsec_last = smi_microsecond;
  
  SMI_Get_timer(&dummy0, &dummy1);
  
  *sec      = smi_second - smi_sec_last;
  *microsec = smi_microsecond - smi_microsec_last;

  if (*microsec < 0) {
    (*sec)--;
    (*microsec) = 1000000 + *microsec;
  }
  
  return(SMI_SUCCESS);
}

/* this is a mini SMI_Wtime() replacement for internal timeout mechanisms */
int _smi_get_seconds () {
    struct timeval tv;
    
    gettimeofday(&tv, 0);
    return ((int)tv.tv_sec);
}

/********************************************************************************/
/* initialization function of this module                                       */
/********************************************************************************/
void _smi_init_timer()
{
  int cpu_frq;
  
  /* get CPU information */
  SMI_Query (SMI_Q_SYS_CPUFREQ, 0, (void *)&cpu_frq);
  nsecs_per_tick = 1e+3/((double)cpu_frq);
  
  SMI_Get_timer(&smi_second, &smi_microsecond);
}



