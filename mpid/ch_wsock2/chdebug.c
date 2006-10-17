/*
 *  $Id$
 *
 */

 
#include "mpid.h"
#include "mpiddev.h"
#include "wsock2debug.h"
#include <string.h>

/* 
   Unfortunately, stderr is not a guarenteed to be a compile-time
   constant in ANSI C, so we can't initialize MPID_DEBUG_FILE with
   stderr.  Instead, we set it to null, and check for null.  Note
   that stdout is used in chinit.c 
 */
FILE *MPID_DEBUG_FILE = 0;
FILE *MPID_TRACE_FILE = 0;
int MPID_DebugFlag = 0;

void MPID_Get_print_pkt( FILE *, MPID_PKT_T *);
int  MPID_Rndv_print_pkt(FILE *, MPID_PKT_T *);
void MPID_Print_Send_Handle( MPIR_SHANDLE * );
int MPID_Print_mode( FILE*,MPID_PKT_T * );

/* Should each mode have its own print routines? */

int MPID_Rndv_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
    /* A "send_id" is a 64bit item on heterogeneous systems.  On 
       systems without 64bit longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that
       */
	

    char sendid[40];
    MPID_Aint send_id;

	if (pkt->request_pkt.send_id == NULL)
	{
		;
	}

    if (pkt->head.mode != MPID_PKT_OK_TO_SEND) 
	send_id = pkt->request_pkt.send_id;
    else
	send_id = pkt->sendok_pkt.send_id;
#ifdef MPID_AINT_IS_STRUCT
    if (sizeof(void *) <= 4) 
	sprintf( sendid, "%x", send_id.low );
    else
	sprintf( sendid, "%x%x", send_id.high, send_id.low );
#else
	sprintf( sendid, "%p", send_id );
#endif

    if (pkt->head.mode != MPID_PKT_OK_TO_SEND) {
	fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tsend_id    = %s\n\
\tsend_hndl  = %ld\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	sendid, (long)pkt->request_pkt.send_handle );
    }
    else {
	fprintf( fp, "\
\tsend_id    = %s\n\
\trecv_hndl  = %ld\n\
\tlen	     = %u\n\
\tmode       = ", sendid, (long)pkt->sendok_pkt.recv_handle,pkt->sendok_pkt.len );
    }
    return MPI_SUCCESS;
}

int MPID_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    fprintf( fp, "[%d] PKT =\n", MPID_MyWorldRank );
    switch (pkt->head.mode) {
    case MPID_PKT_SHORT:
    case MPID_PKT_LONG:
	fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank );
	break;
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_OK_TO_SEND:
	MPID_Rndv_print_pkt( fp, pkt );
	break;
    case MPID_PKT_FLOW:
#ifdef MPID_FLOW_CONTROL
	fprintf( fp, "\
\tflow info  = %d\n", pkt->head.flow_info );
#endif
	break;
	case MPID_PKT_REQUEST_SEND_GET:
    case MPID_PKT_OK_TO_SEND_GET:
    case MPID_PKT_CONT_GET:
	MPID_Get_print_pkt( fp, pkt );
	break;
    default:
	fprintf( fp, "\n" );
    }
    MPID_Print_mode( fp, pkt );
#ifdef MPID_HAS_HETERO
    switch ((MPID_Msgrep_t)pkt->head.msgrep) {
    case MPID_MSGREP_RECEIVER:
    fprintf( fp, "\n\tmsgrep = MPID_MSGREP_RECEIVER\n" ); break;
    case MPID_MSGREP_SENDER:
    fprintf( fp, "\n\tmsgrep = MPID_MSGREP_SENDER\n" ); break;
    case MPID_MSGREP_XDR:
    fprintf( fp, "\n\tmsgrep = MPID_MSGREP_XDR\n" ); break;
    default:
    fprintf( fp, "\n\tmsgrep = %d !UNKNOWN!\n", 
	     (int) pkt->head.msgrep ); break;
    }
#endif
    fputs( "\n", fp );
    return MPI_SUCCESS;
}

void MPID_Get_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
#ifndef MPID_HAS_HETERO
    fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tcur_offset = %d\n\
\tlen_avail  = %d\n\
\tsend_id    = %p\n\
\trecv_id    = %p\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->get_pkt.cur_offset, pkt->get_pkt.len_avail, 
	     pkt->get_pkt.send_id, pkt->get_pkt.recv_id );
