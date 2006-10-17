/* mpichconf.h.  Generated automatically by configure.  */
/* mpichconf.h.in.  Generated automatically from .tmp by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if the `long double' type works.  */
#define HAVE_LONG_DOUBLE 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
/* #undef RETSIGTYPE */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if Fortran functions are pointers to pointers */
/* #undef FORTRAN_SPECIAL_FUNCTION_PTR */

/* Define is C supports volatile declaration */
#define HAS_VOLATILE 1

/* Define if XDR libraries available */
/* #undef HAS_XDR */

/* Define if message catalog programs available */
#define HAVE_GENCAT 1

/* Define if getdomainname function available */
/* #undef HAVE_GETDOMAINNAME */

/* Define in gethostbyname function available */
/*#define HAVE_GETHOSTBYNAME */

/* Define if C has long long int */
/*#define HAVE_LONG_LONG_INT 0*/

/* Define if C supports long doubles */
#define HAVE_LONG_DOUBLE 1 

/* Define if msem_init function available */
/* #undef HAVE_MSEM_INIT */

/* Define if C does NOT support const */
/* #undef HAVE_NO_C_CONST */

/* Define if C supports prototypes (but isn't ANSI C) */
#define HAVE_PROTOTYPES 1

/* Define if uname function available */
/*#define HAVE_UNAME 1*/ 

/* Define if an int is smaller than void * */
#ifdef _WIN64
#define INT_LT_POINTER 1
#else
#undef INT_LT_POINTER
#endif 

/* Define if malloc returns void * (and is an error to return char *) */
#define MALLOC_RET_VOID 1

/* Define if MPE extensions are included in MPI libraries */
/*#define MPE_USE_EXTENSIONS 1*/

/* Define if MPID contains special case code for collective over world */
/* #undef MPID_COLL_WORLD */

/* Define if MPID supports ADI collective */
/* #undef MPID_USE_ADI_COLLECTIVE */

/* Define is ADI should maintain a send queue for debugging */
/* #undef MPI_KEEP_SEND_QUEUE */

/* Define if mpe debug features should NOT be included */
/* #undef MPI_NO_MPEDBG */

/* Define if struct msemaphore rather than msemaphore required */
/* #undef MSEMAPHORE_IS_STRUCT */

/* Define if void * is 8 bytes */
#ifdef _WIN64
#define POINTER_64_BITS 1
#else
#undef POINTER_64_BITS
#endif 

/* Define if stdarg can be used */
#define USE_STDARG 1

/* For Cray, define two word character descriptors in use */
/* #undef _TWO_WORD_FCD */

/* Define if extra traceback information should be kept */
/* #undef DEBUG_TRACE */

/* Define if Fortran is NOT available */
/* #undef MPID_NO_FORTRAN */

/* Define if memory debugging should be enabled */
/* #undef MPIR_MEMDEBUG */

/* Define if object debugging should be enabled */
/* #undef MPIR_OBJDEBUG */

/* Define if ptr conversion debugging should be enabled */
/* #undef MPIR_PTRDEBUG */

/* Define if ADI is ADI-2 (required!) */
#define MPI_ADI2 1

/* Define if mmap does not work correctly for anonymous memory */
/* #undef HAVE_NO_ANON_MMAP */

/* Define if signals reset to the default when used (SYSV vs BSD semantics).
   Such signals are essentially un-usable, because of the resulting race
   condition.  The fix is to use the sigaction etc. routines instead (they're
   usually available, since without them signals are entirely useless) */
/* #undef SIGNALS_RESET_WHEN_USED */

/* Define if MPI Structs should align on the largest basic element */
#define USE_BASIC_ALIGNMENT 1

/* The number of processors expected on an SMP.  Usually undefined */
/* #undef PROCESSOR_COUNT */

/* Define this to force a choice of shared memory allocator */
/* #undef SHMEM_PICKED */

/* Define this to force SysV shmat for shared memory allocator */
/* #undef USE_SHMAT */

/* Define this to force a choice for memory locking */
/* #undef LOCKS_PICKED */

/* Define this to force SysV semop for locks */
/* #undef USE_SEMOP */

/* Define if you have BSDgettimeofday.  */
/* #undef HAVE_BSDGETTIMEOFDAY */

/* Define if you have catclose.  */
/*#define HAVE_CATCLOSE 1*/

/* Define if you have catgets.  */
/*#define HAVE_CATGETS 1 */

/* Define if you have catopen.  */
/*#define HAVE_CATOPEN 1*/

/* Define if you have gethostname.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have gettimeofday.  */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have mmap.  */
/* #undef HAVE_MMAP */

/* Define if you have mutex_init.  */
/* #undef HAVE_MUTEX_INIT */

/* Define if you have nice.  */
/*#define HAVE_NICE 1*/

/* Define if you have semop.  */
/* #undef HAVE_SEMOP */

/* Define if you have shmat.  */
/* #undef HAVE_SHMAT */

/* Define if you have sigaction.  */
/*#define HAVE_SIGACTION 1*/

/* Define if you have sigmask.  */
/* #undef HAVE_SIGMASK */

/* Define if you have signal.  */
/* #undef HAVE_SIGNAL */

/* Define if you have sigprocmask.  */
/* #undef HAVE_SIGPROCMASK */

/* Define if you have sigset.  */
/* #undef HAVE_SIGSET */

/* Define if you have sysinfo.  */
/*#define HAVE_SYSINFO 1*/

/* Define if you have system.  */
#define HAVE_SYSTEM 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <mpproto.h> header file.  */
/* #undef HAVE_MPPROTO_H */

/* Define if you have the <netdb.h> header file.  */
/*#define HAVE_NETDB_H 1*/

/* Define if you have the <nl_types.h> header file.  */
/*#define HAVE_NL_TYPES_H 1*/

/* Define if you have the <signal.h> header file.  */
/*#define HAVE_SIGNAL_H 1*/

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/systeminfo.h> header file.  */
/*#define HAVE_SYS_SYSTEMINFO_H 1*/

/* Define if you have the <unistd.h> header file.  */
/*#define HAVE_UNISTD_H 1*/

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the rpc library (-lrpc).  */
/* #undef HAVE_LIBRPC */

/* Define if you have the thread library (-lthread).  */
/* #undef HAVE_LIBTHREAD */
