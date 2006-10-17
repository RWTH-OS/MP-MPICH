/*--------------------------------------------------------------------------*/
/*                                                                          */
/* PhD-Project                                                              */
/* (c) 1998 Martin Schulz                                                   */
/*                                                                          */
/* Global header file: defines.h                                            */
/* Contains all global defines to ensure conistency across all modules      */
/*                                                                          */
/* Headerfile (exported to all other modules)                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#ifndef _GLOB_DEFINES
#define _GLOB_DEFINES


/*--------------------------------------------------------------------------*/
/* Defines to organize DLL imports / exports */

#ifdef WIN32
#ifdef _IMPL_DLL
#define DLLEXPORT __declspec(dllexport) 
#else
#define DLLEXPORT __declspec(dllimport)
#endif

#define DLLDECL __cdecl
#endif /* WIN32 */

#ifdef LINUX
#define DLLEXPORT
#define DLLDECL
#endif /* LINUX */



/*--------------------------------------------------------------------------*/
/* Various options */

#define SIMPLESYNC_NEWLOCK		/* use new and save locking algorithm */
#define SIMPLESYNC_NEWBARRIER	/* use new and save barrier algorithm */


/*--------------------------------------------------------------------------*/
/* Defines for debugging */

/*  Set WAITON to force <press key> before every step including description print

    Set PRINTWAIT to print information, but do not wait for key
  
    Set VERBOSE to get additional informarion
  
    Set DEBUG to get detailed debugging information (Serious performance loss possible)

    Set ASSERT to activate assertions
    Set INFO to activate configuration printouts
*/

/*#define WAITON */
/*#define VERBOSE */
/*#define DEBUG_SYNCMOD */
#define ASSERT
#define INFO


/* use the Macro from HLRC to measure the time (only Linux) */

#ifdef LINUX
#define HLRC_TIMING
#endif


/* collect statistics yes/no */

#define COLLECT_STATS


/* enable multi team mode for VM */

#define MULTI_TEAM


/*--------------------------------------------------------------------------*/
/* automatic rules / DO NOT EDIT */

#ifdef WAITON
#define VERBOSE	
#define WAITKEY {char charptr[10]; printf(" - Press <ENTER>\n"); gets(charptr); printf("\n");}
#else
#define WAITKEY printf("\n");
#endif

#ifdef VERBOSE
#define WAIT(s) {printf("\nWAIT:");printf(s);WAITKEY;}
#else
#define WAIT(s)
#endif

#ifdef DEBUG_SYNCMOD
#define ASSERT
#endif

#ifdef WIN32
#ifdef MULTI_TEAM
#error No MULTI_TEAM mode under WIN32 yet
#endif
#endif


/*--------------------------------------------------------------------------*/
/* ASM commands */

#ifdef LINUX
#define CPUID { asm volatile ("cpuid" : : : "%eax","%ebx","%ecx","%edx"); }
#endif

#ifdef HLRC_TIMING
#ifdef LINUX
/* trick taken from HLRC distribution / Liviu Iftode / Rutgers */
#define PROCESSOR_MHZ   450000
#define GETTIME(x)      asm volatile (".byte 0x0f, 0x31" : "=A" (x));
#endif
#endif

#ifdef WIN32
#define CPUID _asm _emit 0x0F 	_asm _emit 0xA2	  
#endif

#ifdef WIN32
static DWORD WCdummy2;
#define WCFLUSH { __asm {  xchg  eax, dword ptr [WCdummy2] } }
#endif

#ifdef LINUX
static unsigned WCdummy2;
#define WCFLUSHX {asm("xchg %%eax, WCdummy2" ::: "eax");}
#define WCFLUSH { asm("xchg %eax, WCdummy2"); } 
/* #define WCFLUSH { WCFLUSHX; WCFLUSHX; } */
#endif

#ifdef LINUX
/* the following lines are taken from /usr/src/include/sys/atomic.h */
/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
#define __atomic_fool_gcc(x) (*(volatile struct { int a[100]; } *)x)

#ifdef __SMP__
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

static __inline__ void myatomic_inc(volatile unsigned int *v)
{
	__asm__ __volatile__(
		LOCK "incl %0"
		:"=m" (__atomic_fool_gcc(v))
		:"m" (__atomic_fool_gcc(v)));
}
#endif



/*--------------------------------------------------------------------------*/
/* Some defines to be used in coordination with the SISCI interface */

#define NO_FLAGS     0
#define NO_CALLBACK  NULL


/*--------------------------------------------------------------------------*/
/* The End. */

#endif
