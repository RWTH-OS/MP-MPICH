/* $Id$ 
 *
 * General definitions for the MPID layer.
 *
 */

#ifndef MPID_INC
#define MPID_INC

/* obsolete macro for non-ANSI C-compilers */
#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#define MPID_GET_DEV(grank) (MPID_devset->ndev == 1) ? MPID_devset->dev_list : \
         ((grank) >= 0) ? MPID_devset->dev[grank] : NULL;

/* translate global rank to device relative rank */
#define MPID_GET_DEVRANK(grank) dev->grank_to_devlrank[grank]

#if defined(HAVE_MPICHCONF_H) && !defined(MPICHCONF_INC)
/* This includes the definitions found by configure, and can be found in
   the library directory (lib/$ARCH/$COMM) corresponding to this configuration
 */
#define MPICHCONF_INC
#include "mpichconf.h"
#endif

#include "mpi.h"
#define MPIR_HAS_COOKIES
#include "cookie.h"

/* This include brings in any definitions needed by all that are relevant 
 * to all device.  For example, MPID_HAS_HETERO
 */
#include "adi2config.h"

#define MPID_MIN(a,b) ((a) < (b) ? (a) : (b))
#define MPID_MAX(a,b) ((a) > (b) ? (a) : (b))

#define MPID_HAS_DEBUG

/* Needed for MPI_COMM_WORLD and MPI_ERR_xxx */
#define MPID_ERROR(comm,err,str) MPID_Abort(0, err, " ", str)

#ifdef MPID_DEBUG_ALL
#define MPID_DEBUG_IFCODE(code)  if (MPID_DebugFlag) { code }
#define MPID_DEBUG_CODE(code) code 
#else
#define MPID_DEBUG_IFCODE(code)
#define MPID_DEBUG_CODE(code)
#endif

/* For debugging, use PRINTF, FPRINTF, SPRINTF, FPUTS.  This allows us to 
   grep for printf to find stray error messages that should be handled with
   the error message facility (errorstring/errmsg)
   */
#ifndef PRINTF
#define PRINTF printf
#define FPRINTF fprintf
#define SPRINTF sprintf
#define FPUTS   fputs
#endif

#include "mpid_threads.h"

/*
 * MPID_Aint is a type long enough to hold the address of a request in
 * all circumstances.  For homogeneous systems, this is whatever
 * a void * occupies.  For heterogeneous systems, we use 8 bytes.
 * Note that here, heterogeneous systems are ANY combination of systems
 * that may use heterogeneous address lengths, since the MPID_Aint
 * value is part of the request structure.
 *
 * Since some systems do not support 8 byte ints, we provide an assignment
 * routine MPID_AINT_SET(a,b) and MPID_AINT_GET(a,b)
 * These do
 * MPID_AINT_SET pkt.aint field = address
 * MPID_AINT_GET address = pkt.aint field
 * Just use = to assign MPID_Aint to MPID_Aint (e.g., when saving an MPID_Aint
 * in a request or placing in a pkt.
 */
/* #ifdef MPID_HAS_HETERO */
#if 0

#if SIZEOF_INT == 8 
#define MPID_INT8
#endif
#if SIZEOF_LONG == 8
#define MPID_LONG8
#endif

#ifdef MPID_INT8 
typedef int MPID_Aint;
#define MPID_AINT_SET(a,b) a = (MPID_Aint)(b)
#define MPID_AINT_GET(a,b) a = (void*)(b)
#elif  defined(MPID_LONG8) && !defined(POINTER_64_BITS)
typedef long MPID_Aint;
#define MPID_AINT_SET(a,b) a = (MPID_Aint)(b)
#define MPID_AINT_GET(a,b) a = (void *)(b)
#else 
#define MPID_AINT_IS_STRUCT
/* This is complicated by the need to set only the significant bits when
   getting the address */
typedef struct {unsigned low:32; int high:32; } MPID_Aint;
/* HP doesn't handle *& correctly; so we try without */
/* Note that we are using this because we may connect with a 64 bit system.
   This handles ONLY 32 and 64 bit systems.  The if test should be
   POINTER_64_BITS instead */
