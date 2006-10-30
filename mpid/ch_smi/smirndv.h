/* $Id$ */

#ifndef _MPID_SMI_RNDV_H
#define _MPID_SMI_RNDV_H

#include "reqrndv.h"
#include "smi.h"

#include "sendq.h"	/* For MPIR_FORGET_SEND */
#include "stack.h"

#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "reqalloc.h"

#include "smidef.h"
#include "smistat.h"
#include "smidebug.h"
#include "smisync.h"
#include "dataqueue.h"
#include "smiregionmngmt.h"

/*
 * some protocol parameters 
 */

/* poll for completion of an async transer, or block on a lock? */
#if !DO_ASYNC_CHECK
#define WAIT_ON_LOCK         0
#else
#define WAIT_ON_LOCK         1
#endif

#define MPID_SMI_RNDV_MINMEM   4096     /* minimal size to allocate as transfer-buffer */
#define MPID_SMI_RNDV_MINPOOL (4*4096)  /* minimal size to allocate as rndv memory pool */

#define MPID_SMI_BRNDV_PTRMEM (3*MPID_SMI_STREAMSIZE)

/* how often should a remote connection be retried? */
#define MPID_SMI_RNDV_CNCT_MAX_RETRIES 1

/* fixed-size allocation */
#define INIT_RNDV_INFOS 10
#define INCR_RNDV_INFOS 10


/* globals, declared in smirndv.c */
extern char *MPID_SMI_rndv_scipool;
extern int   MPID_SMI_rndv_scipool_regid;
extern int   MPID_SMI_rndv_scipool_mapsize;
extern char *MPID_SMI_rndv_shmempool;
extern int   MPID_SMI_rndv_shmempool_regid;
extern int MPID_SMI_Rndvrecvs_in_progress;
extern int MPID_SMI_Rndvsends_in_progress;
extern int MPID_SMI_Rndvrecvs_scheduled;
extern int MPID_SMI_Rndvsends_scheduled;
extern int MPID_SMI_Rndv_adpt_nbr; 
extern int MPID_SMI_Rndv_sgmt_id;
extern int MPID_SMI_register_flag;
extern int MPID_SMI_connect_flag;

extern int *MPID_SMI_Rndvsend_to_proc;
extern int  *MPID_SMI_Shregid_rndv;

extern MPID_SBHeader rndvinfo_allocator;

/* This experimental define should be set to '1' unless you know what you are doing. If set to '0',
   deadlocks can occur if all sends operations so far are unexpected at the receiver. */
#define CHECK_TRANSFERS_IN_PROGRESS 1
#if CHECK_TRANSFERS_IN_PROGRESS
#define CURRENT_SENDS MPID_SMI_Rndvsends_in_progress
#define CURRENT_RECVS MPID_SMI_Rndvrecvs_in_progress
#else
#define CURRENT_SENDS MPID_SMI_Rndvsends_scheduled
#define CURRENT_RECVS MPID_SMI_Rndvrecvs_scheduled
#endif

#define RNDV_SEND_SCHEDULED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck); \
                                MPID_SMI_Rndvsends_scheduled++; \
                                MPID_SMI_Rndvsend_to_proc[proc]++; \
                                MPID_n_pending++; \
                                MPID_STAT_PROBE (nbr_rndv_sends, MPID_SMI_Rndvsends_scheduled); \
                                MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); }
#define RNDV_SEND_STARTED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck);  \
                                MPID_SMI_Rndvsends_in_progress++; \
                                MPID_SMI_Rndvsends_scheduled--; \
                                MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); }
#define RNDV_SEND_FINISHED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck); \
                                 MPID_SMI_Rndvsends_in_progress--; \
                                 MPID_SMI_Rndvsend_to_proc[proc]--; \
                                 MPID_n_pending--; \
                                 MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); \
                                 MPID_SMI_Check_postponed(); }
#define RNDV_RECV_SCHEDULED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck); \
                                MPID_SMI_Rndvrecvs_scheduled++; \
                                MPID_STAT_PROBE (nbr_rndv_recvs, MPID_SMI_Rndvrecvs_scheduled); \
                                MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); } 
