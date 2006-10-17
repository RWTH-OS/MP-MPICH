/* $Id$ */

#include "env/smidebug.h"
#include "watchdog.h"
#include "env/general_definitions.h"
#include "env/error_count.h"
#include "startup/startup.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/* Internal functions */
void _smi_SIGINT_Handler(int sig);
void _smi_SIGERROR_Handler(int sig);
void* _smi_wd_watchdog_routine(void* pVoid);
void* _smi_wd_watchdog_cb_routine(void* pVoid);
int _smi_wd_all_alive(void);
int _smi_wd_shutdown_in_progress(void);
void _smi_wd_shutdown_process(pthread_t wd_master_id, int signal);


static int wd_numprocs;
static int wd_myid;
static int wd_callback_used = FALSE;

/* the memory area in the base segment to exchange keep-alive signals */
static volatile int* wd_sgmt = NULL; 

/* watchdog keep-alive counters */
static int* wd_rmt_cntrs;
static int* wd_rmt_timeouts;
static int wd_local_cntr;

#ifndef WIN32
static struct sigaction sOldSIGSEGV;
static struct sigaction sOldSIGBUS;
static struct sigaction sOldSIGFPE;
static struct sigaction sOldSIGTERM;
static struct sigaction sOldSIGQUIT;
static struct sigaction sOldSIGINT;
#endif

#if defined (DISABLE_THREADS) && defined (USE_THREAD_FOR_WATCHDOG)
#undef USE_THREAD_FOR_WATCHDOG
#endif

#if USE_THREAD_FOR_WATCHDOG
static pthread_t wd_thread_id;
static pthread_t wd_MainThread;
#else
static struct itimerval wd_orig_alrmval;
#endif

/* states of the watchdog/signal handlers */
volatile int wd_do_finalize = FALSE;
static int wd_initialized = FALSE;
static int sig_initialized = FALSE;
static int wd_do_alivecheck = FALSE;
static int wd_timeout = WD_TICKS_TIMEOUT_DEF;
static int wd_bad_cnct_cntr = 0;

/* reasons for watchtdog/signal handler activity */
static int wd_sigerror_arrived = FALSE;
static int wd_sigint_arrived = FALSE;
static int wd_internal_abort = FALSE;
static int wd_defunct_detected = FALSE;

/* a flag that indicates an internal shutdown request */
static int wd_shutdown_request = 0;

/* id of the master process */
pthread_t wd_master_id;
static pthread_attr_t scope_system_attr;
  
/* user-definable callback-function */
static void (*wd_callback)( void ) = NULL;
static int wd_callback_called = 0;

/*
 * exported functions 
 */

int _smi_wd_shutdown_in_progress()
{
    return(wd_do_finalize);
}

void _smi_wd_request_shutdown(int errorcode)
{
    wd_shutdown_request = errorcode; 
}

smi_error_t SMI_Watchdog(int wd_timout_secs)
{
    REMDSECTION("SMI_Watchdog");
    DSECTENTRYPOINT;

    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);

#if USE_WATCHDOG
    if (wd_timout_secs > 0)
	/* transform into watchdog ticks */
	_smi_wd_enable(wd_timout_secs*1000000/WD_TICK_TIMESPAN);
    if (wd_timout_secs == SMI_WATCHDOG_DISABLE)
	_smi_wd_disable();
    if (wd_timout_secs == SMI_WATCHDOG_OFF)
	_smi_finalize_watchdog();

    /* XXX hack (for SMI_Signal_wait() */
    wd_do_finalize = FALSE;

    DSECTLEAVE; return SMI_SUCCESS;
#else
    DWARNING ("SMI was compiled without watchdog.");
    DSECTLEAVE; return SMI_ERR_NOTIMPL;
#endif
}

smi_error_t SMI_Watchdog_callback( void (*cb_fcn)(void) )
{
    int watchdog_was_active = 0;
    REMDSECTION("SMI_Watchdog_callback");
    DSECTENTRYPOINT;

    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);

#if USE_WATCHDOG
    if (wd_do_alivecheck) {
	watchdog_was_active = 1;
	_smi_wd_disable();
    }
    wd_callback = cb_fcn;
    if (watchdog_was_active)
	_smi_wd_enable (wd_timeout);

    DSECTLEAVE;
    return SMI_SUCCESS;
#else
    DWARNING ("SMI was compiled without watchdog.");
    DSECTLEAVE; return SMI_ERR_NOTIMPL;
#endif
}

