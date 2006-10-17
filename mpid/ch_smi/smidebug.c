/*
 *  $Id$
 *
 * these are the device-specific functions to print packets in a 
 * human-readable form
 *
 */

#include <string.h>

#include "smidebug.h"
#include "mpid.h"
#include "smidev.h"

void MPID_SMI_Print_address_pkt ( FILE *, MPID_PKT_T *);
void MPID_SMI_Get_print_pkt ( FILE *, MPID_PKT_T *);
int  MPID_SMI_Rndv_print_pkt (FILE *, MPID_PKT_T *);
void MPID_SMI_Print_Send_Handle ( MPIR_SHANDLE * );


#ifdef MPID_DEBUG_ALL  

int MPID_SMI_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    fprintf( fp, "[%d] PKT =\n", MPID_SMI_myid );
    switch (pkt->head.mode) {
    case MPID_PKT_SHORT:
	fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank );
	break;
    case MPID_PKT_SEND_ADDRESS:
	MPID_SMI_Print_address_pkt( fp, pkt );
	break;
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_CONT:
    case MPID_PKT_OK_TO_SEND:
    case MPID_PKT_PART_READY:
    case MPID_PKT_PUT:
    case MPID_PKT_GET:
    case MPID_PKT_ACCU:
#ifdef MPID_FLOW_CONTROL
    case MPID_PKT_FLOW:
#endif
	MPID_SMI_Get_print_pkt( fp, pkt );
	break;
    default:
	fprintf( fp, "\n" );
    }
    MPID_SMI_Print_mode( fp, pkt );
    fputs( "\n", fp );
    return MPI_SUCCESS;
}

void MPID_SMI_Print_address_pkt ( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
#ifndef MPID_HAS_HETERO
    fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\toffset     = %d\n\
\tmode       = ",
	     pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	     pkt->sendadd_pkt.offset );
#endif
}

void MPID_SMI_Get_print_pkt( fp, pkt )
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
\tsend_id    = %lx\n\
\trecv_id    = %lx\n\
\taddress    = %lx\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->rndv_pkt.data_offset, pkt->rndv_pkt.len_avail, 
	     (MPI_Aint)pkt->rndv_pkt.send_id, (MPI_Aint)pkt->rndv_pkt.recv_id,
	     (MPI_Aint)pkt->rndv_pkt.sgmt_offset );
#endif
}

int MPID_SMI_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    char *modename=0;
    switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
	fputs( "short: message", fp );
	break;
    case MPID_PKT_SEND_ADDRESS:
	fputs( "eager: send address", fp );
	break;
    case MPID_PKT_REQUEST_SEND:
	fputs( "rndv: request to send", fp );
	break; 
    case MPID_PKT_CONT:
 	fputs( "rndv: continue", fp );
	break;
    case MPID_PKT_OK_TO_SEND:
	fputs( "rndv: ok to send", fp );
	break;
    case MPID_PKT_PART_READY:
	fputs( "rndv: part ready", fp );
	break;
    case MPID_PKT_ANTI_SEND:
	fputs( "rndv: anti-send", fp );
	break;
    case MPID_PKT_ANTI_SEND_OK:
	fputs( "rndv: anti-send-ok", fp );
	break;
    case MPID_PKT_PUT:
	fputs( "ssided: put", fp );
	break;
    case MPID_PKT_GET:
	fputs( "ssided: get", fp );
	break;
    case MPID_PKT_ACCU:
	fputs( "ssided: accu", fp );
	break;
#ifdef MPID_FLOW_CONTROL
    case MPID_PKT_FLOW:
	fputs( "flow packet", fp );
	break;
#endif


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
    
void MPID_SMI_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
    int i; char *aa = (char *)address;

    if (msg)
	fprintf( MPID_DEBUG_FILE, "[%d]%s\n", MPID_SMI_myid, msg );
    if (len < 78 && address) {
	for (i=0; i<len; i++) {
	    fprintf( MPID_DEBUG_FILE, "%x", aa[i] );
	}
	fprintf( MPID_DEBUG_FILE, "\n" );
    }
    fflush( MPID_DEBUG_FILE );
}

#else

int MPID_SMI_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    return MPI_SUCCESS;
}

void MPID_SMI_Print_address_pkt ( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
}

void MPID_SMI_Get_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
}

int MPID_SMI_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
    return MPI_SUCCESS;
}
    
void MPID_SMI_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
}

#endif /* MPID_DEBUG_ALL */