#endif
}

int MPID_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    char *modename=0;
    switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
	fputs( "short", fp );
	break;
    case MPID_PKT_LONG:
	fputs( "long", fp );
	break;
    case MPID_PKT_REQUEST_SEND:
	fputs( "request send", fp );
	break;
    case MPID_PKT_OK_TO_SEND:
	fputs( "ok to send", fp );
	break;
    case MPID_PKT_FLOW:
	fputs( "flow control", fp );
	break;
    case MPID_PKT_REQUEST_SEND_GET:
	fputs( "do get", fp );
	break; 
    case MPID_PKT_OK_TO_SEND_GET:
	fputs( "ok to send get", fp );
	break;
    case MPID_PKT_CONT_GET:
	fputs( "continue get", fp );
	break;
    default:
	fprintf( fp, "Mode %d is unknown!\n", pkt->short_pkt.mode );
	break;
    }
    /* if (MPID_MODE_HAS_XDR(pkt)) fputs( "xdr", fp ); */

    if (modename) {
	fputs( modename, fp );
    }
    return MPI_SUCCESS;
}
    
void MPID_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
    int i; char *aa = (char *)address;

    if (msg)
	fprintf( MPID_DEBUG_FILE, "[%d]%s\n", MPID_MyWorldRank, msg );
    if (len < 78 && address) {
	for (i=0; i<len; i++) {
	    fprintf( MPID_DEBUG_FILE, "%x", aa[i] );
	}
	fprintf( MPID_DEBUG_FILE, "\n" );
    }
    fflush( MPID_DEBUG_FILE );
}

void MPID_Print_Send_Handle( shandle )
MPIR_SHANDLE *shandle;
{
    fprintf( stdout, "[%d]* dmpi_send_contents:\n\
* totallen    = %d\n\
* recv_handle = %x\n\
*flag         = %p", MPID_MyWorldRank, 
		 shandle->bytes_as_contig, 
		 shandle->recv_handle,
		 shandle->flag);
}

void MPID_SetDebugFile( name )
char *name;
{
    char filename[1024];
    
    if (strcmp( name, "-" ) == 0) {
	MPID_DEBUG_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	sprintf( filename, name, MPID_MyWorldRank );
	MPID_DEBUG_FILE = fopen( filename, "w" );
    }
    else
	MPID_DEBUG_FILE = fopen( name, "w" );

    if (!MPID_DEBUG_FILE) MPID_DEBUG_FILE = stdout;
}

void MPID_Set_tracefile( name )
char *name;
{
    char filename[1024];

    if (strcmp( name, "-" ) == 0) {
	MPID_TRACE_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	sprintf( filename, name, MPID_MyWorldRank );
	MPID_TRACE_FILE = fopen( filename, "w" );
    }
    else
	MPID_TRACE_FILE = fopen( name, "w" );

    /* Is this the correct thing to do? */
    if (!MPID_TRACE_FILE)
	MPID_TRACE_FILE = stdout;
}

void MPID_SetSpaceDebugFlag( flag )
int flag;
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
void MPID_SetDebugFlag( f )
int f;
{
    MPID_DebugFlag = f;
}

/*
   Data about messages
 */
static int DebugMsgFlag = 0;
void MPID_SetMsgDebugFlag( f )
int f;
{
    DebugMsgFlag = f;
}
int MPID_GetMsgDebugFlag()
{
    return DebugMsgFlag;
}
void MPID_PrintMsgDebug()
{
}

/*
 * Print information about a request
 */
void MPID_Print_rhandle( fp, rhandle )
FILE *fp;
MPIR_RHANDLE *rhandle;
{
    fprintf( fp, "rhandle at %p\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tbuf        \t= %p\n", 
	     rhandle, 
#ifdef MPIR_HAS_COOKIES
	     rhandle->cookie, 
#else
	     0,
#endif
	     rhandle->is_complete, 
	     rhandle->buf );
}
void MPID_Print_shandle( fp, shandle )
FILE *fp;
MPIR_SHANDLE *shandle;
{
	fprintf( fp, "shandle at %p\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tstart      \t= %p\n\
\tbytes_as_contig\t= %d\n\
", 
	     shandle, 
#ifdef MPIR_HAS_COOKIES
	     shandle->cookie, 
#else
	     0,
#endif
	     shandle->is_complete, 
	     shandle->start,
	     shandle->bytes_as_contig
 );
}
