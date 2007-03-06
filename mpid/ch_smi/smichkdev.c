/*
 *  $Id$
 *
 */

#include <pthread.h>
#include <signal.h>

#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "smidef.h"
#include "../util/queue.h"
#include "smistat.h"
#include "remote_handler.h"
#include "smirndv.h"

#define MPID_SMI_DEBUG_ASYNC   0

MPID_DEBUG_CODE (static int no_msg = 0;)

#ifdef MPID_USE_DEVTHREADS
	static pthread_cond_t msg_arrived_cond;
static int msgs_pending = 0;
static int waitlck_nest = 0;
#endif

/* Check for incoming messages.
   Input Parameter:
   .   is_blocking - true if this routine should block until a message is
   available

   Returns -1 if nonblocking and no messages pending

   This routine makes use of a single dispatch routine to handle all
   incoming messages.  This makes the code a little lengthy, but each
   piece is relatively simple.
*/    
int MPID_SMI_Check_incoming( MPID_Device *dev, MPID_BLOCKING_TYPE is_blocking )
{
    MPID_PKT_T   *pkt = NULL;
    MPIR_RHANDLE *rhandle = NULL;
    int          from_grank = -1;
    int          is_posted;
    int          err = MPI_SUCCESS;
#ifdef MPID_USE_DEVTHREADS
	static int   have_lock = 1;
#endif

    MPID_STAT_ENTRY(checkdev);
    MPID_DEBUG_CODE (if (!no_msg))
		MPID_SMI_DEBUG_PRINT_MSG("Entering check_incoming");

#ifdef MPID_USE_DEVTHREADS
    /* once an application thread has tried to receive a message, 
       it is save to let the async thread to the same */
    if (have_lock) {
		MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_async_check_lck);
		have_lock = 0;
    }

    if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
		if (pthread_getspecific (MPID_SMI_thread_type) == NULL) {
			/* This is not the device thread, but an application thread. If polling
			   is deactivated, only the device thread is allowed to check the 
			   incoming msg queues after he received a signal. If nothing is
			   available in the unexpected/posted msg queues, it waits
			   for activity of the device thread.*/
			MPID_STAT_COUNT(thread_yield);
	    
			if (is_blocking == MPID_NOTBLOCKING) {
				/* We can not wait in the lock, because a new message wil not necessarily
				   arrive (which would then release us from the lock). This applies 
				   because the device is often only checked (non-blocking) to ensure
				   progress while  doing other things. 
		   
				   XXX This is not really a clean design!? Can we change this so the device 
				   is never polled without knowing that there will be a message arriving? */
				MPID_SMI_YIELD;
			} else {
				MPID_SMI_LOCK(&MPID_SMI_waitmsg_lck);
				while (msgs_pending <= 0) {
					pthread_cond_wait (&msg_arrived_cond, &MPID_SMI_waitmsg_lck);
#if MPID_SMI_DEBUG_ASYNC
					MPID_DEBUG_CODE (fprintf(MPID_DEBUG_FILE, "[%d] got signal, msgs = %d\n", 
											 MPID_SMI_myid, msgs_pending);)
#endif
						}
				msgs_pending = 0;
				MPID_SMI_UNLOCK(&MPID_SMI_waitmsg_lck);
			}

			MPID_STAT_EXIT(checkdev);
			return -1;
		}
    }
#endif

    MPID_SMI_ASYNC_LOCK(&MPID_SMI_incoming_lck);

    /* If nonblocking and no headers available, exit. For blocking receive,
       from_grank = -1 will make ReadControl loop until a message arrives. */
    if (is_blocking == MPID_NOTBLOCKING) {
		from_grank = MPID_SMI_pkt_avail();
		if (from_grank == -1) {
			MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_incoming_lck);
			MPID_DEBUG_CODE (no_msg = 1);
			/* Nothing else to do - check if a postponed transfer can be started now. 
			   This is also done at the end of each rndv transfer. */
			if (1 || (MPID_SMI_Rndvrecvs_in_progress == 0 && MPID_SMI_Rndvsends_in_progress == 0 &&
					  MPID_SMI_Rndvrecvs_scheduled == 0 && MPID_SMI_Rndvsends_scheduled == 0))
				MPID_SMI_Check_postponed();

			MPID_STAT_EXIT(checkdev);
			return -1;
		}
    }

    MPID_SMI_DEBUG_PRINT_MSG("Waiting for message to arrive");
    MPID_SMI_ReadControl (&pkt, &from_grank);
    MPID_SMI_DEBUG_PRINT_PKT("R received message", pkt);
    MPID_DEBUG_CODE (no_msg = 0);

