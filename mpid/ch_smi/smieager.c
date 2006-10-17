/* $Id$ */

/* Changes:

- 16.08.2001: Added call to MPIR_dPack2 in MPID_Eagern_send
*/

#include "smidev.h"
#include "smieager.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*
  Blocking and non-blocking, eager SCI-shared-memory send/recv.   
*/


/* Prototype definitions */
int MPID_SMI_Eagern_MemSetup ( void );
void MPID_SMI_Eagern_preconnect( void );
void MPID_SMI_Eagern_GetConfig (int *, int *);

int MPID_SMI_Eagern_send ( void *, int, int, int, int, int, 
						   MPID_Msgrep_t, struct MPIR_DATATYPE *);
int MPID_SMI_Eagern_isend ( void *, int, int, int, int, int, 
							MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );
int MPID_SMI_Eagern_isend_block ( void *, int, int, int, int, int, 
								  MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );
void *MPID_SMI_Eagern_isend_thread_pio ( void * );
void *MPID_SMI_Eagern_isend_thread_dma ( void * );
int MPID_SMI_Eagern_wait_send ( MPIR_SHANDLE * );
int MPID_SMI_Eagern_finish_send ( MPIR_SHANDLE * );
int MPID_SMI_Eagern_test_send ( MPIR_SHANDLE * );
int MPID_SMI_Eagern_cancel_send ( MPIR_SHANDLE * );

int MPID_SMI_Eagern_save ( MPIR_RHANDLE *, int, void * );
int MPID_SMI_Eagern_unxrecv_start ( MPIR_RHANDLE *, void * );
void MPID_SMI_Eagern_delete ( MPID_Protocol * );
int MPID_SMI_Eagern_recv ( MPIR_RHANDLE *, int, void * );
int MPID_SMI_Eagern_irecv ( MPIR_RHANDLE *, int, void * );
int MPID_SMI_Eagern_cancel_recv ( MPIR_RHANDLE * );

#ifdef MPID_USE_DEVTHREADS
/* data structure for all variables that MPID_SMI_Eagern_isend_thread needs */

typedef struct _MPID_SMI_Eagern_thrddata_t {
    void          *from_buf;
    void          *to_buf;
    void          *local_shmem;
    int           len, tag, context_id, src_lrank, dest;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_Msgrep_t msgrep;
    struct MPIR_DATATYPE *dtype;
    MPIR_SHANDLE  *shandle;
    pthread_mutex_t *complete_lock;
} MPID_SMI_Eagern_thrddata_t;
#endif

/*
 * exports 
 */
int MPID_SMI_EAGERSIZE;
int MPID_SMI_min_EAGERSIZE;  /* smallest eager-buf size of all processses */
int MPID_SMI_EAGERBUFS; 
int MPID_SMI_EAGER_MAXCHECKDEV; 
int MPID_SMI_EAGER_DYNAMIC;
int MPID_SMI_EAGER_IMMEDIATE;
int MPID_SMI_Eager_DMA_working = 0;  /* number of DMA threads which are still working */

/*
 * local variables for the eager protocol that are common for the static and dynamic version 
 */

/* MPID_SMI_Eagern_connect_info[proc] contains information that is needed to send eager messages
   to proc, see type declaration for details, this information is saved here at connect time. */
MPID_SMI_Eagern_connect_info_t *MPID_SMI_Eagern_connect_info = NULL;

/* MPID_SMI_eagerseg_connected[proc] is true if we are connected to proc's eager memory, false otherwise */
int *MPID_SMI_eagerseg_connected;

/* alignment buffer for remote copy operations */
int *MPID_SMI_eager_align_buf;

/* SMI region ID of eager buffers for shared memory communication */
int MPID_SMI_Locregid_eagerbufs = 0;

/* MPID_SMI_Shregid_eagerbufs[proc] is the SMI region ID for global (=SCI) eager shared memory located on proc */
int *MPID_SMI_Shregid_eagerbufs;

/* Pointer to global (=SCI) eager shared memory located on this proc;
   at this address the eager buffers into which this proc receives begin */
char *MPID_SMI_global_eagerbufs;

/* Pointer to local eager shared memory; this is the address at which
   the shared memory begins, not the beginning of the eager buffers into
   which this process receives (different from MPID_SMI_global_eagerbufs!) */
char *MPID_SMI_local_eagerbufs;

/* MPID_SMI_incoming_eagerbufs[proc] points to the beginning of the
   eager buffers into which this proc receives from proc */
char **MPID_SMI_incoming_eagerbufs; 

/* MPID_SMI_incoming_eagerbufs[proc] points to the beginning of the
   eager buffers into which this proc sends to proc */
char **MPID_SMI_outgoing_eagerbufs;

