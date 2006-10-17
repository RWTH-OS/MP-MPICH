/*
 *  $Id$
 *
 */
/* MP_MPICH global */
#include "mpichconf.h" /* MPID_DEBUG_ALL is set there */

#ifdef MPID_DEBUG_ALL

#define _DEBUG_EXTERN_REC
#include "mydebug.h"
 
#include "mpid.h"
#include "usockdev.h"
#include "mpid_debug.h"
#include "usockdebug.h"
#include <string.h>

/* 
   Unfortunately, stderr is not a guarenteed to be a compile-time
   constant in ANSI C, so we can't initialize MPID_DEBUG_FILE with
   stderr.  Instead, we set it to null, and check for null.  Note
   that stdout is used in chinit.c 
 */

void MPID_USOCK_Get_print_pkt( FILE *, MPID_PKT_T *);
int  MPID_USOCK_Rndv_print_pkt(FILE *, MPID_PKT_T *);
void MPID_USOCK_Print_Send_Handle( MPIR_SHANDLE * );
int MPID_USOCK_Print_mode( FILE*,MPID_PKT_T * );

/* Should each mode have its own print routines? */

int MPID_USOCK_Rndv_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
    DSECTION("MPID_USOCK_Rndv_print_pkt");
    /* A "send_id" is a 64bit item on heterogeneous systems.  On 
       systems without 64bit longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that
       */
    char sendid[40];
    MPID_Aint send_id;

    DSECTENTRYPOINT;

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
    sprintf( sendid, "%lx", (long)send_id );
#endif

    if (pkt->head.mode != MPID_PKT_OK_TO_SEND) {
	fprintf( fp, "\
\tlen            = %d\n\
\ttag            = %d\n\
\tcontext_id     = %d\n\
\tsrc_comm_lrank = %d\n\
\tsend_id        = %s\n\
\tsend_hndl      = %ld\n\
\tmode           = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.src_comm_lrank,
	sendid, (long)pkt->request_pkt.send_handle );
    }
    else {
	fprintf( fp, "\
\tsend_id    = %s\n\
\trecv_hndl  = %ld\n\
\tlen	     = %u\n\
\tmode       = ", sendid, (long)pkt->sendok_pkt.recv_handle,pkt->sendok_pkt.len );
    }
    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    DSECTION("MPID_USOCK_Print_packet");
    DSECTENTRYPOINT;

    fprintf( fp, "[%d] PKT =\n", ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank );
    switch (pkt->head.mode) {
    case MPID_PKT_SHORT:
    case MPID_PKT_LONG:
	fprintf( fp, "\
\tlen            = %d\n\
\ttag            = %d\n\
\tcontext_id     = %d\n\
\tsrc_comm_lrank = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.src_comm_lrank );
	break;
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_OK_TO_SEND:
	MPID_USOCK_Rndv_print_pkt( fp, pkt );
	break;
    case MPID_PKT_FLOW:
#ifdef MPID_FLOW_CONTROL
	fprintf( fp, "\
\tflow info  = %d\n", pkt->head.flow_info );
#endif
	break;
	case MPID_PKT_REQUEST_SEND_GET:
    case MPID_PKT_SEND_ADDRESS:
    case MPID_PKT_OK_TO_SEND_GET:
    case MPID_PKT_CONT_GET:
	MPID_USOCK_Get_print_pkt( fp, pkt );
	break;
    default:
	fprintf( fp, "\n" );
    }
    MPID_USOCK_Print_mode( fp, pkt );
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
    DSECTLEAVE
	return MPI_SUCCESS;
}

void MPID_USOCK_Get_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
    DSECTION("MPID_USOCK_Get_print_pkt");
    DSECTENTRYPOINT;
#ifndef MPID_HAS_HETERO
    fprintf( fp, "\
\tlen            = %d\n\
\ttag            = %d\n\
\tcontext_id     = %d\n\
\tsrc_comm_lrank = %d\n\
\tcur_offset     = %d\n\
\tlen_avail      = %d\n\
\tsend_id        = %lx\n\
\trecv_id        = %ld\n\
\tmode           = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.src_comm_lrank,
	pkt->get_pkt.cur_offset, pkt->get_pkt.len_avail, 
	     (long)pkt->get_pkt.send_id, (long)pkt->get_pkt.recv_id );
#endif
    DSECTLEAVE;
}

int MPID_USOCK_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    DSECTION("MPID_USOCK_Print_mode");
    char *modename=0;

    DSECTENTRYPOINT;
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
    case MPID_PKT_SEND_ADDRESS:
	fputs( "send address", fp );
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
    
    DSECTLEAVE
	return MPI_SUCCESS;
}
    
void MPID_USOCK_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
    DSECTION("MPID_USOCK_Print_pkt_data");
    int i; char *aa = (char *)address;

    DSECTENTRYPOINT;

    if (msg)
        fprintf( MPID_DEBUG_FILE, "[%d]%s\n", ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, msg );
    if (len < 78 && address) {
	for (i=0; i<len; i++) {
	    fprintf( MPID_DEBUG_FILE, "%x", aa[i] );
	}
	fprintf( MPID_DEBUG_FILE, "\n" );
    }
    fflush( MPID_DEBUG_FILE );
    
    DSECTLEAVE;
}

void MPID_USOCK_Print_Send_Handle( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Print_Send_Handle");
    DSECTENTRYPOINT;
    fprintf( stdout, "[%d]* dmpi_send_contents:\n\
* totallen    = %d\n\
* recv_handle = %x\n", 
      	         ((MPID_USOCK_Data_global_type*)MPID_devset->active_dev->global_data)->MyWorldRank, 
		 shandle->bytes_as_contig, 
                 shandle->recv_handle);		 

    DSECTLEAVE;
}


/*
 * Print information about a request
 */
void MPID_USOCK_Print_rhandle( fp, rhandle )
FILE *fp;
MPIR_RHANDLE *rhandle;
{
    DSECTION("MPID_USOCK_Print_rhandle");
    DSECTENTRYPOINT;
    fprintf( fp, "rhandle at %lx\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tbuf        \t= %lx\n", 
	     (long)rhandle, 
#ifdef MPIR_HAS_COOKIES
	     rhandle->cookie, 
#else
	     0,
#endif
	     rhandle->is_complete, 
	     (long)rhandle->buf );
    DSECTLEAVE;
}
void MPID_USOCK_Print_shandle( fp, shandle )
FILE *fp;
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Print_shandle");
    DSECTENTRYPOINT;
    fprintf( fp, "shandle at %lx\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tstart      \t= %lx\n\
\tbytes_as_contig\t= %d\n\
", 
	     (long)shandle, 
#ifdef MPIR_HAS_COOKIES
	     shandle->cookie, 
#else
	     0,
#endif
	     shandle->is_complete, 
	     (long)shandle->start,
	     shandle->bytes_as_contig
 );
    DSECTLEAVE;
}

#endif /* MPID_DEBUG_ALL */
