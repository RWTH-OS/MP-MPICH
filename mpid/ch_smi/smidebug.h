#ifndef _MPID_SMI_DEBUG
#define _MPID_SMI_DEBUG

#include "mpid.h"

#include "smipackets.h"

#define MPID_INFO(msg)  if (MPID_SMI_cfg.VERBOSE && !MPID_SMI_myid) { \
	fprintf (stdout, msg); fflush (stdout); }
#define MPID_SMI_NOTICE(msg)  if (MPID_SMI_cfg.VERBOSE) { \
	fprintf (stdout, "[%d] "msg"\n", MPID_SMI_myid); fflush (stdout); }

#ifdef MPID_DEBUG_ALL  
/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;
extern FILE *MPID_DEBUG_FILE;

int  MPID_SMI_Print_mode   ( FILE *, MPID_PKT_T * );
int  MPID_SMI_Print_packet ( FILE *, MPID_PKT_T * );
void MPID_SMI_Print_pkt_data ( char *, char *, int );

#define MPID_SMI_DEBUG_PRINT_SEND_PKT(msg,pkt,dest)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SMI_myid, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, (pkt)->len );\
	MPID_SMI_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt,dest)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
		"[%d] %s to %d\n", MPID_SMI_myid, msg, dest );\
	MPID_SMI_Print_packet( MPID_DEBUG_FILE, \
			  (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, " (%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SMI_myid, msg, (pkt)->tag, dest, \
	       (pkt)->context_id, \
	       (pkt)->len );\
	MPID_SMI_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_SMI_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SMI_myid, msg, (pkt)->head.tag, from_grank, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_SMI_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT(msg,pkt,partner)\
    {if (MPID_DebugFlag) {\
        FPRINTF( MPID_DEBUG_FILE,\
"[%d] %s, partner = %d ", \
	       MPID_SMI_myid, msg, partner ); \
	FPRINTF( MPID_DEBUG_FILE, " (%s:%d)\n", __FILE__, __LINE__ );\
        MPID_SMI_Print_packet( MPID_DEBUG_FILE,\
                          (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_ALLOC() \
    {if (MPID_DebugFlag) {\
       FPRINTF( MPID_DEBUG_FILE,\
"[%d] Allocated %d bytes at %p for rndv msg from %d, using ", \
		MPID_SMI_myid, tlen, addr, sender );\
       if (MPID_SMI_use_localseg[sender]) \
	   FPRINTF( MPID_DEBUG_FILE, "local shmem, offset = %d\n", \
		 (int) ((size_t)addr - (size_t)MPID_SMI_rndv_shmempool)); \
       else \
	   FPRINTF( MPID_DEBUG_FILE, "sci memory, offset = %d\n", \
		 (int) ((size_t)addr - (size_t)MPID_SMI_rndv_scipool)); \
       fflush( MPID_DEBUG_FILE ); \
    }}

#define MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_FREE() \
    {if (MPID_DebugFlag) {\
       FPRINTF( MPID_DEBUG_FILE,\
"[%d] Freeing memory for rndv msg from %d, using ",\
                MPID_SMI_myid, from_grank );\
       if (MPID_SMI_use_localseg[from_grank]) \
           FPRINTF( MPID_DEBUG_FILE, "local shmem, offset = %d\n", \
	         recv_pkt.sgmt_offset ); \
       else \
           FPRINTF( MPID_DEBUG_FILE, "sci memory, offset = %d\n", \
	         recv_pkt.sgmt_offset ); \
    }}


#define MPID_SMI_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	FPRINTF( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_SMI_myid, msg, (pkt)->head.tag, from, \
	       (pkt)->head.context_id, \
	       (pkt)->head.len );\
	MPID_SMI_Print_mode( MPID_DEBUG_FILE, (MPID_PKT_T *)(pkt) );\
	FPRINTF( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	MPID_SMI_Print_packet( MPID_DEBUG_FILE,\
			  (MPID_PKT_T *)(pkt) );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define MPID_SMI_DEBUG_PRINT_PKT(msg,pkt)    \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
   "[%d]%s (%s:%d)\n", MPID_SMI_myid, msg, __FILE__, __LINE__ );\
    MPID_SMI_Print_packet( MPID_DEBUG_FILE, (pkt) );\
    }

#define MPID_SMI_DEBUG_PRINT_PKT_DATA(msg,pkt)\
    if (MPID_DebugFlag) {\
	MPID_SMI_Print_pkt_data( msg, (pkt)->buffer, len );\
	}

#define MPID_SMI_DEBUG_PRINT_LONG_MSG(msg,pkt)     \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, \
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",\
	   MPID_SMI_myid, *(int *)mpid_send_handle->start, \
	   __FILE__, __LINE__ );\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)...\n", \
	    MPID_SMI_myid, msg, __FILE__, __LINE__ );\
    MPID_SMI_Print_packet( MPID_DEBUG_FILE, \
		      (MPID_PKT_T*)(pkt) );\
    fflush( MPID_DEBUG_FILE );\
    }

#define MPID_SMI_DEBUG_PRINT_COPY_MSG(msg,d,s,c) \
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
            "[%d] About to copy with %s to 0x%p from 0x%p (n=%d) (%s:%d)...\n", \
        MPID_SMI_myid, msg, d, s, c, __FILE__, __LINE__);\
    fflush( MPID_DEBUG_FILE ); }\
}


#define MPID_SMI_DEBUG_TEST_FCN(fcn,msg) {\
    if (!fcn) {\
      FPRINTF( stderr, "Bad function pointer (%s) in %s at %d\n",\
	       msg, __FILE__, __LINE__);\
      MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", "Bad function pointer" );\
      }}
/* This is pretty expensive.  It should be an option ... */
#ifdef DEBUG_INIT_MEM
#define MPID_SMI_DEBUG_INIT_STRUCT(s,size) memset(s,0xfa,size)		
#else
#define MPID_SMI_DEBUG_INIT_STRUCT(s,size)
#endif

#define MPID_SMI_DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	    MPID_SMI_myid, msg, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}}
	    
#define MPID_SMI_DEBUG_PRINT_MSG2(msg,val)\
{if (MPID_DebugFlag) {\
    char localbuf[1024]; sprintf( localbuf, msg, val );\
    MPID_SMI_DEBUG_PRINT_MSG(localbuf);}}
	    
#else

/* debug output turned off */
#define MPID_SMI_DEBUG_PRINT_PKT(msg,pkt)
#define MPID_SMI_DEBUG_PRINT_SEND_PKT(msg,pkt,dest)
#define MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt,dest)
#define MPID_SMI_DEBUG_PRINT_FULL_SEND_PKT(msg,pkt)
#define MPID_SMI_DEBUG_PRINT_RECV_PKT(msg,pkt)
#define MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT(msg,pkt,partner)
#define MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_ALLOC() 
#define MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_FREE()
#define MPID_SMI_DEBUG_PRINT_FULL_RECV_PKT(msg,pkt)
#define MPID_SMI_DEBUG_PRINT_PKT_DATA(msg,pkt)
#define MPID_SMI_DEBUG_PRINT_LONG_MSG(msg,pkt)     
#define MPID_SMI_DEBUG_PRINT_COPY_MSG(msg,a,b,c) 
#define MPID_SMI_DEBUG_TEST_FCN(fcn,msg)
#define MPID_SMI_DEBUG_INIT_STRUCT(s,size)
#define MPID_SMI_DEBUG_PRINT_MSG(msg)
#define MPID_SMI_DEBUG_PRINT_MSG2(msg,val)

#endif /* MPID_DEBUG_ALL */

#endif
