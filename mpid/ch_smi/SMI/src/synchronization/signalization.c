/* $Id$ */

#define SMI_PT2PT_SIGNALIZATION 0	/* set to 1 to enable signalization
					 * from distinct processes */
#define SMI_GLOBAL_SIGNALIZATION 1	/* set to 1 to enable signalization
					 * from any process */

#include "signalization.h"
#include "message_passing/lowlevelmp.h"
#include "proper_shutdown/watchdog.h"

static int      sigMyId;
static int      sigNrOfProcesses;
static int      sigInit = FALSE;


#ifdef NO_SISCI

#include <signal.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static sigset_t look_for_these;
static siginfo_t extra;

smi_error_t SMI_Signal_wait (int proc_rank)
{
  DSECTION("SMI_Signal_wait");
  smi_error_t eError;
  int error;
  int iTargetPid;
  int iFinished = FALSE;
  union sigval sVal;

  DSECTENTRYPOINT;

  ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);
  ASSERT_R(proc_rank != sigMyId, "can not receive a signal from myself!", SMI_ERR_PARAM);
  if (proc_rank != SMI_SIGNAL_ANY) {
    ASSERT_R(proc_rank < sigNrOfProcesses, "invalid proc rank!", SMI_ERR_PARAM);
    ASSERT_R(proc_rank >= 0, "invalid proc rank!", SMI_ERR_PARAM);
  }
#if 0 
  while( iFinished == FALSE) {
    DNOTICE("Waiting for signal");
    error = sigwaitinfo(&look_for_these, &extra);
    ASSERT_R(error >= 0, "could not wait for signal!", SMI_ERR_OTHER);
    iFinished = (proc_rank == SMI_SIGNAL_ANY) || (proc_rank == extra.si_value.sival_int);
    DNOTICEI("Got a signal from", extra.si_value.sival_int);
  }
#endif 
  DSECTLEAVE; return(SMI_SUCCESS);
}

