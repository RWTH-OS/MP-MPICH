/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _MPID_SBCNST
#define _MPID_SBCNST

#include <stdio.h>

#if defined(HAVE_MPICHCONF_H) && !defined(MPICHCONF_INC)
/* This includes the definitions found by configure, and can be found in
   the library directory (lib/$ARCH/$COMM) corresponding to this configuration
 */
#define MPICHCONF_INC
#include "mpichconf.h"
#endif
/* for THREAD definitions */
#include "mpid.h"
/* Needed for MPI_Aint (int that is the size of void *) */
#include "mpi.h"

#ifdef MPID_DEBUG_ALL
#define SB_DEBUG
#define SB_DEBUG1
#endif

/*
 * struct/type definitions
 */

/* If you change this, you must change the format spec (%lx) to match */
typedef long PointerInt;

/* This is the allocation unit. */
typedef struct _sbialloc {
    struct _sbialloc *next;
    int              nbytes, nballoc;
    int              nbinuse;
} MPID_SBiAlloc;

/* Blocks are linked together; they are (much) larger than this */
#ifdef SB_DEBUG
typedef struct {
    long sentinal_1;
    char *next;
    long sentinal_2;
} MPID_SBblock;
#else
typedef struct {
    char *next;
} MPID_SBblock;
#endif

/* Context for fixed-block allocator */
struct _MPID_SBHeader {
    MPID_THREAD_DS_LOCK_DECLARE; /* Lock variable for thread locking */
    MPID_SBiAlloc *blocks;	 /* allocated storage */
    MPID_SBblock  *avail;        /* fixed blocks (of size sizeb) to provide */
    int nbfree, nballoc,     /* blocks free and in use */
	sizeb,               /* sizes in bytes */
	sizeincr;            /* # of blocks to allocate when more needed */
};
typedef struct _MPID_SBHeader *MPID_SBHeader;


/*
 * prototypes 
 */
extern MPID_SBHeader MPID_SBinit ( int, int, int );
extern void  MPID_SBfree ( MPID_SBHeader, void * );
extern void *MPID_SBalloc ( MPID_SBHeader );
extern void  MPID_SBPrealloc ( MPID_SBHeader, int );
extern void  MPID_SBdestroy ( MPID_SBHeader );
extern void  MPID_SBrelease ( MPID_SBHeader, void * );
extern void  MPID_SBFlush ( MPID_SBHeader );
extern void  MPID_SBDump ( FILE *, MPID_SBHeader );
extern void  MPID_SBReleaseAvail ( MPID_SBHeader );
extern void  MPID_SBvalid ( MPID_SBHeader );


/* Chameleon/PETSc includes memory tracing functions that can be used
   to track storage leaks.  This code chooses that or the copy that 
   has been placed into mpich/util/tr.c 
 */
#ifndef MALLOC

#if defined(MPIR_MEMDEBUG)
/* Use MPI tr version of MALLOC/FREE */
#include "tr2.h"
/* Also replace the SB allocators so that we can get the trmalloc line/file
   tracing. */
#define MPID_SBinit(a,b,c) ((void *)(a))
#define MPID_SBalloc(a)    MPID_trmalloc((size_t)(a),__LINE__,__FILE__)
#define MPID_SBfree(a,b)   MPID_trfree((char *)(b),__LINE__,__FILE__)
#define MPID_SBdestroy(a)
#else

/* We also need to DECLARE malloc etc here.  Note that P4 also declares
   some of these, and thus if P4 in including this file, we skip these
   declarations ... */
#ifndef P4_INCLUDED

#if HAVE_STDLIB_H || STDC_HEADERS
#include <stdlib.h>

#else
#ifdef __STDC__
extern void 	*calloc(/*size_t, size_t*/);
extern void	free(/*void * */);
extern void	*malloc(/*size_t*/);
#elif defined(MALLOC_RET_VOID)
extern void *malloc();
extern void *calloc();
#else
extern char *malloc();
extern char *calloc();
/* extern int free(); */
#endif /* __STDC__ */
#endif /* HAVE_STDLIB_H || STDC_HEADERS */
#endif /* !defined(P4_INCLUDED) */

#define MALLOC(a)    malloc((unsigned)(a))
#define CALLOC(a,b)  calloc((unsigned)(a),(unsigned)(b))
#define FREE(a)      free((char *)(a))
#define NEW(a)    (a *)MALLOC(sizeof(a))
#endif /*MPIR_MEMDEBUG*/
#endif /*MALLOC*/

#endif