#ifdef MPID_USE_DEVTHREADS
    if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
		/* Only the device thread will get here. We need waitlck_nest to 
		   avoid a deadlock with MPID_SMI_waitmsg_lck in case that the device
		   thread recursively enters MPID_SMI_Check_incoming() - it must lock
		   only on the top-level entry! */
		if (waitlck_nest == 0) {
			MPID_SMI_LOCK(&MPID_SMI_waitmsg_lck);
		}
		msgs_pending++;
		waitlck_nest++;
    }
#endif

    /* Separate the incoming messages from control messages */
    if (MPID_PKT_IS_MSG(pkt->head.mode)) {
		/* Is the message expected or not?  This routine RETURNS a rhandle, 
		   creating one if the message is unexpected (is_posted == 0). */
		MPID_Msg_arrived( pkt->head.lrank, pkt->head.tag, pkt->head.context_id, &rhandle, &is_posted );
		/* we need to lock up to this point to ensure the correct FIFO-order of the queues ! */
		MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_incoming_lck);

		MPID_DEBUG_IFCODE(fprintf( MPID_DEBUG_FILE, "[%d] R msg was %s (%s:%d)\n", MPID_SMI_myid, 
								   is_posted ? "posted" : "unexpected", __FILE__, __LINE__ ););

		if (is_posted) {
			/* We should check the size here for internal errors .... */
			switch (pkt->head.mode) {
			case MPID_PKT_SHORT:
				MPID_SMI_DEBUG_TEST_FCN(dev->short_msg->recv,"dev->short->recv");
				err = (*dev->short_msg->recv)( rhandle, from_grank, pkt );
				break;

			case MPID_PKT_SEND_ADDRESS:
				MPID_SMI_DEBUG_TEST_FCN(dev->eager->recv,"dev->short->recv");
				err = (*dev->eager->recv)( rhandle, from_grank, pkt );
				break;

			case MPID_PKT_REQUEST_SEND:
				MPID_SMI_DEBUG_TEST_FCN(dev->rndv->irecv,"dev->rndv->irecv");
				err = (*dev->rndv->irecv)( rhandle, from_grank, pkt );
				break;
	      
			case MPID_PKT_REQUEST_SEND_RDY:
				MPID_SMI_DEBUG_TEST_FCN(dev->ready->irecv,"dev->ready->irecv");
				err = (*dev->ready->irecv)( rhandle, from_grank, pkt );
				break;
	      
			default:
				MPID_ABORT ("Unknown msg packet mode in Check_incoming() (exepected recv).");
			}
		} else {
			/* no matching recv has been posted -> msg is unexpected */
			switch (pkt->head.mode) {
			case MPID_PKT_SHORT:
				MPID_SMI_DEBUG_TEST_FCN(dev->short_msg->unex,"dev->short->unex");
				err = dev->short_msg->unex( rhandle, from_grank, pkt );
				break;

			case MPID_PKT_SEND_ADDRESS:
				MPID_SMI_DEBUG_TEST_FCN(dev->eager->unex,"dev->eager->unex");
				err = dev->eager->unex( rhandle, from_grank, pkt );
				break;

			case MPID_PKT_REQUEST_SEND:
				/* synchronous or asynchronous rndv requests are treated the same way */
				MPID_SMI_DEBUG_TEST_FCN(dev->rndv->unex,"dev->rndv->unex");
				err = dev->rndv->unex( rhandle, from_grank, pkt );
				break;

			case MPID_PKT_REQUEST_SEND_RDY:
				/* Messages should never be unexpected for the ready protocol! We could
				   abort here, but instead, we just handle the situation... */
				MPID_SMI_DEBUG_TEST_FCN(dev->ready->unex,"dev->ready->unex");
				err = dev->ready->unex( rhandle, from_grank, pkt );
				break;
		
			default:
				MPID_ABORT ("Unknown msg packet mode in Check_incoming() (unexepected recv).");
			}
		}
    } else {
		MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_incoming_lck);
		/* This is no new message, but a control packet for an ongoing message 
		   transmission or internal control purposes. */

		switch (pkt->head.mode) {
			/* rendez-vous transfers */
		case MPID_PKT_CONT:
		case MPID_PKT_OK_TO_SEND:
			MPID_SMI_DEBUG_TEST_FCN(dev->rndv->do_ack,"dev->rndv->do_ack");
			err = dev->rndv->do_ack( pkt, from_grank );
			break;
		case MPID_PKT_PART_READY:
			err = MPID_SMI_Nbrndv_push_recv( pkt, from_grank );
			break;
		case MPID_PKT_REQUEST_SEND_NOZC: 
			/* this is a control message, although it's request to send: the rhandle does
			   already exist, but is not in the posted queue. */
			err = MPID_SMI_Rndv_irecv_nozc (pkt, from_grank);
			break;

			/* Blocking rendezvous protocol */
		case MPID_PKT_CONT_RDY:
			err = MPID_SMI_Brndv_recv_ack( pkt, from_grank );
			break;
		case MPID_PKT_OK_TO_SEND_RDY:
			err = MPID_SMI_Brndv_send_ack( pkt, from_grank );
			break;

			/* one-sided communication */
		case MPID_PKT_PUT:
			MPID_SMI_Do_put_emulation (pkt, from_grank);
			break;
		case MPID_PKT_GET:
			MPID_SMI_Do_get_emulation (pkt, from_grank);
			break;
		case MPID_PKT_ACCU:
			MPID_SMI_Do_accumulate (pkt, from_grank);
			break;
		case MPID_PKT_ONESIDED:
			MPID_SMI_Os_delayed_process (pkt, from_grank);
			break;

			/* Pipelined Broadcast */
		case MPID_PKT_PIPE_READY:
			MPID_SMI_Pipe_init (pkt, from_grank);
			break;

			/* inter-process resource management */
		case MPID_PKT_RSRC_REQ:
			MPID_SMI_rsrc_req (pkt, from_grank); 	    
			break;
		case MPID_PKT_RSRC_OK:
			MPID_SMI_rsrc_ack (pkt, from_grank);
			break;
	    
			/* message cancellation */		
		case MPID_PKT_ANTI_SEND:
			MPID_SMI_SendCancelOkPacket( pkt, from_grank ); 
			break;
		case MPID_PKT_ANTI_SEND_OK:
			MPID_SMI_RecvCancelOkPacket( pkt, from_grank ); 
			break;

		default:
			MPID_ABORT ("Unknown control packet mode in Check_incoming().");
		}
		/* Really should remember error in case subsequent events are successful. */
    }

