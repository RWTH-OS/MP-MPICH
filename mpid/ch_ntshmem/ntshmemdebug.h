#ifndef _SHMEM_DEBUG_H
#define _SHMEM_DEBUG_H
#include <stdio.h>

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#include "shpackets.h"
#include "mpid_debug.h"
/* whole file: MPID_PKT_TSH -> MPID_PKT_TSH */

int  MPID_Print_mode   ANSI_ARGS( ( FILE *, MPID_PKT_TSH * ) );
int  MPID_Print_packet ANSI_ARGS( ( FILE *, MPID_PKT_TSH * ) );
void MPID_Print_pkt_data ANSI_ARGS(( char *, char *, int ));
void MPID_SetMsgDebugFlag ANSI_ARGS((int));
int  MPID_GetMsgDebugFlag ANSI_ARGS((void));
void MPID_PrintMsgDebug   ANSI_ARGS((void));
void MPID_Print_rhandle   ANSI_ARGS(( FILE *, MPIR_RHANDLE * ));
void MPID_Print_shandle   ANSI_ARGS(( FILE *, MPIR_SHANDLE * ));

#ifdef MPID_DEBUG_ALL  
/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;
extern FILE *MPID_DEBUG_FILE;

#ifdef DEBUG_PRINT_SEND_PKT
#undef DEBUG_PRINT_SEND_PKT
#endif
#define DEBUG_PRINT_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, (pkt)->len );\
	MPID_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_TSH *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#ifdef DEBUG_PRINT_BASIC_SEND_PKT
#undef DEBUG_PRINT_BASIC_SEND_PKT
#endif
#define DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
		"[%d]%s ", MPID_MyWorldRank, msg );\
	MPID_Print_packet( MPID_DEBUG_FILE, \
			  (MPID_PKT_TSH *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#ifdef DEBUG_PRINT_FULL_SEND_PKT
#undef DEBUG_PRINT_FULL_SEND_PKT
#endif
#define DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, \
	       (pkt)->len );\
	MPID_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_TSH *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_TSH *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#ifdef DEBUG_PRINT_RECV_PKT
#undef DEBUG_PRINT_RECV_PKT
#endif
#define DEBUG_PRINT_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, (pkt)->head.tag, from_grank, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_TSH *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#ifdef DEBUG_PRINT_FULL_RECV_PKT
#undef DEBUG_PRINT_FULL_RECV_PKT
#endif
#define DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, (pkt)->head.tag, from, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_TSH *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_TSH *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#ifdef DEBUG_PRINT_PKT
#undef DEBUG_PRINT_PKT
#endif
#define DEBUG_PRINT_PKT(msg,pkt)    \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
   "[%d]%s (%s:%d)\n", MPID_MyWorldRank, msg, __FILE__, __LINE__ );\
    MPID_Print_packet( MPID_DEBUG_FILE, (pkt) );\
    }

#ifdef DEBUG_PRINT_PKT_DATA
#undef DEBUG_PRINT_PKT_DATA
#endif
#define DEBUG_PRINT_PKT_DATA(msg,pkt)\
    if (MPID_DebugFlag) {\
	MPID_Print_pkt_data( msg, (pkt)->buffer, len );\
	}

#ifdef DEBUG_PRINT_LONG_MSG
#undef DEBUG_PRINT_LONG_MSG
#endif
#define DEBUG_PRINT_LONG_MSG(msg,pkt)     \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, \
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",\
	   MPID_MyWorldRank, *(int *)mpid_send_handle->start, \
	   __FILE__, __LINE__ );\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)...\n", \
	    MPID_MyWorldRank, msg, __FILE__, __LINE__ );\
    MPID_Print_packet( MPID_DEBUG_FILE, \
		      (MPID_PKT_T*)(pkt) );\
    fflush( MPID_DEBUG_FILE );\
    }

#else
#define DEBUG_PRINT_PKT(msg,pkt)
#define DEBUG_PRINT_MSG(msg)
#define DEBUG_PRINT_MSG2(msg,val)
#define DEBUG_PRINT_ARGS(msg) 
#define DEBUG_PRINT_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_RECV_PKT(msg,pkt)
#define DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)
#define DEBUG_PRINT_PKT_DATA(msg,pkt)
#define DEBUG_PRINT_LONG_MSG(msg,pkt)     
#define DEBUG_TEST_FCN(fcn,msg)
#define DEBUG_INIT_STRUCT(s,size)
#endif


#endif
