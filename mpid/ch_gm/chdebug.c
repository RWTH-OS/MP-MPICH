/*
 *
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include "mpid.h"
#include "mpiddev.h"
#include "chpackflow.h"
#include "gmpid_debug.h"
#include <string.h>

/* 
   Unfortunately, stderr is not a guarenteed to be a compile-time
   constant in ANSI C, so we can't initialize MPID_DEBUG_FILE with
   stderr.  Instead, we set it to null, and check for null.  Note
   that stdout is used in chinit.c 
 */

extern FILE *MPID_TRACE_FILE;
extern FILE *MPID_DEBUG_FILE;
int MPID_UseDebugFile = 0;

/* Setting DebugFlag to 1 causes output to the MPID_DEBUG_FILE */
extern int MPID_DebugFlag;

int  MPID_GM_Rndv_print_pkt (FILE *, MPID_PKT_T *);
int  MPID_GM_Cancel_print_pkt (FILE *, MPID_PKT_T *);
void MPID_GM_Print_Send_Handle ( MPIR_SHANDLE * );

/* Should each mode have its own print routines? */

int MPID_GM_Rndv_print_pkt( 
	FILE       *fp,
	MPID_PKT_T *pkt)
{
    /* A "send_id" is a 8byte item on heterogeneous systems.  On 
       systems without 8byte longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that.  %x format uses 2 characters
       for each byte; we allow a few extra for the null.
       */
    char sendid[20], recvid[20], targetptr[20];

    MPID_Aint send_id, recv_id;

    if (pkt->head.mode == MPID_PKT_REQUEST_SEND) {
	send_id = pkt->request_pkt.send_id;
#ifdef MPID_AINT_IS_STRUCT
	recv_id.high = 0;
	recv_id.low  = 0;
#else
	recv_id = (MPID_Aint)(0);
#endif
      }
    else 
      {
	if (pkt->head.mode == MPID_PKT_DONE_SEND)
	  {
	    send_id = 0;
#ifdef GM_ORIGINAL
	    recv_id = pkt->done_pkt.recv_id; */
#endif
	  }
	else 
	  {
	    send_id = pkt->sendok_pkt.send_id;
	    recv_id = pkt->sendok_pkt.recv_id;
	  } 
      }

#if defined(MPID_AINT_IS_STRUCT) && !defined(POINTER_64_BITS)
    SPRINTF( sendid, "%x", send_id.low );
    SPRINTF( recvid, "%x", recv_id.low );
    SPRINTF( targetptr, "%x", pkt->sendok_pkt.target_ptr.low );
#elif defined(MPID_AINT_IS_STRUCT) && defined(POINTER_64_BITS) 
    SPRINTF( sendid, "%x%x", send_id.high, send_id.low );
    SPRINTF( recvid, "%x%x", recv_id.high, recv_id.low );
    SPRINTF( targetptr, "%x%x", pkt->sendok_pkt.target_ptr.high,
                                pkt->sendok_pkt.target_ptr.low ); 
#else
    SPRINTF( sendid, "%lx", (long)send_id );
    SPRINTF( recvid, "%lx", (long)recv_id );
#ifdef GM_ORIGINAL
    SPRINTF( targetptr, "%lx", (long)(pkt->sendok_pkt.target_ptr));
#endif
#endif

    if (pkt->head.mode == MPID_PKT_REQUEST_SEND) {
	FPRINTF( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tto         = %d\n\
\tsend_id    = %s\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->head.to, sendid );
    }
    else if (pkt->head.mode == MPID_PKT_DONE_SEND) {
              FPRINTF( fp, "\
\tlrank      = %d\n\
\tto         = %d\n\
\trecv_id    = %s\n\
\tmode       = ", pkt->head.lrank, pkt->head.to,
                 recvid );
    } else {
	FPRINTF( fp, "\
\tlrank      = %d\n\
\tto         = %d\n\
\tsend_id    = %s\n\
\trecv_id    = %s\n\
\ttarget_ptr = %s\n\
\tmode       = ", pkt->head.lrank, pkt->head.to, 
		 sendid, recvid , targetptr);
    }
    return MPI_SUCCESS;
}

int MPID_GM_Cancel_print_pkt( FILE *fp, MPID_PKT_T *pkt )
{
    /* A "send_id" is a 64bit item on heterogeneous systems.  On 
       systems without 64bit longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that
       */
#if defined MPID_AINT_IS_STRUCT && defined(POINTER_64_BITS)
    char sendid[64];
#else
    char sendid[40];
#endif

    MPID_Aint send_id;

    send_id = pkt->antisend_pkt.send_id;
#if defined(MPID_AINT_IS_STRUCT) && !defined(POINTER_64_BITS)
    sprintf( sendid, "%x", send_id.low );
#elif defined(MPID_AINT_IS_STRUCT) && defined(POINTER_64_BITS)
    sprintf( sendid, "%x%x", send_id.high, send_id.low );
#else
    sprintf( sendid, "%lx", (long)send_id );
#endif

    if (pkt->head.mode != MPID_PKT_ANTI_SEND_OK)
	fprintf( fp, "\
\tlrank      = %d\n\
\tdest       = %d\n\
\tsend_id    = %s\n\
\tmode       = ", 
	pkt->head.lrank, pkt->head.to, sendid);
    else
	fprintf( fp, "\
\tlrank      = %d\n\
\tdest       = %d\n\
\tcancel     = %d\n\
\tsend_id    = %s\n\
\tmode       = ", 
	pkt->head.lrank, pkt->head.to, pkt->antisend_pkt.cancel, sendid);

    return MPI_SUCCESS;
}