smi_error_t SMI_Signal_setCallBack (int proc_rank, void (*callback_fcn)(void *), 
				void *callback_arg, smi_signal_handle* h)
{
  DSECTION("SMI_Signal_setCallBack");
  DPROBLEM("not implemented in SMP version!");
  return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_Signal_joinCallBack (smi_signal_handle* h)
{ 
  DSECTION("SMI_Signal_joinCallBack");
  DPROBLEM("not implemented in SMP version!");
  return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_Signal_send ( int proc_rank )
{
  REMDSECTION("SMI_Signal_send");
  smi_error_t eError;
  int error;
  int iTargetPid;
  union sigval sVal;
  int iStart;
  int iStop;
  int i;

  DSECTENTRYPOINT;

  ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);
 
  if (proc_rank & SMI_SIGNAL_ANY)
    proc_rank -= SMI_SIGNAL_ANY;
  
  ASSERT_R(proc_rank != sigMyId, "can not send a signal to myself!", SMI_ERR_PARAM);
  if (proc_rank != SMI_SIGNAL_BCAST) {
    ASSERT_R(proc_rank < sigNrOfProcesses, "invalid proc rank!", SMI_ERR_PARAM);
    ASSERT_R(proc_rank >= 0, "invalid proc rank!", SMI_ERR_PARAM);
  }
  
  sVal.sival_int = sigMyId;
  
  if (proc_rank == SMI_SIGNAL_BCAST) {
    iStart = 0; 
    iStop = sigNrOfProcesses-1;
  }
  else {
    iStart = proc_rank;
    iStop = proc_rank;
  }
#if 0
  for(i = iStart; i <= iStop; i++) {
    if (i != sigMyId) {
      eError = SMI_Query(SMI_Q_SYS_PID, proc_rank, &iTargetPid);
      ASSERT_R((eError == SMI_SUCCESS), "could not determine target pid!", eError);
      DNOTICEI("Sending signal to process",i);
      error = sigqueue(iTargetPid, SIGRTMIN, sVal);
      ASSERT_R((error >= 0), "could not send signal!", SMI_ERR_OTHER);
    }
  }
#endif

  DSECTLEAVE
    return(SMI_SUCCESS);
}

int _smi_signal_init(int myid, int nbr_procs)
{ 
  struct sigaction sRTMIN;
  int error;

  /* signals for SMP-internal signalization */
  sigemptyset(&look_for_these);
  /* XXX FreeBSD, and therefore Darwin, doesn't define SIGRTMIN;
	I'm	not sure if we can just leave this out, though */
#ifndef DARWIN
  sigaddset(&look_for_these, SIGRTMIN);
#endif
  if (sigprocmask(SIG_BLOCK, &look_for_these, NULL)) {
      DERROR ("signal mask could not be set");
  }

  sigMyId = myid; 
  sigNrOfProcesses = nbr_procs;
  sigInit = TRUE;

  return(SMI_SUCCESS);
}

int _smi_signal_finalize()
{
  sigInit = FALSE;
  return(SMI_SUCCESS);
} 

#else /* NO_SISCI */

#include "proper_shutdown/sci_desc.h"

extern int     *_smi_sci_rank;

static smi_sci_desc_t *pt2pt_locint_desc;
static smi_sci_desc_t *pt2pt_rmtint_desc;

static sci_local_interrupt_t *sigRecvInterrupts;
static unsigned int *sigRecvIntIds;
static sci_remote_interrupt_t *sigSendInterrupts;
static unsigned int *sigSendIntIds;

#ifdef SMI_GLOBAL_SIGNALIZATION
static smi_sci_desc_t *global_rmtint_desc;
static smi_sci_desc_t global_locint_desc;
static sci_local_interrupt_t sigRecvAnyInterrupt;
static unsigned int sigRecvAnyIntId;
static sci_remote_interrupt_t *sigSendAnyInterrupts;
static unsigned int *sigSendAnyIntIds;
static pthread_mutex_t sigAnyUsed;
#endif

static pthread_mutex_t *sigUsed;

int      thread_retval = 0;

int _smi_signal_init(int myid, int nbr_procs)
{
    DSECTION("_smi_signal_init");
    int             i, itry, *dummy_gather;
#if SMI_PT2PT_SIGNALIZATION
	int				k, *gather_buffer;
#endif
    sci_error_t     sci_err;
    sci_desc_t      int_desc;

    DSECTENTRYPOINT;

    TEST_R(_smi_all_on_one == false, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);

    if (sigInit == TRUE)
	return (-2);

    sigMyId = myid;
    sigNrOfProcesses = nbr_procs;

    ALLOCATE(sigRecvInterrupts, sci_local_interrupt_t *, sizeof(sci_local_interrupt_t) * nbr_procs);
    ALLOCATE(sigRecvIntIds, unsigned int *, sizeof(int) * nbr_procs);
    ALLOCATE(sigSendInterrupts, sci_remote_interrupt_t *, sizeof(sci_remote_interrupt_t) * nbr_procs);
    ALLOCATE(sigSendIntIds, unsigned int *, sizeof(int) * nbr_procs);
    ALLOCATE(dummy_gather, int *, sizeof(int) * nbr_procs);

    ALLOCATE(sigUsed, pthread_mutex_t *, sizeof(pthread_mutex_t) * nbr_procs);
    for (i = 0; i < nbr_procs; i++) {
	SMI_INIT_LOCK(&(sigUsed[i]));
    }

#if SMI_PT2PT_SIGNALIZATION
    DNOTICE("creating interrupts");
    ALLOCATE(pt2pt_locint_desc, smi_sci_desc_t *, nbr_procs*sizeof(sci_desc_t));
    /* create the local interrupts */
    for (i = 0; i < nbr_procs; i++) {
	if (i != myid) {
	    _smi_get_int_scidesc(&pt2pt_locint_desc[i], &sci_error); 
	    int_desc = (sci_error == SCI_ERR_OK) ?
		_smi_trans_scidesc(&pt2pt_locint_desc[i]) : NULL;
	    rs_SCICreateInterrupt(int_desc, &sigRecvInterrupts[i], _smi_DefAdapterNbr, 
				  &sigRecvIntIds[i], NULL, NULL, 0, &sci_err);
	    EXIT_IF_FAIL("could not create interrupts", sci_err, -1);
	    DNOTICEI("created intrerrupt with id", sigRecvIntIds[i]);
	} else
	    sigRecvInterrupts[i] = 0;
    }

    DNOTICE ("exchanging the interrupt IDs");
    /* communicate the ids */
    for (k = 0; k < nbr_procs; k++) {
	/* distribute the ids that are needed by process k */
	gather_buffer = (k == myid) ? sigSendIntIds : dummy_gather;
	_smi_ll_allgather (&sigRecvIntIds[k], nbr_procs, gather_buffer, myid);
    }

    ALLOCATE(pt2pt_rmtint_desc, smi_sci_desc_t *, nbr_procs*sizeof(sci_desc_t));
    for (i = 0; i < nbr_procs; i++) {
	if (i != myid) {
	    DNOTICEI("connecting to interrupt of process", i);
	    DNOTICEI("         id of remote interrupt is", sigSendIntIds[i]);
	    /* this loop should not be necessary, but the current (?)
	     * SCI driver sometimes refuses to connect after a
	     * reboot - strange. As this loop causes no harm, we
	     * just did it this way. */
	    _smi_get_int_scidesc(&pt2pt_rmtint_desc[i], &sci_error);
	    EXIT_IF_FAIL("could not get SCI descriptor for interrupt", sci_error, -1);
	    int_desc = _smi_trans_scidesc(&pt2pt_rmtint_desc[i]);
	    for (itry = 0; itry < SMI_SIGNAL_TRY_NUM; itry++) {
		rs_SCIConnectInterrupt(int_desc, &sigSendInterrupts[i], _smi_sci_rank[i],
				       _smi_DefAdapterNbr, sigSendIntIds[i], SCI_INFINITE_TIMEOUT, 0, &sci_err);
		if (sci_err == SCI_ERR_OK)
		    break;
		DNOTICEP("SCIConnectInterrupt failed with SCI error", sci_err);
		usleep(SMI_SIGNAL_TRY_DELAY);
	    }
	    EXIT_IF_FAIL("could not connect interrupt", sci_err, -1);
	} else
	    sigSendInterrupts[i] = NULL;
    }
#endif	/* SMI_PT2PT_SIGNALIZATION */

#if SMI_GLOBAL_SIGNALIZATION
    SMI_INIT_LOCK(&sigAnyUsed);

    ALLOCATE(sigSendAnyInterrupts, sci_remote_interrupt_t *, sizeof(sci_remote_interrupt_t)*nbr_procs);
    ALLOCATE(sigSendAnyIntIds, unsigned int *, sizeof(int) * nbr_procs);
    
    _smi_get_int_scidesc(&global_locint_desc, &sci_err);
    int_desc = (sci_err == SCI_ERR_OK) ?
	_smi_trans_scidesc(&global_locint_desc) : NULL;
    rs_SCICreateInterrupt(int_desc, &sigRecvAnyInterrupt, _smi_DefAdapterNbr, &sigRecvAnyIntId,
			  NULL, NULL, 0, &sci_err);
    DNOTICEI("created local any-interrupt with id", sigRecvAnyIntId);
    
    /* distribute interrupt id's and connect to remote interrupts */
    _smi_ll_allgather ((int *)&sigRecvAnyIntId, 1, (int *)sigSendAnyIntIds, myid);

    ALLOCATE(global_rmtint_desc, smi_sci_desc_t *, nbr_procs*sizeof(sci_desc_t));
    for (i = 0; i < nbr_procs; i++) {
	DNOTICEI("connecting to interrupt of process", i);
	DNOTICEI("         id of remote interrupt is", sigSendAnyIntIds[i]);
	/* this loop should not be necessary, but the current
	 * SCI driver sometimes refuses to connect after a
	 * reboot - strange. As this loop causes no harm, we
	 * just did it this way. */
	_smi_get_int_scidesc(&global_rmtint_desc[i], &sci_err);
	int_desc = (sci_err == SCI_ERR_OK) ?
	    _smi_trans_scidesc(&global_rmtint_desc[i]) : NULL;
	for (itry = 0; itry < SMI_SIGNAL_TRY_NUM; itry++) {
	    rs_SCIConnectInterrupt(int_desc, &sigSendAnyInterrupts[i], _smi_sci_rank[i],
				   _smi_DefAdapterNbr, sigSendAnyIntIds[i], SCI_INFINITE_TIMEOUT, 0, &sci_err);
	    if (sci_err == SCI_ERR_OK)
		break;
	    DNOTICEP("SCIConnectInterrupt failed with SCI error", sci_err);
	    usleep(SMI_SIGNAL_TRY_DELAY);
	}
	EXIT_IF_FAIL("could not connect interrupt", sci_err, -1);
    }
#endif	/* SMI_GLOBAL_SIGNALIZATION */
    
    _smi_ll_barrier();
    sigInit = TRUE;

    free (dummy_gather);
    
    DSECTLEAVE;
    return (SMI_SUCCESS);
}


int _smi_signal_finalize()
{
    DSECTION("_smi_signal_finalize");
    int             i;
    sci_error_t     sci_err;

    DSECTENTRYPOINT;

    TEST_R(_smi_all_on_one == false, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);

    if (sigInit == TRUE) {
	for (i = 0; i < sigNrOfProcesses; i++) {
	    SMI_DESTROY_LOCK(&(sigUsed[i]));
	}
#if SMI_GLOBAL_SIGNALIZATION
	SMI_DESTROY_LOCK(&sigAnyUsed);
#endif

	/* disconnect from remote interrupts */
	DNOTICE("disconnecting interrupts");
	for (i = 0; i < sigNrOfProcesses; i++) {
#if SMI_PT2PT_SIGNALIZATION
	    if (sigSendInterrupts[i] != NULL) {
		rs_SCIDisconnectInterrupt(sigSendInterrupts[i], 0, &sci_err);
		WARN_IF_FAIL("could not disconnect interrupt", sci_err);
		_smi_free_int_scidesc (&pt2pt_rmtint_desc[i], &sci_err);
	    }
#endif
#if SMI_GLOBAL_SIGNALIZATION
	    if (sigSendAnyInterrupts[i] != NULL) {
		rs_SCIDisconnectInterrupt(sigSendAnyInterrupts[i], 0, &sci_err);
		WARN_IF_FAIL("could not disconnect interrupt", sci_err);
		_smi_free_int_scidesc (&global_rmtint_desc[i], &sci_err);
	    }
#endif
	}
	_smi_ll_barrier();
	
#if SMI_PT2PT_SIGNALIZATION
	/* remove local interrupts */
	DNOTICE("removing interrupts");
	for (i = 0; i < sigNrOfProcesses; i++)
	    if (sigRecvInterrupts[i] != NULL) {
		do {
		    rs_SCIRemoveInterrupt(sigRecvInterrupts[i], 0, &sci_err);
		} while (sci_err == SCI_ERR_BUSY);
		WARN_IF_FAIL("could not remove interrupt", sci_err);
		_smi_free_int_scidesc (&pt2pt_locint_desc[i], &sci_err);
	    }
#endif
	
#if SMI_GLOBAL_SIGNALIZATION
	do {
	    rs_SCIRemoveInterrupt(sigRecvAnyInterrupt, 0, &sci_err);
	} while (sci_err == SCI_ERR_BUSY);
	WARN_IF_FAIL("could not remove interrupt", sci_err);
	_smi_free_int_scidesc (&global_locint_desc, &sci_err);
#endif

	DNOTICE("freeing memory");
#if SMI_GLOBAL_SIGNALIZATION
	free(sigSendAnyInterrupts);
	free(sigSendAnyIntIds);
	free(global_rmtint_desc);
#endif
#if SMI_PT2PT_SIGNALIZATION
	free(pt2pt_rmtint_desc);
#endif
	free(sigSendInterrupts);
	free(sigSendIntIds);
	free(sigRecvInterrupts);
	free(sigRecvIntIds);
	free(sigUsed);
    }
    sigInit = FALSE;

    DSECTLEAVE;
    return 0;
}

smi_error_t SMI_Signal_send(int proc_rank)
{
    DSECTION("SMI_Signal_send");
    sci_error_t     sci_err;
    sci_remote_interrupt_t *pInterrupts;
    int             i;

    DSECTENTRYPOINT;

    TEST_R(_smi_all_on_one == false, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);
    ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);

    if (proc_rank & SMI_SIGNAL_ANY) {
#if SMI_GLOBAL_SIGNALIZATION
	DNOTICE("SMI_GLOBAL_SIGNALIZATION detected");
	proc_rank -= SMI_SIGNAL_ANY;
	pInterrupts = sigSendAnyInterrupts;
#else
	DPROBLEM("SMI_SIGNAL_ANY not yet implemented");
	DSECTLEAVE;
	return (SMI_ERR_NOTIMPL);
#endif	/* SMI_GLOBAL_SIGNALIZATION */
    } else {
#if SMI_PT2PT_SIGNALIZATION
	pInterrupts = sigSendInterrupts;
#else
	DPROBLEM("pt2pt signalization not yet activated");
	DSECTLEAVE;
	return (SMI_ERR_NOTIMPL);
#endif	/* SMI_PT2PT_SIGNALIZATION */
    }

    if (proc_rank == SMI_SIGNAL_BCAST) {
	for (i = 0; i < sigNrOfProcesses; i++)
	    if (i != sigMyId) {
		SCITriggerInterrupt(pInterrupts[i], 0, &sci_err);
		EXIT_IF_FAIL("could not trigger interrupt", sci_err, -1);
	    }
	DSECTLEAVE;
	return (SMI_SUCCESS);
    }
    if ((proc_rank < 0) || (proc_rank >= sigNrOfProcesses)) {
	DPROBLEMI("invalid process rank ", proc_rank);
	DSECTLEAVE;
	return (SMI_ERR_PARAM);
    }
    
    /* testing if signals can be sent to the process itself */
#if 0
    if (proc_rank == sigMyId) {
	DPROBLEM("can not send a signal to myself");
	DSECTLEAVE;
	return (SMI_ERR_PARAM);
    }
#endif

    SCITriggerInterrupt(pInterrupts[proc_rank], 0, &sci_err);
    EXIT_IF_FAIL("could not trigger interrupt", sci_err, -1);

    DSECTLEAVE;
    return (SMI_SUCCESS);
}

