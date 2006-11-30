/*
 *  $Id$
 *
 */


#ifdef SINGLECOPY

/* This file implements a single-copy Eager protocol
   that can be used on machines that provied direct access
   to the address space of processes on the sam machine.
   Currently it is used only on NT-machines.
*/
   

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "ntshmemdebug.h"
#include "LogMpid.h"


/* Prototype definitions */
int MPID_SHMEM_LEagern_send ANSI_ARGS(( void *, int, int, int, int, int, 
				       MPID_Msgrep_t,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagern_send_local ANSI_ARGS(( void *, int, int, int, int, int, 
				       MPID_Msgrep_t,MPIR_SHANDLE*,int,struct MPIR_DATATYPE* ));
int MPID_SHMEM_LEagern_isend ANSI_ARGS(( void *, int, int, int, int, int, 
					MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE* ));
int MPID_SHMEM_LEagern_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_LEagern_wait_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_LEagern_test_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_wait_recv ANSI_ARGS(( MPIR_RHANDLE * ));
int MPID_SHMEM_Eagern_test_recv ANSI_ARGS(( MPIR_RHANDLE * ));

int MPID_SHMEM_LEagern_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_LEagern_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_SHMEM_Eagern_unxrecv_start_local ANSI_ARGS(( MPIR_RHANDLE *, void * ));
void MPID_SHMEM_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));

/* KARSTEN: Added [I]Recv prototypes */
/*int MPID_SHMEM_LEagern_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));*/
int MPID_SHMEM_LEagern_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Rndvn_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Rndvn_ack ANSI_ARGS(( void *, int ));

/* 
 * Blocking operations come from chbeager.c
 */
extern int MPID_SHMEM_Eagerb_send ANSI_ARGS(( void *, int, int, int, int, 
					   int, MPID_Msgrep_t,struct MPIR_DATATYPE* ));
extern int MPID_SHMEM_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));

VOLATILE int* GetFlag ANSI_ARGS((void));
void FreeFlag ANSI_ARGS((volatile int *flag));

//#define PACK_RNDV


/*
 * Definitions of the actual functions
 */

int MPID_SHMEM_LEagern_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle,dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE* dtypeptr;
{
    int  in_len;
    
    MPID_PKT_SEND_LADDRESS_TSH   *pkt; //, spkt;
    //pkt = &spkt;
    
    /* Store partners rank in request in case message is cancelled */
    shandle->partner     = dest;
    
    if(dest==MPID_MyWorldRank)
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	msgrep, shandle,MPID_NOTBLOCKING,dtypeptr );
    
    

    if(dtypeptr) {
	int d1,d2=0;
#ifndef PACK_RNDV
	shandle->curr_buf = MALLOC(len*dtypeptr->size);
	if(!shandle->curr_buf) {
	    shandle->s.MPI_ERROR = MPI_ERR_EXHAUSTED;
	    return shandle->s.MPI_ERROR;
	}
	/*if(!MPIR_Pack_flat_type(shandle->curr_buf,buf,len,dtypeptr)) {
	    shandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    return MPI_ERR_INTERN;
	}*/
	
	MPIR_Pack2(buf,len,len*dtypeptr->size,dtypeptr,0,0,shandle->curr_buf,&d1,&d2);
	//len *= dtypeptr->size;
	len = d2;
	buf = shandle->curr_buf;
#else
#error RNDV!!!
	return MPID_SHMEM_Rndvn_isend(buf,len,src_lrank,tag,context_id,dest,msgrep,shandle,dtypeptr);
#endif
    } else shandle->curr_buf = 0;

    MPID_SHMEM_GetSendPacket((MPID_PKT_TSH**)&pkt,dest);
    MPID_AINT_SET(pkt->send_id,shandle);

    pkt->mode	    = MPID_PKT_REQUEST_SENDSH;
    pkt->context_id = context_id;
    pkt->lrank	    = src_lrank;
    pkt->tag	    = tag;
    pkt->len	    = len;
    
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",pkt);
    
    /* Place in shared memory */
    in_len = len;
    
    pkt->address = buf;
    pkt->PID=GetCurrentProcessId();
    
    /* this is just for performance testing...*/
    pkt->read_complete = (int*)GetFlag();//(int*)& shandle->is_complete;
    if(!pkt->read_complete) return MPI_ERR_INTERN;
    
    shandle->flag = pkt->read_complete;
    *pkt->read_complete =0;
    
    
    shandle->test = MPID_SHMEM_LEagern_test_send;
    shandle->wait = MPID_SHMEM_LEagern_wait_send;
    /* Set the cancel routine, to free the flag if canceled*/
    shandle->cancel = MPID_SHMEM_LEagern_cancel_send;
    shandle->is_complete = 0;
    /* Send as packet only */
    MPID_SHMEM_SetPacketReady((MPID_PKT_TSH*)pkt,dest);
    //MPID_SHMEM_SendControl( (MPID_PKT_TSH*)pkt, sizeof(MPID_PKT_SEND_LADDRESS_T), dest );
    
    return MPI_SUCCESS;
}