/* pointers to functions which are different between static and dynamic eager buffer
   memory management */
MPID_SMI_Eagern_int_t MPID_SMI_Eagern_int;

/* these variables are only used in smieager.c */
#ifdef MPID_USE_DEVTHREADS
/* concurrent memory operations have a bad performance -> synchronize them */
static pthread_mutex_t eager_async_memcpy_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

/* stack for locks synchronizing DMA transfers */
MPID_stack_t DMAlock_stack;
/* stack for thread function parameters */
MPID_stack_t thrddata_stack;



/* Initialize the eager protocol. */
MPID_Protocol *MPID_SMI_Eagern_setup()
{
    MPID_Protocol *p;

    if (MPID_SMI_EAGER_DYNAMIC) {
		MPID_INFO(" ok.\n dEAGER:");

		MPID_SMI_Eagern_int.init          = MPID_SMI_dEager_init;
		MPID_SMI_Eagern_int.init_complete = MPID_SMI_dEager_init_complete;
		MPID_SMI_Eagern_int.connect       = MPID_SMI_dEager_connect;
		MPID_SMI_Eagern_int.disconnect    = MPID_SMI_dEager_disconnect;
		MPID_SMI_Eagern_int.free_buf      = MPID_SMI_dEager_free_buf;
		MPID_SMI_Eagern_int.get_buf       = MPID_SMI_dEager_get_buf;
		MPID_SMI_Eagern_int.sendcpy       = MPID_SMI_dEager_sendcpy;
		MPID_SMI_Eagern_int.recvcpy       = MPID_SMI_dEager_recvcpy;
		MPID_SMI_Eagern_int.delete        = MPID_SMI_dEager_delete;
    }
    else {
		MPID_INFO(" ok.\n sEAGER:");

		MPID_SMI_Eagern_int.init          = MPID_SMI_sEager_init;
		MPID_SMI_Eagern_int.init_complete = MPID_SMI_sEager_init_complete;
		MPID_SMI_Eagern_int.connect       = MPID_SMI_sEager_connect;
		MPID_SMI_Eagern_int.disconnect    = MPID_SMI_sEager_disconnect;
		MPID_SMI_Eagern_int.free_buf      = MPID_SMI_sEager_free_buf;
		MPID_SMI_Eagern_int.get_buf       = MPID_SMI_sEager_get_buf;
		MPID_SMI_Eagern_int.sendcpy       = MPID_SMI_sEager_sendcpy;
		MPID_SMI_Eagern_int.recvcpy       = MPID_SMI_sEager_recvcpy;
		MPID_SMI_Eagern_int.delete        = MPID_SMI_sEager_delete;
    }

    /* set up the memory buffers */
    if (MPID_SMI_Eagern_MemSetup() != MPI_SUCCESS) {
        /* no error since eager might be deactivated on purpose 
		   - we'll use  rndv instead */
		MPID_INFO(" not in use.\n");
		return 0;
    }
    MPID_INFO(" ok.\n");

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) 
		return 0;

    p->send	   = MPID_SMI_Eagern_send;
    p->recv	   = MPID_SMI_Eagern_recv;
#if 0
    /* XXX asynchronous send currently disabled for eager until it works 100%
       We will probably never fix this as it is just not efficient. */
    if (MPID_SMI_cfg.ASYNC_PROGRESS)
		p->isend   = MPID_SMI_Eagern_isend;        
    else
		p->isend   = MPID_SMI_Eagern_isend_block;
#else
    p->isend   = MPID_SMI_Eagern_isend_block;
#endif
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = MPID_SMI_Eagern_cancel_send;

    p->irecv	   = MPID_SMI_Eagern_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = MPID_SMI_Eagern_cancel_recv;
    p->do_ack      = 0;
    p->unex        = MPID_SMI_Eagern_save;
    p->delete      = MPID_SMI_Eagern_delete;

    return p;
}

/* this function does the memory setup which is common to the static and 
   dynamic version of the eager protocol */
int MPID_SMI_Eagern_MemSetup ( void )
{
    
    DMAlock_stack = MPID_stack_init( 0 );
    thrddata_stack = MPID_stack_init( MPID_UTIL_THREADSAFE );

    /* allocate local memory */
    ZALLOCATE (MPID_SMI_incoming_eagerbufs, char **, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_outgoing_eagerbufs, char **, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_Shregid_eagerbufs, int *, MPID_SMI_numids*sizeof(int));
    ZALLOCATE (MPID_SMI_eagerseg_connected, int *, MPID_SMI_numids*sizeof(int));

    ALLOCATE (MPID_SMI_eager_align_buf, int *, MPID_SMI_EAGER_ALIGN_SIZE);

    /* call specific memory setup for static and dynamic version */
    return MPID_SMI_Eagern_int.init();
}