void SMI_Abort(int a)
{
  DSECTION("SMI_Abort");
  
  DNOTICE("shutting down");

#if USE_WATCHDOG
  _smi_wd_disable();

  if (wd_initialized && !wd_sigint_arrived && !wd_internal_abort &&
      !wd_defunct_detected && !wd_sigerror_arrived) { 
      /* this process has been internally aborted by a direct call to SMI_Abort */
      if (wd_sgmt)
	  wd_sgmt[wd_numprocs+wd_myid] = WD_ABORTED;
      if (_smi_my_proc_rank == 0)
	  fprintf (stderr, "\n*** Application aborted internally by process 0.\n");
  }

  if (_smi_my_proc_rank == 0) {
      if (wd_sigint_arrived)
	  fprintf (stderr, "\n*** Application aborted by user.\n");

      if (wd_defunct_detected)
	  fprintf (stderr, "\n*** Application aborted by watchdog.\n");

      if (wd_sigerror_arrived)
	  fprintf (stderr, "\n*** Application aborted by fatal error \n"
		   "    (SEGV, general protection fault, ...).\n");
  }

#if 0
  if ((wd_callback != NULL) && !wd_defunct_detected && !wd_sigerror_arrived) {
      wd_callback();
      _smi_ll_barrier();   
  }
#else
  if ((wd_callback != NULL) && !wd_callback_called) {
      wd_callback_called = 1;
      wd_callback();
  }
#endif

  _smi_finalize_watchdog();
#endif

  _smi_clear_all_resources();
#if 0 
  /* not necessary */
  _smi_wd_shutdown_process (wd_master_id, SIGKILL);
  /* _smi_wd_shutdown_process (wd_master_id, SIGTERM); */
#endif

  exit(a);
}

/*
 * internal functions 
 */

int _smi_wd_set_timeout(int timeout)
{
  REMDSECTION("_smi_wd_set_timeout");
  DSECTENTRYPOINT;

  ASSERT_R(timeout > 0,"Illegal value for timeout",SMI_ERR_PARAM);
  wd_timeout = timeout;
  
  DSECTLEAVE;
  return(SMI_SUCCESS);
}

void _smi_wd_set_rank(int new_wd_rank)
{
    int my_counter;

    if (wd_initialized == TRUE) {
	if (wd_sgmt) my_counter = wd_sgmt[wd_myid];
	
	wd_myid = new_wd_rank;
	if (wd_sgmt) wd_sgmt[wd_myid] = my_counter + 1;
    }
}

void _smi_wd_disable()
{
  wd_do_alivecheck = FALSE;
}

void _smi_wd_enable(int timeout)
{
#if USE_WATCHDOG
  if (wd_initialized == FALSE) {
      /* we need to re-initialize the watchdog  (it has been turned off) */
      if (wd_sgmt) _smi_init_watchdog (wd_myid, wd_numprocs, wd_sgmt);
  }

  if (timeout > 0) {
      wd_timeout = timeout;
      wd_do_alivecheck = TRUE;
  }
#endif
}

