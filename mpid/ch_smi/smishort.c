/* $Id$ */

/* blocking send and receive of short messages which fit into a control packet */

#include "smidef.h"
#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "reqalloc.h"
#include "smistat.h"
#include "smicheck.h"
#include "smidebug.h"

#ifdef WIN32
#include <crtdbg.h>
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Prototype definitions */
int MPID_SMI_Short_init ( void );
int MPID_SMI_Short_MemSetup ( void );
void MPID_SMI_Short_MemInit ( int proc );

int MPID_SMI_Short_send ( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *);
int MPID_SMI_Short_isend ( void *, int, int, int, int, int, 
						   MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE *);
int MPID_SMI_Short_recv ( MPIR_RHANDLE *, int, void * );
int MPID_SMI_Short_save ( MPIR_RHANDLE *, int, void *);
int MPID_SMI_Short_cancel_recv( MPIR_RHANDLE * );
int MPID_SMI_Short_unxrecv_start ( MPIR_RHANDLE *, void * );
void MPID_SMI_Short_delete ( MPID_Protocol * );

/* exports */ 
int MPID_SMI_SHORTSIZE;     /* size of the short message incl. headers */
int MPID_SMI_SHORTBUFS;     /* nbr of short messages in the queues between each two processes */
/* process-local information for sending & receiving control packets  */
MPID_SMI_lShortsend_t *MPID_SMI_lShortsend;
MPID_SMI_lShortrecv_t *MPID_SMI_lShortrecv;

/* global variables for the short protocol */
/* base adresses of the SMI shared regions located at the corresponding node */
char **MPID_SMI_shmem_short;
/* the SMI region ID */
static int SMI_Shregid_short;
/* base address of local memory for communication between processes running on the same node */
static char *MPID_SMI_local_short = 0;
/* the corresponding SMI region ID */
static int SMI_Locregid_short = 0;

/* local rank of processes, declared and initialized in smiinit.c */
extern int *MPID_SMI_localRankForProc;

/*
 * Definitions of the actual functions
 */

MPID_Protocol *MPID_SMI_Short_setup()
{
    MPID_Protocol *p;

    ZALLOCATE(p, MPID_Protocol *, sizeof(MPID_Protocol));

    /* set up the memory buffers */
    MPID_INFO("  SHORT: ");
    SMIcall( SMI_Barrier() );    

    /* maximum payload of short messages */
    MPID_SMI_cfg.MAX_SHORT_PAYLOAD = 
		MPID_SMI_SCI_TA_SIZE - MPID_PKT_SHORT_SIZE - CSUMHEAD_SIZE 
		- sizeof(MPID_SMI_MSGFLAG_T) - MPID_SMI_cfg.SENDCTRL_PAD;
    if (MPID_SMI_SHORTSIZE/MPID_SMI_SCI_TA_SIZE > 1)
		MPID_SMI_cfg.MAX_SHORT_PAYLOAD += 
			MPID_SMI_SCI_TA_SIZE -  sizeof(MPID_SMI_MSGFLAG_T) - sizeof(CSUM_VALUE_TYPE);
    if (MPID_SMI_SHORTSIZE/MPID_SMI_SCI_TA_SIZE > 2)
		MPID_SMI_cfg.MAX_SHORT_PAYLOAD += 
			(MPID_SMI_SHORTSIZE/MPID_SMI_SCI_TA_SIZE - 2)*MPID_SMI_SCI_TA_SIZE;
#ifdef MPI_LINUX_ALPHA
    /* hack! */
    if ((sizeof(CSUM_VALUE_TYPE) == 4) && (MPID_SMI_SHORTSIZE/MPID_SMI_SCI_TA_SIZE > 1)) {
		/* we make room for a long csum value instead of int af the end (-> performance) */
		MPID_SMI_cfg.MAX_SHORT_PAYLOAD -= 8;
    }
#endif
	    
    if (MPID_SMI_Short_MemSetup() != MPI_SUCCESS) {
		MPID_ERROR(0, MPI_ERR_EXHAUSTED, "MPID_SMI_Short_MemSetup() failed");
		return 0;
    }
	
    /* all others are set to zero */
    p->send    = MPID_SMI_Short_send;
    p->recv    = MPID_SMI_Short_recv;
    p->isend   = MPID_SMI_Short_isend;
    p->unex    = MPID_SMI_Short_save;
    p->delete  = MPID_SMI_Short_delete;

    return p;
}

