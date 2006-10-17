/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "mpid.h"
#include "mpiddev.h"
#include "gmpi.h"
#include "gmpi_smpi.h"

#include <string.h>

int SMPI_Cancel_print_pkt( fp, pkt )
FILE       *fp;
SMPI_PKT_T *pkt;
{
    /* A "send_id" is a 64bit item on heterogeneous systems.  On 
       systems without 64bit longs, we need special code to print these.
       To help keep the output "nearly" atomic, we first convert the
       send_id to a string, and then print that
       */
    char sendid[64];
    MPID_Aint send_id;

    send_id = pkt->cancel_pkt.send_id;
    sprintf( sendid, "%lx", (long)send_id );

    if (pkt->head.mode != MPID_PKT_ANTI_SEND_OK)
	fprintf( fp, "\
\tlrank      = %d\n\
\tsend_id    = %s\n\
\tmode       = ", 
	pkt->head.lrank, sendid);
    else
	fprintf( fp, "\
\tlrank      = %d\n\
\tcancel     = %d\n\
\tsend_id    = %s\n\
\tmode       = ", 
	pkt->head.lrank, pkt->cancel_pkt.cancel, sendid);

    return MPI_SUCCESS;
}

int SMPI_Print_mode( fp, pkt )
FILE        *fp;
SMPI_PKT_T  *pkt;
{
  switch (pkt->short_pkt.mode) {
  case MPID_PKT_SHORT:
    FPUTS( "short", fp );
    break;
#if !SMP_ENABLE_DIRECTCOPY
  case MPID_PKT_REQUEST_SEND:
    FPUTS( "request send", fp );
    break; 
  case MPID_PKT_OK_TO_SEND:
    FPUTS( "ok to send", fp );
    break;
  case MPID_PKT_CONT_GET:
    FPUTS( "cont get", fp );
    break;
#else
  case MPID_PKT_DO_GET:
    FPUTS( "do get", fp );
    break;
#endif
  case MPID_PKT_DONE_GET:
    FPUTS( "done get", fp );
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
    
  return MPI_SUCCESS;
}

int SMPI_Print_packet( fp, pkt )
FILE        *fp;
SMPI_PKT_T  *pkt;
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
	     pkt->short_pkt.len, pkt->short_pkt.tag, 
	     pkt->short_pkt.context_id, pkt->short_pkt.lrank);
    break;
  case MPID_PKT_REQUEST_SEND:
  case MPID_PKT_OK_TO_SEND:
    FPRINTF( fp, "\
\tlength     = %d\n\
\trecv_id    = %p\n\
\tsend_id    = %p\n\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	     pkt->rndv_pkt.len, pkt->rndv_pkt.recv_id, 
	     pkt->rndv_pkt.send_id, pkt->rndv_pkt.len, pkt->rndv_pkt.tag, 
	     pkt->rndv_pkt.context_id, pkt->rndv_pkt.lrank);
    break;
  case MPID_PKT_ANTI_SEND:
  case MPID_PKT_ANTI_SEND_OK:
    FPRINTF( fp, "\
\tcancel     = %x\n\
\tsend_id    = %p\n\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	     pkt->cancel_pkt.cancel, pkt->cancel_pkt.send_id, 
	     pkt->cancel_pkt.len, pkt->cancel_pkt.tag, 
	     pkt->cancel_pkt.context_id, pkt->cancel_pkt.lrank);
    
    break;
  case MPID_PKT_DO_GET:
  case MPID_PKT_DONE_GET:
    FPRINTF( fp, "\
\tlength     = %d\n\
\taddress    = %p\n\
\tsend_id    = %p\n\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	     pkt->get_pkt.len, pkt->get_pkt.address, 
	     pkt->get_pkt.send_id, pkt->get_pkt.len, pkt->get_pkt.tag, 
	     pkt->get_pkt.context_id, pkt->get_pkt.lrank);
    break;
  case MPID_PKT_CONT_GET:
    FPRINTF( fp, "\
\trecv_id    = %p\n\
\tsend_id    = %p\n\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	     pkt->cont_pkt.recv_id, pkt->cont_pkt.send_id, 
	     pkt->cont_pkt.len, pkt->cont_pkt.tag, 
	     pkt->cont_pkt.context_id, pkt->cont_pkt.lrank);
    break;
  default:
    FPRINTF( fp, "\n" );
  }
  SMPI_Print_mode( fp, pkt );
  FPUTS( "\n", fp );
  return MPI_SUCCESS;
}