smi_error_t SMI_Signal_wait(int proc_rank)
{
    DSECTION("SMI_Signal_wait");
    sci_error_t     sci_err;

    DSECTENTRYPOINT;

    ASSERT_R(!_smi_all_on_one, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);
    ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);

    if (_smi_wd_shutdown_in_progress()) {
	DPROBLEM("avoiding signal wait during shutdown");
	DSECTLEAVE; return(SMI_ERR_OTHER);
    }

    if (proc_rank == SMI_SIGNAL_ANY) {
#if SMI_GLOBAL_SIGNALIZATION
	if (SMI_TRYLOCK(&sigAnyUsed) != 0)
	    return (SMI_ERR_PENDING);

	/* wait for interrupt and ignore timeouts */
	do {
	    SCIWaitForInterrupt(sigRecvAnyInterrupt, SCI_INFINITE_TIMEOUT, 0, &sci_err);
	} while (sci_err != SCI_ERR_OK /*  && sci_err != SCI_ERR_CANCELLED */);
	EXIT_IF_FAIL("could not wait for interrupt", sci_err, -1);

	SMI_UNLOCK(&sigAnyUsed);
	DSECTLEAVE;
	return (SMI_SUCCESS);
#else
	DWARNING("SMI_SIGNAL_ANY not yet implemented");
	DSECTLEAVE;
	return (SMI_ERR_NOTIMPL);
#endif	/* SMI_GLOBAL_SIGNALIZATION */
    }