void MPID_SMI_Eagern_GetConfig (int *number, int *size) {
    *number = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].nbr_bufs;
    *size   = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;

    return;
}


/* Complete the initialization of the eager protocol using the information from
   the remote processes. This has to be done separately from the local initialization
   because we need to be sure that the remote processes have posted the description of
   their buffer setup. */
void MPID_SMI_Eagern_init_complete(void)
{
	MPID_SMI_Eagern_int.init_complete();

	return;
}

   
/* pre-connect to all remote segments (required for CONNECT_ON_INIT = 1) */
void MPID_SMI_Eagern_preconnect( void )
{
    int proc;

    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		if (!MPID_SMI_eagerseg_connected[proc])
			MPID_SMI_Eagern_int.connect(proc);
    }
	
    return;
}


void MPID_SMI_Eagern_delete( p )
	 MPID_Protocol *p;
{
    /* call specific cleanup function for static and dynamic version */
    MPID_SMI_Eagern_int.delete();

    if (MPID_SMI_use_SMP && MPID_SMI_EAGERBUFS > 0) {
		/* free local shared memory */
		SMIcall (SMI_Free_shreg( MPID_SMI_Locregid_eagerbufs) );	
    }
    /* Remote shared memory will be free'd by the resource manager. */

    MPID_stack_destroy (DMAlock_stack);
    MPID_stack_destroy (thrddata_stack);

    FREE (MPID_SMI_incoming_eagerbufs);
    FREE (MPID_SMI_outgoing_eagerbufs);
    FREE (MPID_SMI_Shregid_eagerbufs);
    FREE (MPID_SMI_eagerseg_connected);
    FREE (MPID_SMI_eager_align_buf);

    if (p != NULL) {
		FREE( p );
    }
}


/* Usage of shandle->sid[]:
 * sid[EAGERN_ISEND_ID_N_LOCK] PIO modus: contains threadID
 *                             DMA modus: contains pointer to condition variable
 * sid[EAGERN_ISEND_TRANSFER]  EAGERN_ISEND_TRANSFER_UNCOMPLETE indicates that data transfer is not complete yet
 EAGERN_ISEND_TRANSFER_COMPLETE   indicates that data transfer is complete
 * sid[EAGERN_ISEND_MODE] contains mode number: EAGERN_ISEND_MODE_BLOCKING for blocking isend
 *                                              EAGERN_ISEND_MODE_PIO      for non-blocking with PIO
 *                                              EAGERN_ISEND_MODE_DMA      for non-blocking with DMA
 */             

