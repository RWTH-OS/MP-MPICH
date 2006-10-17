/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "../util/queue.h"

MPID_Protocol *MPID_CH_Self_setup(void);

int MPID_CH_Self_send( void *buf, int len, int src_lrank, int tag, 
		       int context_id, int dest, MPID_Msgrep_t msgrep );
int MPID_CH_Self_isend( void *buf, int len, int src_lrank, int tag, 
		       int context_id, int dest, MPID_Msgrep_t msgrep,
			MPIR_SHANDLE *shandle );
void MPID_CH_Self_delete(MPID_Protocol * );
int MPID_CH_Self_Check_incoming(MPID_Device *dev,MPID_BLOCKING_TYPE blocking);
int MPID_CH_Self_Abort( struct MPIR_COMMUNICATOR * comm_ptr, int code, char *msg );
int MPID_CH_Self_End( MPID_Device * dev );
int MPID_CH_Self_push( MPIR_RHANDLE *rhandle, void *in_runex );

int MPID_CH_Self_long_len( int );
int MPID_CH_Self_vlong_len( int );

static unsigned int gmpi_self_sync_send;

static int MPID_CH_Self_long_len_value;
static int MPID_CH_Self_vlong_len_value;

MPID_Device *MPID_CH_InitSelfMsg(int * argc, char ***argv, int short_len, int long_len)
{
    MPID_Device *dev;
    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev) return 0;
    MPID_CH_Self_long_len_value = 0;
    dev->long_len     = &MPID_CH_Self_long_len;
    MPID_CH_Self_vlong_len_value = 0;
    dev->vlong_len    = &MPID_CH_Self_vlong_len;

    dev->short_msg    = MPID_CH_Self_setup();
    dev->long_msg     = dev->short_msg;
    dev->vlong_msg    = dev->short_msg;
    dev->eager        = dev->short_msg;
    dev->rndv         = dev->short_msg;
    dev->check_device = MPID_CH_Self_Check_incoming;
    dev->terminate    = MPID_CH_Self_End;
    dev->abort	      = MPID_CH_Self_Abort;
    dev->next         = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
    return dev;
}



MPID_Protocol *MPID_CH_Self_setup(void)
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_CH_Self_send;
    p->recv	   = 0;
    p->isend	   = MPID_CH_Self_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = 0; /* could be save, but directly called from send */
    p->delete      = MPID_CH_Self_delete;

    return p;
}


int MPID_CH_Self_abort( MPIR_SHANDLE *shandle )
{
  fprintf(stderr,"Error: blocking send or recv on a self communication\n");
  gmpi_abort (0);

  return 0;   /* can't reach this; it's just to placate the sgi compiler */
}

int MPID_CH_Self_isend(void *buf, int len, int src_lrank, int tag, 
		       int context_id, int dest, MPID_Msgrep_t msgrep,
		       MPIR_SHANDLE *shandle )
{
  MPIR_RHANDLE *rhandle;
  int is_posted;
  
  DEBUG_PRINT_MSG("S Starting Self_isend");
  /* don't know how to add pack control stuff here */
  
  if (gmpi_self_sync_send == 0)
    {
      GMPI_PROGRESSION_LOCK();
    }
  MPID_Msg_arrived( src_lrank, tag, context_id, 
		    &rhandle, &is_posted );
  
  if (is_posted)
    {
      int msglen = len;
      int err = 0;

      /* FIXME: what should be done there in case rhandle->len < msglen */
      MPID_CHK_MSGLEN(rhandle,msglen,err);
      gm_bcopy(buf, rhandle->buf, msglen);
      rhandle->s.MPI_TAG	  = tag;
      rhandle->s.MPI_SOURCE = src_lrank;
      rhandle->s.MPI_ERROR  = err;
      rhandle->s.count      = msglen;
      rhandle->is_complete = 1;
      if (rhandle->finish)
	{
	  (rhandle->finish)(rhandle);
	}
      shandle->is_complete = 1;
      if (shandle->finish)
	{
	  (shandle->finish)(shandle);
	}
      gmpi_debug_assert(shandle->gm.ptr == NULL);
    } 
  else
    {
      shandle->wait = MPID_CH_Self_abort;
      shandle->test = 0;
      shandle->is_complete = 0;
      
      rhandle->s.MPI_TAG    = tag;
      rhandle->s.MPI_SOURCE = src_lrank;
      rhandle->s.MPI_ERROR  = 0;
      rhandle->s.count      = len;
      rhandle->from         = MPID_GM_rank;
      rhandle->buf          = buf;
      rhandle->start        = 0;
      rhandle->is_complete  = 0;
      MPID_AINT_SET(rhandle->send_id, shandle);
      rhandle->push = MPID_CH_Self_push;
    }
  
  if (gmpi_self_sync_send == 0)
    {
      GMPI_PROGRESSION_UNLOCK();
    }
  return 0;
}

