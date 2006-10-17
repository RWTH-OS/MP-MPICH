#ifndef __MPID_GETUS
#define __MPID_GETUS


#ifdef WIN32
#define GETTICKS(t) QueryPerformanceCounter((LARGE_INTEGER*)t);

#else
/* binding for an assembler subroutine to read processor clocks 
   on x86 CPUs via the rdtsc command */

/* CLOCK is the clock frequency of the processor - it has to be
   set according to the current configuration to the CPU frequency 
   in 1e+7 Hz, i.e. for 450 MHz the setting would be 45 */
#define CLOCK    45 

/* Sun cc */
#ifdef __SUNPRO_C

/* get processor ticks */
void getticks(long long *us);
#pragma inline (getticks);

/* this macro returns a number of CPU ticks */
#define GETTICKS(t) getticks(t)

#else 
/* Gnu gcc */
#define GETTICKS(t) asm volatile ("push %%esi\n\t" "mov %0, %%esi" : : "r" (t)); \
                      asm volatile ("push %eax\n\t" "push %edx"); \
                      asm volatile ("rdtsc"); \
                      asm volatile ("movl %eax, (%esi)\n\t" "movl %edx, 4(%esi)"); \
                      asm volatile ("pop %edx\n\t" "pop %eax\n\t" "pop %esi"); 

#endif /* SUNPRO_C */

/* this macro returns a time stamp as a multiple of 100ns */
#define GETUS(ts)   GETTICKS(ts); \
                     *ts /= CLOCK;

#endif /* WIN32 */

#endif /* __MPID_GETUS */
