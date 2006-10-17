/* $Id$ */

/* This is the config.h-version for Win32. Win32 has no configure-like feature, this
   means you have to set this up manually to match your environment.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <sisci_api.h> header file.  */
#define HAVE_SISCI_API_H 1

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the -local library (-l-local).  */
/* #undef HAVE_LIB_LOCAL */


/* 
 * SMI specific defines
 */
#define INT_SHMSEG_BASESIZE 32768

/* Enable the use of C++ code (loop scheduling stuff) ? */
#define SMI_NOCPP 1

/* Disable the use of threads within SMI */
/* #undef DISABLE_THREADS */

/* Define the type of watchdog to use */
/* #undef WD_TYPE_NONE */
/* #undef WD_TYPE_SIGNAL */
#define WD_TYPE_THREAD 1
/* #undef WD_TYPE_CALLBACK */

/* Define the type of PCI-SCI adapter to use (only relevant for Scali, Dolphin supports autoprobe */
/* #undef D31x */
/* #undef D32x */
/* #undef D33x */

/* Which kind of SISCI is used, if any? */
/* #undef SCALI_SISCI */
#define DOLPHIN_SISCI 1
/* #undef NO_SISCI */

/* Where is the SISCI we use? SCI-MPICH looks for this one. */
#define SISCIDIR /path/to/SISCI

/* Use SCI or SMP, or maybe something else (somewhat related to SISCI_TYPE) */
/* #undef SMP */
#define SCIDEV_PRESENT 1

/* Very old SISCI versions have another prototype for SCIStoreBarrier() */
#define SCISTOREBARRIER_TWOARGS 1

/* Very old SISCI versions have *no* SCIInitialize() */
#define HAVE_SCIINITIALIZE 1

/* Size of the internal shared memory segments (one per node) */
#define INT_SHMSEG 262144

/* Historical: this one is always set nowadays. */
#define SMI_NONFIXED_MODE 1

/* SCI topology as determined during configure - adapt manually, if required. */
#define CONFIGURE_TOPOLOGY_TORUS 1
#define CONFIGURE_TOPOLOGY_NDIMS 2
#define CONFIGURE_TOPOLOGY_EXTENT_X 4
#define CONFIGURE_TOPOLOGY_EXTENT_Y 4
#define CONFIGURE_TOPOLOGY_EXTENT_Z 1
