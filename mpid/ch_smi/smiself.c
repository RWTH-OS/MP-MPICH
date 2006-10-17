/* $Id$ */

#include <unistd.h>

#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "reqalloc.h"
#include "smidef.h"
#include "smistat.h"
#include "queue.h"

#include "smi.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* 
   shared memory by rendezvous - special case of sending a message to myself. 
*/

/* external prototypes */
int MPID_SMI_Isend_self ( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *);
int MPID_SMI_Send_self ( void *, int, int, int, int, int, MPID_Msgrep_t);

/* internal prototypes */
int MPID_SMI_Recv_self (MPIR_RHANDLE *, void *);
int MPID_SMI_Recv_self_test ( MPIR_RHANDLE *);
int MPID_SMI_Recv_self_wait ( MPIR_RHANDLE *);
int MPID_SMI_Send_self_test ( MPIR_SHANDLE *);
int MPID_SMI_Send_self_wait ( MPIR_SHANDLE *);


/* implementations */

int MPID_SMI_Isend_self ( buf, len, src_comm_lrank, tag, context_id, dest, msgrep, shandle )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
{
    MPIR_RHANDLE *rhandle;
    int err = MPI_SUCCESS;
    int is_posted, msglen;
    char *tmp_buf;

    MPID_Msg_arrived (src_comm_lrank, tag, context_id, &rhandle, &is_posted);
    rhandle->s.MPI_TAG    = tag;
    rhandle->s.MPI_SOURCE = src_comm_lrank;
    
    msglen = len;

    if (is_posted) {
	rhandle->s.count     = msglen;

	MPID_CHK_MSGLEN(rhandle,msglen,err);
	if (msglen > 0) {
	    if (rhandle->buf)
		memcpy (rhandle->buf, buf, len);
	    else {
		/* XXX This is only a first workaround!!! It would be better to avoid a
		   temp buffer completely and use ff to copy the data from src to dest!!! */
		ALLOCATE (tmp_buf, char *, len);
		MPID_SMI_Pack_ff ((char *)buf, rhandle->datatype, tmp_buf, MPID_SMI_myid, len, 0);
		MPID_SMI_UnPack_ff (tmp_buf, rhandle->datatype, (char *) rhandle->start, MPID_SMI_myid, len, 0);
		free (tmp_buf);
	    }
	}
	if (rhandle->finish) {
	    (rhandle->finish)( rhandle );
	}
	shandle->is_complete  = 1;
	if (shandle->finish)
	    (shandle->finish)( shandle );
	shandle->wait = 0;
	shandle->test = 0;

  	rhandle->is_complete  = 1;
	rhandle->wait = 0;
	rhandle->test = 0;
	rhandle->push = 0;	
	
	rhandle->s.MPI_ERROR = err;
    } else {
	shandle->is_complete  = 0;
	shandle->wait = MPID_SMI_Send_self_wait;
	shandle->test = MPID_SMI_Send_self_test;

   	rhandle->is_complete  = 0;
	rhandle->wait = MPID_SMI_Recv_self_wait;
	rhandle->test = MPID_SMI_Recv_self_test;
	rhandle->push = MPID_SMI_Recv_self;
	rhandle->send_id  = shandle;
	rhandle->unex_buf = buf;
	rhandle->count    = msglen;
	rhandle->from     = src_comm_lrank;
    }
    rhandle->is_valid = 1;

    return err;
}

