#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "ntshmemdebug.h"
#include "LogMpid.h"


/*
   Nonblocking, eager shared-memory send/recv.
 */

/*
 * Hubert Ritzdorf has reported the following bug:

    As I have already reported at July 18th, there is a bug in
    ''shmemneager.c''.

    If an eager send has to be performed,

    MPID_SetupGetAddress( buf, &len, dest );

    is called to get an address in the shared memory. This works as long as
    ``p2p_shmalloc'' can allocate a shared memory region of size ``len''.
    If this is not possible, it allocates a smaller size. This size is
    returned in ``len''. But routine ''MPID_SHMEM_Eagern_isend''
    doesn't control this length and copies only the corresponding number
    number of bytes into the shared memory. And ``MPID_SHMEM_Eagerb_recv''
    tries to copy out the original length out of the shared memory.

  Hubert provided a fix, but I prefer to use one that synchronizes with
  ch_shmem (which also probably has this bug).  His fix is 

    a) I added a file ``flow.h'' in the directory ``mpid/ch_lfshmem''.
----------------- flow.h -------------------------------------
/ * Allocate shared memory if possible; otherwise use rendezvous * /

extern void  *MPID_Eager_address;

#define MPID_FLOW_MEM_OK(size,partner) \
(MPID_Eager_address = p2p_shmalloc (size))
------------------ end of flow.h -----------------------------

    b) I added the following 2 lines in ``lfshmempriv.c''

/ * Pointer used to store the address for eager sends * /
void               *MPID_Eager_address;

    c) In file ``shmemneager.c'', I add the statement

.    #include "flow.h"

    and replaced the statement

    pkt->address = MPID_SetupGetAddress( buf, &len, dest );

    by

    pkt->address = MPID_Eager_address;

 He also reports the problem with p2p.c not having MPID_myid == 0 be the 
 father process.  I believe that this is fixed now.

 */