#ifndef POINTER_64_BITS
#define MPID_AINT_SET(a,b) (a).low = (unsigned)(b)
#define MPID_AINT_GET(a,b) (a) = (void *)(b).low
#else
#define MPID_AINT_SET(a,b) (a) = *(MPID_Aint *)&(b)
#define MPID_AINT_GET(a,b) *(MPID_Aint *)&(a) = *&(b)
#endif
#endif
#else /* Not MPID_HAS_HETERO */
typedef void * MPID_Aint;
#define DEBUG_H_INT(a)
#define MPID_AINT_SET(a,b) {\
    a = b;\
DEBUG_H_INT(fprintf( stderr, "[%d] Aint set %x <- %x\n", MPID_MyWorldRank, a, b ));\
	      }
#define MPID_AINT_GET(a,b) {\
a = b;\
DEBUG_H_INT(fprintf( stderr, "[%d] Aint get %x <- %x\n", MPID_MyWorldRank, a, b ));\
	  }
#endif

typedef long ASYNCSendId_t[4];
typedef long ASYNCRecvId_t[4];

/* Whether an operation should block or not */
typedef enum { MPID_NOTBLOCKING = 0, MPID_BLOCKING } MPID_BLOCKING_TYPE;

/* Heterogeneous data representations.  First, for the 
   message representation (we'd like to use an enum, but we can't give a
   bit length to an enum !)
   We fix that by using an in in the packet and an enum everywhere else.

   Here is an explanation for the different datatypes.

   An MPID_Msgrep_t describes "how a message is formatted", and is used 
   by the RECEIVER.  

 */
typedef enum { MPID_MSGREP_UNKNOWN = -1, 
	       MPID_MSGREP_RECEIVER = 0, 
	       MPID_MSGREP_XDR = 1,
	       MPID_MSGREP_SENDER = 2 } MPID_Msgrep_t;
#ifdef FOO
#define MPID_MSGREP_UNKNOWN	-1
/* Encoded in the receiver's native format (may be same as senders) */
#define MPID_MSGREP_RECEIVER	0
/* Encoded with XDR */
#define MPID_MSGREP_XDR		1
/* Encoded in the sender's native format */
#define MPID_MSGREP_SENDER	2
#endif

/* 
   For collective PACK operations, we current support (a subset) of
   three representations: homogeneous (OK), XDR, and "receiver makes right".
   These are NOT used for point-to-point operations.  Note that the
   numbers match the MSGREP values.

   OK implies homogeneous.
 */
/*
typedef enum { MPID_MSGFORM_OK = 0, MPID_MSGFORM_XDR = 1, 
	       MPID_MSGFORM_SENDER = 2 } MPID_Msgform_t;
 */
/* This is used in the MPI_TAG area of a receive status to indicate a
   cancelled message 
 */
#define MPIR_MSG_CANCELLED (-3)

#include "req.h"
#include "comm.h"
#include "datatype.h"
/* Heterogeneous only; needs MPID_INFO from dev? */
#include "chhetero.h"
#include "attach.h"
#include "objtrace.h"
#include "calltrace.h"

#include "mpi_error.h"
/* We need to reference MPIR_COMM_WORLD for some error handling */
#ifdef WIN32
#ifdef IN_MPICH_DLL
extern _declspec(dllexport) struct MPIR_COMMUNICATOR *MPIR_COMM_WORLD;
#else
extern _declspec(dllimport) struct MPIR_COMMUNICATOR *MPIR_COMM_WORLD;
#endif
#else
extern struct MPIR_COMMUNICATOR *MPIR_COMM_WORLD;
#endif


#define MPID_TAG_UB (1<<30)-1
#define MPID_MAX_CONTEXT_ID (1<<16)-1

/* 
 * A device may need to set the count and private_count fields of status
 * for things like communication with MPI_PROC_NULL.  This macro provides
 * a way to do that (some devices may have special needs and will need to
 * change this)
 */
#define MPID_ZERO_STATUS_COUNT(status) (status)->count = 0
/*
 * A device can also define how to set the status for an arbitrary value.
 */