int MPID_SMI_Send_self ( buf, len, src_comm_lrank, tag, context_id, dest, msgrep )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest;
MPID_Msgrep_t msgrep;
{
    MPIR_RHANDLE *rhandle;
    int err = MPI_SUCCESS;
    int is_posted, msglen;
    char *tmp_buf;

    MPID_Msg_arrived (src_comm_lrank, tag, context_id, &rhandle, &is_posted);
    rhandle->s.MPI_TAG    = tag;
    rhandle->s.MPI_SOURCE = src_comm_lrank;
    
    msglen = len;

    if (is_posted) {
	MPID_CHK_MSGLEN(rhandle,msglen,err);
	if (msglen > 0) {
	    if (rhandle->buf) 
		memcpy (rhandle->buf, buf, len);
	    else {
		/* XXX This is only a first workaround!!! It would be better to avoid a
		   temp buffer completely and use ff to copy the data from src to dest!!! */
		ALLOCATE (tmp_buf, char *, len);
		MPID_SMI_Pack_ff ((char *)buf, rhandle->datatype, tmp_buf, MPID_SMI_myid, len, 0);
		MPID_SMI_UnPack_ff (tmp_buf, rhandle->datatype, (char *) rhandle->start, MPID_SMI_myid, len, 0);
		free (tmp_buf);
	    }
	}
	if (rhandle->finish) {
	    (rhandle->finish)( rhandle );
	}

  	rhandle->is_complete  = 1;
	rhandle->wait = 0;
	rhandle->test = 0;
	rhandle->push = 0;	
	
	rhandle->s.count     = msglen;
	rhandle->s.MPI_ERROR = err;
    } else {
	/* this case should not happen and indicates an incorrect MPI programm
	   which calls a blocking send before posting the matching receive => deadlock */

	/* we can't procede at this point since we have no send-handle available */
	MPID_ABORT("Can not send to myself w/o a matching posted recv.");

    }

    return err;
}


/* Receiving a message which has been sent via Isend() (and with the recv not being posted 
   at this time of sending). */
int MPID_SMI_Recv_self (rhandle, in_runex)
MPIR_RHANDLE *rhandle;
void *in_runex;
{
    MPIR_RHANDLE *unex_rhandle = (MPIR_RHANDLE *)in_runex;
    MPIR_SHANDLE *shandle;
    int msglen = unex_rhandle->count;
    int err = MPI_SUCCESS;
    char *tmp_buf;

    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* unex_randle->unex_buf is the actual user-provided send buffer. */

    if (rhandle->buf)
	memcpy (rhandle->buf, unex_rhandle->unex_buf, msglen);
    else {
	/* XXX This is only a first workaround!!! It would be better to avoid a
	   temp buffer completely and use ff to copy the data from src to dest!!! */
	ALLOCATE (tmp_buf, char *, msglen);
	MPID_SMI_Pack_ff ((char *)unex_rhandle->unex_buf, rhandle->datatype, tmp_buf, MPID_SMI_myid, msglen, 0);
	MPID_SMI_UnPack_ff (tmp_buf, rhandle->datatype, (char *) rhandle->start, MPID_SMI_myid, msglen, 0);
	free (tmp_buf);
    }
    
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    /* set status */
    rhandle->s.count      = unex_rhandle->s.count;
    rhandle->s.MPI_TAG    = unex_rhandle->s.MPI_TAG;
    rhandle->s.MPI_SOURCE = unex_rhandle->s.MPI_SOURCE;
    rhandle->s.MPI_ERROR  = err;
    
    /* now we are finished */
    rhandle->is_complete  = 1;
    MPID_AINT_GET(shandle, unex_rhandle->send_id);
    shandle->is_complete  = 1;

    return err;
}


/* For a correctly written MPI programm, the Recv_self_test and Recv_self_wait 
   functions do always return immedeately  indicating that the recv is complete. If not, 
   the programm must be doing a MPI_Test or MPI_Recv without a prior MPI_*Recv, 
   which is just nonsense. */
int MPID_SMI_Recv_self_test ( rhandle )
MPIR_RHANDLE *rhandle;
{
    if ( rhandle->is_complete )
	return MPI_SUCCESS;
    else
	return MPIR_ERR_NOMATCH;
}

int MPID_SMI_Recv_self_wait ( rhandle )
MPIR_RHANDLE *rhandle;
{
    if ( rhandle->is_complete )
	return MPI_SUCCESS;
    else
	return MPIR_ERR_NOMATCH;
}


/* we can't do anything here then waiting for the Recv
   to do its job */
int MPID_SMI_Send_self_test ( shandle )
MPIR_SHANDLE *shandle;
{
    return MPI_SUCCESS;
}

int MPID_SMI_Send_self_wait ( shandle )
MPIR_SHANDLE *shandle;
{
    int backoff = 0;
    int wait;
    
    while ( !shandle->is_complete ) {
	usleep (backoff);
	/* backoff to release bus - this code should not be optimized! */
	if (++backoff % MPID_SMI_BACKOFF_LMT) {
	    wait = backoff;
	    while (wait)
		wait--;
	}
    }

    return MPI_SUCCESS;
}

