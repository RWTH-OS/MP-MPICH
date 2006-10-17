/* $Id$ */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdio.h>
#include <signal.h>

#include "mpid.h"
#include "shmemdev.h"
#include "shmemdebug.h"
#include "shmemdef.h"
#include "shmemsignals.h"
#include "p2p.h"
#include "shmemsysv.h"
#include "shmemcommon.h"

/* *********************************
   Forward declarations of functions
***********************************/

MPID_SHMEM_shmem_header_t **xx_init_shmalloc ANSI_ARGS(( char *, unsigned ));
MPID_SHMEM_sem_admin_t *MPID_SHMEM_init_semaphore_admin ( void );

/* *************************************
   global variables not in shared memory
***************************************/

MPID_SHMEM_shmem_header_t **MPID_SHMEM_shared_memory; /* pointer to pointer to start of free list
					   *MPID_SHMEM_shared_memory = NULL: shared memory is entirely
					   used */


/* ----------------------------------- */

#if 0
/* This is for handling process groups. Currently disabled, has to
   be revisited later.*/
static int MPID_SHMEM_ppid = 0;

/* This is a process group, used to help clean things up when a process dies 
   It turns out that this causes strange failures when running a program
   under another program, like a debugger or mpirun.  Until this is
   resolved, I'm ifdef'ing this out.
 */
void
p2p_setpgrp()
{
   MPID_SHMEM_ppid = getpid();
   if(setpgid((pid_t)MPID_SHMEM_ppid,(pid_t)MPID_SHMEM_ppid)) {
       perror("failure in p2p_setpgrp");
       exit(-1);
   }
}
#endif

/* ----------------------------------- */

void p2p_init( memsize )
int memsize;
{
    caddr_t p2p_start_shared_area;

    /* Save process ID of this (the master) process */
    MPID_SHMEM_sysv_mypid = getpid();

    /* Get a set of semaphores and initialize them */
    MPID_SHMEM_sysv_semid = MPID_SHMEM_sysv_init_semset( MPID_SHMEM_SYSV_MAX_SEMS, MPID_SHMEM_sysv_mypid );

    /* Get shared memory and attach to it */
    p2p_start_shared_area = MPID_SHMEM_sysv_create_shmem( memsize );
    if( p2p_start_shared_area == (caddr_t)-1 )
      p2p_syserror("OOPS : cannot get shared memory, size=", memsize);
    
    /* Before we initialize shmalloc, we need to initialize any lock 
       information.  Some locks may use some of the shared memory */
    MPID_SHMEM_shared_memory = xx_init_shmalloc(p2p_start_shared_area,memsize);
    
    MPID_SHMEM_semaphore_admin = MPID_SHMEM_init_semaphore_admin();
}

#if defined(HAVE_SIGPROCMASK)
static sigset_t mpir_oldset;
#define SIGNAL_BLOCK(sig) {\
    sigset_t newset;\
    sigemptyset(&newset);\
    sigaddset(&newset,sig);\
    sigprocmask( SIG_BLOCK, &newset, &mpir_oldset );}

#define SIGNAL_UNBLOCK() {\
    sigprocmask( SIG_SETMASK, &mpir_oldset, (sigset_t *)0 );}
#elif defined(HAVE_SIGMASK)
static int _oldset;
#define SIGNAL_BLOCK(sig) {int _mask = sigmask(sig);\
	       _oldset = sigblock(_mask);}
#define SIGNAL_UNBLOCK() sigblock(_oldset);

#endif

/* End of signal handler definitions */

/*
 * Establish a handler for termination signals from the child.
 * Process ids are stored in an array and this array is used to wait on
 * terminating processes.  If a SIGCHLD/SIGCLD is received, the handler
 * does a wait and removes the child from the list.  If the signal is 
 * unexpected (the "join"/finalize routine hasn't been called), then 
 * initiate a termination of the job.
 */



/* Set SIGCHLD handler */
static int MPID_child_status = 0;
#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif
/* Define standard signals if SysV version is loaded */
#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD SIGCLD
#endif