/* #define MPID_STATUS_SET_ELEMENTS(status,datatype,count) */

/* rank and size regarding the predefined communicators, declared in adi2init.c */
/* is this #ifdef really still necessary ? boris */
#ifdef __cplusplus /* David Ashton */
extern "C" {
extern int MPID_MyWorldSize, MPID_MyWorldRank;
extern int MPID_MyHostSize,  MPID_MyHostRank;
extern int MPID_MyLocalSize, MPID_MyLocalRank;
extern int MPID_MyMetaSize,  MPID_MyMetaRank;
extern int MPID_MyAllSize,   MPID_MyAllRank;
};
#else
extern int MPID_MyWorldSize, MPID_MyWorldRank;
extern int MPID_MyHostSize,  MPID_MyHostRank;
extern int MPID_MyLocalSize, MPID_MyLocalRank;
extern int MPID_MyMetaSize,  MPID_MyMetaRank;
extern int MPID_MyAllSize,   MPID_MyAllRank;
#endif


#ifdef META
#ifdef VIOLAIO
extern int          MPID_MyLocalExtraSize, MPID_MyLocalExtraRank;
extern int          MPID_MyMetaExtraSize, MPID_MyMetaExtraRank;
#endif /* VIOLAIO */
#endif /* META */

/* External routines */
#include "mpid_bind.h"

/* Things that don't belong here, but are needed to develop code */
#define MPIR_ERR_MAY_BLOCK MPI_ERR_INTERN
/* mpimem is MALLOC/FREE/NEW */
/* #include "mpimem.h" */
/* Force files to include mpimem.h as needed */
/* #define MPIR_MEMDEBUG */

#if defined(NEEDS_STDLIB_PROTOTYPES)
#include <stdio.h>
/* 
   Some gcc installations have out-of-date include files and need these
   definitions to handle the "missing" prototypes.  This is NOT
   autodetected, but is provided and can be selected by using a switch
   on the options line.

   These are from stdlib.h, stdio.h, and unistd.h
 */
extern int fprintf(FILE*,const char*,...);
extern int printf(const char*,...);
extern int fflush(FILE *);
extern int fclose(FILE *);
extern int fscanf(FILE *,const char *,...);
extern int fputs(const char *,FILE *);
extern int sscanf(const char *, const char *, ... );
#include <sys/types.h>
extern void *memset(void *, int, size_t);
#endif

/* "Real" Rsend's are now possible via the 'ready' protocol */
#if 0
/* Following the Standard, we implement Rsend as just Send */
#define MPID_IrsendDatatype MPID_IsendDatatype
#define MPID_RsendDatatype MPID_SendDatatype
#endif

/* 
 * These macros define an interface between the device and the rest of the 
 * MPI code for attributes.
 *
 * MPID_ATTR_SET(struct MPIR_COMMUNICATOR *comm, int keyval, void *attr_value)
 * is called when the user sets an attribute value for any keyval.
 *
 * MPID_ATTR_GET(struct MPIR_COMMUNICATOR *comm, int keyval, void *attr_value)
 * is called when the user gets an attribute value The last argument is a 
 * pointer to a value, not a pointer to a pointer (store into the storage
 * defined by the user, don't change the pointer)
 *
 * MPID_KEYVAL_INIT()
 * The device should also call MPI_Keyval_create() for any keyvals that 
 * it wishes to be available for users.  Otherwise, the keyvals will have
 * value MPI_KEYVAL_INVALID.
 */
#define MPID_ATTR_SET(a,b,c)
#define MPID_ATTR_GET(a,b,c)
#define MPID_KEYVAL_INIT()

/* Definitions for the device only are now in mpiddev.h (link to 
   mpiddevbase.h for channel code) */


/* By putting these at the end, we reduce the chance that we run into a
   declaration of malloc in some header file */
/* But not enough! */
#if defined(MPIR_MEMDEBUG) && !defined(malloc) && defined(MPIR_CATCH_MALLOC)
#define malloc $'Use mpimem.h'$
#define free   $'Use mpimem.h'$
#define calloc $'Use mpimem.h'$
#endif

#endif