#ifdef MPID_USE_DEVTHREADS
    if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
		waitlck_nest--;
		if (waitlck_nest == 0) {
			MPID_SMI_UNLOCK(&MPID_SMI_waitmsg_lck);
			pthread_cond_signal (&msg_arrived_cond);
#if MPID_SMI_DEBUG_ASYNC
			MPID_DEBUG_CODE(fprintf(MPID_DEBUG_FILE, "[%d] sending signal, msgs = %d\n", 
									MPID_SMI_myid, msgs_pending););
#endif
		}
    }
#endif

    MPID_SMI_DEBUG_PRINT_MSG("Exiting check_incoming");
    MPID_STAT_EXIT(checkdev);
    return err;
}

/* import this to get information when to terminate the checkdev thread */
extern volatile int wd_do_finalize;

/* Worker thread. This thread is started to wait for a signal, then
   check the device queues and waits again */
void *MPID_SMI_Async_devcheck (void *args)
{
#ifdef MPID_USE_DEVTHREADS
    MPID_SMI_LOCK_T wait_lck;
    pthread_cond_t wait_cond;
    sigset_t signals_to_ignore;
    int *thread_type;
    int have_lock = 0;
    int check_cnt, got_msg, delay_us;
    double check_start;
    int old_cancel_type;

    sigemptyset(&signals_to_ignore);
    sigaddset(&signals_to_ignore, SIGINT);
    sigaddset(&signals_to_ignore, SIGALRM);   
    sigaddset(&signals_to_ignore, SIGHUP);   
    sigaddset(&signals_to_ignore, SIGKILL);   
    sigaddset(&signals_to_ignore, SIGQUIT);
    sigaddset(&signals_to_ignore, SIGTERM);
    sigaddset(&signals_to_ignore, SIGUSR1);
    sigaddset(&signals_to_ignore, SIGUSR2);  
    pthread_sigmask(SIG_BLOCK, &signals_to_ignore, NULL);

    ALLOCATE (thread_type, int *, sizeof(int));
    *thread_type = DEVICE_THREAD;
    pthread_setspecific (MPID_SMI_thread_type, (void *)thread_type);

    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    if (MPID_SMI_cfg.MSGCHK_TYPE != MSGCHK_TYPE_POLLING) {
		pthread_cond_init (&msg_arrived_cond, NULL);
		if (MPID_SMI_cfg.MSGCHK_TYPE == MSGCHK_TYPE_IRQ_BLOCK) {
			pthread_cond_init (&wait_cond, NULL);
			MPID_SMI_INIT_LOCK(&wait_lck);
			delay_us = (int)(MPID_SMI_cfg.MSGCHK_DELAY*1e+6);
		}
    }

    /* we run, until someone resets this! */
    MPID_SMI_Do_async_devcheck = 1;	

    while (MPID_SMI_Do_async_devcheck && !wd_do_finalize) {
		/* Wait for any (sourced from any rank) signal */
#if WAIT_FOR_SIGNAL
        MPID_STAT_CALL(signal_wait);
		SMI_Signal_wait (SMI_SIGNAL_ANY);
        MPID_STAT_RETURN (signal_wait);
#endif
		/* quit, if we should */
		if (!MPID_SMI_Do_async_devcheck)
			break;

		/* we need to prevent that the thread starts receiving messages
		   before the main application is ready */
		if (!have_lock) {
			MPID_SMI_ASYNC_LOCK(&MPID_SMI_async_check_lck);
			have_lock = 1;
		}

#if DO_ASYNC_CHECK
		/* DeviceCheck returns after it found a ctrl pkt and called the
		   corresponding handler. So we call DeviceCheck, until there
		   was no further ctrl pkt found to deliver. */
		switch (MPID_SMI_cfg.MSGCHK_TYPE) {
		case MSGCHK_TYPE_IRQ_POLL:
			do {
				/* First, drain all incoming queues. */
				while (MPID_DeviceCheck (MPID_NOTBLOCKING) != -1 ) {
#if MPID_SMI_DEBUG_ASYNC
					MPID_DEBUG_CODE (fprintf (MPID_DEBUG_FILE, "[%d] got msg (IRQ)\n", MPID_SMI_myid);)
#endif
						MPID_STAT_COUNT (thread_got_msg);
				}
		
				/* Then, continue polling for the specified delay. If a new message
				   comes in during this period, we start all over. If not, we go
				   waiting for the next signal from a remote process. */
				check_start = SMI_Wtime();
				got_msg = 0;
				do {
					if (MPID_DeviceCheck (MPID_NOTBLOCKING) != -1 ) {
#if MPID_SMI_DEBUG_ASYNC
						MPID_DEBUG_CODE (fprintf (MPID_DEBUG_FILE, "[%d] got msg (POLL)\n", MPID_SMI_myid);)
#endif
							MPID_STAT_COUNT (thread_got_msg);
						got_msg = 1;
						break;
					}
				} while (SMI_Wtime() - check_start < MPID_SMI_cfg.MSGCHK_DELAY);
			} while (got_msg);
#if MPID_SMI_DEBUG_ASYNC
			MPID_DEBUG_CODE (fprintf (MPID_DEBUG_FILE, "[%d] waiting for signal\n", MPID_SMI_myid);)
#endif
				break;
		case MSGCHK_TYPE_IRQ_BLOCK:
			do {
				struct timespec ts;
				struct timeval  tv;

				/* First, drain all incoming queues. */
				while (MPID_DeviceCheck (MPID_NOTBLOCKING) != -1)
					MPID_STAT_COUNT (thread_got_msg);

				/* Same as above, but sleep for the specified delay and probe again. */
				check_cnt = 0;
				got_msg = 0;
				do {
					/* The timout is given as *absolute* time! 
					   PROBLEM: the resolution of the clock for cond_timedwait()
					   is typically 10ms! */
					MPID_SMI_LOCK(&wait_lck);
					gettimeofday (&tv, NULL);
					ts.tv_nsec = (tv.tv_usec + delay_us)*1000;
					ts.tv_sec = tv.tv_sec;
					while (ts.tv_nsec > 1000000000) {
						ts.tv_nsec -= 1000000000;
						ts.tv_sec++;
					}
					pthread_cond_timedwait (&wait_cond, &wait_lck, &ts);
					MPID_SMI_UNLOCK(&wait_lck);
					check_cnt++;
		    
					if (MPID_DeviceCheck (MPID_NOTBLOCKING) != -1) {
						MPID_STAT_COUNT (thread_got_msg);
						got_msg = 1;
						break;
					}
				} while (check_cnt < MPID_SMI_cfg.MSGCHK_REPEAT);
			} while (got_msg);
			break;
		case MSGCHK_TYPE_IRQ:
		case MSGCHK_TYPE_POLLING:
			/* Drain all incoming queues, then wait for the next signal to come in. */
			while (MPID_DeviceCheck (MPID_NOTBLOCKING) != -1) {
				MPID_STAT_COUNT (thread_got_msg);
			}
			break;
		}
#endif
    }
    
    pthread_key_delete (MPID_SMI_thread_type);
    FREE (thread_type);
    pthread_exit (&have_lock);
#endif
    return NULL;
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