/* 
   We add useful handlers that will allow us to catch MOST BUT NOT ALL
   problems (jobs killed with uncatchable signals may still run away) 
 */
SIGNAL_HAND_DECL(MPID_handle_abort)
{
  /* Really need to block further signals until done ... */
  p2p_clear_signal();
  p2p_kill_procs();
}

SIGNAL_HAND_DECL(MPID_dump_internals)
{
  extern void MPID_SHMEM_Print_internals ANSI_ARGS(( FILE * ));
  fprintf( stderr, "[%d] Got Signal to exit .. \n", MPID_SHMEM_rank);
  MPID_SHMEM_Print_internals( stderr );
  
  /* Really need to block further signals until done ... */
  p2p_clear_signal();
  /* MPID_SHMEM_sysv_shmem_cleanup(); is done by p2p_kill_procs() */
  p2p_kill_procs();

  exit ((int) 1);
}

SIGNAL_HAND_DECL(MPID_handle_exit)
{
  fprintf( stderr, "[%d] Got Signal to exit .. \n", MPID_SHMEM_rank);

  /* Really need to block further signals until done ... */
  p2p_clear_signal();
  /* MPID_SHMEM_sysv_shmem_cleanup(); is done by p2p_kill_procs() */
  p2p_kill_procs();

  exit ((int) 1);
}

/*
 * A child failure is ALWAYS fatal.  When in shutdown mode, this signal
 * is cleared.
 */
SIGNAL_HAND_DECL(MPID_handle_child)
{
  int prog_stat, pid;
  int i;

  /* Really need to block further signals until done ... */
  /* fprintf( stderr, "Got SIGCHLD...\n" ); */
  pid	   = waitpid( (pid_t)(-1), &prog_stat, WNOHANG );
  if (pid && 
      (WIFEXITED(prog_stat) || WIFSIGNALED(prog_stat))) {
#ifdef MPID_DEBUG_ALL
    if (MPID_DebugFlag) printf("Got signal for child %d (exited)... \n", pid );
#endif
    /* The child has stopped. Remove it from the jobs array */
    for (i = 0; i<MPID_MAX_PROCS; i++) {
      if (MPID_SHMEM_sysv_pid[i] == pid) {
	MPID_SHMEM_sysv_pid[i] = 0;
#ifdef DYNAMIC_CHILDREM
	if (WIFEXITED(prog_stat)) {
	  MPID_child_status |= WEXITSTATUS(prog_stat);
	}
	else 
#endif
	if (WIFSIGNALED(prog_stat)) {
	  /* If we're not exiting, cause an abort. */
	    p2p_error( "Child process died unexpectedly from signal", 
		       WTERMSIG(prog_stat) );
	}
	else
	  p2p_error( "Child process exited unexpectedly", i );
	break;
      }
    }
    /* Child may already have been deleted by Leaf exit; we should
       use conn->state to record it rather than zeroing it */
  }
/* Re-enable signals if necessary */
SIGNAL_HAND_CLEANUP(SIGCHLD,MPID_handle_child);
}