int MPID_SMI_Short_MemSetup ( void )
{
    smi_region_info_t shreg_info;
    int mem_per_proc, proc, i, dummy, segmode;

    /* allocate local memory */
    ZALLOCATE (MPID_SMI_lShortsend, MPID_SMI_lShortsend_t *, MPID_SMI_numids*sizeof(MPID_SMI_lShortsend_t));
    ZALLOCATE (MPID_SMI_lShortrecv, MPID_SMI_lShortrecv_t *, MPID_SMI_numids*sizeof(MPID_SMI_lShortrecv_t));
    ZALLOCATE (MPID_SMI_shmem_short, char ** , MPID_SMI_numids*sizeof(char *));

    /* Allocate shared memory (SCI or local, depending on the number of nodes). 
       We allocate additionaly memory next to the incoming queue for short messages 
       to store additional information there (internal single-sided communication, for eager & rndv). */
    mem_per_proc = MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS * MPID_SMI_numids + MPID_SMI_Int_bufsize;
    MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);
        
    /* we should not be the owner (only relevant for IMPEXE adapter scheduling) */
	SMI_Init_reginfo (&shreg_info, MPID_SMI_numids*mem_per_proc, 0, 
					  (MPID_SMI_myid + 1) % MPID_SMI_numids, SMI_ADPT_DEFAULT, 0, 0, NULL);

    MPID_INFO(" global -"); 
    if (SMI_Create_shreg(SMI_SHM_FRAGMENTED, &shreg_info, &SMI_Shregid_short, (void **)MPID_SMI_shmem_short) 
		!= SMI_SUCCESS) {
		MPID_ABORT ("Not enough remote shared memory for control packet queues.");
    }

    /* initialize the receive buffers */
    for (i = 0; i < mem_per_proc; i++)
		MPID_SMI_shmem_short[MPID_SMI_myid][i] = 0;

    /* locate the buffers for internal single-sided communication */
    for (i = 0; i < MPID_SMI_numids; i++) {
		/* IMPort information from other processes - this memory is local. */
		MPID_SMI_Int_info_imp[i] = (MPID_SMI_Int_data *)
			(MPID_SMI_shmem_short[MPID_SMI_myid] + MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS*MPID_SMI_numids
			 + i*sizeof(MPID_SMI_Int_data));
		/* EXPort information to other proceses - this memory is located on the node of 
		   the respective process. */
		MPID_SMI_Int_info_exp[i] = (MPID_SMI_Int_data *)
			(MPID_SMI_shmem_short[i] + MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS*MPID_SMI_numids
			 + MPID_SMI_myid*sizeof(MPID_SMI_Int_data));
    }

    if (MPID_SMI_use_SMP) {
		int region_size;
		/* allocate local shared memory for communication between processes
		   running on the same node */
		SMIcall( SMI_First_proc_on_node (MPID_SMI_myNode, &shreg_info.owner));
		region_size = (MPID_SMI_numProcsOnNode[MPID_SMI_myNode] > 1) ? 
			MPID_SMI_numProcsOnNode[MPID_SMI_myNode]*MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc :
			2 * MPID_SMI_PAGESIZE;
		SMI_Init_reginfo (&shreg_info, region_size, 0, MPID_SMI_myid, SMI_ADPT_DEFAULT, 0, 0, NULL);

		MPID_INFO(" local -"); 
		if (SMI_Create_shreg( SMI_SHM_SMP|SMI_SHM_NONFIXED|SMI_SHM_INTERN, &shreg_info, &SMI_Locregid_short, 
							  (void **)&MPID_SMI_local_short) != SMI_SUCCESS)
			MPID_ABORT ("Not enough local shared memory control packet queues.");
    }

    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		MPID_SMI_Short_MemInit (proc);
    }
    
    return MPI_SUCCESS;
}


void MPID_SMI_Short_GetConfig (int *number, int *size) {
    *number = MPID_SMI_SHORTBUFS;
    *size   = MPID_SMI_SHORTSIZE;

    return;
}