/* Prototype definitions */
int MPID_SHMEM_Eagern_send ANSI_ARGS(( void *, int, int, int, int, int, 
				       MPID_Msgrep_t,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagern_send_local ANSI_ARGS(( void *, int, int, int, int, int, 
				       MPID_Msgrep_t,MPIR_SHANDLE*,int,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagern_isend ANSI_ARGS(( void *, int, int, int, int, int, 
					MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagern_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_wait_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_test_send ANSI_ARGS(( MPIR_SHANDLE * ));

int MPID_SHMEM_Eagern_wait_recv ANSI_ARGS(( MPIR_RHANDLE * ));
int MPID_SHMEM_Eagern_test_recv ANSI_ARGS(( MPIR_RHANDLE * ));

int MPID_SHMEM_Eagern_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Eagern_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_SHMEM_Eagern_unxrecv_start_local ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_SHMEM_Eagern_cancel_recv ANSI_ARGS((MPIR_RHANDLE*)); 
void MPID_SHMEM_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));

/* KARSTEN: Added [I]Recv prototypes */
int MPID_SHMEM_Eagern_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Eagern_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Eagern_unxrecv_cancel_local_recv ANSI_ARGS(( MPIR_RHANDLE *));
/* 
 * Blocking operations come from chbeager.c
 */
extern int MPID_SHMEM_Eagerb_send ANSI_ARGS(( void *, int, int, int, int, 
					   int, MPID_Msgrep_t ));
extern int MPID_SHMEM_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));



int MPID_SHMEM_Eagern_send_local( buf, len, src_lrank, tag, context_id, dest,
				 msgrep, shandle,is_blocking, dtypeptr )
				 void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
MPID_BLOCKING_TYPE is_blocking;
struct MPIR_DATATYPE* dtypeptr;
{
    int is_posted;
    MPIR_RHANDLE *rhandle,*dummy;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    
    MPID_Msg_arrived( src_lrank, tag, context_id, &rhandle, &is_posted );
    
    rhandle->s.MPI_TAG	  = tag;
    rhandle->s.MPI_SOURCE = src_lrank;
    
    if(is_posted) {
	msglen = len;	
	if (msglen > 0) {	    
	    if(dtypeptr) {
		msglen *= dtypeptr->size;
		MPID_CHK_MSGLEN(rhandle,msglen,err);
		if(!MPIR_Copy_flat_type(rhandle->buf,buf,len,dtypeptr)) {	
		    err=MPI_ERR_INTERN;
		    msglen = 0;
		}
	    } else {
		MPID_CHK_MSGLEN(rhandle,msglen,err);	
		MEMCPY( rhandle->buf, buf, msglen ); 
	    }
	}
	rhandle->s.count = msglen;	
	if (rhandle->finish) {
	    (rhandle->finish)( rhandle );
	}
	
	rhandle->wait	 = 0;
	shandle->wait	 = 0;
	rhandle->test	 = 0;
	shandle->test	 = 0;
	rhandle->cancel  = 0;
	rhandle->push = 0;
	rhandle->is_complete = 1;
	shandle->is_complete = 1;
    } else {
	if(is_blocking == MPID_BLOCKING && len>0) {
	/* This is really silly. Doing a blocking send to self
	    without posting a recv first leads to a deadlock...*/
	    msglen=0;
	    shandle->s.MPI_ERROR = MPI_ERR_RANK;
	    shandle->s.count = 0;
	    shandle->is_complete = 1;
	    MPID_Search_unexpected_queue( src_lrank, tag, context_id, 1, &dummy );
	    MPID_Recv_free(rhandle);
	    shandle->cancel = 0;
	    return MPI_ERR_RANK;
	} else {
	    
	    rhandle->test = MPID_SHMEM_Eagern_test_recv;
	    rhandle->wait = MPID_SHMEM_Eagern_wait_recv;
	    rhandle->push = MPID_SHMEM_Eagern_unxrecv_start_local;
	    rhandle->is_complete = 0;
	    rhandle->send_id = shandle;
	    rhandle->start   = buf;
	    rhandle->from    = src_lrank;
	    rhandle->cancel = MPID_SHMEM_Eagern_unxrecv_cancel_local_recv;
	    msglen=len;
	    
	    if(len) {
		shandle->is_complete = 0;
		shandle->test = MPID_SHMEM_Eagern_test_send;
		shandle->wait = MPID_SHMEM_Eagern_wait_send;
	    } else {
		shandle->is_complete = 1;
		shandle->test = 0;
		shandle->wait = 0;
	    }
	}
    }
    if(shandle->is_complete && shandle->finish)	
        (shandle->finish)( shandle );
    
    rhandle->s.count	  = msglen;	
    rhandle->s.MPI_ERROR  = shandle->s.MPI_ERROR = err;
    shandle->cancel  = 0;
    return err;
}

/*
 * Definitions of the actual functions
 */

int MPID_SHMEM_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle,dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE* dtypeptr;
{
    int  in_len,totallen=0;
	
    MPID_PKT_SEND_ADDRESS_TSH   *pkt;//, spkt;
    //pkt = &spkt;

	
    DEBUG_PRINT_MSG("Entereing SHMEM_Eagern_Isend");
    /* Store partners rank in request in case message is cancelled */
    shandle->partner     = dest;
    shandle->cancel = 0;

    if(dest==MPID_MyWorldRank)
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
		 msgrep, shandle,MPID_NOTBLOCKING,dtypeptr );

    
    MPID_SHMEM_GetSendPacket((MPID_PKT_TSH**)&pkt,dest);
    pkt->mode	    = MPID_PKT_LONGSH;
    pkt->context_id = context_id;
    pkt->lrank	    = src_lrank;
    pkt->tag	    = tag;
    pkt->len	    = len;
	MPID_AINT_SET(pkt->send_id,shandle);
	
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",pkt);

    /* Place in shared memory */
    in_len = len;    
#ifndef RNDV_STATIC
    if(dtype_ptr) len = len*dtypeptr->size;
    hlen = len;
    pkt->address = MPID_SetupGetAddress( buf, &len, dest );
	if (hlen != len) {
	MPID_FreeGetAddress( pkt->address );
#else
	pkt->address = MPID_SHMEM_GetRndvBuf(dest);
	if(pkt->address == NULL) {
		DEBUG_PRINT_MSG("Switching to rndv due to lack of buffer");
#endif
#ifdef SINGLECOPY
	return MPID_SHMEM_LEagern_isend( buf, in_len, src_lrank, tag, context_id,
				       dest, msgrep, shandle,dtypeptr );
#else
	return MPID_SHMEM_Rndvn_isend( buf, in_len, src_lrank, tag, context_id,
				       dest, msgrep, shandle,dtypeptr );
#endif
	}
	if(dtypeptr) {
	    pkt->len	    *= dtypeptr->size;
	    DEBUG_PRINT_MSG("Calling MPIR_Pack_flat_type\n");
	    //if(pkt->len/dtypeptr->flatcount < 16) 
		MPIR_Pack2(buf,in_len,pkt->len,dtypeptr,0,0,pkt->address,&pkt->len,&totallen);
	    /*else
		if(!MPIR_Pack_flat_type(pkt->address,buf,in_len,dtypeptr)) 
		    return MPI_ERR_INTERN;
	    */
	} else {
	    MEMCPY( pkt->address, buf, len );
	    
	}
    shandle->wait	 = 0;
    shandle->test	 = 0;
    shandle->is_complete = 1;
    /* Send as packet only */
	MPID_SHMEM_SetPacketReady((MPID_PKT_TSH*)pkt,dest);
    //MPID_SHMEM_SendControl( (MPID_PKT_TSH*)pkt, sizeof(MPID_PKT_SEND_ADDRESS_T), dest );

    DEBUG_PRINT_MSG("Leaving SHMEM_Eagern_Isend");
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_send( buf, len, src_lrank, tag, context_id, dest,msgrep,dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
struct MPIR_DATATYPE* dtypeptr;
MPID_Msgrep_t msgrep;
{
    MPIR_SHANDLE shandle;
    
    DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    DEBUG_PRINT_MSG("Entering SHMEM_Eagern_send");
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE)
	MPID_Send_init( &shandle );	
    shandle.finish = 0;  /* Just in case (e.g., Eagern_isend -> Rndvn_isend) */
    
    if(dest==MPID_MyWorldRank)
	MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	msgrep, &shandle,MPID_BLOCKING,dtypeptr );
    else 
	MPID_SHMEM_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
	msgrep, &shandle,dtypeptr );
    /* Note that isend is (probably) complete */
    if (!shandle.is_complete) {
	DEBUG_TEST_FCN(shandle.wait,"req->wait");
	shandle.wait( &shandle );
    }
    DEBUG_PRINT_MSG("Leaving SHMEM_Eagern_send");
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    return 0;
}

int MPID_SHMEM_Eagern_test_send( shandle )
MPIR_SHANDLE *shandle;
{
    /* Test for completion */
    if (shandle->is_complete == 1) {
	if (shandle->finish) 
	    (shandle->finish)( shandle );
    }
    else 
	MPID_DeviceCheck( MPID_NOTBLOCKING );

    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_wait_send( shandle )
MPIR_SHANDLE *shandle;
{
	int cnt = 0;
	LOG_SHM_WAIT_SEND(0);
    while (!shandle->is_complete) {
#ifndef WSOCK2
		MPID_DeviceCheck( MPID_BLOCKING );
#else
		IDLE(cnt)
#endif
    }
    if (shandle->finish) 
	(shandle->finish)( shandle );

	LOG_SHM_WAIT_SEND(1);
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_wait_recv( rhandle )
MPIR_RHANDLE *rhandle;
{
	int cnt = 0;
	LOG_SHM_WAIT_RECV(0);
    while (!rhandle->is_complete) {
#ifndef WSOCK2
		MPID_DeviceCheck( MPID_BLOCKING );
#else
		IDLE(cnt)
#endif
    }
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );

	LOG_SHM_WAIT_RECV(1);
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_test_recv( rhandle )
MPIR_RHANDLE *rhandle;
{
	if (rhandle->is_complete == 1) {
	if (rhandle->finish) 
	    (rhandle->finish)( rhandle );
    }
    else 
	MPID_DeviceCheck( MPID_NOTBLOCKING );

    return MPI_SUCCESS;
}

/*
 * This is the routine called when a packet of type MPID_PKT_LONG is
 * seen that was expected.
 */
int MPID_SHMEM_Eagern_recv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_TSH   *pkt = (MPID_PKT_SEND_ADDRESS_TSH *)in_pkt;
    int    msglen, err = MPI_SUCCESS,dummy = 0;
    int	   totallen=0;
    
    msglen = pkt->len;
    
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;

#if 0
    This does not work, since MPI_BOTTOM is zero,
    which causes rhandle->start to be zero too, even if the message 
    has been packed by MPI_SEND!!!
    if(rhandle->count && !rhandle->start) {
	/* This is a noncontig message, that we have to handle here...*/
	
	if(rhandle->len>msglen) {
	    rhandle->start = rhandle->buf;
	    MPIR_Unpack_flat_restricted(&rhandle->start,pkt->address,&dummy,msglen,rhandle->datatype);
	} else {
	    //if(pkt->len/rhandle->datatype->flatcount < 16) 
		MPIR_Unpack2(pkt->address,rhandle->count,rhandle->datatype,0,0,rhandle->buf,msglen,&len1,&totallen);
	    //else
		//MPIR_Unpack_flat_type(rhandle->buf,pkt->address,rhandle->count,rhandle->datatype);
	}
	
    } else 
#endif
    {
	MEMCPY( rhandle->buf, pkt->address, msglen );
    }
#ifndef RNDV_STATIC
    MPID_FreeGetAddress( pkt->address );
#else
    MPID_SHMEM_FreeRndvBuf(from,pkt->address);
#endif

    MPID_PKT_READY_CLR(&(pkt->ready));
    SIGNAL(from)
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }    
    rhandle->is_complete = 1;    
	
    return err;
}


int MPID_SHMEM_Eagern_cancel_recv(runex) 
	MPIR_RHANDLE *runex;
{
#ifndef RNDV_STATIC
	MPID_FreeGetAddress( runex->start );
#else
	MPID_SHMEM_FreeRndvBuf(runex->from,runex->start);
#endif
    MPID_Recv_free( runex );
	return 0;
}
/* Save an unexpected message in rhandle */
int MPID_SHMEM_Eagern_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_TSH *pkt = (MPID_PKT_SEND_ADDRESS_TSH *)in_pkt;
    
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 0;
    /* Save the address */
    rhandle->start        = pkt->address;
    MPID_AINT_SET(rhandle->send_id,pkt->send_id);
    
#ifdef RNDV_STATIC
    rhandle->from		  =	from;
#endif
    
    MPID_PKT_READY_CLR(&(pkt->ready));
    SIGNAL(from)
	rhandle->push = MPID_SHMEM_Eagern_unxrecv_start;
    rhandle->cancel = MPID_SHMEM_Eagern_cancel_recv;
    return 0;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SHMEM_Eagern_unxrecv_start( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0,dummy=0;


    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
     
    /* Copy the data from the local area and free that area */
    if (msglen > 0) {
#if 0
    This does not work, since MPI_BOTTOM is zero,
    which causes rhandle->start to be zero too, even if the message 
    has been packed by MPI_SEND!!!
	if(rhandle->count && !rhandle->start) {
	    /* This is a noncontig message, that we have to handle here...*/
	    if(rhandle->len>msglen) {
		rhandle->start = rhandle->buf;
		MPIR_Unpack_flat_restricted(&rhandle->start,runex->start,&dummy,msglen,rhandle->datatype);
	    } else {
		MPIR_Unpack_flat_type(rhandle->buf,runex->start,rhandle->count,rhandle->datatype);
	    }
	} else 
#endif
	{
	    MEMCPY( rhandle->buf, runex->start, msglen );
	}
#ifndef RNDV_STATIC
	MPID_FreeGetAddress( runex->start );
#else
	MPID_SHMEM_FreeRndvBuf(runex->from,runex->start);
#endif

    }
    rhandle->s		 = runex->s;
    MPID_Recv_free( runex );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );

    return err;
}


int MPID_SHMEM_Eagern_unxrecv_cancel_local_recv( runex )
MPIR_RHANDLE *runex;
{
    /*MPIR_SHANDLE *shandle = (MPIR_SHANDLE *)runex->send_id;*/
    MPID_Recv_free( runex );
	return 0;

}

int MPID_SHMEM_Eagern_unxrecv_start_local( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    MPIR_SHANDLE *shandle = (MPIR_SHANDLE *)runex->send_id;
    
    int  msglen, err = 0;
    
    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    if (runex->s.count > 0) {
	MEMCPY( rhandle->buf, runex->start, msglen );
	shandle->is_complete = 1;
    }
    rhandle->s		 = runex->s;
    MPID_Recv_free( runex );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    
    
    return err;
}

void MPID_SHMEM_Eagern_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_SHMEM_Eagern_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SHMEM_Eagern_send;
    p->recv	   = MPID_SHMEM_Eagern_recv;
    p->isend	   = MPID_SHMEM_Eagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;/*MPID_SHMEM_Eagern_irecv;*/
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_SHMEM_Eagern_save;
    p->delete      = MPID_SHMEM_Eagern_delete;

    return p;
}