void p2p_clear_signal()
{
  SIGNAL_HAND_SET( SIGCHLD, SIG_IGN );
#if defined(MPID_SETUP_SIGNALS)
  SIGNAL_HAND_SET( SIGHUP,  SIG_DFL );
  SIGNAL_HAND_SET( SIGINT,  SIG_DFL );
  SIGNAL_HAND_SET( SIGQUIT, SIG_DFL );
  SIGNAL_HAND_SET( SIGILL,  SIG_DFL );
  SIGNAL_HAND_SET( SIGTRAP, SIG_DFL );
  SIGNAL_HAND_SET( SIGABRT, SIG_DFL );
  SIGNAL_HAND_SET( SIGEMT,  SIG_DFL );
  SIGNAL_HAND_SET( SIGFPE,  SIG_DFL );
  SIGNAL_HAND_SET( SIGBUS,  SIG_DFL );
  SIGNAL_HAND_SET( SIGSEGV, SIG_DFL );
  SIGNAL_HAND_SET( SIGSYS,  SIG_DFL );
  SIGNAL_HAND_SET( SIGPIPE, SIG_DFL );
  SIGNAL_HAND_SET( SIGALRM, SIG_DFL );
  SIGNAL_HAND_SET( SIGTERM, SIG_DFL );
  SIGNAL_HAND_SET( SIGXCPU, SIG_DFL );
  SIGNAL_HAND_SET( SIGXFSZ, SIG_DFL );

#ifdef SIGDEAD
  SIGNAL_HAND_SET( SIGDEAD, SIG_DFL );
#endif

#ifdef SIGXMEM
  SIGNAL_HAND_SET( SIGXMEM, SIG_DFL );
#endif

#ifdef SIGXDSZ
  SIGNAL_HAND_SET( SIGXDSZ, SIG_DFL );
#endif

#ifdef SIGMEM32
  SIGNAL_HAND_SET( SIGMEM32, SIG_DFL );
#endif

#ifdef SIGNMEM
  SIGNAL_HAND_SET( SIGNMEM, SIG_DFL );
#endif

#ifdef SIGXXMU
  SIGNAL_HAND_SET( SIGXXMU, SIG_DFL );
#endif

#ifdef SIGXRLG0
  SIGNAL_HAND_SET( SIGXRLG0, SIG_DFL );
#endif

#ifdef SIGXRLG1
  SIGNAL_HAND_SET( SIGXRLG1, SIG_DFL );
#endif

#ifdef SIGXRLG2
  SIGNAL_HAND_SET( SIGXRLG2, SIG_DFL );
#endif

#ifdef SIGXRLG3
  SIGNAL_HAND_SET( SIGXRLG3, SIG_DFL );
#endif

#ifdef SIGXMERR
  SIGNAL_HAND_SET( SIGXMERR, SIG_DFL );
#endif

#endif
}

void p2p_create_procs(argc, argv )
int argc;
char **argv;
{
    int i, rc;
     
    /* set signal handler */
#if defined(MPID_DEBUG_SPECIAL)
    SIGNAL_HAND_SET( SIGINT,  MPID_dump_internals );
#endif
#if defined(MPID_SETUP_SIGNALS)
    SIGNAL_HAND_SET( SIGCHLD, MPID_handle_child );

    SIGNAL_HAND_SET( SIGABRT, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGINT,  MPID_handle_exit );
    SIGNAL_HAND_SET( SIGHUP,  MPID_handle_exit );
    SIGNAL_HAND_SET( SIGINT,  MPID_handle_exit );
    SIGNAL_HAND_SET( SIGQUIT, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGILL,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGTRAP, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGABRT, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGEMT,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGFPE,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGBUS,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGBUS,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGSEGV, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGSYS,  MPID_handle_abort );
    SIGNAL_HAND_SET( SIGPIPE, MPID_handle_exit );
    SIGNAL_HAND_SET( SIGALRM, MPID_handle_exit );
    SIGNAL_HAND_SET( SIGTERM, MPID_handle_exit );
    SIGNAL_HAND_SET( SIGXCPU, MPID_handle_abort );
    SIGNAL_HAND_SET( SIGXFSZ, MPID_handle_abort );
  
#ifdef SIGDEAD
    SIGNAL_HAND_SET( SIGDEAD, MPID_handle_abort );
#endif

#ifdef SIGXMEM
    SIGNAL_HAND_SET( SIGXMEM, MPID_handle_abort );
#endif

#ifdef SIGXDSZ
    SIGNAL_HAND_SET( SIGXDSZ, MPID_handle_abort );
#endif

#ifdef SIGMEM32
    SIGNAL_HAND_SET( SIGMEM32, MPID_handle_abort );
#endif

#ifdef SIGNMEM
    SIGNAL_HAND_SET( SIGNMEM, MPID_handle_abort );
#endif

#ifdef SIGXXMU
    SIGNAL_HAND_SET( SIGXXMU, MPID_handle_abort );
#endif

#ifdef SIGXRLG0
    SIGNAL_HAND_SET( SIGXRLG0, MPID_handle_abort );
#endif

#ifdef SIGXRLG1
    SIGNAL_HAND_SET( SIGXRLG1, MPID_handle_abort );
#endif

#ifdef SIGXRLG2
    SIGNAL_HAND_SET( SIGXRLG2, MPID_handle_abort );
#endif

#ifdef SIGXRLG3
    SIGNAL_HAND_SET( SIGXRLG3, MPID_handle_abort );
#endif

#ifdef SIGXMERR
    SIGNAL_HAND_SET( SIGXMERR, MPID_handle_abort );
#endif

#endif /* Setup signal handlers */

    /* Make sure that the master process is process zero */
    MPID_SHMEM_sysv_lock( &MPID_shmem->globlock );
    MPID_SHMEM_rank = MPID_shmem->globid++;

    SIGNAL_HAND_SET( SIGCHLD, MPID_handle_child );

#if NOT_YET_TESTED
    /* Make sure the children don't receive the SIGTRAPs which are going to 
     * the master because he's being debugged...
     */
    SIGNAL_HAND_SET( SIGTRAP, SIG_IGN );
#endif
    SIGNAL_BLOCK(SIGCHLD);

    MPID_SHMEM_sysv_pid[0] = MPID_SHMEM_sysv_mypid;
    for( i = 1; i < MPID_SHMEM_size; i++ ) {
	/* Do this in the master to avoid race conditions */
	int nextId = MPID_shmem->globid++;
	
        /* Clear in case something happens ... */
        MPID_SHMEM_sysv_pid[i] = 0;
	rc = fork();
	if (rc == -1)
	    p2p_error("p2p_init: fork failed\n",(-1));
	else if (rc == 0) {
 	    MPID_SHMEM_rank = nextId;
	    SIGNAL_UNBLOCK();

	    return;
	  }
	else {
	  /* Save pid of child so that we can detect child exit */
	  MPID_SHMEM_sysv_pid[i] = rc;
	}
    }
    SIGNAL_UNBLOCK(); /* on SIGCHLD */
    /* This prevents any of the newly created processes decrementing
     * the global ID before everyone has started
     */
    MPID_SHMEM_sysv_unlock( &MPID_shmem->globlock );
}