#ifdef MPID_USE_DEVTHREADS
/* real asynchronous version of  MPID_SMI_Eagern_isend (using threads & DMA) */
int MPID_SMI_Eagern_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dtype )
	 void          *buf;
	 int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
	 MPID_Msgrep_t msgrep;
	 MPIR_SHANDLE  *shandle;
	 struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_Eagern_thrddata_t *threaddata;
    pthread_t        isendthread;
    pthread_attr_t   attr;
    void *eager_buf;
 
    MPID_STAT_ENTRY(eager_isend);
    MPID_TRACE_CODE("Eagern_isend", dest_dev_lrank);

    if (MPID_SMI_cfg.SENDSELF && (dest_dev_lrank == MPID_SMI_myid)) {
		return MPID_SMI_Isend_self (buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle);
    }
    
    /* get address at destination process */
    if (!(eager_buf = MPID_SMI_Eagern_int.get_buf(dest_dev_lrank, len))) {
		/* switch to rendezvous because no free eager buffer is available */
		MPID_STAT_COUNT(no_eagerbuf);
		return MPID_SMI_Rndv_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dtype );
    }

    shandle->is_complete     = 0;                           /* mark as not yet completed */
    shandle->bytes_as_contig = len;
    shandle->wait            = MPID_SMI_Eagern_wait_send;
    shandle->finish          = MPID_SMI_Eagern_finish_send; /* to free thread resources */
    shandle->test            = MPID_SMI_Eagern_test_send;
    shandle->cancel          = MPID_SMI_Eagern_cancel_send;
    shandle->partner         = dest_dev_lrank;

    /* initialize attributes for new thread to create
     * joinable state for PIO is set by default
     */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* this piece of memory is freed again at the end of MPID_SMI_Eagern_isend_thread */
    if ((threaddata = (MPID_SMI_Eagern_thrddata_t *) MPID_stack_pop(thrddata_stack)) == NULL)
		ALLOCATE (threaddata, MPID_SMI_Eagern_thrddata_t *, sizeof(MPID_SMI_Eagern_thrddata_t));
    threaddata->from_buf   = buf;
    threaddata->to_buf     = eager_buf;
    threaddata->len        = len;
    threaddata->tag        = tag;
    threaddata->context_id = context_id;
    threaddata->src_lrank  = src_comm_lrank;
    threaddata->dtype      = 0;
    threaddata->msgrep     = msgrep;
    threaddata->shandle    = shandle;
    threaddata->dest       = dest_dev_lrank;
    
    /* get address of free eager buffer in local shared segment (only needed for DMA-transfer)
     * this has to be done before creating new thread to avoid race conditions  */     
    if (MPID_SMI_cfg.USE_DMA_PT2PT && len >= MPID_SMI_cfg.ASYNC_DMA_MINSIZE)
		threaddata->local_shmem = MPID_SMI_Eagern_int.get_buf(MPID_SMI_myid, len);

    /* GetSendPacket hangs until succesful */
    (threaddata->pkt_desc).dest = dest_dev_lrank;
    MPID_SMI_GetSendPkt(0, &(threaddata->pkt_desc));

    /* to indicate that data transfer is not completed yet */
    shandle->sid[EAGERN_ISEND_TRANSFER] = EAGERN_ISEND_TRANSFER_UNCOMPLETE;

    /* create new thread which performs the data transfers */
    if (MPID_SMI_cfg.USE_DMA_PT2PT && len >= MPID_SMI_cfg.ASYNC_DMA_MINSIZE && threaddata->local_shmem) {
		/* set state of new thread to detached, so that we do not have to join it for this
		 * has no use in using DMA
		 */
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      
		/* MPID_SMI_Eagern_wait_send() waits on this lock */
		/* use lock from stack if possible, otherwise create one */
		if((threaddata->complete_lock = (pthread_mutex_t *) MPID_stack_pop( DMAlock_stack )) == NULL) {
			ALLOCATE (threaddata->complete_lock, pthread_mutex_t *, sizeof (pthread_mutex_t));
		}
		pthread_mutex_init (threaddata->complete_lock, NULL);
		pthread_mutex_lock (threaddata->complete_lock);
		shandle->sid[EAGERN_ISEND_ID_N_LOCK] = (long)threaddata->complete_lock;

		shandle->sid[EAGERN_ISEND_MODE] = EAGERN_ISEND_MODE_DMA;
		MPID_SMI_Eager_DMA_working++;

		pthread_create(&isendthread, &attr, MPID_SMI_Eagern_isend_thread_dma, threaddata);
    } else {
		shandle->sid[EAGERN_ISEND_MODE] = EAGERN_ISEND_MODE_PIO;

		pthread_create(&isendthread, &attr, MPID_SMI_Eagern_isend_thread_pio, threaddata);
	
		/* thread ID is needed by MPID_SMI_Eagern_wait_send() */
		shandle->sid[EAGERN_ISEND_ID_N_LOCK] = (long)isendthread;
    }
    
    MPID_STAT_EXIT(eager_isend);
    return MPI_SUCCESS;
}


/* isend_thread can transfer the data via an ordinary memcpy() (PIO) or via DMA. The first
   solution is fine if you have additional CPUs which can serve as "message processors".
   DMA is the way to go if each MPI process has only one CPU available. The define
   MPID_SMI_cfg.USE_DMA_PT2PT chooses DMA.

   However, because DMA via SCI only works between two SCI shared memory segments, you 
   also need to define MPID_SMI_SEND_SELF in this case so that the local eager buffers
   can be used as intermediate buffers for DMA */