int MPID_GM_Print_packet( FILE *fp, MPID_PKT_T *pkt )
{
    FPRINTF( fp, "[%d] PKT =\n", MPID_GM_rank );
    switch (pkt->head.mode) {
    case MPID_PKT_SHORT:
	FPRINTF( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, 
        pkt->head.lrank);
	break;
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_OK_TO_SEND:
    case MPID_PKT_DONE_SEND:
	MPID_GM_Rndv_print_pkt( fp, pkt );
	break;
    case MPID_PKT_ANTI_SEND:
    case MPID_PKT_ANTI_SEND_OK:
	MPID_GM_Cancel_print_pkt( fp, pkt );
	break;
    default:
	FPRINTF( fp, "\n" );
    }
    MPID_GM_Print_mode( fp, pkt );
#ifdef MPID_HAS_HETERO
    if ( (pkt->head.mode != MPID_PKT_OK_TO_SEND) &&
	 (pkt->head.mode != MPID_PKT_ANTI_SEND) &&
	 (pkt->head.mode != MPID_PKT_ANTI_SEND_OK) ) {
      switch ((MPID_Msgrep_t)pkt->head.msgrep) {
      case MPID_MSGREP_RECEIVER:
	FPRINTF( fp, "\n\tmsgrep = MPID_MSGREP_RECEIVER\n" ); break;
      case MPID_MSGREP_SENDER:
	FPRINTF( fp, "\n\tmsgrep = MPID_MSGREP_SENDER\n" ); break;
      case MPID_MSGREP_XDR:
	FPRINTF( fp, "\n\tmsgrep = MPID_MSGREP_XDR\n" ); break;
      default:
	FPRINTF( fp, "\n\tmsgrep = %d !UNKNOWN!\n", 
		 (int) pkt->head.msgrep ); break;
      }
    }
#endif
    FPUTS( "\n", fp );
    return MPI_SUCCESS;
}


int MPID_GM_Print_mode( 
	FILE        *fp,
	MPID_PKT_T  *pkt)
{
    char *modename=0;
    switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
	FPUTS( "short", fp );
	break;
    case MPID_PKT_REQUEST_SEND:
	FPUTS( "request send", fp );
	break;
    case MPID_PKT_OK_TO_SEND:
	FPUTS( "ok to send", fp );
	break;
    case MPID_PKT_FLOW:
	FPUTS( "flow control", fp );
	break;
    case MPID_PKT_PROTO_ACK:
	fputs( "protocol ACK", fp );
        break;
    case MPID_PKT_ACK_PROTO:
	fputs( "Ack protocol", fp );
	break;
    case MPID_PKT_ANTI_SEND:
	fputs( "anti send", fp );
	break;
    case MPID_PKT_ANTI_SEND_OK:
	fputs( "anti send ok", fp );
	break;
    default:
	FPRINTF( fp, "Mode %d is unknown!\n", pkt->short_pkt.mode );
	break;
    }
    /* if (MPID_MODE_HAS_XDR(pkt)) FPUTS( "xdr", fp ); */

    if (modename) {
	FPUTS( modename, fp );
    }
    return MPI_SUCCESS;
}
    
void MPID_GM_Print_pkt_data( char *msg, char *address, int len )
{
    int i; char *aa = (char *)address;

    if (msg)
	FPRINTF( MPID_DEBUG_FILE, "[%d]%s\n", MPID_GM_rank, msg );
    if (len < 78 && address) {
	for (i=0; i<len; i++) {
	    FPRINTF( MPID_DEBUG_FILE, "%x", aa[i] );
	}
	FPRINTF( MPID_DEBUG_FILE, "\n" );
    }
    fflush( MPID_DEBUG_FILE );
}

void MPID_GM_Print_Send_Handle( 
	MPIR_SHANDLE *shandle)
{
    FPRINTF( stdout, "[%d]* dmpi_send_contents:\n\
* totallen    = %d\n\
* recv_handle = %x\n", MPID_GM_rank, 
		 shandle->bytes_as_contig, 
		 shandle->recv_handle );
}

void MPID_GM_SetDebugFile( char *name )
{
    char filename[1024];
    
    if (strcmp( name, "-" ) == 0) {
	MPID_DEBUG_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	SPRINTF( filename, name, MPID_GM_rank );
	MPID_DEBUG_FILE = fopen( filename, "w" );
    }
    else
	MPID_DEBUG_FILE = fopen( name, "w" );

    if (!MPID_DEBUG_FILE) MPID_DEBUG_FILE = stdout;
}

void MPID_GM_Set_tracefile( char *name )
{
    char filename[1024];

    if (strcmp( name, "-" ) == 0) {
	MPID_TRACE_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	SPRINTF( filename, name, MPID_GM_rank );
	MPID_TRACE_FILE = fopen( filename, "w" );
    }
    else
	MPID_TRACE_FILE = fopen( name, "w" );

    /* Is this the correct thing to do? */
    if (!MPID_TRACE_FILE)
	MPID_TRACE_FILE = stdout;
}

void MPID_GM_SetSpaceDebugFlag( int flag )
{
/*      DebugSpace = flag; */
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
    if (flag) {
	/* Check the validity of the malloc arena on every use of 
	   trmalloc/free */
	trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}
void MPID_GM_SetDebugFlag( int f )
{
    MPID_DebugFlag = f;
    MPID_UseDebugFile = f;
}

