#ifndef _TRALLOC
#define _TRALLOC

#if defined(HAVE_MPICHCONF_H) && !defined(MPICHCONF_INC)
/* This includes the definitions found by configure, and can be found in
   the library directory (lib/$ARCH/$COMM) corresponding to this configuration
 */
#define MPICHCONF_INC
#include "mpichconf.h"
#endif

/* Define MPIR_MEMDEBUG to enable these memory tracing routines */

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#if defined(MPIR_MEMDEBUG) || defined(_TR_SOURCE)
#define MALLOC(a)    MPID_trmalloc((size_t)(a),__LINE__,__FILE__)
#define CALLOC(a,b)  \
    MPID_trcalloc((unsigned)(a),(unsigned)(b),__LINE__,__FILE__)
#define FREE(a)      MPID_trfree(a,__LINE__,__FILE__)
#define NEW(a)        (a *)MALLOC(sizeof(a))

void MPID_trinit ANSI_ARGS(( int ));
void *MPID_trmalloc ANSI_ARGS(( size_t, int, char * ));
void MPID_trfree ANSI_ARGS(( void *, int, char * ));
int MPID_trvalid ANSI_ARGS(( char * ));
void MPID_trspace ANSI_ARGS(( int *, int * ));
void MPID_trdump ANSI_ARGS(( FILE * ));
void MPID_trSummary ANSI_ARGS(( FILE * ));
void MPID_trid ANSI_ARGS(( int ));
void MPID_trlevel ANSI_ARGS(( int ));
void MPID_trpush ANSI_ARGS(( int ));
void MPID_trpop ANSI_ARGS((void));
void MPID_trDebugLevel ANSI_ARGS(( int ));
void *MPID_trcalloc ANSI_ARGS(( unsigned, unsigned, int, char * ));
void *MPID_trrealloc ANSI_ARGS(( void *, int, int, char * ));
void MPID_trdumpGrouped ANSI_ARGS(( FILE * ));
void MPID_TrSetMaxMem ANSI_ARGS(( int ));
#else
/* Should these use size_t for ANSI? */
#define MALLOC(a)    malloc((unsigned)(a))
#define CALLOC(a,b)  calloc((unsigned)(a),(unsigned)(b))
#define FREE(a)      free((void *)(a))
#define NEW(a)    (a *)MALLOC(sizeof(a))
#endif

#endif