void *MPID_SMI_Eagern_isend_thread_dma( values )
	 void *values;
{
    MPID_SMI_Eagern_thrddata_t *data = (MPID_SMI_Eagern_thrddata_t *) values;
    MPID_PKT_SEND_ADDRESS_T prepkt;
    smi_memcpy_handle  memcpy_h;
    char *eager_addr = (char *)data->to_buf;
	unsigned int eager_offset;

    MPID_TRACE_CODE ("Eagern_isend_thread_dma", data->dest);

    /* copy into local SCI shared memory - which is a local eager buffer */
    MPID_STAT_CALL(eager_scopy);
    MEMCPY_W(data->local_shmem, data->from_buf, data->len, MPID_SMI_myid);
    MPID_STAT_RETURN (eager_scopy);

    /* mark transfer as complete which means that the user buffer can be used again */
    (data->shandle)->sid[EAGERN_ISEND_TRANSFER] = EAGERN_ISEND_TRANSFER_COMPLETE;
    pthread_mutex_unlock (data->complete_lock); 

    /* writes into bitfields are not SCI-friendly -> prepare locally and
       copy in one piece */
	eager_offset = eager_addr - MPID_SMI_outgoing_eagerbufs[data->dest]; /* get offset from pointer */
    MPID_INIT_EAGER_PREPKT(prepkt, MPID_PKT_SEND_ADDRESS, data->context_id, data->src_lrank, data->tag,
						   data->len, data->shandle, eager_offset);

    /* prepare data for MPID_SMI_SendControl */
    (data->pkt_desc).hsize = sizeof(MPID_PKT_SEND_ADDRESS_T);
    (data->pkt_desc).header = (MPID_PKT_T *)&prepkt;
    (data->pkt_desc).dsize = 0;
    (data->pkt_desc).data = NULL;

    /* now issue the DMA transfer and wait for its completion */
    MPID_STAT_CALL(eager_dma);
    pthread_mutex_lock (&eager_async_memcpy_lock);
	eager_addr -= eager_offset;
#if 1
    SMI_Imemcpy( eager_addr, data->local_shmem, data->len, SMI_MEMCPY_LS_RS, &memcpy_h);
    SMI_Memwait( memcpy_h );
#else
    SMI_Memcpy( eager_addr, data->local_shmem, data->len, SMI_MEMCPY_LS_RS);
#endif
    pthread_mutex_unlock (&eager_async_memcpy_lock);
    MPID_STAT_RETURN(eager_dma);

    MPID_SMI_Eagern_int.free_buf(data->local_shmem, data->len, MPID_SMI_myid);

    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Sending eager message with isend_thread_dma", 
												&prepkt, data->dest);
    while (MPID_SMI_SendControl( &(data->pkt_desc) ) != MPI_SUCCESS)
		;

    /* XXX access needs to be synchronized for more than one working thread */
    MPID_SMI_Eager_DMA_working--;

    MPID_stack_push(thrddata_stack, values);

    return (NULL);
}

void *MPID_SMI_Eagern_isend_thread_pio( values )
	 void *values;
{
    MPID_SMI_Eagern_thrddata_t *data = (MPID_SMI_Eagern_thrddata_t *) values;
    MPID_PKT_SEND_ADDRESS_T prepkt;
    char *eager_addr = (char *)data->to_buf;
	unsigned int eager_offset;

    MPID_TRACE_CODE ("Eagern_isend_thread_pio", data->dest);

    MPID_STAT_CALL(eager_scopy);
    /* concurrent SCI writes do not perform well */
    pthread_mutex_lock (&eager_async_memcpy_lock);
    MEMCPY_W(eager_addr, data->from_buf, data->len, data->dest);
    pthread_mutex_unlock (&eager_async_memcpy_lock);
    MPID_STAT_RETURN(eager_scopy);
	
    /* prepare data for MPID_SMI_SendControl */
    (data->pkt_desc).hsize = sizeof(MPID_PKT_SEND_ADDRESS_T);
    (data->pkt_desc).header = (MPID_PKT_T *)&prepkt;
    (data->pkt_desc).dsize = 0;
    (data->pkt_desc).data = NULL;

    /* writes into bitfields are not SCI-friendly -> prepare locally and copy in one piece */
	eager_offset = eager_addr - MPID_SMI_outgoing_eagerbufs[data->dest]; /* get offset from pointer */
    MPID_INIT_EAGER_PREPKT(prepkt, MPID_PKT_SEND_ADDRESS, data->context_id, data->src_lrank, data->tag,
						   data->len, data->shandle, eager_offset);

    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Sending eager message with isend_thread_pio", 
												&prepkt, data->dest);
    while (MPID_SMI_SendControl( &(data->pkt_desc) ) != MPI_SUCCESS)
		;

    /* this marks that data transfer is completed and is used by MPID_SMI_test_send */
    (data->shandle)->sid[EAGERN_ISEND_TRANSFER] = EAGERN_ISEND_TRANSFER_COMPLETE;
    MPID_stack_push(thrddata_stack, values);

    return (NULL);
}
#endif /* MPID_USE_DEVTHREADS */