#define RNDV_RECV_STARTED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck); \
                                MPID_SMI_Rndvrecvs_in_progress++; \
                                MPID_SMI_Rndvrecvs_scheduled--; \
                                MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); }
#define RNDV_RECV_FINISHED(proc) { MPID_SMI_ASYNC_LOCK(&MPID_SMI_counters_lck); \
                                 MPID_SMI_Rndvrecvs_in_progress--;\
                                 MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_counters_lck); \
                                 MPID_SMI_Check_postponed(); }


/* Handle the special case of 0-length data which can occur in strange situations 
   like synchronous send with 0 bytes */
#define RNDV_CHECK_ZEROMSG(pkt, handle) if ((pkt)->len_avail == 0) { \
    if ((handle)->bytes_as_contig == 0) { \
	    (pkt)->sgmt_offset = 0; (pkt)->data_offset = 0; \
	} else { \
	    MPID_ABORT ("The receiver did not provide a recv buffer for non-zero-length message!"); \
	} \
    }

#define ALLOC_RNDVINFO_SEND(h) if ((h)->handle_type == MPIR_SEND) {\
                                 (h)->recv_handle = (MPID_RNDV_T)MPID_SBalloc(rndvinfo_allocator); \
                                 memset((h)->recv_handle, 0, sizeof(MPID_SMI_RNDV_info)); \
                                 (h)->recv_handle->finish = (int (*)(void *))((h)->finish); \
	                         (h)->finish = (int (*)(MPIR_SHANDLE *))MPID_SMI_Rndv_free_rndvinfo; \
                               }
#define ALLOC_RNDVINFO_RECV(h) if ((h)->handle_type == MPIR_RECV) { \
                                 (h)->recv_handle = (MPID_RNDV_T)MPID_SBalloc(rndvinfo_allocator); \
                                 memset((h)->recv_handle, 0, sizeof(MPID_SMI_RNDV_info)); \
                                 (h)->recv_handle->finish = (int (*)(void *))((h)->finish); \
	                         (h)->finish = (int (*)(MPIR_RHANDLE *))MPID_SMI_Rndv_free_rndvinfo; \
                               }
#define FREE_RNDVINFO(h)   if ((h)->handle_type == MPIR_RECV || (h)->handle_type == MPIR_SEND) { \
                              MPID_SBfree (rndvinfo_allocator, (h)->recv_handle); \
                              (h)->recv_handle = NULL; \
                           }


#ifdef MPID_STATISTICS
#define INIT_RNDV_SYNCTIME(h) (h)->recv_handle->sync_delay = SMI_Wticks();
#define SAVE_RNDV_SYNCTIME(h) (h)->recv_handle->sync_delay = \
                               SMI_Wticks() - (h)->recv_handle->sync_delay; \
                               MPID_STAT_PERIOD(rndv_sync_delay, (h)->recv_handle->sync_delay);
#else
#define INIT_RNDV_SYNCTIME(h) 
#define SAVE_RNDV_SYNCTIME(h) 
#endif

/* flags for packet */
#define MPID_SMI_RNDV_ZEROCOPY 1      /* recv buffer is the user buffer (no pipelining) */ 
#define MPID_SMI_RNDV_DMAONLY  (1<<1) /* recv buffer can be accessed remotely via DMA only */
#define MPID_SMI_RNDV_DMAOK    (1<<2) /* sender can use DMA for its src buffer */
#define MPID_SMI_RNDV_PIOONLY  (1<<3) /* sender can only use PIO for its src buffer */
#define MPID_SMI_RNDV_ASYNC    (1<<4) /* this is an asynchronous rendez-vous transfer */


#ifdef MPIR_HAS_COOKIES
#define MPID_CHECK_COOKIE(h) if ((h)->cookie != MPIR_REQUEST_COOKIE) { \
		fprintf (stderr, "handle is %p\n", h); \
		fprintf (stderr, "handle cookie is %lx\n", h->cookie); \
		MPID_ABORT ("Illegal handle (cookie check failed)" ); } 
#if 0
/* XXX should be inserted into macro above, need generic MPID_Print_handle() for this. */
		MPID_Print_shandle (stderr, handle); 
#endif
#else
#define MPID_CHECK_COOKIE(handle)
#endif

#define VALIDATE_HANDLE(h) while (h->is_valid == 0) { MPID_SMI_YIELD; } 