/* We can use common code to handle stopping processes */
void p2p_kill_procs()
{
  if (MPID_SHMEM_rank == 0) {
    int i;
    /* We are no longer interested in signals from the children */
    SIGNAL_HAND_SET( SIGCHLD, SIG_IGN );

    /* for all child processes that were fork'd / client processes that
       the master process contacted to, the saved PID should be above 0 */
    i = 1;
    while( (i < MPID_MAX_PROCS) && (MPID_SHMEM_sysv_pid[i] > 0 ) ) {
	kill( MPID_SHMEM_sysv_pid[i], SIGINT );
	i++;
    }
  }

#ifdef FOO
    if (MPID_SHMEM_ppid) 
	kill( -MPID_SHMEM_ppid, SIGKILL );
#endif

}




/* This must be called AFTER MD_initmem but before anything else (like
   fork!)  Before MPI_initmem is called, we must initialize the MPID_SHMEM_sysv_semid
   (used for shmat allocation).
 */
MPID_SHMEM_sem_admin_t *MPID_SHMEM_init_semaphore_admin( void )
{
    MPID_SHMEM_sem_admin_t *sem_adm;

    /* Get shared memory.  Since this is called BEFORE any fork,
       we don't need to do any locks but we DO need to get the memory
       from a shared location 
     */
    sem_adm = (MPID_SHMEM_sem_admin_t *)MPID_SHMEM_sysv_malloc( sizeof(MPID_SHMEM_sem_admin_t) );
    if (!sem_adm) 
	p2p_error("Could not get sem_adm data\n", sizeof(MPID_SHMEM_sem_admin_t));

    sem_adm->slave_lock.semnum = 1;
    sem_adm->sysv_next_lock    = 2; /* shmem_lock is 0 & slave_lock is 1 */

    return sem_adm;
}


/* p2p_lock_init() initializes the given lock, i.e. it identifies it with a SYSV semaphore
   in setting the semaphore set ID and the semaphore number */