/* simple, blocking version of MPID_SMI_Eagern_isend (used, if MPID_SMI_NON BLOCKING is false) */
int MPID_SMI_Eagern_isend_block( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dtype )
	 void          *buf;
	 int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
	 MPID_Msgrep_t msgrep;
	 MPIR_SHANDLE *shandle;
	 struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_SEND_ADDRESS_T prepkt;
    char *eager_addr;
	unsigned int eager_offset;

    MPID_STAT_ENTRY(eager_isend);
    MPID_TRACE_CODE ("Eagern_isend_block", dest_dev_lrank);

    if (MPID_SMI_cfg.SENDSELF && (dest_dev_lrank == MPID_SMI_myid)) {
		return MPID_SMI_Isend_self (buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle);
    }

    /* get address at destination process */
    if(!(eager_addr = (char *)MPID_SMI_Eagern_int.get_buf(dest_dev_lrank, len))) {
		/* switch to rendezvous because no free eager buffer is available */
		MPID_STAT_COUNT(no_eagerbuf);
		return MPID_SMI_Rndv_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dtype );
    }

    MPID_SMI_Eagern_int.sendcpy(dest_dev_lrank, (void*)eager_addr, buf, len, dtype);
	eager_offset = eager_addr - MPID_SMI_outgoing_eagerbufs[dest_dev_lrank]; /* get offset from pointer */

    MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for eager send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_SEND_ADDRESS_T), &prepkt, 0, NULL, dest_dev_lrank, 0);
    MPID_INIT_EAGER_PREPKT(prepkt, MPID_PKT_SEND_ADDRESS, context_id, src_comm_lrank, tag, len, 
						   shandle, eager_offset);

    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Sending eager message with isend_block", &prepkt, dest_dev_lrank);
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

    /* Inidicate that this connection is no longer used by this transfer. */
    MPID_SMI_Eagern_int.disconnect(dest_dev_lrank);

    shandle->partner     = dest_dev_lrank;
    shandle->wait        = 0;
    shandle->test        = 0;
    shandle->cancel      = MPID_SMI_Eagern_cancel_send;
    shandle->is_complete = 1;

    MPID_STAT_EXIT(eager_isend);
    return MPI_SUCCESS;
}

int MPID_SMI_Eagern_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dtype )
	 void          *buf;
	 int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
	 MPID_Msgrep_t msgrep;
	 struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_SEND_ADDRESS_T prepkt;
    char *eager_addr;
	unsigned int eager_offset;

    MPID_STAT_ENTRY(eager_send);

    MPID_TRACE_CODE ("Eagern_send", dest_dev_lrank);

    if (MPID_SMI_cfg.SENDSELF && (dest_dev_lrank == MPID_SMI_myid)) {
		return MPID_SMI_Send_self (buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep);
    }

    /* get address at destination process */
    if(!(eager_addr = (char *)MPID_SMI_Eagern_int.get_buf(dest_dev_lrank, len))) {
		/* switch to rendezvous because no free eager buffer is available */
		MPID_STAT_COUNT(no_eagerbuf);
		return MPID_SMI_Rndv_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dtype);
    }

    MPID_SMI_Eagern_int.sendcpy(dest_dev_lrank, (void *)eager_addr, buf, len, dtype);
	eager_offset = eager_addr - MPID_SMI_outgoing_eagerbufs[dest_dev_lrank]; /* get offset from pointer */
    
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_SEND_ADDRESS_T), &prepkt, 0, NULL, dest_dev_lrank, 0);
    MPID_INIT_EAGER_PREPKT (prepkt, MPID_PKT_SEND_ADDRESS, context_id, src_comm_lrank, tag, len, 0, eager_offset);

    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Sending eager message with send", &prepkt, dest_dev_lrank);
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;
    
    /* Inidicate that this connection is no longer used by this transfer. */
    MPID_SMI_Eagern_int.disconnect(dest_dev_lrank);

    MPID_STAT_EXIT(eager_send);
    return MPI_SUCCESS;
}


int MPID_SMI_Eagern_cancel_send( shandle )
	 MPIR_SHANDLE *shandle;
{
    return MPI_SUCCESS;
}

int MPID_SMI_Eagern_test_send( shandle )
	 MPIR_SHANDLE *shandle;
{
    if( (shandle->sid[EAGERN_ISEND_TRANSFER]) == EAGERN_ISEND_TRANSFER_COMPLETE )
		shandle->is_complete = 1;
    return MPI_SUCCESS;
}

#ifdef MPID_USE_DEVTHREADS
/* this function is no longer redundant! */
int MPID_SMI_Eagern_wait_send( shandle )
	 MPIR_SHANDLE *shandle;
{
    void *status;

    if (shandle->sid[EAGERN_ISEND_MODE] == EAGERN_ISEND_MODE_DMA) {
		/* if DMA is used, the send is complete even if the DMA is still in progress and
		   the concerned thread is stil waiting for its completion. This requires a 
		   pthread mutex synchronisation to avoid polling */
		pthread_mutex_lock ((pthread_mutex_t *)shandle->sid[EAGERN_ISEND_ID_N_LOCK]);
    } else {
		if(shandle->sid[EAGERN_ISEND_MODE] == EAGERN_ISEND_MODE_PIO){
			pthread_join((pthread_t) shandle->sid[EAGERN_ISEND_ID_N_LOCK], &status);    
		}
    }
	
    /* mark transfer as completed
     * this combination of flags is used by MPID_SMI_Eagern_finish_send
     * to indicate that shandle->sid[EAGERN_ISEND_ID_N_LOCK] has already been freed(DMA) or
     * thread has already been joined(PIO)
     */
    shandle->sid[EAGERN_ISEND_TRANSFER] = EAGERN_ISEND_TRANSFER_UNCOMPLETE;
    shandle->is_complete = 1;

    return MPI_SUCCESS;
}

