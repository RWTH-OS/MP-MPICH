/*
 * $Id: mpxdebug.h 4397 2006-01-30 10:41:47Z carsten $
 *
 */

#ifndef _MPX_DEBUG_H
#define _MPX_DEBUG_H
#include <stdio.h>

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#include "mpxpackets.h"

int  MPID_MPX_Print_mode   ANSI_ARGS( ( FILE *, MPID_PKT_T * ) );
int  MPID_MPX_Print_packet ANSI_ARGS( ( FILE *, MPID_PKT_T * ) );
void MPID_MPX_Print_pkt_data ANSI_ARGS(( char *, char *, int ));
void MPID_SetMsgDebugFlag ANSI_ARGS((int));
int  MPID_GetMsgDebugFlag ANSI_ARGS((void));
void MPID_PrintMsgDebug   ANSI_ARGS((void));
void MPID_MPX_Print_rhandle   ANSI_ARGS(( FILE *, MPIR_RHANDLE * ));
void MPID_MPX_Print_shandle   ANSI_ARGS(( FILE *, MPIR_SHANDLE * ));

#ifdef MPID_DEBUG_ALL  

/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;
extern FILE *MPID_DEBUG_FILE;
extern FILE *MPID_TRACE_FILE;


#define MPID_MPX_DEBUG_TEST_FCN(fcn,msg) {\
    if (!fcn) {\
      FPRINTF( stderr, "Bad function pointer (%s) in %s at %d\n",\
	       msg, __FILE__, __LINE__);\
      MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", "Bad function pointer" );\
      }}
/* This is pretty expensive.  It should be an option ... */
#ifdef DEBUG_INIT_MEM
#define MPID_MPX_DEBUG_INIT_STRUCT(s,size) memset(s,0xfa,size)		
#else
#define MPID_MPX_DEBUG_INIT_STRUCT(s,size)
#endif

#define MPID_MPX_DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	     ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, __FILE__, __LINE__ ); \
    fflush( MPID_DEBUG_FILE );}}
	    
#define MPID_MPX_DEBUG_PRINT_MSG2(msg,val)\
{if (MPID_DebugFlag) {\
    char localbuf[1024]; sprintf( localbuf, msg, val );\
    MPID_MPX_DEBUG_PRINT_MSG(localbuf);}}

#define MPID_MPX_DEBUG_PRINT_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
		 ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, (pkt)->tag, dest_dev_lrank, \
	       (pkt)->context_id, (pkt)->len );\
	MPID_MPX_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_MPX_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
		 "[%d]%s ", ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg ); \
	MPID_MPX_Print_packet( MPID_DEBUG_FILE, \
			  (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_MPX_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
		 ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, \
	       (pkt)->len );\
	MPID_MPX_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_MPX_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}


	    
#define MPID_MPX_DEBUG_PRINT_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, (pkt)->head.tag, from_grank, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_MPX_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_MPX_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, (pkt)->head.tag, from, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_MPX_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_MPX_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_MPX_DEBUG_PRINT_PKT(msg,pkt)    \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
     "[%d]%s (%s:%d)\n", ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, __FILE__, __LINE__ ); \
    MPID_MPX_Print_packet( MPID_DEBUG_FILE, (pkt) );\
    }

#define MPID_MPX_DEBUG_PRINT_PKT_DATA(msg,pkt)\
    if (MPID_DebugFlag) {\
	MPID_MPX_Print_pkt_data( msg, (pkt)->buffer, len );\
	}

#define MPID_MPX_DEBUG_PRINT_LONG_MSG(msg,pkt)     \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, \
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",\
	     ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, *(int *)mpid_send_handle->start, \
	   __FILE__, __LINE__ );\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)...\n", \
	     ((MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg, __FILE__, __LINE__ ); \
    MPID_MPX_Print_packet( MPID_DEBUG_FILE, \
		      (MPID_PKT_T*)(pkt) );\
    fflush( MPID_DEBUG_FILE );\
    }

#define MPID_MPX_DEBUG_PRINT_SHANDLE(file,handle) \
if (MPID_DebugFlag) {\
    MPID_MPX_Print_shandle( file, handle );\
    }

#define MPID_MPX_DEBUG_PRINT_RHANDLE(file,handle) \
if (MPID_DebugFlag) {\
    MPID_MPX_Print_rhandle( file, handle );\
    }

#else /* MPID_DEBUG_ALL */

#define MPID_MPX_DEBUG_PRINT_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_MSG(msg)
#define MPID_MPX_DEBUG_PRINT_MSG2(msg,val)
#define MPID_MPX_DEBUG_PRINT_ARGS(msg) 
#define MPID_MPX_DEBUG_PRINT_SEND_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_RECV_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_PKT_DATA(msg,pkt)
#define MPID_MPX_DEBUG_PRINT_LONG_MSG(msg,pkt)     
#define MPID_MPX_DEBUG_TEST_FCN(fcn,msg)
#define MPID_MPX_DEBUG_INIT_STRUCT(s,size)
#define MPID_MPX_DEBUG_PRINT_SHANDLE(file,handle)
#define MPID_MPX_DEBUG_PRINT_RHANDLE(file,handle)

#endif /* MPID_DEBUG_ALL */

#endif
