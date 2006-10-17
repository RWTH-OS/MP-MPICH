/* $Id$ */
#ifndef _1351145_SHMEMDEBUG_H
#define _1351145_SHMEMDEBUG_H

#include "mpid.h"
#include "shmempackets.h"
#include "shmemcommon.h"
#include "shmemdef.h"

#ifdef MPID_DEBUG_ALL  
int  MPID_SHMEM_Print_mode   ( FILE *, MPID_PKT_T * );
int  MPID_SHMEM_Print_packet ( FILE *, MPID_PKT_T * );
void MPID_SHMEM_Print_pkt_data ( char *, char *, int );

/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;
extern FILE *MPID_DEBUG_FILE;

#define MPID_SHMEM_DEBUG_PRINT_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SHMEM_rank, msg, (pkt)->tag, (pkt)->to, \
	       (pkt)->context_id, (pkt)->len );\
	MPID_SHMEM_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
		"[%d] %s\n", MPID_SHMEM_rank, msg );\
	MPID_SHMEM_Print_packet( MPID_DEBUG_FILE, \
			  (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, " (%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SHMEM_rank, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, \
	       (pkt)->len );\
	MPID_SHMEM_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_SHMEM_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SHMEM_rank, msg, (pkt)->head.tag, from_dev_lrank, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_SHMEM_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT(msg,pkt,partner)\
    {if (MPID_DebugFlag) {\
        FPRINTF( MPID_DEBUG_FILE,\
"[%d] %s, partner = %d ", \
	       MPID_SHMEM_rank, msg, partner ); \
	FPRINTF( MPID_DEBUG_FILE, " (%s:%d)\n", __FILE__, __LINE__ );\
        MPID_SHMEM_Print_packet( MPID_DEBUG_FILE,\
                          (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SHMEM_rank, msg, (pkt)->head.tag, from, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_SHMEM_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_SHMEM_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SHMEM_DEBUG_PRINT_PKT(msg,pkt)    \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
   "[%d]%s (%s:%d)\n", MPID_SHMEM_rank, msg, __FILE__, __LINE__ );\
    MPID_SHMEM_Print_packet( MPID_DEBUG_FILE, (pkt) );\
    }

#define MPID_SHMEM_DEBUG_PRINT_PKT_DATA(msg,pkt)\
    if (MPID_DebugFlag) {\
	MPID_SHMEM_Print_pkt_data( msg, (pkt)->buffer, len );\
	}

#define MPID_SHMEM_DEBUG_PRINT_LONG_MSG(msg,pkt)     \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, \
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",\
	   MPID_SHMEM_rank, *(int *)mpid_send_handle->start, \
	   __FILE__, __LINE__ );\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)...\n", \
	    MPID_SHMEM_rank, msg, __FILE__, __LINE__ );\
    MPID_SHMEM_Print_packet( MPID_DEBUG_FILE, \
		      (MPID_PKT_T*)(pkt) );\
    fflush( MPID_DEBUG_FILE );\
    }

#define MPID_SHMEM_DEBUG_PRINT_COPY_MSG(msg,a,b,c) \
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
            "[%d] About to copy with %s to 0x%lx from 0x%lx (n=%d) (%s:%d)...\n", \
        MPID_SHMEM_rank, msg, (long)a, (long)b, c, __FILE__, __LINE__);\
    fflush( MPID_DEBUG_FILE ); }\
}

#define MPID_SHMEM_DEBUG_TEST_FCN(fcn,msg) {\
    if (!fcn) {\
      FPRINTF( stderr, "Bad function pointer (%s) in %s at %d\n",\
	       msg, __FILE__, __LINE__);\
      MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", "Bad function pointer" );\
      }}

/* This is pretty expensive.  It should be an option ... */
#ifdef DEBUG_INIT_MEM
#define MPID_SHMEM_DEBUG_INIT_STRUCT(s,size) memset(s,0xfa,size)		
#else
#define MPID_SHMEM_DEBUG_INIT_STRUCT(s,size)
#endif

#define MPID_SHMEM_DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	    MPID_SHMEM_rank, msg, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}}
	    
#define MPID_SHMEM_DEBUG_PRINT_MSG2(msg,val)\
{if (MPID_DebugFlag) {\
    char localbuf[1024]; sprintf( localbuf, msg, val );\
    MPID_SHMEM_DEBUG_PRINT_MSG(localbuf);}}
	    

#else

/* debug output turned off */
#define MPID_SHMEM_DEBUG_PRINT_MSG(msg)
#define MPID_SHMEM_DEBUG_PRINT_MSG2(msg,val)
#define MPID_SHMEM_DEBUG_PRINT_ARGS(msg) 
#define MPID_SHMEM_DEBUG_TEST_FCN(fcn,msg)
#define MPID_SHMEM_DEBUG_INIT_STRUCT(s,size)
#define MPID_SHMEM_DEBUG_PRINT_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_SEND_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_RECV_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT(msg,pkt,partner)
#define MPID_SHMEM_DEBUG_PRINT_RNDV_ADDRESS() 
#define MPID_SHMEM_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_PKT_DATA(msg,pkt)
#define MPID_SHMEM_DEBUG_PRINT_LONG_MSG(msg,pkt)     
#define MPID_SHMEM_DEBUG_PRINT_COPY_MSG(msg,a,b,c) 

#endif /* MPID_DEBUG_ALL */

#endif /* _1351145_SHMEMDEBUG_H */