#if WAIT_ON_LOCK
#define WAIT_LOCK_RELEASE(lock) if (lock) MPID_SMI_UNLOCK (lock);
#else
#define WAIT_LOCK_RELEASE(lock) 
#endif

/* ref_count == 0 would mean orphaned handle (usually not, may happen if MPI_Request_free()
   has been called.. This construct can *not* be simplified because after unlocking, 
   we are not allowed to touch the shandle any more. */
#define COMPLETE_SHANDLE(sh)  if ((sh)->finish) ((sh)->finish) (sh); \
                        (sh)->finish = 0; \
			if ((sh)->ref_count == 0) { \
				MPIR_FORGET_SEND( sh ); \
				MPID_Send_free( sh ); \
			} else { \
				(sh)->is_complete = 1; \
			}

/* We have all the data; the transfer is complete; release memory */
#define COMPLETE_RHANDLE(rh)  if ((rh)->finish) ((rh)->finish) (rh); \
                        (rh)->finish = 0; \
			if ((rh)->ref_count == 0) { \
				MPID_Recv_free( rh ); \
			} else { \
				(rh)->is_complete = 1; \
			}

#define COMPLETE_SHANDLE_ASYNC(sh)  if ((sh)->finish) ((sh)->finish) (sh); \
                        (sh)->finish = 0; \
			if ((sh)->ref_count == 0) { \
				WAIT_LOCK_RELEASE ((sh)->recv_handle->complete_lock); \
				arndv_free_lock((sh)->recv_handle->complete_lock); \
				MPIR_FORGET_SEND( sh ); \
				MPID_Send_free( sh ); \
			} else { \
				(sh)->is_complete = 1; \
				WAIT_LOCK_RELEASE( (sh)->recv_handle->complete_lock ); \
			}

/* We have all the data; the transfer is complete; release memory */
#define COMPLETE_RHANDLE_ASYNC(rh)  if ((rh)->finish) ((rh)->finish) (rh); \
                        (rh)->finish = 0; \
			if ((rh)->ref_count == 0) { \
				WAIT_LOCK_RELEASE( (rh)->recv_handle->complete_lock ); \
				arndv_free_lock((rh)->recv_handle->complete_lock); \
				MPID_Recv_free( rh ); \
			} else { \
				(rh)->is_complete = 1; \
				WAIT_LOCK_RELEASE( (rh)->recv_handle->complete_lock ); \
			}

#define MPID_RNDV_POSTPONE_SEND( buf, len, lrank, tag, context, dst, msgrep, sh, dtype ) \
  (sh)->start            = buf; \
  (sh)->bytes_as_contig  = len; \
  (sh)->src_lrank        = lrank; \
  (sh)->tag              = tag; \
  (sh)->context_id       = context; \
  (sh)->partner          = dst; \
  (sh)->msgrep           = msgrep; \
  (sh)->datatype         = dtype; \
  MPID_SMI_Send_postpone (sh);


typedef struct {
    void *pkt;
    int	 from_grank;
} MPID_SMI_RndvAck_Args;

/* this structure allows to choose among various versions of the rndv protocol */
typedef struct {
    int (*Send_ack_async) (void *, int);
    int (*Recv_ack_async) (void *, int);
    int (*Send_ack_sync) (void *, int);
    int (*Recv_ack_sync) (void *, int);
    int (*Setup_rndv_addr) (int *, ulong *, int *, int *, int);
} MPID_SMI_Rndv_int_t;

int MPID_SMI_Rndv_connect_zerocopy (char **, int, int, int, ulong, ulong, int, int *);
int MPID_SMI_Rndv_map_remote_mem(int , MPID_PKT_RNDV_T *, MPIR_SHANDLE *, boolean);
int MPID_SMI_Rndv_unxrecv_start ( MPIR_RHANDLE *, void * );
int MPID_SMI_Rndv_start_recv ( MPIR_RHANDLE *, void * );
int MPID_SMI_Rndv_unxrecv_end ( MPIR_RHANDLE * );
int MPID_SMI_Rndv_unxrecv_test_end ( MPIR_RHANDLE * );
int MPID_SMI_Rndv_cancel_recv ( MPIR_RHANDLE * );
int MPID_SMI_Rndv_cancel_send ( MPIR_SHANDLE * );
int MPID_SMI_Rndv_send_wait ( MPIR_SHANDLE * );
int MPID_SMI_Rndv_send_test_ack ( MPIR_SHANDLE * );
int MPID_SMI_Rndv_send_wait_ack ( MPIR_SHANDLE * );
int MPID_SMI_Rndv_free_rndvinfo(void *h);