#if !SMI_PT2PT_SIGNALIZATION
    DWARNING("pt2pt signalization not activated");
    DSECTLEAVE;
    return (SMI_ERR_NOTIMPL);
#else
    if ((proc_rank < 0) || (proc_rank >= sigNrOfProcesses)) {
	DPROBLEMI("invalid process rank ", proc_rank);
	return (SMI_ERR_PARAM);
    }
    if (proc_rank == sigMyId) {
	DPROBLEM("can not receive a signal from myself");
	return (SMI_ERR_PARAM);
    }
    /* check if this process already listens for this signal */
    if (SMI_TRYLOCK(&(sigUsed[proc_rank])) != 0) {
	DSECTLEAVE;
	return (SMI_ERR_PENDING);
    }
    SCIWaitForInterrupt(sigRecvInterrupts[proc_rank], SCI_INFINITE_TIMEOUT, 0, &sci_err);
    EXIT_IF_FAIL("could not wait for interrupt", sci_err, -1);

    SMI_UNLOCK(&(sigUsed[proc_rank]));

    DSECTLEAVE;
    return (SMI_SUCCESS);
#endif	/* SMI_PT2PT_SIGNALIZATION */
}

static void *_smi_sig_iwait_routine(void *pVoid)
{
    DSECTION("_smi_sig_iwait_routine");
    smi_signal_handle h = (smi_signal_handle) pVoid;

    DNOTICE("callback thread waits for signal");
    SMI_Signal_wait(h->iProcRank);
    DNOTICE ("callback thread received signal, executes callback");
    h->callback_fcn(h->callback_arg);
    
    DNOTICE("callback thread terminates");
    pthread_exit((void *)&thread_retval);
    return 0;
}