void p2p_lock_init( L )
p2p_lock_t *L;
{
    /* start of critical region, lock with lock in *MPID_SHMEM_semaphore_admin */
    MPID_SHMEM_sysv_lock( &(MPID_SHMEM_semaphore_admin->slave_lock) );

    L->semnum = MPID_SHMEM_semaphore_admin->sysv_next_lock;
    MPID_SHMEM_semaphore_admin->sysv_next_lock++;

    /* end of critical region */
    MPID_SHMEM_sysv_unlock(&(MPID_SHMEM_semaphore_admin->slave_lock));
}

/*
 * Generate an error message for operations that have errno values.
 */
void p2p_syserror( string, value )
char *string;
int  value;
{
    perror( "Error detected by system routine: " );
    p2p_error( string, value );
}


void p2p_error(string,value)
char * string;
int value;
{
    printf("%s %d\n",string, value);
    /* printf("p2p_error is not fully cleaning up at present\n"); */
    MPID_SHMEM_sysv_shmem_cleanup();

    /* Manually kill all processes */
    if (MPID_SHMEM_rank == 0) {
	int i;
	/* We are no longer interested in signals from the children */
	p2p_clear_signal();

	/* for all child processes that were fork'd / client processes that
	   the master process contacted to, the saved PID should be above 0 */
	i = 1;
	while( (i < MPID_MAX_PROCS) && (MPID_SHMEM_sysv_pid[i] > 0 ) ) {
		   kill( MPID_SHMEM_sysv_pid[i], SIGINT );
		   i++;
	}
    }
#ifdef FOO
    if (MPID_SHMEM_ppid) 
	kill( -MPID_SHMEM_ppid, SIGKILL );
#endif

    /* We need to do an abort to make sure that the children get killed */
    abort();
    /* exit(99); */
}

/* This is not machine dependent code but is only used on some machines */

/*
  Memory management routines from ANSI K&R C, modified to manage
  a single block of shared memory.
  Have stripped out all the usage monitoring to keep it simple.

  To initialize a piece of shared memory:
    xx_init_shmalloc(char *memory, unsigned nbytes)

  Then call MPID_SHMEM_sysv_malloc() and MPID_SHMEM_sysv_free() as usual.
*/

MPID_SHMEM_shmem_header_t **xx_init_shmalloc(memory, nbytes)
char *memory;
unsigned nbytes;
/*
  memory points to a region of shared memory nbytes long.
  initialize the data structures needed to manage this memory
*/
{
    MPID_SHMEM_shmem_header_t **shared_area;
    int nunits = nbytes >> MPID_SHMEM_HEADER_LOG_ALIGN;
    MPID_SHMEM_shmem_header_t *region = (MPID_SHMEM_shmem_header_t *) memory;

    /* Quick check that things are OK */

    if (MPID_SHMEM_HEADER_ALIGNMENT != sizeof(MPID_SHMEM_shmem_header_t) ||
	MPID_SHMEM_HEADER_ALIGNMENT < (sizeof(MPID_SHMEM_shmem_header_t *) + sizeof(p2p_lock_t)))
    {
        printf( "%d %d\n",sizeof(MPID_SHMEM_shmem_header_t),sizeof(p2p_lock_t));
	p2p_error("xx_init_shmem: Alignment is wrong", MPID_SHMEM_HEADER_ALIGNMENT);
    }

    if (!region)
	p2p_error("xx_init_shmem: Passed null pointer", 0);

    if (nunits < 2)
	p2p_error("xx_init_shmem: Initial region is ridiculously small",
		 (int) nbytes);

    shared_area = (MPID_SHMEM_shmem_header_t **) region;	/* Free space pointer in first block  */
    MPID_SHMEM_sysv_firstlock = (p2p_lock_t *) (shared_area + 1);/* Lock still in first block */

    (region + 1)->s.ptr = *shared_area = region + 1;	/* Data in rest */
    (region + 1)->s.size = nunits - 1;	/* One header consumed already */

    MPID_SHMEM_sysv_firstlock->semnum = 0;

    return shared_area;
}