/* implemented in smiarndv.c */
int MPID_SMI_Arndv_setup (void);
int MPID_SMI_Arndv_isend (void *, int, int, int, int, int,
			   MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );
int MPID_SMI_Arndv_irecv (MPIR_RHANDLE *, int, void *);
void MPID_SMI_Arndv_delete ( void );
void MPID_SMI_Arndv_get_config (int *size);
int MPID_SMI_Arndv_send_ack (void *, int);
int MPID_SMI_Arndv_send_ack_zc (void *, int);
int MPID_SMI_Arndv_recv_ack (void *, int);
int MPID_SMI_Arndv_unxrecv_start (MPIR_RHANDLE *, void *);
int MPID_SMI_Arndv_unxrecv_end (MPIR_RHANDLE *);
int MPID_SMI_Arndv_unxrecv_test_end (MPIR_RHANDLE *);
int MPID_SMI_Arndv_cancel_recv (MPIR_RHANDLE *);
int MPID_SMI_Arndv_send_wait (MPIR_SHANDLE *);
int MPID_SMI_Arndv_send_test_ack (MPIR_SHANDLE *);
int MPID_SMI_Arndv_send_wait_ack (MPIR_SHANDLE *);

int MPID_SMI_Arndv_cancel_send (MPIR_SHANDLE *shandle);
int MPID_SMI_Arndv_cancel_recv (MPIR_RHANDLE *runex);

/* implemented in smibrndv.c */
int MPID_SMI_Brndv_send_ack( void *, int );
int MPID_SMI_Brndv_recv_ack( void *, int );
int MPID_SMI_Brndv_get_recvbuf( int *, ulong *, int *, int *, int);

/* implmented in sminbrndv.c */
int MPID_SMI_Nbrndv_send_ack ( void *, int );
int MPID_SMI_Nbrndv_recv_ack ( void *, int );
int MPID_SMI_Nbrndv_get_recvbuf( int *, ulong *, int *, int *, int);
void MPID_SMI_Brndv_memdelete( void );
int MPID_SMI_Brndv_ack( void *in_pkt, int from_grank );
int MPID_SMI_Brndv_send(void *buf, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *);
int MPID_SMI_Brndv_resume_send ( MPIR_SHANDLE *sh );
int MPID_SMI_Brndv_isend( void *buf, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *,
				struct MPIR_DATATYPE *);
int MPID_SMI_Brndv_irecv( MPIR_RHANDLE *, int, void *);
int MPID_SMI_Brndv_save( MPIR_RHANDLE *, int, void *);
int MPID_SMI_Brndv_resume_recv (MPIR_RHANDLE *, void *);
int MPID_SMI_Brndv_unxrecv_start (MPIR_RHANDLE *, void *);

/* Prototype definitions */
MPID_Protocol *MPID_SMI_Rndv_setup(void);

int MPID_SMI_Rndv_mem_setup ( void  );
void MPID_SMI_Rndv_GetConfig (int *, int *);
void MPID_SMI_Rndv_preconnect( void );
void MPID_SMI_Rndv_delete ( MPID_Protocol * );

int MPID_SMI_Rndv_send ( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * );
int MPID_SMI_Rndv_isend ( void *, int, int, int, int, int, 
						  MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );

int MPID_SMI_Rndv_irecv ( MPIR_RHANDLE *, int, void * );
int MPID_SMI_Rndv_irecv_nozc (void *in_pkt, int from_grank );
int MPID_SMI_Rndv_save ( MPIR_RHANDLE *, int, void *);


int MPID_SMI_Rndv_ack ( void *, int );

int MPID_SMI_Nbrndv_push_recv ( void *, int );
int MPID_SMI_Nbrndv_push_send ( MPID_PKT_RNDV_T *, MPIR_SHANDLE *, int, int );

#endif