int MPID_SHMEM_LEagern_send( buf, len, src_lrank, tag, context_id, dest,
			    msgrep,dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE* dtypeptr;
{
    MPIR_SHANDLE shandle;

    DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE)
    MPID_Send_init( &shandle );	
    shandle.finish = 0;

    if(dest==MPID_MyWorldRank)
	MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	msgrep, &shandle,MPID_BLOCKING,dtypeptr );
    else {
	MPID_SHMEM_LEagern_isend( buf, len, src_lrank, tag, context_id, dest,
	    msgrep, &shandle,dtypeptr );
    }
    if (!shandle.is_complete) {
	DEBUG_TEST_FCN(shandle.wait,"req->wait");
	shandle.wait( &shandle );
    }
    
    return MPI_SUCCESS;
}

int MPID_SHMEM_LEagern_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    FreeFlag(shandle->flag);
    if(shandle->curr_buf) FREE(shandle->curr_buf);
    return 0;
}

int MPID_SHMEM_LEagern_wait_send( shandle )
MPIR_SHANDLE *shandle;
{
    int count=0;
    LOG_SHM_WAIT_SEND(0);
    shandle->is_complete = *(shandle->flag);
    while (!shandle->is_complete) {
	IDLE(count)
	/*MPID_DeviceCheck( MPID_NOTBLOCKING );*/
	shandle->is_complete = *(shandle->flag);
    }
    FreeFlag(shandle->flag);
    if(shandle->curr_buf) FREE(shandle->curr_buf);
    if (shandle->finish) 
	(shandle->finish)( shandle );
    
    LOG_SHM_WAIT_SEND(1);
    return MPI_SUCCESS;
}

int MPID_SHMEM_LEagern_test_send( shandle )
MPIR_SHANDLE *shandle;
{
    /* Test for completion */
    shandle->is_complete = *(shandle->flag);
    if (shandle->is_complete == 1) {
	FreeFlag(shandle->flag);
	if(shandle->curr_buf) FREE(shandle->curr_buf);
	if (shandle->finish) 
	    (shandle->finish)( shandle );
    }
    else 
	MPID_DeviceCheck( MPID_NOTBLOCKING );
    
    return MPI_SUCCESS;
}

#ifdef FOO
/*
 * This is the routine called when a packet of type MPID_PKT_SEND_ADDRESS is
 * seen.  It receives the data as shown (final interface not set yet)
 */
int MPID_SHMEM_LEagern_recv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    
	MPID_PKT_SEND_LADDRESS_T   *pkt = (MPID_PKT_SEND_LADDRESS_T*)in_pkt;
    int    msglen, err = MPI_SUCCESS;
	int pid,*read_complete;
	void *address;

	address = pkt->address;
	pid = pkt->PID;
	read_complete = pkt->read_complete;
    msglen = pkt->len;
	
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_ERROR = err;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;

	MPID_PKT_READY_CLR(&(pkt->ready));
	MPID_SHMEM_CopyFromProcess(pid,rhandle->buf,address,msglen,read_complete);
	
	SIGNAL(from)
    
	if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    
    rhandle->is_complete = 1;
    
    return err;
}
#endif

/* This routine is called when a message (MPID_PKT_REQUEST_SEND) 
   arrives and was expected */
int MPID_SHMEM_LEagern_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_LADDRESS_TSH *pkt = (MPID_PKT_SEND_LADDRESS_TSH *)in_pkt;
    int    msglen, err = MPI_SUCCESS;
    
    int pid,*read_complete;
    void *address;
    int unpack=0,dummy=0;
    
    address = pkt->address;
    pid = pkt->PID;
    read_complete = pkt->read_complete;
    msglen = pkt->len;
    
    
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;
    
    
    MPID_PKT_READY_CLR(&(pkt->ready));