int MPID_SMI_Eagern_finish_send( shandle )
	 MPIR_SHANDLE *shandle;
{
    /* if shandle->sid[EAGERN_ISEND_TRANSFER] is set we know, that shandle->is_complete
     * has been set by MPID_SMI_test_send and so we still need to
     * store shandle->sid[EAGERN_ISEND_ID_N_LOCK] (DMA) / join the thread (PIO)
     */
    switch (shandle->sid[EAGERN_ISEND_MODE]) {
    case EAGERN_ISEND_MODE_DMA:
		{
			pthread_mutex_t *DMA_lock = (pthread_mutex_t *)shandle->sid[EAGERN_ISEND_ID_N_LOCK];
	   
			pthread_mutex_unlock (DMA_lock);
			pthread_mutex_destroy (DMA_lock);
			/* put mutex on stack for later re-use */
			MPID_stack_push(DMAlock_stack, (void *)DMA_lock);
	   
			break;
		}
    case EAGERN_ISEND_MODE_PIO:
		{
			void *status;;
			if (shandle->sid[EAGERN_ISEND_TRANSFER] == EAGERN_ISEND_TRANSFER_COMPLETE) {
				pthread_join((pthread_t) shandle->sid[EAGERN_ISEND_ID_N_LOCK], &status);       
			}
		}
		break;
    }
   
    return MPI_SUCCESS;
}
#endif /* MPID_USE_DEVTHREADS */


/*
 * This is the routine called when a packet of type MPID_PKT_SEND_ADDRESS is
 * seen.  It receives the data as shown (final interface not set yet)
 */
int MPID_SMI_Eagern_recv( rhandle, from, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from;
	 void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T   *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;
    struct MPIR_DATATYPE * dtype_ptr;
    void *destbuf;
    int   msglen, err = MPI_SUCCESS;
    char *eager_addr;

    MPID_STAT_ENTRY(eager_recv);

    MPID_TRACE_CODE ("Eagern_recv", from);
    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Receiving eager message with recv", pkt, from);

    msglen = pkt->len;
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_ERROR  = err;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;

    EAGER_DEBUG(fprintf (stderr, "[%d] got eager from (%d): size 0x%x,  offset 0x%x\n", MPID_SMI_myid, from,
						 msglen, pkt->address););

    /* get ptr from offset and copy recv buffer to user buffer */
	eager_addr = MPID_SMI_incoming_eagerbufs[from] + pkt->offset;
    
    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from, IS_CTRL_MSG);

	/* If rhandle->buf is given, we have an intermediate buffer and copy from there, in that case
	   unpacking is done elsewhere, so we give NULL as datatype. Otherwise use the rhandle->start
	   which should pouint directly to the starting point of the data and give rhandle->datatype
	   for direct unpack. */
    destbuf   = (rhandle->buf != NULL) ? rhandle->buf : rhandle->start;
    dtype_ptr = (rhandle->buf != NULL) ? NULL : rhandle->datatype;
    /* Support for direct_reduce: if rhandle->op_ptr != NULL, we do pass the 
       dtype-ptr for "direct reduction" and adapt the count to relate to the datatype. */
    if (rhandle->op_ptr != NULL) {
		dtype_ptr = rhandle->datatype;
		msglen = rhandle->op_cnt;
    }
    MPID_SMI_Eagern_int.recvcpy(from, eager_addr, destbuf, msglen, dtype_ptr, rhandle->op_ptr);

    MPID_SMI_Eagern_int.free_buf (eager_addr, msglen, from );
    if (rhandle->finish) {
		(rhandle->finish)( rhandle );
    }
    rhandle->is_complete = 1;
    
    MPID_STAT_EXIT(eager_recv);

    return err;
}

/* This routine is called when a message arrives and was expected */
int MPID_SMI_Eagern_irecv( rhandle, from, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from;
	 void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;
    struct MPIR_DATATYPE * dtype_ptr;
    void *destbuf;
    int   msglen, err = MPI_SUCCESS;
    char *eager_addr;
	
    MPID_TRACE_CODE ("Eagern_irecv", from);
    MPID_SMI_DEBUG_PRINT_FULL_EAGER_ADDRESS_PKT("S Receiving eager messag with irecv", pkt, from);
	
    msglen = pkt->len;
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;

    /* get ptr from offset */
	eager_addr = MPID_SMI_incoming_eagerbufs[from] + pkt->offset;

    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from, IS_CTRL_MSG);

    /* if the buf exists, we have an intermediate buffer and copy from there, in that case
       unpacking is done elsewhere, so we give NULL as datatype. Otherwise use the rhandl->start
       which should pouint directly to the starting point of the data and give rhandle->datatype
       for direct unpack */
    destbuf   = (rhandle->buf != NULL) ? rhandle->buf : rhandle->start;
    dtype_ptr = (rhandle->buf != NULL) ? NULL : rhandle->datatype;
    /* Support for direct_reduce: if rhandle->op_ptr != NULL, we do pass the 
       dtype-ptr for "direct reduction" and adapt the count to relate to the datatype. */
    if (rhandle->op_ptr != NULL) {
		dtype_ptr = rhandle->datatype;
		msglen = rhandle->op_cnt;
    }
    MPID_SMI_Eagern_int.recvcpy(from, eager_addr, destbuf, msglen, dtype_ptr, rhandle->op_ptr);

    MPID_SMI_Eagern_int.free_buf( eager_addr, msglen, from );

    if (rhandle->finish)
		(rhandle->finish)( rhandle );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    
    return err;
}