#ifdef WIN32
/* This function creates a message describing an exception */
char *makeExceptionMessage(DWORD code,void *address,DWORD *info,BOOL *Critical) {
  static char m[255];
  
  switch(code) {
  case EXCEPTION_ACCESS_VIOLATION : 
	  sprintf(m,"Invalid %s of address 0x%x at 0x%p",(info[0]?"write":"read"),info[1],address); 
	  *Critical = TRUE;								
	  break;
  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED : 
	  sprintf(m,"Index out of bounds at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_BREAKPOINT :	
	  sprintf(m,"Breakpoint at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_DATATYPE_MISALIGNMENT : 
	  sprintf(m,"Datatype misalignement at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_FLT_DENORMAL_OPERAND : 
	  sprintf(m,"Floatingpoint denormal operand at 0x%p",address);
	  *Critical = FALSE;
	  break;
  case EXCEPTION_FLT_DIVIDE_BY_ZERO :  
	  sprintf(m,"Floatingpoint division by zero at 0x%p",address); 
	  *Critical = FALSE;
	  break;
  case EXCEPTION_FLT_INEXACT_RESULT : 
	  sprintf(m,"Inexact floatingpoint result at 0x%p",address); 
	  *Critical = FALSE;
	  break;
  case EXCEPTION_FLT_INVALID_OPERATION : 
	  sprintf(m,"Invalid floatingpoint operation at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_FLT_OVERFLOW : 
	  sprintf(m,"Floatingpoint overflow at 0x%p",address);  
	  *Critical = FALSE;
	  break;
  case EXCEPTION_FLT_STACK_CHECK : 
	  sprintf(m,"Floatingpoint stack over/underflow at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_FLT_UNDERFLOW : 
	  sprintf(m,"Floatingpoint underflow at 0x%p",address); 
	  *Critical = FALSE;
	  break;
  case EXCEPTION_ILLEGAL_INSTRUCTION : 
	  sprintf(m,"Illegal instruction at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_IN_PAGE_ERROR : 
	  sprintf(m,"Invalid page at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_INT_DIVIDE_BY_ZERO : 
	  sprintf(m,"Integer division by zero at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_INT_OVERFLOW : 
	  sprintf(m,"Integer overflow at 0x%p",address); 
	  *Critical = FALSE;
	  break;
  case EXCEPTION_INVALID_DISPOSITION : 
	  sprintf(m,"Invalid disposition at 0x%p (How did you do that ?) ",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_NONCONTINUABLE_EXCEPTION : 
	  sprintf(m,"noncontinuable exception at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_PRIV_INSTRUCTION : 
	  sprintf(m,"Privileged instruction at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  case EXCEPTION_SINGLE_STEP : 
	  sprintf(m,"Single step at 0x%p",address); 
	  *Critical = FALSE;
	  break;
  case EXCEPTION_STACK_OVERFLOW : 
	  sprintf(m,"Stack overflow at 0x%p",address); 
	  *Critical = TRUE;
	  break;
  default: sprintf(m,"Unknown exception 0x%x at 0x%p",code,address); 
	  *Critical = FALSE;
	  break;
  }
  
  return m;
}

/* This function actually handles  exceptions */
LONG _smi_MemoryExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo) {

  BOOL Critical;
  EXCEPTION_RECORD *ExRec=ExceptionInfo->ExceptionRecord;
  fprintf(stderr,"Unhandled Exception of Type %s\n",
	  makeExceptionMessage(ExRec->ExceptionCode,ExRec->ExceptionAddress,
			       ExRec->ExceptionInformation, &Critical));

  if (Critical) {
	_smi_SIGERROR_Handler(11);
	return EXCEPTION_CONTINUE_SEARCH;
  } 

  return EXCEPTION_CONTINUE_EXECUTION;
}


BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType ) {

	/* The console control handler routine
	   is called by a new thread. So we suspend the main
	   thread to avoid problems while finalizing the application*/
	/*SuspendThread(wd_MainThread);*/
/*SI: 01.02.2005 NO cleanup at Windows CTRL_LOGOFF_EVENT*/
	
	
#ifdef WIN32
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT: 
#endif
		_smi_SIGINT_Handler(2);
		return TRUE; /* disable other handlers*/
#ifdef WIN32
	};
#endif
	return FALSE; /* enable other handlers*/
	
}


void _smi_init_signal_handler() 
{
	DSECTION("_smi_init_signal_handler");
	BOOL res;
	if (sig_initialized)  return;
	
	sig_initialized = TRUE;
	/* Avoid messageboxes when critical errors occur */
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)_smi_MemoryExceptionFilter);
	SetConsoleCtrlHandler(ConsoleHandlerRoutine,TRUE);
	res=DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),
		            &wd_MainThread,0,FALSE,DUPLICATE_SAME_ACCESS);

	if(!res) {
		DERROR("Could not duplicate the main thread handle");
		SMI_Abort(-1);
	}

}

#else
void _smi_init_signal_handler() 
{
  DSECTION("_smi_init_signal_handler");
  
  struct sigaction sSIG;
  
  /* this makes valgrind happy, but is not really necessary */
  memset(&sSIG,0,sizeof(sSIG));
  memset(&sOldSIGINT,0,sizeof(sOldSIGINT));
  memset(&sOldSIGTERM,0,sizeof(sOldSIGTERM));
  memset(&sOldSIGQUIT,0,sizeof(sOldSIGQUIT));
  memset(&sOldSIGSEGV,0,sizeof(sOldSIGSEGV));

  if (sig_initialized == FALSE) {     
    /* asynchronous signals (created by external events) */
    sSIG.sa_handler = _smi_SIGINT_Handler;
    sSIG.sa_flags = 0;
    if (sigaction(SIGINT, &sSIG, &sOldSIGINT) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
    if (sigaction(SIGTERM, &sSIG, &sOldSIGTERM) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
    if (sigaction(SIGQUIT, &sSIG, &sOldSIGQUIT) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }

    /* synchronous signals (created by internal events) */
    sSIG.sa_handler = _smi_SIGERROR_Handler;
    sSIG.sa_flags = SA_RESETHAND;
    if (sigaction(SIGSEGV, &sSIG, &sOldSIGSEGV) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
    if (sigaction(SIGFPE, &sSIG, &sOldSIGFPE) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
#ifndef LINUX
    /* there is no SIGBUS under LINUX */
    if (sigaction(SIGBUS, &sSIG, &sOldSIGBUS) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
#endif
    
#if USE_THREAD_FOR_WATCHDOG
    /* signal for internal abort by watchdog thread */
    sSIG.sa_handler = SMI_Abort;
    sSIG.sa_flags = SA_RESETHAND;
    if (sigaction(WD_SHUTDOWN_SIGNAL, &sSIG, &sOldSIGSEGV) < 0 ) {
      DERROR("sigaction failed");
      SMI_Abort(-1);
    }
#endif
    
    sig_initialized = TRUE;
  }
}
#endif /* WIN32 */

void _smi_init_watchdog(int my_rank, int numprocs, int volatile *wd_addr) 
{
#if USE_WATCHDOG
#if !USE_THREAD_FOR_WATCHDOG
  struct sigaction sSIG;
  struct itimerval alrmval;
#endif
  int i,res;

  DSECTION("_smi_init_watchdog");
  DSECTENTRYPOINT;

  if (wd_initialized == FALSE) { 
    wd_myid = my_rank;
    wd_numprocs = numprocs;

    wd_sgmt = wd_addr;
    wd_rmt_cntrs = (int*) malloc(numprocs * sizeof(int));
    ASSERT_A(wd_rmt_cntrs, "no memory left for operation", SMI_ERR_NOMEM);
    wd_rmt_timeouts = (int*) malloc(numprocs * sizeof(int));
    ASSERT_A(wd_rmt_timeouts, "no memory left for operation", SMI_ERR_NOMEM);
    for(i = 0; i < numprocs; i++) {
      wd_rmt_cntrs[i] = 0;
      wd_rmt_timeouts[i] = 0;
      wd_addr[i]= 0;
      wd_addr[i+numprocs]= WD_RUNNING;
    }

    /* check signal handler */
    if (sig_initialized == FALSE) { 
	DWARNING ("SMI signal handlers not installed - watchdog can't be initialized");
	return;
    }
    wd_do_alivecheck = TRUE;
    
#if USE_THREAD_FOR_WATCHDOG
    DNOTICE ("initializing watchdog-thread");
    pthread_attr_init(&scope_system_attr);
    pthread_attr_setscope(&scope_system_attr, PTHREAD_SCOPE_SYSTEM);
    if ((res = pthread_create(&wd_thread_id, &scope_system_attr, _smi_wd_watchdog_routine, 
#ifndef WIN32
		       (void*) pthread_self()
#else
			   wd_MainThread
#endif
			   )) != 0) {
      DERRORI("pthread_create failed with ",res);
      SMI_Abort(-1);    
    }

#else /* USE_THREAD_FOR_WATCHDOG */
    DNOTICE ("initializing signal-based watchdog");
    /* realize the watchdog by timer-signal combination */
    sSIG.sa_handler = (void(*)())_smi_wd_watchdog_routine;
    sSIG.sa_flags = 0;
    if (sigaction(SIGVTALRM, &sSIG, NULL) < 0) {
      DERRORI("sigaction failed, errno =", errno);
      SMI_Abort(-1);
    }

    alrmval.it_interval.tv_sec = WD_TICK_TIMESPAN / 10000;
    alrmval.it_interval.tv_usec = WD_TICK_TIMESPAN % 10000;
    alrmval.it_value.tv_sec = WD_TICK_TIMESPAN / 10000;
    alrmval.it_value.tv_usec = WD_TICK_TIMESPAN % 10000;
    if (setitimer (ITIMER_VIRTUAL, &alrmval, &wd_orig_alrmval) < 0) {
	DERRORI("watchdog timer could not be initialized, errno =", errno);
	SMI_Abort(-1);
    }
#endif /* USE_THREAD_FOR_WATCHDOG */

    wd_initialized = TRUE;
  }
  DSECTLEAVE;
#endif /* USE_WATCHDOG */
}

#ifdef WD_TYPE_CALLBACK
#ifdef USE_THREAD_FOR_WATCHDOG
void _smi_init_watchdog_cb(int my_rank, int numprocs, int volatile *wd_addr, smi_sci_seg_t* wd_seg) 
{
    int i,res;
    
    DSECTION("_smi_init_watchdog_cb");
    DSECTENTRYPOINT;
    
    if (wd_initialized == FALSE) { 
	
	wd_myid = my_rank;
	wd_numprocs = numprocs;
	
	/* check signal handler */
	if (sig_initialized == FALSE) { 
	    DWARNING ("SMI signal handlers not installed - watchdog can't be initialized");
	    return;
	}
	wd_do_alivecheck = TRUE;
	wd_callback_used = TRUE;
	wd_thread_id = -1;
	
	DNOTICE ("initializing watchdog-thread");
#if USE_IMPLICIT_LOCALSEG_CALLBACK
	/* Process 0 is handled in scistartup */
	if (wd_myid != 0) {
#endif
#if USE_IMPLICIT_REMOTESEG_CALLBACK
	/* all processes except 0 are handled in scistartup */
	if (wd_myid == 0) {
#endif
	  DNOTICE("starting watchdog-thread");
	  pthread_attr_init(&scope_system_attr);
	  pthread_attr_setscope(&scope_system_attr, PTHREAD_SCOPE_SYSTEM);
	  if ((res = pthread_create(&wd_thread_id, &scope_system_attr,
				    _smi_wd_watchdog_cb_routine, wd_seg )) != 0) {
	    DERRORI("pthread_create failed with ",res);
	    SMI_Abort(-1);    
	  }
#if USE_IMPLICIT_REMOTESEG_CALLBACK
        }
#endif
#if USE_IMPLICIT_LOCALSEG_CALLBACK
	}
#endif	
	wd_initialized = TRUE;
    }
    DSECTLEAVE;
}
#endif
#endif

void _smi_finalize_watchdog()
{
#if USE_WATCHDOG
  DSECTION("_smi_finalize_watchdog");
#if 0
  double t0;
  int nbr_acks, proc;
#endif
  int rc;   

  DSECTENTRYPOINT;
  
  if (wd_initialized == TRUE) {
      /* indicate that we are already finalizing */  
      wd_do_alivecheck = FALSE;
      wd_do_finalize = TRUE;

      /* sending signal for own process to avoid deadlocks */
      SMI_Signal_send(_smi_my_proc_rank|SMI_SIGNAL_ANY);
      /* if P2PT is enabled this might be usable */
      /* SMI_Signal_send(_smi_my_proc_rank); */

    
#if USE_THREAD_FOR_WATCHDOG
      if (wd_callback_used == FALSE) {
	  DNOTICE ("Waiting for watchdog to shut down...");
#if 0
	  /* There are occasions in which the thread can not be joined - some obscure deadlock
	     under Winodws. */
	  pthread_join(wd_thread_id, NULL);
#else 
	  /* Instead, we terminate it the hard way. */
	  pthread_cancel(wd_thread_id);
	  pthread_join(wd_thread_id, NULL);
#endif
#ifndef WIN32
	  /* XXX need to add this to nt2unix */
	  pthread_attr_destroy(&scope_system_attr);
#endif
	  DNOTICE ("...watchdog has shut down.");
      }
#else /* USE_THREAD_FOR_WATCHDOG */
      {
	  struct sigaction sSIG;
	  struct itimerval alrmval;

	  alrmval.it_interval.tv_sec = 0;
	  alrmval.it_interval.tv_usec = 0;
	  if (setitimer (ITIMER_VIRTUAL, &wd_orig_alrmval, NULL) < 0) {
	      DPROBLEMI("watchdog timer could not be deactivated, errno =", errno);
	  }
	  if (sigaction(SIGVTALRM, NULL, NULL) < 0) {
	      DPROBLEMI("sigaction failed, errno =", errno);
	  }
      }
#endif /* USE_THREAD_FOR_WATCHDOG */
 
      /* if using watchdog without callback */
      if (wd_callback_used == FALSE) {
#if 0	  
	  /* process 0 as the owner of the watchdog segment waits until all other
	     processes have ackknowledged the shutdown of the watchdog/application
	     or if the timer commands a unconditional shutdown. */
	  if (_smi_my_proc_rank == 0) {
	      if (wd_sgmt[wd_numprocs + wd_myid] == WD_RUNNING)
		  wd_sgmt[wd_numprocs + wd_myid] = WD_FINALIZING;
	      
	      t0 = SMI_Wtime();
	      do {
		  usleep(WD_TICK_TIMESPAN);
		  for (nbr_acks = 0, proc = 0; proc < wd_numprocs; proc++) {
		      if (wd_sgmt[wd_numprocs + proc] != WD_RUNNING)
			  nbr_acks++;
		  }
	      } while ((nbr_acks < wd_numprocs)
		       && (SMI_Wtime() - t0 < (WD_TICK_TIMESPAN*WD_TRIGGER_BADCNCT)/1e6));
	      if (nbr_acks == wd_numprocs) { 
		  DNOTICE ("watchdog finalized properly");
	      } else {
		  DNOTICE ("watchdog finalized unconditionally by timeout");
	      }
	      
	      /* now everyone can shutdown */
	      if (wd_sgmt)
		  for (proc = 0; proc < wd_numprocs; proc++) 
		      wd_sgmt[wd_numprocs + proc] = WD_SHUTDOWN;
	      usleep(2*WD_TICK_TIMESPAN);
	  } 
	  else {
	      t0 = SMI_Wtime();
	      
	      /* first, check connection */
	      do {
		  _smi_load_barrier();
		  if ((_smi_probe_connection_state(0) == SMI_SUCCESS)
		      && (wd_sgmt[wd_numprocs + wd_myid] == WD_RUNNING)) 
		      wd_sgmt[wd_numprocs + wd_myid] = WD_FINALIZING;
		  else
		      usleep(WD_TICK_TIMESPAN);
	      } while ((_smi_probe_connection_state(0) != SMI_SUCCESS)
		       &&  (SMI_Wtime() - t0 < (WD_TICK_TIMESPAN*WD_TRIGGER_BADCNCT)/1e6));
	      /* now, wait for acknowledge from process 0 */
	      nbr_acks = 0;
	      do {
		  _smi_load_barrier();
		  if ((_smi_probe_connection_state(0) == SMI_SUCCESS)
		      && (wd_sgmt[wd_numprocs + wd_myid] == WD_SHUTDOWN)) 
		      nbr_acks = 1;
		  else
		      usleep(WD_TICK_TIMESPAN);
	      } while ((nbr_acks == 0) && (_smi_probe_connection_state(0) == SMI_SUCCESS)
		       &&  (SMI_Wtime() - t0 < (WD_TICK_TIMESPAN*WD_TRIGGER_BADCNCT)/1e6));
	      if (nbr_acks == 1) { 
		  DNOTICE ("watchdog finalized properly");
	      } else {
		  DNOTICE ("watchdog finalized unconditionally by timeout");
	      }
	  }
#endif
      } else {
	  /* segment-callback watchdog */

#if USE_THREAD_FOR_WATCHDOG
	  if (wd_thread_id != (pthread_t)-1) {
	      do {
		  rc = pthread_cancel(wd_thread_id);
	      } while (rc == EINTR);
	      if (rc != 0) {
		  DPROBLEMI("canceling callback watchdog thread failed with errno", errno);
	      } else {
		  pthread_join(wd_thread_id, NULL);
		  DNOTICE("callback watchdog thread terminated.");
	      }
	  }
#endif
      }
      
      wd_initialized = FALSE;
  }
  
  DSECTLEAVE;
#endif /* USE_WATCHDOG */
}

void _smi_SIGINT_Handler(int sig)
{
#if 0
  /* calling fprintf() in a signal handler may lead to deadlocks */
  DSECTION("_smi_SIGINT_Handler");
  DNOTICE ("handling signal INT, TERM or QUIT");
#endif
  
 
  _smi_wd_disable();

#if 0
#ifndef WIN32
  /* reinstall old handlers to avoid race-conditions */
  sigaction(SIGQUIT, &sOldSIGQUIT, NULL);
  sigaction(SIGTERM, &sOldSIGTERM, NULL);
  sigaction(SIGINT, &sOldSIGINT, NULL);
#endif
#endif

  if (wd_initialized && !wd_defunct_detected && !wd_internal_abort && !wd_sigerror_arrived) { 
      /* this process has been received an external SIGINT */
      wd_sigint_arrived = TRUE;
      if (wd_sgmt)
	  wd_sgmt[wd_numprocs+wd_myid] = WD_SIGINT;
  }

 
  SMI_Abort(-1);
}

/* XXX we might need to distinguish the different FPE signal variants! */
void _smi_SIGERROR_Handler(int sig)
{
#if 0
  /* calling fprintf() in a signal handler may lead to deadlocks */
  DSECTION("_smi_SIGERROR_Handler");
  DNOTICE ("handling fatal signal");
#endif
  
#ifndef WIN32
  switch (sig) {
  case SIGSEGV:
      DERROR("SIGSEGV in local process - aborting.");      
      break;
  case SIGBUS:
      DERROR("SIGBUS in local process - aborting.");      
      break;
  case SIGFPE:
      DERROR("SIGFPE in local process - aborting.");      
      break;
  }
#endif

  if (wd_initialized && !wd_sigerror_arrived) {
      wd_sigerror_arrived = TRUE;
      
      /* inform the other processes of the problem */
      if (wd_sgmt)
	  wd_sgmt[wd_numprocs + wd_myid] = WD_SIGERROR;
#if 0
      if (_smi_my_proc_rank == 0)
	  /* calling fprintf() in a signal handler may lead to deadlocks */
	  fprintf (stderr, "\n*** Application aborted by fatal error \n"
		   "    (SEGV, general protection fault, ...).\n");
#endif
      _smi_finalize_watchdog();
      _smi_clear_all_resources();
#if 0
      /* we will shut down automatically */
      _smi_wd_shutdown_process (wd_master_id, SIGTERM);
#endif
  }

#ifndef WIN32
  switch (sig) {
      /* reinstall old handlers - they will take final care of this signal */
  case SIGSEGV:
      sigaction(SIGSEGV, &sOldSIGSEGV, NULL);
      break;
  case SIGBUS:
      sigaction(SIGBUS, &sOldSIGBUS, NULL);
      break;
  case SIGFPE:
      sigaction(SIGFPE, &sOldSIGFPE, NULL);
      break;
  }
#endif
}

/* Called if the watchdog has found a condition which indicates that
   the process should shut down.  This may be called from within a 
   thread (for thread based watchdog) or within a signal handler. */
void _smi_wd_shutdown_process(pthread_t wd_master_id, int signal) {
#if USE_THREAD_FOR_WATCHDOG
#ifdef WIN32
  SuspendThread(wd_master_id);
  wd_do_finalize = TRUE;
  wd_initialized = FALSE;
  _smi_SIGINT_Handler(signal);
#endif /* WIN32 */
  pthread_kill (wd_master_id, signal);
#else 
  /* watchdog is based on signals */
#ifndef WIN32
  /* reinstall old handlers to avoid race-conditions */
  sigaction(SIGQUIT, &sOldSIGQUIT, NULL);
  sigaction(SIGTERM, &sOldSIGTERM, NULL);
  sigaction(SIGINT, &sOldSIGINT, NULL);
  sigaction(SIGVTALRM, NULL, NULL);
#endif /* WIN32 */
#if 1
  SMI_Abort(-1);
#endif
#endif /* USE_THREAD_FOR_WATCHDOG */  
}


static int _smi_wd_watchdog_core(void) 
{
    DSECTION("_smi_wd_watchdog_core");
    int i, wd_alive;
    
    DSECTENTRYPOINT;

    if (wd_shutdown_request != 0) {
	DNOTICE("ABORT - local abort requested");
	wd_do_finalize = TRUE;
	wd_internal_abort = TRUE;
	_smi_wd_shutdown_process (wd_master_id, WD_SHUTDOWN_SIGNAL);
	DSECTLEAVE; return(0);
    }
    
    if (wd_do_alivecheck) {
	/* increment my heartbeat counter - remote SCI write */
	wd_sgmt[wd_myid] = ++wd_local_cntr;
	
	/* if i am master process or master process sigalled me */
	{
	    /* check the state of the other processes */
	    for (i = 0; i < wd_numprocs; i++) {
		if (wd_sgmt[wd_numprocs+i] == WD_ABORTED) {
		    if (i == wd_myid) {
			DNOTICE("ABORT - local abort function called");
		    } else {
			if (_smi_my_proc_rank == 0)
			    fprintf (stderr, "\n*** Application aborted internally by process %d.\n", i);
			DNOTICEI("ABORT - abort function called in process", i);
		    }
		    wd_do_finalize = TRUE;
		    wd_internal_abort = TRUE;
		    _smi_wd_shutdown_process (wd_master_id, WD_SHUTDOWN_SIGNAL);
		    DSECTLEAVE; return(0);
		}
		if (wd_sgmt[wd_numprocs+i] == WD_SIGINT) {
		    if (i == wd_myid) {
			DNOTICE("ABORT - SIGINT received");
		    } else {
			DNOTICEI("ABORT - SIGINT received by process", i);
			wd_sigint_arrived = TRUE;
			_smi_wd_shutdown_process (wd_master_id, WD_SHUTDOWN_SIGNAL);
		    }
		    wd_do_finalize = TRUE;
		    DSECTLEAVE; return (0);
		}
		if (wd_sgmt[wd_numprocs+i] == WD_SIGERROR) {
		    DPROBLEMI("ABORT - fatal error (SIGSEGV, SIGBUS or SIGFPE) in process", i);
		    wd_sigerror_arrived = TRUE;
		    
		    if (wd_do_finalize == FALSE) {
			/* only signal the master thread if necessary */
			wd_do_finalize = TRUE;
			_smi_wd_shutdown_process (wd_master_id, WD_SHUTDOWN_SIGNAL);
		    }
		    DSECTLEAVE; return (0);
		}
	    }
	    wd_alive = _smi_wd_all_alive();
	    if (wd_alive != ALL_ALIVE) {
		if (wd_alive >= 0) {
		    DERRORI("ABORT - detected defunct process", wd_alive);
		} else {
		    DERROR("ABORT - some process is defunct");
		}
		if (wd_do_finalize == FALSE) {
		    wd_sgmt[wd_numprocs + wd_myid] = WD_DEFUNCT;
		    wd_defunct_detected = TRUE;
		    wd_do_finalize = TRUE;
		    _smi_wd_shutdown_process (wd_master_id, WD_SHUTDOWN_SIGNAL);
		}
	    }
	}
    }
    
    DSECTLEAVE return(1);
}

/* This is the watchdog routine processed by the watchdog thread */
void* _smi_wd_watchdog_routine(void* pVoid)
{
    DSECTION("_smi_watchdog_routine");
    
    /* the id of the master thread */
    /* int i; */
#if USE_THREAD_FOR_WATCHDOG 
#ifndef WIN32
    sigset_t signals_to_ignore;
    int old_cancel_type;
    DSECTENTRYPOINT;
    
    wd_master_id = (pthread_t) pVoid;
    
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
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &old_cancel_type);
#else
    DSECTENTRYPOINT;
    
    wd_master_id = (pthread_t) pVoid;
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
#endif /* WIN32 */
    do {
	usleep(WD_TICK_TIMESPAN);
	/* Not really required for asynchronous-canceling, but doesn't do any harm, too. */
#ifndef WIN32
	pthread_testcancel();
#endif
	
#else /* USE_THREAD_FOR_WATCHDOG */
     DSECTENTRYPOINT;
	
     wd_master_id = (pthread_t) pVoid;
     if (!wd_do_finalize) {
#endif /* USE_THREAD_FOR_WATCHDOG */
	    
	    if (_smi_wd_watchdog_core() == 0) { 
		DSECTLEAVE; return(NULL);
	    }
     }    
#if USE_THREAD_FOR_WATCHDOG    
    while (wd_do_finalize == FALSE);
#endif
    
  DSECTLEAVE; return (NULL);
}


/* thread routine for callback based watchdog  */

#if WD_TYPE_CALLBACK
#if USE_THREAD_FOR_WATCHDOG
void* _smi_wd_watchdog_cb_routine(void *arg)
{
    DSECTION ("_smi_wd_watchdog_cb_routine");
    sci_segment_cb_reason_t sgmt_event;
    sci_error_t sgmt_status, sci_err;
	smi_sci_seg_t* pSegment;
    unsigned int src_node_id, adapt_nbr;
    int cb_arg = 0, cb_action;   
    sigset_t signals_to_ignore;
    int old_cancel_type;
    
    sigemptyset(&signals_to_ignore);
    sigaddset(&signals_to_ignore, SIGINT);
    sigaddset(&signals_to_ignore, SIGQUIT);
    sigaddset(&signals_to_ignore, SIGTERM);
    sigaddset(&signals_to_ignore, SIGSEGV);
    sigaddset(&signals_to_ignore, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &signals_to_ignore, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &old_cancel_type);

    pSegment = (smi_sci_seg_t*) arg;
    /* DNOTICEP("segment",pSegment->segment.local); */

    while (wd_do_finalize == FALSE) {
	DNOTICE ("waiting for segment event for base segment");
	if (pSegment->type == localseg) {
	    sgmt_event = SCIWaitForLocalSegmentEvent (*(pSegment->pSegment.local),&src_node_id, &adapt_nbr,
						      SCI_INFINITE_TIMEOUT, 0,
						      &sci_err);
	    /* XXX whe do not know why this occures */
            if (sci_err == SCI_ERR_OVERFLOW) {
                usleep(1000000);
	    }
        } else {
	    sgmt_event = SCIWaitForRemoteSegmentEvent (*(pSegment->pSegment.remote), &sgmt_status,
						       SCI_INFINITE_TIMEOUT, 0, &sci_err);
	    /* XXX again, this seems to be a SISCI bug: in certain topologies,
	     * 	(4 procs on 2 nodes), this event/error combination keeps occuring.
	     * 	This is only a dirty workaround until this is fixed! */
	    if (sci_err == SCI_ERR_OVERFLOW) {
		    usleep(1000000);
	    }
	} 
	
	DNOTICE ("Callback event occured for base-segment");
	DNOTICEP("  type", sgmt_event);
	DNOTICEP("   err", sci_err);

	if (sgmt_event == SCI_CB_DISCONNECT || sgmt_event == SCI_CB_LOST) {
	    _smi_ll_sgmt_ok = FALSE;

	    if (!_smi_wd_shutdown_in_progress()) {
		DPROBLEM("Lost connection to base segment");
		_smi_wd_shutdown_process (pSegment->mainthread, SIGTERM);
	    }
	}
    }
    
    return (NULL);
}
#else /* USE_THREAD_FOR_WATCHDOG */
#error cannot use watchdog callback without threads enabled
#endif /* USE_THREAD_FOR_WATCHDOG */
#endif /* WD_TYPE_CALLBACK */

int _smi_wd_all_alive(void)
{
  int proc, rmt_cntr;
  REMDSECTION("_smi_wd_all_alive");
  DSECTENTRYPOINT;

  /* XXX paranoid - should never happen. Debug. */
  if (wd_timeout <= 0) {
    DERRORI("illegal watchdog timeout", wd_timeout);
    return DEFUNCT_DETECTED;
  }

  /* as long as the SCI connection towards proc 0 is bad, we can not
     do anything else then returning TRUE. If not, we might return FALSE
     although all processes are running fine and there are only problems
     with the SCI connections. However, we introduce a limit here, too. */
  if (_smi_probe_connection_state(0) != SMI_SUCCESS) {
      DNOTICE ("bad SCI connection towards proc 0, can not set/read watchdog");
      DSECTLEAVE;  
      return ((++wd_bad_cnct_cntr/wd_timeout >= WD_TRIGGER_BADCNCT) ? 
	      DEFUNCT_DETECTED : ALL_ALIVE);
  }
  wd_bad_cnct_cntr = 0;
  
  for (proc = 0; proc < wd_numprocs; proc++) { 
      _smi_load_barrier();
      /* remote SCI read */
      rmt_cntr = wd_sgmt[proc];

      if (rmt_cntr != wd_rmt_cntrs[proc]) {
	  /* the remote process incremented its counter and thus seems to be alive */
	  wd_rmt_cntrs[proc] = rmt_cntr;
	  wd_rmt_timeouts[proc] = 0;
      } else {
	  wd_rmt_timeouts[proc]++;
	  if (wd_rmt_timeouts[proc] > wd_timeout) {
	    /* Test if the connection towards this proc is up before declaring 
	       this process as defunct. However, a bad connection between the
	       local proc and proc 'proc' does not mean that the connection 
	       between 'proc' and 0 is bad, too -> potential problem! */
	    if (_smi_probe_connection_state(proc) == SMI_SUCCESS) {
		DNOTICEI ("connection to 0 is o.k., detected defunct process", proc);
		DSECTLEAVE;  return (proc);
	    }
	    /* even if the connection is disturbed, we need to abort the
	       application after a certain period */
	    if (wd_rmt_timeouts[proc]/wd_timeout >= WD_TRIGGER_BADCNCT) {
		DNOTICEI ("connection to 0 is bad (-> timeout), detected defunct process", proc);
		DSECTLEAVE;  return (proc);
	    }
	  }
      }
      /* has anyone else detected a defunct process ? */
      if (wd_sgmt[wd_numprocs+proc] == WD_DEFUNCT) {
	  DNOTICEI ("defunct process detected by proc", proc);
	  DSECTLEAVE; return (DEFUNCT_DETECTED);
      }
  }
  
  DSECTLEAVE; return (ALL_ALIVE);
}

