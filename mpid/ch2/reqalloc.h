/* $Id$ */

#ifndef MPIREQALLOC
#define MPIREQALLOC

/* for memset().
   XXX instead of using memset() to preset a new handle with ZEROs (and setting
   many values explicitely afterwards), it might be more efficient to set just the 
   values to ZERO which are need to be ZERO in the function which allocates the
   handle. */
#ifndef WIN32
#include <string.h>
#else
#include <stdlib.h>
#endif

/* Allocation of handles */
#include "sbcnst2.h"
#include "ptrcvt.h"

/* Because we may need to provide integer index values for the handles
   in converting to/from Fortran, we provide a spot for a separate index
   free operation 

   If you initialize anything, also check the MPID_Request_init macro in
   req.h
 */

extern MPID_SBHeader MPIR_shandles;
extern MPID_SBHeader MPIR_rhandles;
/* These four are used to initialize a structure that is allocated
   off of the stack */
#define MPID_Recv_init( a ) {memset(a, 0, sizeof(MPIR_RHANDLE)); \
 (a)->handle_type = MPIR_RECV;MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
 (a)->self_index = 0;(a)->ref_count=1;}
#define MPID_PRecv_init( a ) {memset(a, 0, sizeof(MPIR_PRHANDLE)); \
 (a)->handle_type = MPIR_PERSISTENT_RECV;MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
 (a)->self_index = 0;(a)->ref_count=1;}
#define MPID_Send_init( a ) {memset(a, 0, sizeof(MPIR_SHANDLE)); \
 (a)->handle_type = MPIR_SEND;MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
 (a)->self_index = 0;(a)->ref_count=1;}
#define MPID_PSend_init( a ) {memset(a, 0, sizeof(MPIR_RSHANDLE)); \
 (a)->handle_type = MPIR_PERSISTENT_SEND;MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
 (a)->self_index = 0;(a)->ref_count=1;}

#ifdef MPIR_MEMDEBUG 
#define MPID_Recv_alloc( a ) {a=(MPIR_RHANDLE *)MALLOC(sizeof(MPIR_RHANDLE)); \
         memset(a,0,sizeof(MPIR_RHANDLE)); MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
         ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
         ((MPIR_COMMON*)(a))->handle_type = MPIR_RECV; }
#define MPID_Send_alloc( a ) {a=(MPIR_SHANDLE *)MALLOC(sizeof(MPIR_SHANDLE)); \
         memset(a,0,sizeof(MPIR_SHANDLE)); MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_SEND;}
#define MPID_PRecv_alloc( a ) {a=(MPIR_PRHANDLE *)MALLOC(sizeof(MPIR_PRHANDLE)); \
        memset(a,0,sizeof(MPIR_PRHANDLE)); MPIR_SET_COOKIE(&((a)->rhandle), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_PERSISTENT_RECV;}
#define MPID_PSend_alloc( a ) {a=(MPIR_PSHANDLE *)MALLOC(sizeof(MPIR_PSHANDLE)); \
        memset(a,0,sizeof(MPIR_PSHANDLE)); MPIR_SET_COOKIE(&((a)->shandle), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_PERSISTENT_SEND;}
#define MPID_Recv_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
        MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);FREE( a );}
#define MPID_Send_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
        MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);FREE( a );}
#define MPID_PRecv_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
        MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);FREE( a );}
#define MPID_PSend_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
        MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);FREE( a );}
#else
#define MPID_Recv_alloc( a ) {a = (MPIR_RHANDLE *)MPID_SBalloc(MPIR_rhandles); \
          memset(a,0,sizeof(MPIR_RHANDLE)); MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
         ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
         ((MPIR_COMMON*)(a))->handle_type = MPIR_RECV; }
#define MPID_Send_alloc( a ) {a = (MPIR_SHANDLE *)MPID_SBalloc(MPIR_shandles); \
         memset(a,0,sizeof(MPIR_SHANDLE)); MPIR_SET_COOKIE((a), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_SEND; }
#define MPID_PRecv_alloc( a ) {a = (MPIR_PRHANDLE *)MPID_SBalloc(MPIR_rhandles); \
         memset(a,0,sizeof(MPIR_PRHANDLE)); MPIR_SET_COOKIE(&((a)->rhandle), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_PERSISTENT_RECV; }
#define MPID_PSend_alloc( a ) {a = (MPIR_PSHANDLE *)MPID_SBalloc(MPIR_shandles); \
         memset(a,0,sizeof(MPIR_PSHANDLE)); MPIR_SET_COOKIE(&((a)->shandle), MPIR_REQUEST_COOKIE);\
        ((MPIR_COMMON*)(a))->self_index=0;((MPIR_COMMON*)(a))->ref_count=1;\
        ((MPIR_COMMON*)(a))->handle_type = MPIR_PERSISTENT_SEND; }
#define MPID_Recv_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
         MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);MPID_SBfree( MPIR_rhandles, a );}
#define MPID_Send_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
         MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);MPID_SBfree( MPIR_shandles, a );}
#define MPID_PRecv_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
         MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);MPID_SBfree( MPIR_rhandles, a );}
#define MPID_PSend_free( a ) {if (((MPIR_COMMON*)(a))->self_index) \
         MPIR_RmPointer(((MPIR_COMMON*)(a))->self_index);MPID_SBfree( MPIR_shandles, a );}
#endif

/* inserted for compatibility to original mpich */
#define MPID_RecvInit(a) MPID_Recv_init(a) 
#define MPID_PRecvInit(a) MPID_PRecv_init(a)
#define MPID_SendInit(a) MPID_Send_init(a)
#define MPID_PSendInit(a) MPID_PSend_init(a)
#define MPID_RecvAlloc(a) MPID_Recv_alloc(a)
#define MPID_SendAlloc(a) MPID_Send_alloc(a)
#define MPID_PRecvAlloc(a) MPID_PRecv_alloc(a)
#define MPID_PSendAlloc(a) MPID_PSend_alloc(a)
#define MPID_RecvFree(a) MPID_Recv_free(a)
#define MPID_SendFree(a) MPID_Send_free(a)
#define MPID_PRecvFree(a) MPID_PRecv_free(a)
#define MPID_PSendFree(a) MPID_PSend_free(a)

#endif