/* init structures - all non-mentioned members shall be zero'ed by calloc() */
void MPID_SMI_Short_MemInit ( int proc )
{
    char *adr;
    
    /*** sending msgs to 'proc' ***/
    if (!MPID_SMI_use_localseg[proc]) {
		/* communication with remote process or pure SMP mode */
		adr = MPID_SMI_shmem_short[proc] + MPID_SMI_myid * MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS;
    } else {
		/* communication with process running on the same node, but more than 1 node active */
		adr = MPID_SMI_local_short + MPID_SMI_localRankForProc[proc] * (MPID_SMI_numProcsOnNode[MPID_SMI_myNode]) 
			* MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS 
			+ MPID_SMI_localRankForProc[MPID_SMI_myid] * MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS;
    }
    
    /* local send information for sending msgs to 'proc' */
    MPID_SMI_lShortsend[proc].recv_buffer = adr;
    MPID_SMI_lShortsend[proc].recv_ptr    = adr;
    MPID_SMI_lShortsend[proc].avail_msgs  = MPID_SMI_SHORTBUFS;
    MPID_SMI_lShortsend[proc].read_msgs   = &(MPID_SMI_Int_info_exp[proc]->short_msgcnt);
    MPID_SMI_lShortsend[proc].msg_id      = 1;
    
    /*** receiving msgs from 'proc' ***/
    if (!MPID_SMI_use_localseg[proc]) {
		/* initialize pointers for communication with remote process */
		adr = MPID_SMI_shmem_short[MPID_SMI_myid] + proc * MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS;
    } else {
		/* initialize pointers for communication with process running on the same node */
		adr = MPID_SMI_local_short + MPID_SMI_localRankForProc[MPID_SMI_myid] * (MPID_SMI_numProcsOnNode[MPID_SMI_myNode])
			* MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS +
			MPID_SMI_localRankForProc[proc] * MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS;
    }
    
    /* local recv information for receiving msgs from 'proc' */
    MPID_SMI_lShortrecv[proc].recv_buffer = adr;
    MPID_SMI_lShortrecv[proc].recv_ptr    = adr;
    MPID_SMI_lShortrecv[proc].msg_flag    = 
		(MPID_SMI_MSGFLAG_T *) (adr + MPID_SMI_SCI_TA_SIZE - sizeof(MPID_SMI_MSGFLAG_T));
    MPID_SMI_lShortrecv[proc].newmsg_id   = 1;
    MPID_SMI_lShortrecv[proc].read_msgs   = &(MPID_SMI_Int_info_imp[proc]->short_msgcnt);


    return;
}

void MPID_SMI_Short_delete( p )
	MPID_Protocol *p;
{
    int proc;

    /* free SMI memory */
    SMIcall (SMI_Free_shreg( SMI_Shregid_short ));
      
    /* free local shared memory */
    if (MPID_SMI_use_SMP) {
		SMIcall (SMI_Free_shreg(SMI_Locregid_short));
    }
    
    /* free local memory */
    FREE (MPID_SMI_lShortsend);
    FREE (MPID_SMI_lShortrecv);
    FREE (MPID_SMI_shmem_short);

    FREE( p );
}

int MPID_SMI_Short_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dtype )
	void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_SHORT_T prepkt;

    MPID_STAT_ENTRY(short_send);
    MPID_TRACE_CODE ("send_short", dest_dev_lrank);

    MPID_SMI_DEBUG_PRINT_MSG("S Getting a short send packet");
    MPID_GETSENDPKT(pkt_desc, MPID_PKT_SHORT_SIZE, &prepkt, len, buf, dest_dev_lrank, 0);

    MPID_INIT_PREPKT (prepkt, MPID_PKT_SHORT, context_id, src_comm_lrank, tag, len, 0); 

    MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending short message", &prepkt, dest_dev_lrank);
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

    MPID_SMI_DEBUG_PRINT_MSG("S Sent message in a single packet");
    MPID_STAT_EXIT(short_send);
    return MPI_SUCCESS;
}