smi_error_t SMI_Signal_setCallBack(int proc_rank, void (*callback_fcn) (void *),
			       void *callback_arg, smi_signal_handle * h) 
{
    DSECTION("SMI_Signal_setCallBack");
#if defined WIN32 || defined DISABLE_THREADS
    return (SMI_ERR_NOTIMPL);
#else
    pthread_attr_t  scope_system_attr;

    DSECTENTRYPOINT;

    TEST_R(!_smi_all_on_one, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);
    ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);

    *h = NULL;

#if !SMI_GLOBAL_SIGNALIZATION
    if (proc_rank == SMI_SIGNAL_ANY) {
	DWARNING("SMI_SIGNAL_ANY not activated");
	DSECTLEAVE;
	return (SMI_ERR_NOTIMPL);
    }
#endif

    if ((proc_rank < 0) || (proc_rank >= sigNrOfProcesses)&&(proc_rank != SMI_SIGNAL_ANY)) {
	DPROBLEMI("invalid process rank ", proc_rank);
	DSECTLEAVE;
	return (SMI_ERR_PARAM);
    }
    if (proc_rank == sigMyId) {
	DPROBLEM("can not send a signal to myself");
	DSECTLEAVE;
	return (SMI_ERR_PARAM);
    }
    if (SMI_TRYLOCK(&(sigUsed[proc_rank])) != 0) {
	DSECTLEAVE;
	return (SMI_ERR_PENDING);
    }
    DNOTICE ("Allocating signal handle");
    ALLOCATE (*h, smi_signal_handle, sizeof(smi_signal_handle_t));

    (*h)->callback_fcn = callback_fcn;
    (*h)->callback_arg = callback_arg;
    (*h)->iProcRank = proc_rank;

    DNOTICE("creating callback thread");
    pthread_attr_init(&scope_system_attr);
    pthread_attr_setscope(&scope_system_attr, PTHREAD_SCOPE_SYSTEM);
    if (pthread_create(&((*h)->ptThread), &scope_system_attr, _smi_sig_iwait_routine, (void *) (*h)) != 0) {
	DERROR("pthread_create failed");
	SMI_Abort(-1);
    }
    DSECTLEAVE;
    return (SMI_SUCCESS);
#endif
}

smi_error_t SMI_Signal_joinCallBack(smi_signal_handle * h)
{
    DSECTION("SMI_Signal_joinCallBack");
#ifdef WIN32
    return (SMI_ERR_NOTIMPL);
#endif

    TEST_R(!_smi_all_on_one, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);
    ASSERT_R(sigInit, "usage of signals without initialization", SMI_ERR_NOINIT);

    if (*h == NULL) {
	DPROBLEMP("invalid handle ", *h);
	return (SMI_ERR_PARAM);
    }
#ifndef DISABLE_THREADS
    DNOTICE ("waiting for callback thread to terminate");
    pthread_join((*h)->ptThread, NULL);
#endif
    SMI_UNLOCK(&(sigUsed[(*h)->iProcRank]));

    free(*h);
    *h = NULL;
    
    DNOTICE ("callback is finished");
    return (SMI_SUCCESS);
}
#endif /* NO_SISCI */
