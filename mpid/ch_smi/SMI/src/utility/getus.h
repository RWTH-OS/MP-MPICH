/* $Id$ */

#ifndef _SMI_GETUS_
#define _SMI_GETUS_

/* Die OS-IFDEFS muessen noch angepasst werden!!! */

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
#error Please use gcc!!!
/* get processor ticks */
void getticks(long long *us);
#pragma inline (getticks);
/* this macro returns a number of CPU ticks */
#define GETTICKS(t) getticks(t)

#else 
#ifdef X86 
#ifdef __GNUC__
/* Gnu gcc */
#define GETTICKS(t) asm ("push %%esi\n\t" "mov %0, %%esi" : : "r" (t)); \
                    asm ("push %eax\n\t" "push %edx"); \
                    asm ("rdtsc"); \
                    asm ("movl %eax, (%esi)\n\t" "movl %edx, 4(%esi)"); \
                    asm ("pop %edx\n\t" "pop %eax\n\t" "pop %esi"); 
#else
/* probably pgcc */
#define GETTICKS(t) asm ("push %%esi\n\t" "mov t, %%esi"); \
                    asm ("push %eax\n\t" "push %edx"); \
                    asm ("rdtsc"); \
                    asm ("movl %eax, (%esi)\n\t" "movl %edx, 4(%esi)"); \
                    asm ("pop %edx\n\t" "pop %eax\n\t" "pop %esi"); 
#endif
#elif defined X86_64

#define GETTICKS(t) getticks(t)

#elif defined ALPHA
#define GETTICKS(t) {\
        int cc;\
        *t = 0;\
        asm ("rpcc %0" : "=r"(cc));\
        *t = cc;\
        }
#endif

#endif /* SUNPRO_C */

/* this macro returns a time stamp as a multiple of 100ns */
#define GETUS(ts)   GETTICKS(ts); \
                     *ts /= CLOCK;

#endif /* WIN32 */

#endif /* _SMI_GETUS_ */
