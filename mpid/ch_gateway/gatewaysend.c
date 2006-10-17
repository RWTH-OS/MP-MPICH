/* $Id$ */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"

/* remember: for the gateway device, device local ranks are always global ranks */
int MPID_Gateway_Unified_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			       msgrep, dty )
     void *buf;
     int len, tag, context_id, src_comm_lrank, dest_dev_lrank;
     MPID_Msgrep_t msgrep;
     struct MPIR_DATATYPE *dty;
{
    Meta_Header *gw_msg;
    int mpi_errno;
    
    DEBUG_PRINT_MSG("Gateway: Entering MPID_Gateway_Unified_send()");
    
    /* wrap the message */
    gw_msg = MPID_Gateway_Wrap_Msg (buf, src_comm_lrank, dest_dev_lrank, len, tag, context_id, BLOCKING, (int)msgrep, 0);
    if (!gw_msg) {
	return MPI_ERR_INTERN;
    }

#ifdef META_DEBUG
    fprintf ( stderr, "[h%d] MPID_Gateway_Unified_Send(): gating msg, tag = %d, len = %d, from [w%d] (communicator rank) to [a%d] (global rank) via [h%d] (global rank)\n", 
	      MPID_MyHostRank, tag, len, src_comm_lrank, dest_dev_lrank, MPIR_meta_cfg.granks_to_router[dest_dev_lrank] );
    fflush(stderr);     
#endif
    
    /* send the wrapped message to the router process */
    MPID_SendContig( gw_msg, len + sizeof(Meta_Header),
		     MPID_MyHostRank, MPIR_MPIMSG_TAG, MPIR_HOST_PT2PT_CONTEXT, MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, &mpi_errno );
    DEBUG_PRINT_MSG("Gateway: Leaving MPID_Gateway_Unified_send()");
    
    return mpi_errno;
}


/* remember: for the gateway device, device local ranks are always global ranks */
int MPID_Gateway_Unified_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
				msgrep, shandle, dty )
     void          *buf;
     int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
     MPID_Msgrep_t msgrep;
     MPIR_SHANDLE *shandle;
     struct MPIR_DATATYPE * dty;
{
    int mpi_errno;
    int msgid;
    Meta_Header metadata;
    
    DEBUG_PRINT_MSG("Gateway: Entering MPID_Gateway_Unified_isend()");
    
    msgid = MPID_Gateway_get_global_msgid();
    shandle->msgid = msgid;
    
    /* create a short packet with meta data */
    metadata.mode                   = MPI_MSG;
    metadata.msg.MPI.src_comm_lrank = src_comm_lrank;
    metadata.msg.MPI.dest_grank     = dest_dev_lrank;
    metadata.msg.MPI.count          = len;
    metadata.msg.MPI.tag            = tag;
    metadata.msg.MPI.context_id     = context_id;
    metadata.msg.MPI.mode           = BLOCKING;
    metadata.msg.MPI.msgrep         = msgrep;
    metadata.msg.MPI.msgid          = msgid;
    
    /* send meta data */
    MPID_SendContig( &metadata, sizeof(Meta_Header), MPID_MyHostRank, MPIR_SEPARATE_META_HEADER_TAG, MPIR_HOST_PT2PT_CONTEXT,
		     MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, &mpi_errno );
    
    
#ifdef META_DEBUG
    fprintf ( stderr, "[h%d] MPID_Gateway_Unified_isend(): gating msg, tag = %d, len = %d, from [w%d] (communicator rank) to [a%d] (global rank) via [h%d] (global rank)\n", 
	      MPID_MyHostRank, tag, len, src_comm_lrank, dest_dev_lrank, MPIR_meta_cfg.granks_to_router[dest_dev_lrank]);
    fflush(stderr);     
#endif
    
    
    MPID_IsendContig( buf, len, MPID_MyHostRank, MPIR_SEPARATE_MSG_TAG, MPIR_HOST_PT2PT_CONTEXT,
		      MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, (MPI_Request)shandle, &mpi_errno );
    
    
    DEBUG_PRINT_MSG("Gateway: Leaving MPID_Gateway_Unified_isend()");
    
    return mpi_errno;
}

/* remember: for the gateway device, device local ranks are always global ranks */
int MPID_Gateway_Pipelined_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
				 msgrep, dty )
     void          *buf;
     int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
     MPID_Msgrep_t msgrep;
     struct MPIR_DATATYPE * dty;
{
    char *buf_part = (char *)buf;
    int mpi_errno;
    int msgid;
    Meta_Header metadata;
    
    DEBUG_PRINT_MSG("Gateway: Entering MPID_Gateway_Pipelined_send()");
    
    /* create a short packet with meta data */
    metadata.mode                   = MPI_MSG;
    metadata.msg.MPI.src_comm_lrank = src_comm_lrank;
    metadata.msg.MPI.dest_grank     = dest_dev_lrank;
    metadata.msg.MPI.count          = len;
    metadata.msg.MPI.tag            = tag;
    metadata.msg.MPI.context_id     = context_id;
    metadata.msg.MPI.mode           = BLOCKING;
    metadata.msg.MPI.msgrep         = msgrep;
    metadata.msg.MPI.msgid          = msgid;
    
    /* send meta data */
    MPID_SendContig( &metadata, sizeof(Meta_Header), MPID_MyHostRank, MPIR_SEPARATE_META_HEADER_TAG, MPIR_HOST_PT2PT_CONTEXT,
		     MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, &mpi_errno );
    
    
#ifdef META_DEBUG
    fprintf ( stderr, "[h%d] MPID_Gateway_Pipelined_isend(): gating msg, tag = %d, len = %d, from [w%d] (communicator rank)to [a%d] (global rank) via [h%d] (global rank)\n", 
	      MPID_MyHostRank, tag, len, src_comm_lrank, dest_dev_lrank, MPIR_meta_cfg.granks_to_router[dest_dev_lrank]);
    fflush(stderr);     
#endif
    
    
    while ( len > PIPELINE_BLOCKSIZE ) {
	MPID_SendContig( buf_part, PIPELINE_BLOCKSIZE, MPID_MyHostRank, MPIR_SEPARATE_MSG_TAG, MPIR_HOST_PT2PT_CONTEXT,
			 MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, &mpi_errno );
	len      -= PIPELINE_BLOCKSIZE;
	buf_part += PIPELINE_BLOCKSIZE;
    }
    MPID_SendContig( buf, len, MPID_MyHostRank, MPIR_SEPARATE_MSG_TAG, MPIR_HOST_PT2PT_CONTEXT,
		     MPIR_meta_cfg.granks_to_router[dest_dev_lrank], msgrep, &mpi_errno );
    
    
    DEBUG_PRINT_MSG("Gateway: Leaving MPID_Gateway_Pipelined_send()");
    
    return mpi_errno;
}