/* This function is called when a packet of type MPID_PKT_SEND_ADDRESS has arrived for
   a message that has not yet been posted. It saves the message data (from the packet) 
   in rhandle and frees the packet. If MPID_SMI_EAGER_IMMEDIATE is set, the message is copied to 
   an intermediate buffer and the eager buffer is freed. */
int MPID_SMI_Eagern_save( rhandle, from, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from;
	 void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;
    char *inbuf_addr;

    MPID_TRACE_CODE ("Eagern_save", from);

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 0;
    rhandle->from         = from;
    rhandle->partner      = from;
    rhandle->send_id      = pkt->send_id;  /* this should be done in queue.c */
	
    EAGER_DEBUG(fprintf (stderr, "[%d] got eager from (%d) (unexpct): size 0x%x,  offset 0x%x\n", 
						 MPID_SMI_myid, from, pkt->len, pkt->address););

    /* get ptr from offset */
	inbuf_addr = MPID_SMI_incoming_eagerbufs[from] + pkt->offset;

    if (MPID_SMI_EAGER_IMMEDIATE && pkt->len > 0) {

		/* Allocate intermediate buffer */
		ALLOCATE(rhandle->start, void *, pkt->len);

		/* Copy message to intermediate buffer */
		MPID_SMI_Eagern_int.recvcpy (from, (void *)inbuf_addr, rhandle->start, pkt->len, 
									 rhandle->datatype, NULL);

		/* Free eager buffer */
		MPID_SMI_Eagern_int.free_buf (inbuf_addr, pkt->len, from);

		rhandle->is_complete  = 1;
    } else {
		/* Leave the message in in incoming eager buffer until there is a matchin recv - 
		   this will save one memcpy(), but is only possible with static eager. */
		rhandle->start = inbuf_addr;
    }

	/* Free control packet */
    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from, IS_CTRL_MSG);

    rhandle->push   = MPID_SMI_Eagern_unxrecv_start;
    rhandle->cancel = MPID_SMI_Eagern_cancel_recv;

    rhandle->is_valid = 1;
    return 0;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SMI_Eagern_unxrecv_start( rhandle, in_runex )
	 MPIR_RHANDLE *rhandle;
	 void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    struct MPIR_DATATYPE * dtype_ptr;
    int   msglen, err = 0;
    void *destbuf;

    MPID_TRACE_CODE ("Eagern_unexrecv_start", runex->from);

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle, msglen, err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
        destbuf   = (rhandle->buf != NULL) ? rhandle->buf : rhandle->start;
		dtype_ptr = (rhandle->buf != NULL) ? NULL : rhandle->datatype;
		/* Support for direct_reduce: if rhandle->op_ptr != NULL, we do pass the 
		   dtype-ptr for "direct reduction" and adapt the count to relate to the datatype. */
		if (rhandle->op_ptr != NULL) {
			dtype_ptr = rhandle->datatype;
			msglen = rhandle->op_cnt;
		}
		MPID_SMI_Eagern_int.recvcpy (runex->from, runex->start, destbuf, msglen, dtype_ptr, 
									 rhandle->op_ptr);

		if (MPID_SMI_EAGER_IMMEDIATE)
			FREE (runex->start);
		else
			MPID_SMI_Eagern_int.free_buf (runex->start, msglen, runex->from);
    }

    rhandle->s		 = runex->s;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    MPID_Recv_free( runex );
    if (rhandle->finish) 
		(rhandle->finish)( rhandle );

    return err;
}


int MPID_SMI_Eagern_cancel_recv( runex )
	 MPIR_RHANDLE *runex;
{
    /* free the local eager buffer */
    if (MPID_SMI_EAGER_IMMEDIATE && runex->len > 0) {
		FREE( runex->start );
    } else {
		MPID_SMI_Eagern_int.free_buf(runex->start, runex->len, runex->from );
    }

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
