/* $Id$ */

#ifndef MPID_SMI_TYPES_H
#define MPID_SMI_TYPES_H


/* 
 * common basic types 
 */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef WIN32
typedef short boolean;
#endif

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#if defined(MPI_LINUX) || defined(MPI_LINUX_ALPHA) || defined(MPI_LINUX_IA64) || defined(MPI_LINUX_X86_64)
typedef long long int longlong_t;
#define LLONG_MAX 9223372036854775807LL
#elif defined(WIN32)
#include <wtypes.h>
#include <limits.h>
typedef  unsigned short ushort;
typedef LONGLONG longlong_t;
typedef _int32 int32_t;
#ifndef LLONG_MAX
 #define LLONG_MAX 9223372036854775807
#endif
#endif

#ifdef WIN32
typedef unsigned int  uint;
typedef unsigned long ulong;
#endif

/* for resource management */
typedef enum { LOCAL = 0, RMT_MAP = 1, RMT_CNCT = 2, ANY = 3 } MPID_SMI_rsrc_type_t;


#endif /* MPID_SMI_TYPES_H */



/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
