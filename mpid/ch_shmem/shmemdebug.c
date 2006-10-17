/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include <string.h>

#include "mpid.h"
#include "shmemdev.h"
#include "shmemcommon.h"
#include "chpackflow.h"

extern FILE *MPID_DEBUG_FILE;
extern FILE *MPID_TRACE_FILE;
extern int MPID_DebugFlag;

void MPID_SHMEM_Get_print_pkt ANSI_ARGS(( FILE *, MPID_PKT_T *));
int MPID_SHMEM_Cancel_print_pkt ANSI_ARGS(( FILE *, MPID_PKT_T *));

/* Should each mode have its own print routines? */
int MPID_SHMEM_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    FPRINTF( fp, "[%d] PKT =\n", MPID_SHMEM_rank );
    switch (pkt->head.mode) {
    case MPID_PKT_SHORT:
	FPRINTF( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tseqnum     = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
        pkt->head.seqnum );
	break;
    case MPID_PKT_REQUEST_SEND_GET:
    case MPID_PKT_SEND_ADDRESS:
    case MPID_PKT_OK_TO_SEND_GET:
    case MPID_PKT_CONT_GET:
	MPID_SHMEM_Get_print_pkt( fp, pkt );
	break;
    case MPID_PKT_ANTI_SEND:
    case MPID_PKT_ANTI_SEND_OK:
	MPID_SHMEM_Cancel_print_pkt( fp, pkt );
	break;
    case MPID_PKT_PROTO_ACK:
    case MPID_PKT_ACK_PROTO:
#ifdef MPID_PACK_CONTROL
	fprintf( fp, "\
\tlrank  = %d\n\
\tto     = %d\n\
\tmode   = ",
	pkt->head.lrank, pkt->head.to);
#endif      
	break;
    default:
	FPRINTF( fp, "\n" );
    }
    MPID_SHMEM_Print_mode( fp, pkt );
    FPUTS( "\n", fp );
    return MPI_SUCCESS;
}

int MPID_SHMEM_Cancel_print_pkt( fp, pkt )
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

    send_id = pkt->antisend_pkt.send_id;
#ifdef MPID_AINT_IS_STRUCT
    sprintf( sendid, "%lx%lx", (long)send_id.high, (long)send_id.low);
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

void MPID_SHMEM_Get_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{

    /* A "send_id" and "recv_id" are 64bit items on heterogeneous systems.  On 
       systems without 64bit longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that
       */
    char sendid[40];
    char recvid[40];
    MPID_Aint send_id;
    MPID_Aint recv_id;

    if (pkt->head.mode != MPID_PKT_SEND_ADDRESS) {
	/* begin if mode != address */
	send_id = pkt->get_pkt.send_id;
	if (pkt->head.mode != MPID_PKT_REQUEST_SEND_GET)
	    recv_id = pkt->get_pkt.recv_id;

#ifdef MPID_AINT_IS_STRUCT
	sprintf( sendid, "%lx%lx", (long)send_id.high, (long)send_id.low);
#else
	sprintf( sendid, "%lx", (long)send_id );
#endif
	if (pkt->head.mode != MPID_PKT_REQUEST_SEND_GET)
#ifdef MPID_AINT_IS_STRUCT
	    sprintf( recvid, "%lx%lx", (long)recv_id.high, (long)recv_id.low);
#else
	sprintf( recvid, "%lx", (long)recv_id );
#endif
	    
    }  /* end if mode != address */

#ifndef MPID_HAS_HETERO
	if (pkt->head.mode == MPID_PKT_SEND_ADDRESS)
	    fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\taddress    = %lx\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	     (MPI_Aint)pkt->get_pkt.address );
	
	else if (pkt->head.mode == MPID_PKT_REQUEST_SEND_GET)
	    fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tsend_id    = %lx\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->get_pkt.send_id );
	else fprintf( fp, "\
\tcur_offset = %d\n\
\tlen_avail  = %d\n\
\tsend_id    = %lx\n\
\trecv_id    = %lx\n\
\taddress    = %lx\n\
\tmode       = ", 
	pkt->get_pkt.cur_offset, pkt->get_pkt.len_avail, pkt->get_pkt.send_id,
	pkt->get_pkt.recv_id, (MPI_Aint)pkt->get_pkt.address );
#endif
}

int MPID_SHMEM_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    char *modename=0;
    switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
	FPUTS( "short", fp );
	break;
    case MPID_PKT_SEND_ADDRESS:
	FPUTS( "send address", fp );
	break;
    case MPID_PKT_REQUEST_SEND_GET:
	FPUTS( "do get", fp );
	break; 
    case MPID_PKT_OK_TO_SEND_GET:
	FPUTS( "ok to send get", fp );
	break;
    case MPID_PKT_CONT_GET:
	FPUTS( "continue get", fp );
	break;
    case MPID_PKT_FLOW:
	fputs( "flow control", fp );
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
	fprintf( fp, "Mode %d is unknown!\n", pkt->short_pkt.mode );
	break;
    }
    /* if (MPID_MODE_HAS_XDR(pkt)) FPUTS( "xdr", fp ); */

    if (modename) {
	FPUTS( modename, fp );
    }
    return MPI_SUCCESS;
}
    
void MPID_SHMEM_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
    int i; char *aa = (char *)address;

    if (!MPID_DEBUG_FILE) MPID_DEBUG_FILE = stderr;
    if (msg)
	FPRINTF( MPID_DEBUG_FILE, "[%d]%s\n", MPID_SHMEM_rank, msg );
    if (len < 78 && address) {
	for (i=0; i<len; i++) {
	    FPRINTF( MPID_DEBUG_FILE, "%x", aa[i] );
	}
	FPRINTF( MPID_DEBUG_FILE, "\n" );
    }
    fflush( MPID_DEBUG_FILE );
}

