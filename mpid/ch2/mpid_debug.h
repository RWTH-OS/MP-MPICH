#ifndef _MPID_DEBUG_H
#define _MPID_DEBUG_H
#include <stdio.h>

#define CH_MAX_DEBUG_LINE 128
#define CH_LAST_DEBUG 128

#ifdef MPID_DEBUG_ALL  
void MPID_SetMsgDebugFlag (int);
int  MPID_GetMsgDebugFlag (void);
void MPID_PrintMsgDebug   (void);
void MPID_Print_rhandle   ( FILE *, MPIR_RHANDLE * );
void MPID_Print_shandle   ( FILE *, MPIR_SHANDLE * );

/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;
extern FILE *MPID_DEBUG_FILE;

/* Use these instead of printf to simplify finding stray error messages */
#ifndef FPRINTF
#define FPRINTF fprintf
#define PRINTF printf
#define SPRINTF sprintf
#endif

#ifdef MEMCPY
#undef MEMCPY
#endif
#define MEMCPY(a,b,c)\
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, \
	    "[%d]R About to copy to %p from %p (n=%d) (%s:%d)...\n", \
	MPID_MyWorldRank, a, b, c, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE ); }\
memcpy( a, b, c );}

/* Print standard send/recv args */
#define DEBUG_PRINT_ARGS(msg) \
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
   "[%d]%s for tag = %d, source = %d, ctx = %d, (%s:%d)\n", \
	    MPID_MyWorldRank, msg, tag, src_lrank, context_id, \
__FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}

/* Print standard send/recv args with additional parameter for source rank */
#define DEBUG_PRINT_ARGS2(src_rank, msg)		\
if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE,\
   "[%d]%s for tag = %d, source = %d, ctx = %d, (%s:%d)\n", \
	    MPID_MyWorldRank, msg, tag, src_rank, context_id, \
__FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}

/* The following macros are used in several files in the ch2 directory and
   are therefore defined here. Devices may have their own versions of these
   macros, they have to undef them to prevent macro redefinitions if they
   include this file. Perhaps we'll provide some generic implementation here
   later on */
#define DEBUG_PRINT_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_PKT(msg,pkt)
#define DEBUG_PRINT_PKT_DATA(msg,pkt)
#define DEBUG_PRINT_RECV_PKT(msg,pkt)
#define DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)

/* ****************************************************************** */

#define DEBUG_TEST_FCN(fcn,msg) {\
    if (!fcn) {\
      FPRINTF( stderr, "Bad function pointer (%s) in %s at %d\n",\
	       msg, __FILE__, __LINE__);\
      MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", "Bad function pointer" );\
      }}
/* This is pretty expensive.  It should be an option ... */
#ifdef DEBUG_INIT_MEM
#define DEBUG_INIT_STRUCT(s,size) memset(s,0xfa,size)		
#else
#define DEBUG_INIT_STRUCT(s,size)
#endif

#define DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    FPRINTF( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	    MPID_MyWorldRank, msg, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}}
	    
#define DEBUG_PRINT_MSG2(msg,val)\
{if (MPID_DebugFlag) {\
    char localbuf[1024]; sprintf( localbuf, msg, val );\
    DEBUG_PRINT_MSG(localbuf);}}
	    
#else
#define DEBUG_PRINT_MSG(msg)
#define DEBUG_PRINT_MSG2(msg,val)
#define DEBUG_PRINT_ARGS(msg) 
#define DEBUG_PRINT_ARGS2(msg, src_lrank) 
#define DEBUG_TEST_FCN(fcn,msg)
#define DEBUG_INIT_STRUCT(s,size)
#define DEBUG_PRINT_PKT(msg,pkt)
#define DEBUG_PRINT_BASIC_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_PKT_DATA(msg,pkt)
#define DEBUG_PRINT_RECV_PKT(msg,pkt)
#define DEBUG_PRINT_SEND_PKT(msg,pkt)
#endif /* MPID_DEBUG_ALL */


#endif /* _MPID_DEBUG_H */