int MPID_CH_Self_send( void *buf, int len, int src_lrank, int tag, 
		       int context_id, int dest, MPID_Msgrep_t msgrep )
{
  MPIR_RHANDLE *rhandle;
  int is_posted;

  DEBUG_PRINT_MSG("S Starting Self_send");

  GMPI_PROGRESSION_LOCK();
  gmpi_self_sync_send = 1;
  
  MPID_Msg_arrived( src_lrank, tag, context_id, 
		    &rhandle, &is_posted );
  
  if (is_posted)
    {
      int msglen = len;
      int err = 0;
      
      /* FIXME: what should be do there in case rhandle->len < msglen */
      MPID_CHK_MSGLEN(rhandle,msglen,err);
      gm_bcopy(buf, rhandle->buf, msglen);
      rhandle->s.MPI_TAG	  = tag;
      rhandle->s.MPI_SOURCE = src_lrank;
      rhandle->s.MPI_ERROR  = err;
      rhandle->s.count      = msglen;
      rhandle->is_complete = 1;
      if (rhandle->finish)
	{
	  (rhandle->finish)(rhandle);
	}
    } 
  else 
    {
      /* This is bad, it should never happen, it's a programming 
	 fault in the application */
      if (len < gmpi.eager_size) 
	{
	  rhandle->s.MPI_TAG	= tag;
	  rhandle->s.MPI_SOURCE = src_lrank;
	  rhandle->s.MPI_ERROR  = 0;
	  rhandle->s.count      = len;
	  rhandle->from         = MPID_GM_rank;
	  rhandle->is_complete  = 0;
	  
	  rhandle->start = malloc(len);
	  gmpi_malloc_assert(rhandle->start,
			     "MPID_CH_Self_send",
			     "MALLOC:unexpected buffer");
	  if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    return 1;
	  }
	  memcpy(rhandle->start, buf , len);
	  rhandle->push = MPID_CH_Self_push;
	}
      else 
	{
	  MPID_CH_Self_abort(NULL);
	}
    }

  gmpi_self_sync_send = 0;
  GMPI_PROGRESSION_UNLOCK();
 
  return 0;
}



int MPID_CH_Self_push( MPIR_RHANDLE *rhandle, void *in_runex )
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    MPIR_SHANDLE *shandle;
    int          msglen, err = 0;

    DEBUG_PRINT_MSG("S Starting Self_push");
    MPID_AINT_GET(shandle,runex->send_id);

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    if (runex->start != 0) {
      /* Copy the data from the local area and free that area */
      if (msglen > 0) {
	MEMCPY( rhandle->buf, runex->start, msglen );
	FREE(runex->start);
      }
    } else {
      /* data is in place, no intermediate buffer to free */
      if (msglen > 0) {
	MEMCPY( rhandle->buf, runex->buf, msglen );
      }
    }
    if (shandle) {
      shandle->is_complete = 1;
      if (shandle->finish)
	(shandle->finish)(shandle);
    }

    /* MPID_FLOW_MEM_RECV(msglen,runex->from); */
    MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
    rhandle->s		 = runex->s;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    MPID_RecvFree( runex );
    return err;
}


void MPID_CH_Self_delete(MPID_Protocol *p)
{
  DEBUG_PRINT_MSG("S Starting Self_delete");

  FREE( p );
}

int MPID_CH_Self_Check_incoming(MPID_Device *dev,MPID_BLOCKING_TYPE blocking)
{
  return -1;
}

int MPID_CH_Self_End(MPID_Device *dev)
{
  DEBUG_PRINT_MSG("Entering MPID_CH_Self_end\n");
  /* Finish off any pending transactions */
  /* MPID_CH_Complete_pending(); */
  
  
  if (MPID_GetMsgDebugFlag()) {
    MPID_PrintMsgDebug();
  }
  GMPI_PROGRESSION_LOCK();
  (dev->short_msg->delete)( dev->short_msg );
  FREE( dev );
  /* We should really generate an error or warning message if there 
     are uncompleted operations... */
  GMPI_PROGRESSION_UNLOCK();
  return 0;
}

int MPID_CH_Self_Abort(struct MPIR_COMMUNICATOR *comm_ptr, int code, char *msg)
{
  if (msg) {
    fprintf( stderr, "[%d] %s\n", MPID_GM_rank, msg );
  } else {
    fprintf( stderr, "[%d] Aborting program!\n", MPID_GM_rank );
  }
  fflush( stderr );
  fflush( stdout );

  /* Some systems (e.g., p4) can't accept a (char *)0 message argument. */
  SYexitall( "", code );
  return 0;
}

int MPID_CH_Self_long_len( int partner_devlrank )
{
    return MPID_CH_Self_long_len_value;
}


int MPID_CH_Self_vlong_len( int partner_devlrank )
{
    return MPID_CH_Self_vlong_len_value;
}