#if 0
    This does not work, since MPI_BOTTOM is zero,
    which causes MPI start to be zero too, even if the message 
    has been packed by MPI_SEND!!!
    if(rhandle->count && !rhandle->start) {
	/* This is a noncontig message, that we have to handle here...*/
	rhandle->start = rhandle->buf;
	rhandle->buf = MALLOC(msglen);
	unpack = 1;
	if(!rhandle->buf) {
	    rhandle->s.MPI_ERROR = MPI_ERR_EXHAUSTED;
	    rhandle->buf = rhandle->start;
	    return rhandle->s.MPI_ERROR;
	}
    }
#endif
    MPID_SHMEM_CopyFromProcess(pid,rhandle->buf,address,msglen,read_complete);
    SIGNAL(from)

    if(unpack) {
	int d1,d2;
	/* This is a noncontig message, that we have to handle here...*/
	if(rhandle->len>msglen) {
	    address = rhandle->start;
	    MPIR_Unpack_flat_restricted(&address,rhandle->buf,&dummy,msglen,rhandle->datatype);
	} else {
	    //MPIR_Unpack2(pkt->address,rhandle->count,rhandle->datatype,0,0,rhandle->buf,msglen,&len1,&totallen);
	    MPIR_Unpack2(rhandle->buf,rhandle->count,rhandle->datatype,0,0,rhandle->start,msglen,&d1,&d2);
	    //MPIR_Unpack_flat_type(rhandle->start,rhandle->buf,rhandle->count,rhandle->datatype);
	}
	FREE(rhandle->buf);
	rhandle->buf = rhandle->start;
    }
	
    if (rhandle->finish)
	(rhandle->finish)( rhandle );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    
    return err;
}

int MPID_SHMEM_LEagern_cancel_recv(MPIR_RHANDLE*); 
/* Cancel a previously saved message*/
int MPID_SHMEM_LEagern_cancel_recv(runex) 
MPIR_RHANDLE *runex;
{
	*(VOLATILE int*)runex->buf = 1;
	MPID_Recv_free( runex );
	return 0;
}


/* Save an unexpected message in rhandle (MPID_PKT_REQUEST_SEND and unexpected)*/
int MPID_SHMEM_LEagern_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_LADDRESS_TSH *pkt = (MPID_PKT_SEND_LADDRESS_TSH *)in_pkt;
    
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 0;
    MPID_AINT_SET(rhandle->send_id,pkt->send_id);
    /* Save the address */
    rhandle->start        = pkt->address;
    
    rhandle->buf		  = pkt->read_complete;
    rhandle->from		  = from;
    rhandle->contextid	  = pkt->PID;
    MPID_PKT_READY_CLR(&(pkt->ready));
    if(pkt->len<=0) *pkt->read_complete=1;
    
    SIGNAL(from)
	
    rhandle->push = MPID_SHMEM_LEagern_unxrecv_start;
    rhandle->cancel = MPID_SHMEM_LEagern_cancel_recv;
    return 0;
}

int MPID_SHMEM_very_large_save( MPIR_RHANDLE *rhandle,int from, void *in_pkt )
{
    if(((MPID_PKT_TSH*)in_pkt)->head.mode == MPID_PKT_REQUEST_SENDSH)
	return MPID_SHMEM_LEagern_save(rhandle,from,in_pkt);
    else 
	return MPID_SHMEM_Rndvn_save(rhandle,from,in_pkt);
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SHMEM_LEagern_unxrecv_start( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
	MPID_SHMEM_CopyFromProcess(runex->contextid,rhandle->buf,runex->start,msglen,runex->buf);
	SIGNAL(runex->from)
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


MPID_Protocol *MPID_SHMEM_LEagern_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SHMEM_LEagern_send;
    p->recv	   = MPID_SHMEM_Rndvn_irecv; /*MPID_SHMEM_LEagern_recv;*/
    p->isend	   = MPID_SHMEM_LEagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = MPID_SHMEM_LEagern_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = MPID_SHMEM_Rndvn_ack;
    p->unex        = MPID_SHMEM_very_large_save;/*MPID_SHMEM_LEagern_save;*/
    p->delete      = MPID_SHMEM_Eagern_delete;

	return p;
}

#endif /* SINGLECOPY*/