int MPID_SMI_Short_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
						  msgrep, shandle, dtype )
	void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_SHORT_T prepkt;

    MPID_STAT_ENTRY(short_send);
    MPID_TRACE_CODE ("isend_short", dest_dev_lrank);

    MPID_SMI_DEBUG_PRINT_MSG("S Getting a packet for short send");
    MPID_GETSENDPKT(pkt_desc, MPID_PKT_SHORT_SIZE, &prepkt, len, buf, dest_dev_lrank, 0)

		MPID_INIT_PREPKT (prepkt, MPID_PKT_SHORT, context_id, src_comm_lrank, tag, len, shandle); 

    MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending short message",&prepkt, dest_dev_lrank);
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

    MPID_SMI_DEBUG_PRINT_MSG("S Sent message in a single packet");

    shandle->is_complete  = 1;
    shandle->cancel       = 0;
    shandle->partner      = dest_dev_lrank;

    MPID_STAT_EXIT(short_send);
    return MPI_SUCCESS;
}




/* for short msgs which have extend beyond one singe SCI transaction, 
   the payload is not stored continously in the packet due to the 
   synchronizier flags at the end of each transaction. Thus, the data contained in
   such a packet needs to be "packed" when transferred into the user buffer. */
static int MPID_SMI_short_pack (char *buf, MPID_PKT_SHORT_T *pkt, int data_len, int from)
{
    int cpy_len, csum_retry, csum_ok;
    int rest_len = data_len;
    int dst_offset = 0;
    int src_offset = 0;
    char *data_start = ((char *)pkt) + MPID_PKT_SHORT_SIZE;
    int queue_size = MPID_SMI_SHORTSIZE * MPID_SMI_SHORTBUFS;
    int total_len, remaining_space, end_len;

    
    MPID_STAT_ENTRY (short_pack);
    MPID_SMI_DEBUG_PRINT_MSG("R packing msg from recv to user buffer ");

    /* copy the data out of the first part - first wait until the CRC is o.k. which means
       that all data has arrived */

    cpy_len = MPID_SMI_SCI_TA_SIZE - MPID_SMI_cfg.SENDCTRL_PAD - sizeof(MPID_SMI_MSGFLAG_T) 
		- MPID_PKT_SHORT_SIZE - CSUMHEAD_SIZE;
    cpy_len = data_len < cpy_len ? data_len : cpy_len;

    csum_retry = 0;
    MPID_SMI_CSUM_OK (pkt->csum, data_start, cpy_len, 0xff, from, csum_ok);
    while (!csum_ok) {
		MPID_STAT_COUNT(csum_retry_data);
		if (++csum_retry > MAX_CSUM_RETRIES) {
			MPID_ABORT ("unresolvable CSUM mismatch in short_pack() [1]");
		}
		MPID_DEBUG_CODE (if (csum_retry % (MAX_CSUM_RETRIES/100) == 0) \
						 fprintf (stderr, "[%d] short_pack: high CSUM error rate!\n", MPID_SMI_myid); );

		MPID_SMI_CSUM_OK (pkt->csum, data_start, cpy_len, 0xff, from, csum_ok);
    }

    MEMCPY (buf, data_start, cpy_len);
    src_offset += cpy_len + MPID_SMI_cfg.SENDCTRL_PAD + sizeof(MPID_SMI_MSGFLAG_T);
    dst_offset += cpy_len;
    rest_len   -= cpy_len;
    
    /* This is a "long short-msg" - get the data from the second part. */
    if (rest_len > 0) {
		int csum_offset = rest_len, nbr_ta, dst_offset_1, src_offset_1, cpy_len_1=0;

		remaining_space = (size_t)MPID_SMI_lShortrecv[from].recv_buffer + queue_size - (size_t)&(pkt->buffer) - src_offset;
		dst_offset_1 = dst_offset;
		src_offset_1 = src_offset;

		if (rest_len <= MPID_SMI_SCI_TA_SIZE-sizeof(MPID_SMI_MSGFLAG_T)-sizeof(CSUM_VALUE_TYPE))
			nbr_ta = 1;
		else
			nbr_ta = 2 + (rest_len - MPID_SMI_SCI_TA_SIZE +sizeof(MPID_SMI_MSGFLAG_T)+sizeof(CSUM_VALUE_TYPE) - 1)
				/ MPID_SMI_SCI_TA_SIZE;	

		/* Check if the remaining data doesn't fit into the queue anymore */
		if (nbr_ta * MPID_SMI_SCI_TA_SIZE > remaining_space) {
			total_len = rest_len;

			/* copy data from the end of the msg queue to user buffer if there's enough space*/
			if (remaining_space >= MPID_SMI_SCI_TA_SIZE){
				if (rest_len < remaining_space)
					cpy_len_1 = rest_len;
				else
					cpy_len_1 = remaining_space;

				MEMCPY (buf + dst_offset_1, data_start + src_offset, cpy_len_1);
				dst_offset_1 += cpy_len_1;
				rest_len -= cpy_len_1;
			}
			csum_offset = rest_len;

			/* Turn to the beginning of the queue */
			src_offset = 0;

			/* copy the remaining data at the beginning of the queue to user buffer */
			if (rest_len)
				MEMCPY (buf + dst_offset_1, MPID_SMI_lShortrecv[from].recv_buffer, rest_len);

			if (rest_len & (SENDCTRL_ALIGN - 1))
				csum_offset = SENDCTRL_ALIGN*(rest_len/SENDCTRL_ALIGN + 1);

			/* check the integrity of the second part */
			MPID_SMI_DEBUG_PRINT_MSG("R    copying remaining parts");
			csum_retry = 0;

			MPID_SMI_CSUM_OK (*(CSUM_VALUE_TYPE *)((char *)MPID_SMI_lShortrecv[from].recv_buffer + csum_offset), 
							  buf + dst_offset, total_len, 0xff, from, csum_ok);
			/*csum_ok = 1;*/
			while (!csum_ok) {
				/* Data hasn't yet completely arrived, copy again to user buffer */
				if (cpy_len_1 > 0)
					MEMCPY (buf + dst_offset, data_start + src_offset_1, cpy_len_1);
				if (rest_len > 0)
					MEMCPY (buf + dst_offset_1, MPID_SMI_lShortrecv[from].recv_buffer, rest_len);

				MPID_STAT_COUNT(csum_retry_data);
				if (++csum_retry > MAX_CSUM_RETRIES) {
					MPID_ABORT ("Unresolvable CSUM mismatch short_pack() [2]" );
				}
				MPID_DEBUG_CODE (if (csum_retry % (MAX_CSUM_RETRIES/10) == 0) \
								 fprintf (stderr, "[%d] short_pack: high CSUM error rate!\n", MPID_SMI_myid); );
				MPID_SMI_CSUM_OK (*(CSUM_VALUE_TYPE *)(MPID_SMI_lShortrecv[from].recv_buffer + csum_offset), 
								  buf + dst_offset, total_len, 0xff, from, csum_ok);
			}
		} else {
			if (rest_len & (SENDCTRL_ALIGN - 1))
				csum_offset = SENDCTRL_ALIGN*(rest_len/SENDCTRL_ALIGN + 1);
	    
			/* check the integrity of the second part */
			MPID_SMI_DEBUG_PRINT_MSG("R    copying remaining parts");
			csum_retry = 0;
			MPID_SMI_CSUM_OK (*(CSUM_VALUE_TYPE *)(data_start + src_offset + csum_offset),
							  data_start + src_offset, rest_len, 0xff, from, csum_ok);
			while (!csum_ok) {
				MPID_STAT_COUNT(csum_retry_data);
				if (++csum_retry > MAX_CSUM_RETRIES) {
					MPID_ABORT ("Unresolvable CSUM mismatch in short_pack() [2]");
				}
				MPID_DEBUG_CODE (if (csum_retry % (MAX_CSUM_RETRIES/10) == 0) \
								 fprintf (stderr, "[%d] short_pack: high CSUM error rate!\n", MPID_SMI_myid); );
				MPID_SMI_CSUM_OK (*(CSUM_VALUE_TYPE *)(data_start + src_offset + csum_offset), 
								  data_start + src_offset, rest_len, 0xff, from, csum_ok);
			}
			MEMCPY (buf + dst_offset, &(pkt->buffer) + src_offset, rest_len);
		}
    }
    
    MPID_STAT_EXIT (short_pack);
    return 0;
}


int MPID_SMI_Short_recv( rhandle, from_grank, in_pkt )
	MPIR_RHANDLE *rhandle;
int          from_grank;
void         *in_pkt;
{
    MPID_PKT_SHORT_T *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    MPID_STAT_ENTRY (short_recv);
    MPID_TRACE_CODE ("recv_short", from_grank);

    msglen = pkt->len;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;

    /* handling truncated msgs is a bit clumsy here because we need to 
       checksum-verify the complete message, but only write the truncated
       message into the user buffer. The way we do it here is not efficient, 
       but who cares for truncated messages anyway ? */


    if (rhandle->len < msglen) {
		err = MPI_ERR_TRUNCATE;
       
		if (msglen > 0) {
			void *trunc_buf = NULL;

		/*ALLOCATE (trunc_buf, void *, rhandle->len);*/ /*#define ALLOCATE(ptr,type,size)*/
		ALLOCATE (trunc_buf, void *, msglen);
		/* //SI// use msglen instead of rhandle->len, because MPID_SMI_short_pack
		copies msglen in the buffer which is otherwise corrupted */ 
			MPID_SMI_short_pack(trunc_buf, pkt, msglen, from_grank);
#ifdef WIN32
			_ASSERTE(_CrtIsValidHeapPointer(trunc_buf)); /* //SI// check if heap pointer is still valid */
#endif			
			MEMCPY (rhandle->buf, trunc_buf, rhandle->len);
			FREE (trunc_buf);
			msglen = rhandle->len;
		}
    } else 
		/* non-truncated message */
		if (msglen > 0) {
			MPID_SMI_short_pack(rhandle->buf, pkt, msglen, from_grank);
		}

    rhandle->s.count     = msglen;
    rhandle->s.MPI_ERROR = err;
    if (rhandle->finish) {
		(rhandle->finish)( rhandle );
    }

    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from_grank, pkt->len);
    rhandle->is_complete = 1;
    rhandle->cancel = 0;

    MPID_STAT_EXIT (short_recv);
    return err;
}


/* This routine is called when it is time to receive an unexpected message */
int MPID_SMI_Short_unxrecv_start( rhandle, in_runex )
	MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;

    MPID_TRACE_CODE ("unexrecv_start_short", runex->from);

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle, msglen, err);

    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
		MEMCPY(rhandle->buf, runex->start, msglen);
		FREE (runex->start);
    }

    rhandle->s		 = runex->s;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
		(rhandle->finish)( rhandle );
    MPID_Recv_free( runex );

    return err;
}

/* Save an unexpected message in rhandle */
int MPID_SMI_Short_save( rhandle, from, in_pkt )
	MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SHORT_T   *pkt = (MPID_PKT_SHORT_T *)in_pkt;

    MPID_STAT_ENTRY (short_recv);
    MPID_TRACE_CODE ("save_short", from);

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 1;

    /* Need to save msgrep for heterogeneous systems */
    if (pkt->len > 0) {
		/* We should use a fixed-size-allocator here, with the size
		   set to the max. short msg length! */
		ALLOCATE (rhandle->start, void *, pkt->len);
		if (!rhandle->start) {
			rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
			MPID_STAT_EXIT (short_recv);
			return 1;
		}
		MPID_SMI_short_pack (rhandle->start, pkt, pkt->len, from);
    } else {
		rhandle->start = NULL;
    }
	
    rhandle->push     = MPID_SMI_Short_unxrecv_start;
    rhandle->cancel   = MPID_SMI_Short_cancel_recv;
    rhandle->send_id  = pkt->send_id;  /* this should be done in queue.c */
    rhandle->is_valid = 1;

    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from, pkt->len); 
    MPID_STAT_EXIT (short_recv);
	return 0;
}

/* when a short-recv is canceled, free the buffer in which the msg was saved */
int MPID_SMI_Short_cancel_recv( runex )
	MPIR_RHANDLE    *runex;
{
    if (runex->start != NULL)
		FREE( runex->start );
    MPID_Recv_free( runex );

    return MPI_SUCCESS;
}






/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
