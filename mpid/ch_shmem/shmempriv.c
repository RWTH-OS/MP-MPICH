/* $Id$ */
/*
   This file contains routines that are private and unique to the ch_shmem
   implementation
 */

#include <sched.h>

#include "mpid.h"
#include "shmemdev.h"
#include "shmemdebug.h"
#include "shmemsysv.h"
#include "shmemcommon.h"

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;

/* LOCAL copy of some of MPID_shmem */
MPID_SHMEM_lglobmem MPID_lshmem;

int                 MPID_SHMEM_rank = -1;
int                 MPID_SHMEM_size = 0;
MPID_PKT_T          *MPID_local = 0;
VOLATILE MPID_PKT_T **MPID_incoming = 0;
static int	    MPID_pktflush;
static int          MPID_op = 0;
static int          MPID_readcnt = 0;
static int          MPID_freecnt = 0;
int MPID_SHMEM_is_master;
int MPID_SHMEM_do_fork;
int MPID_SHMEM_master_pid;

/* Forward declarations */ 
void	MPID_SHMEM_lbarrier  ANSI_ARGS((void));
void  MPID_SHMEM_FreeSetup ANSI_ARGS((void));
void MPID_SHMEM_FlushPkts ANSI_ARGS((void));

void MPID_SHMEM_init( argc, argv )
int  *argc;
char **argv;
{
    int i, separator = 5000;
    int cnt, j, pkts_per_proc;
    int memsize;
    
    /* default: a single process is started and then forks to other processes */
    MPID_SHMEM_is_master = 1;
    MPID_SHMEM_do_fork = 1;

    /* Make one process the default */
    MPID_SHMEM_size = 1;
    
    /* delete -- */
    for (i=1; i<*argc; i++) {
	if( strcmp( argv[i], "--" ) == 0 ) {
	    argv[i] = 0;
	    separator = i;
	}
    }

    /* Number of processes */
    for (i=1; i<*argc; i++) {
	if( argv[i] ) {
	    if( ( strcmp( argv[i], "-n" ) == 0 ) && ( i < separator ) ) {
		MPID_SHMEM_size = atoi( argv[i+1] );
		argv[i] = 0;
		argv[i+1] = 0;
		break;
	    }
	}
    }

    /* Provide device rank of process, this automatically selects
     starting all procs individually (no fork())*/ 
    for (i=1; i<*argc; i++) {
	if( argv[i] ) {
	    if( ( strcmp( argv[i], "-r" ) == 0 ) && ( i < separator ) ) {
		MPID_SHMEM_rank = atoi( argv[i+1] );
		argv[i] = 0;
		argv[i+1] = 0;
		MPID_SHMEM_do_fork = 0;
		MPID_SHMEM_is_master = (MPID_SHMEM_rank == 0 ? 1 : 0 );
		PRINTF( "Individual startup of processes in ch_shmem selected!\n" );
		PRINTF( "\tMPID_SHMEM_size = %d\n", MPID_SHMEM_size );
		PRINTF( "\tMPID_SHMEM_rank = %d\n", MPID_SHMEM_rank );
		PRINTF( "\tMPID_SHMEM_do_fork = %d\n", MPID_SHMEM_do_fork );
		PRINTF( "\tMPID_SHMEM_is_master = %d\n", MPID_SHMEM_is_master );
		break;
	    }
	}
    }

    for( i = 1; i < *argc; i++ ) {
	if( argv[i] ) {
	    if( ( strcmp( argv[i], "-m" ) == 0 ) && ( i < separator ) ) {
		MPID_SHMEM_master_pid = atoi( argv[i+1] );
		argv[i] = 0;
		argv[i+1] = 0;
		PRINTF( "\tMPID_SHMEM_master_pid = %d\n", MPID_SHMEM_master_pid );
		break;
	    }
	}
    }
    MPID_ArgSqueeze( argc, argv );

    /* provide additional information on the device implementation */
    for (i=1; i<*argc; i++) {
	
	if (strcmp( argv[i], "-mpiversion" ) == 0) {
	    argv[i] = 0;
	    PRINTF( "ch_shmem device with the following device choices\n" );
	    PRINTF( "Lock type = %s\n", p2p_lock_name );
#if HAS_VOLATILE
	    PRINTF( "Compiler supports volatile\n" );
#else
            PRINTF( "Compiler *does not* support volatile\n" );
#endif
	    PRINTF( "Maximum processor count = %d\n", MPID_MAX_PROCS );
	    PRINTF( "Maximum shared memory region size is %d bytes\n", 
		    MPID_MAX_SHMEM ); 
	    break;
	}
    }
    MPID_ArgSqueeze( argc, argv );
    
    if (MPID_SHMEM_size <= 0 || MPID_SHMEM_size > MPID_MAX_PROCS) {
	fprintf( stderr, "Invalid number of processes (%d) invalid\n", MPID_SHMEM_size );
	exit( 1 );
    }

    /* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );

    if (memsize < sizeof(MPID_SHMEM_globmem) + MPID_SHMEM_size * 128)
	memsize = sizeof(MPID_SHMEM_globmem) + MPID_SHMEM_size * 128;

    if( MPID_SHMEM_do_fork || MPID_SHMEM_is_master ) {
	p2p_init( memsize );

	MPID_shmem = MPID_SHMEM_sysv_malloc(sizeof(MPID_SHMEM_globmem));

	if (!MPID_shmem) {
	    fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
		     (int) sizeof( MPID_SHMEM_globmem ) );
	    exit(1);
	}

	/* Initialize the shared memory */

	MPID_shmem->barrier.phase = 1;
	MPID_shmem->barrier.cnt1  = MPID_SHMEM_size;
	MPID_shmem->barrier.cnt2  = 0;
	MPID_shmem->barrier.size  = MPID_SHMEM_size;
	
	p2p_lock_init( &MPID_shmem->globlock );
	
	MPID_shmem->globid = 0;
	
	/* The following is rough if MPID_SHMEM_size doesn't divide the MAX_PKTS.
	   If this is too small, then if there aren't enough processors, the
	   code will take forever as it each process gets stuck in a loop
	   until the time-slice ends.  */
	pkts_per_proc = MPID_SHMEM_MAX_PKTS / MPID_SHMEM_size;
	
	/*
	 * Determine the packet flush count at runtime.
	 * (delay the harsh reality of resource management :-) )
	 */
	MPID_pktflush = (pkts_per_proc > MPID_SHMEM_size) ? pkts_per_proc / MPID_SHMEM_size : 1;
	
	cnt	= 0;  /* counter for number of allocated packets */
	for (i=0; i<MPID_SHMEM_size; i++) {
	    /* setup the local copy of the addresses of objects in MPID_shmem */
	    MPID_lshmem.availlockPtr[i]    = &MPID_shmem->availlock[i];
	    MPID_lshmem.incominglockPtr[i] = &MPID_shmem->incominglock[i];
	    MPID_lshmem.incomingPtr[i]     = &MPID_shmem->incoming[i];
	    MPID_lshmem.availPtr[i]	       = &MPID_shmem->avail[i];
	
	    /* Initialize the shared memory data structures */
	    MPID_shmem->incoming[i].head     = 0;
	    MPID_shmem->incoming[i].tail     = 0;
	    
	    /* Setup the avail list of packets */
	    MPID_shmem->avail[i].head = MPID_shmem->pool + cnt;
	    for (j=0; j<pkts_per_proc; j++) {
		MPID_shmem->pool[cnt+j].head.next = ((MPID_PKT_T *)MPID_shmem->pool) + cnt + j + 1;
		MPID_shmem->pool[cnt+j].head.owner = i;
	    }
	    /* Clear the last "next" pointer */
	    MPID_shmem->pool[cnt+pkts_per_proc-1].head.next = 0;
	    cnt += pkts_per_proc;
	    
	    p2p_lock_init( MPID_shmem->availlock + i );
	    p2p_lock_init( MPID_shmem->incominglock + i );
	}
    }

    /* Above this point, there was a single process.  After the p2p_create_procs
       call, there are more */
#if 0
    /* currently disabled, see p2p.c */
    p2p_setpgrp();
#endif

    if( MPID_SHMEM_do_fork )
	p2p_create_procs( *argc, argv );
    else {
	if( MPID_SHMEM_is_master )
	    MPID_SHMEM_sysv_connect_to_clients();
	else {
	    MPID_SHMEM_sysv_connect_to_master( MPID_SHMEM_master_pid );

	    pkts_per_proc = MPID_SHMEM_MAX_PKTS / MPID_SHMEM_size;
	    MPID_pktflush = (pkts_per_proc > MPID_SHMEM_size) ? pkts_per_proc / MPID_SHMEM_size : 1;
	    
	    for (i=0; i<MPID_SHMEM_size; i++) {
		/* setup the local copy of the addresses of objects in MPID_shmem */
		MPID_lshmem.availlockPtr[i]    = &MPID_shmem->availlock[i];
		MPID_lshmem.incominglockPtr[i] = &MPID_shmem->incominglock[i];
		MPID_lshmem.incomingPtr[i]     = &MPID_shmem->incoming[i];
		MPID_lshmem.availPtr[i]	       = &MPID_shmem->avail[i];
	    }

	}
    }

    MPID_SHMEM_FreeSetup();

    MPID_incoming = &MPID_shmem->incoming[MPID_SHMEM_rank].head;
}

void MPID_SHMEM_lbarrier()
{
    VOLATILE int *cnt, *cntother;

/* Figure out which counter to decrement */
    if (MPID_shmem->barrier.phase == 1) {
	cnt	     = &MPID_shmem->barrier.cnt1;
	cntother = &MPID_shmem->barrier.cnt2;
    }
    else {
	cnt	     = &MPID_shmem->barrier.cnt2;
	cntother = &MPID_shmem->barrier.cnt1;
    }

/* Decrement it atomically */
    MPID_SHMEM_sysv_lock( &MPID_shmem->globlock );
    *cnt = *cnt - 1;
    MPID_SHMEM_sysv_unlock( &MPID_shmem->globlock );
    
/* Wait for everyone to to decrement it */
    while ( *cnt ) sched_yield();

/* If process 0, change phase. Reset the OTHER counter*/
    if (MPID_SHMEM_rank == 0) {
		MPID_shmem->barrier.phase = ! MPID_shmem->barrier.phase;

		/* This forces writes to be written to cache-coherent memory. Some processors
		   have special, assembly-language instructions for this. Otherwise, you can
		   usually do a lock/unlock. */
		MPID_SHMEM_sysv_lock( &MPID_shmem->globlock );
		MPID_SHMEM_sysv_unlock( &MPID_shmem->globlock );

		*cntother = MPID_shmem->barrier.size;
    }
    else 
	while (! *cntother) sched_yield();
}

void MPID_SHMEM_finalize()
{
    VOLATILE int *globid;

    fflush(stdout);
    fflush(stderr);

/* There is a potential race condition here if we want to catch
   exiting children.  We should probably have each child indicate a successful
   termination rather than this simple count.  To reduce this race condition,
   we'd like to perform an MPI barrier before clearing the signal handler.

   However, in the current code, MPID_xxx_End is called after most of the
   MPI system is deactivated.  Thus, we use a simple count-down barrier.
   Eventually, we the fast barrier routines.
 */
/* MPI_Barrier( MPI_COMM_WORLD ); */
    MPID_SHMEM_lbarrier();
    p2p_clear_signal();

    /* Once the signals are clear (including SIGCHLD), we should be
       able to exit safely */

/* Wait for everyone to finish 
   We can NOT simply use MPID_shmem->globid here because there is always the 
   possibility that some process is already exiting before another process
   has completed starting (and we've actually seen this behavior).
   Instead, we perform an additional MPI Barrier.
*/
    MPID_SHMEM_lbarrier();
/* MPI_Barrier( MPI_COMM_WORLD ); */
#ifdef FOO
    globid = &MPID_shmem->globid;
    MPID_SHMEM_sysv_lock( &MPID_shmem->globlock );
    MPID_shmem->globid--;
    MPID_SHMEM_sysv_unlock( &MPID_shmem->globlock );
/* Note that this forces all jobs to spin until everyone has exited */
    while (*globid) sched_yield(); /* MPID_shmem->globid) ; */
#endif

    MPID_SHMEM_sysv_shmem_cleanup();
}

/* 
  Read an incoming control message.

  NOTE:
  This routine maintains and internal list of elements; this allows it to
  read from that list without locking it.
 */
/* #define BACKOFF_LMT 1048576 */
#define BACKOFF_LMT 1024
/* 
   This version assumes that the packets are dynamically allocated (not off of
   the stack).  This lets us use packets that live in shared memory.

   NOTE THE DIFFERENCES IN BINDINGS from the usual versions.
 */
int MPID_SHMEM_ReadControl( pkt, size, from_dev_lrank )
MPID_PKT_T **pkt;
int        size, *from_dev_lrank;
{
    MPID_PKT_T *inpkt;
    int        backoff, cnt;
    VOLATILE   MPID_PKT_T **ready;

#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 1;
#endif
    if (MPID_local) {
	inpkt      = (MPID_PKT_T *)MPID_local;
	MPID_local = MPID_local->head.next;
    }
    else {
	if (!MPID_lshmem.incomingPtr[MPID_SHMEM_rank]->head) {
	    /* This code tries to let other processes run.  If there
	       are more physical processors than processes, then a simple
	       while (!MPID_shmem->incoming[MPID_SHMEM_rank].head);
	       might be better.
	       We might also want to do

	       VOLATILE MPID_PKT_T *msg_ptr = 
	       &MPID_shmem->incoming[MPID_SHMEM_rank].head;
	       while (!*msg_ptr) { .... }

	       This code should be tuned with vendor help, since it depends
	       on fine details of the hardware and system.

	       An alternate version of this should consider using the
	       SYSV semop to effect a yield until data has arrived.
	       For example, this process could set a lock when it has
	       read data and other processes could clear it when data is 
	       added.  
	       */
		backoff = 1;
/* 		ready = &MPID_lshmem.incomingPtr[MPID_SHMEM_rank]->head; */
/*		while (!*ready) { */
		while (!MPID_lshmem.incomingPtr[MPID_SHMEM_rank]->head) { 
		    cnt	    = backoff;
		    while (cnt--) ;
		    backoff = 2 * backoff;
		    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
		    /* if (*ready) break;*/
		    if (MPID_lshmem.incomingPtr[MPID_SHMEM_rank]->head) break;
		    /* Return the packets that we have before doing a
		       yield */
		    MPID_SHMEM_FlushPkts();
		    sched_yield();
		}
	}
	/* This code drains the ENTIRE list into a local list */
	MPID_SHMEM_sysv_lock( MPID_lshmem.incominglockPtr[MPID_SHMEM_rank] );
	inpkt          = (MPID_PKT_T *) *MPID_incoming;
	MPID_local     = inpkt->head.next;
	*MPID_incoming = 0;
	MPID_lshmem.incomingPtr[MPID_SHMEM_rank]->tail = 0;
	MPID_SHMEM_sysv_unlock( MPID_lshmem.incominglockPtr[MPID_SHMEM_rank] );
    }

/* Deliver this packet to the caller */
    *pkt  = inpkt;

    *from_dev_lrank = (*inpkt).head.src;

    MPID_TRACE_CODE_PKT("Readpkt",*from_dev_lrank,(*inpkt).head.mode);

#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 0;
    MPID_readcnt++;
#endif
    return MPI_SUCCESS;
}


/*
   Rather than free recv packets every time, we accumulate a few
   and then return them in a group.  

   This is useful when a processes sends several messages to the same
   destination.
   
   This keeps a list for each possible processor, and returns them
   all when MPID_pktflush are available FROM ANY SOURCE.
 */
static MPID_PKT_T *FreePkts[MPID_MAX_PROCS];
static MPID_PKT_T *FreePktsTail[MPID_MAX_PROCS];
static int to_free = 0;

void MPID_SHMEM_FreeSetup()
{
    int i;
    for (i=0; i<MPID_SHMEM_size; i++) { 
	FreePkts[i] = 0;
	FreePktsTail[i] = 0;
    }
}

void MPID_SHMEM_FlushPkts()
{
    int i;
    MPID_PKT_T *pkt;
    MPID_PKT_T *tail;

    if (to_free == 0) return;
    for (i=0; i<MPID_SHMEM_size; i++) {
	if ((pkt = FreePkts[i])) {
	    tail			  = FreePktsTail[i];
	    MPID_SHMEM_sysv_lock( MPID_lshmem.availlockPtr[i] );
	    tail->head.next		  = 
		(MPID_PKT_T *)MPID_lshmem.availPtr[i]->head;
	    MPID_lshmem.availPtr[i]->head = pkt;
	    MPID_SHMEM_sysv_unlock( MPID_lshmem.availlockPtr[i] );
	    FreePkts[i] = 0;
	    FreePktsTail[i] = 0;
	}
    }
    to_free = 0;
}

void MPID_SHMEM_FreeRecvPkt( pkt )
MPID_PKT_T *pkt;
{
    int        src, i;
    MPID_PKT_T *tail;

    MPID_TRACE_CODE_PKT("Freepkt",pkt->head.owner,(pkt->head.mode));

    src	       = pkt->head.owner;
/*
    if (src == MPID_SHMEM_rank) {
	pkt->head.next = MPID_localavail;
	MPID_localavail = pkt;
	return;
    }
    */
    pkt->head.next = FreePkts[src];
/* Set the tail if we're the first */
    if (!FreePkts[src])
	FreePktsTail[src] = pkt;
    FreePkts[src]  = pkt;
    to_free++;

    if (to_free >= MPID_pktflush) {
	MPID_SHMEM_FlushPkts();
    }
}

/* 
   If this is set, then packets are allocated, then set, then passed to the
   sendcontrol routine.  If not, packets are created on the call stack and
   then copied to a shared-memory packet.

   We should probably make "localavail" global, then we can use a macro
   to allocate packets as long as there is a local supply of them.

   For example, just
       extern MPID_PKT_T *MPID_localavail = 0;
   and the
   #define ...GetSendPkt(inpkt) \
   {if (MPID_localavail) {inpkt= MPID_localavail; \
   MPID_localavail=inpkt->head.next;}else inpkt = routine();\
   inpkt->head.next = 0;}
 */
MPID_PKT_T *MPID_SHMEM_GetSendPkt(nonblock)
int nonblock;
{
    MPID_PKT_T *inpkt;
    static MPID_PKT_T *localavail = 0;
    int   freecnt=0;

#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 2;
    MPID_freecnt = 0;
#endif
    if (localavail) {
	inpkt      = localavail;
    }
    else {
	/* If there are no available packets, this code does a yield */
	while (1) {
	    if (MPID_lshmem.availPtr[MPID_SHMEM_rank]->head) {
		/* Only lock if there is some hope */
		MPID_SHMEM_sysv_lock( MPID_lshmem.availlockPtr[MPID_SHMEM_rank] );
		inpkt = (MPID_PKT_T *)MPID_lshmem.availPtr[MPID_SHMEM_rank]->head;
		MPID_lshmem.availPtr[MPID_SHMEM_rank]->head = 0;
		MPID_SHMEM_sysv_unlock( MPID_lshmem.availlockPtr[MPID_SHMEM_rank] );
		/* If we found one, exit the loop */
		if (inpkt) break;
	    }

	    /* No packet.  Wait a while (if possible).  If we do this
	       several times without reading a packet, try to drain the
	       incoming queues 
	     */
#ifdef MPID_DEBUG_ALL
	    if (!freecnt) {
		    MPID_TRACE_CODE("No freePkt",MPID_SHMEM_rank);
		}
#endif
	    /* If not blocking, just return a null packet.  Not used 
	       currently (?) */
	    if (nonblock) return(inpkt);
	    freecnt++;
	    sched_yield();
	    if ((freecnt % 8) == 0) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
		/* Return the packets that we have */
		MPID_SHMEM_FlushPkts();
	    }
#ifdef MPID_DEBUG_SPECIAL
	    MPID_freecnt = freecnt;
#endif
        }
    }
    localavail	 = inpkt->head.next;
    inpkt->head.next = 0;

    MPID_TRACE_CODE_PKT("Allocsendpkt",-1,inpkt->head.mode);
#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 2;
#endif

    return inpkt;
}

int MPID_SHMEM_SendControl( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
    MPID_PKT_T *tail;

#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 3;
#endif

/* Place the actual length into the packet */
    MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt->head.mode);

    pkt->head.src     = MPID_SHMEM_rank;
    pkt->head.next    = 0;           /* Should already be true */

    MPID_SHMEM_sysv_lock( MPID_lshmem.incominglockPtr[dest] );
    tail = (MPID_PKT_T *)MPID_lshmem.incomingPtr[dest]->tail;
    if (tail) 
	tail->head.next = pkt;
    else
	MPID_lshmem.incomingPtr[dest]->head = pkt;

    MPID_lshmem.incomingPtr[dest]->tail = pkt;
    MPID_SHMEM_sysv_unlock( MPID_lshmem.incominglockPtr[dest] );

#ifdef MPID_DEBUG_SPECIAL
    MPID_op = 0;
#endif
    return MPI_SUCCESS;
}

/* 
   Return the address the destination (dest) should use for getting the 
   data at in_addr.  len is INOUT; it starts as the length of the data
   but is returned as the length available, incase all of the data can 
   not be transfered 
 */
void * MPID_SHMEM_SetupGetAddress( in_addr, len, dest )
void *in_addr;
int  *len, dest;
{
    void *new;
    int  tlen = *len;

    MPID_TRACE_CODE("Alloc shared space",tlen);
/* To test, just comment out the first line and set new to null */
    /* tlen = tlen/2; */
    new = MPID_SHMEM_sysv_malloc( tlen );
/* new = 0; */
    while (!new) {
	MPID_SHMEM_DEBUG_PRINT_MSG("Allocating partial space");
	tlen = tlen / 2; 
	while(tlen > 0 && !(new = MPID_SHMEM_sysv_malloc(tlen))) 
	    tlen = tlen / 2;
	if (tlen == 0) {
	    /* This failure means that memory has been consumed without
	       being returned.  Since all of this memory is acquired
	       temporarily by the ADI, it will come back as soon as the
	       receiving end catches up with us.  Wait for some packets
	       to be returned ... */
	    /* This won't work, since we DO leave the data in shared
	       memory when the message is unexpected.  We shouldn't do that...*/
	    MPID_SHMEM_DEBUG_PRINT_MSG("Waiting for memory to be available");
	    MPID_DeviceCheck( MPID_NOTBLOCKING );
	    tlen = *len;
/*
	    p2p_error( "Could not get any shared memory for long message!",  
		       *len);
 */
	}
    }
    /* fprintf( stderr, "Message too long; sending partial data\n" ); */
    *len = tlen;
    MPID_SHMEM_DEBUG_PRINT_MSG2("Allocated %d bytes for long msg", tlen );
    /* printf( "Allocated %d bytes at %x in get\n", tlen, (long)new );*/
#ifdef FOO
/* If this mapped the address space, we wouldn't need to copy anywhere */
/*
if (MPID_DEBUG_FILE) {
    fprintf( MPID_DEBUG_FILE, 
	    "[%d]R About to copy to %d from %d (n=%d) (%s:%d)...\n", 
	    MPID_SHMEM_rank, new, in_addr, tlen, 
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
 */

    MEMCPY( new, in_addr, tlen );
#endif

    MPID_TRACE_CODE_X("Allocated space at",(long)new );
    return new;
}

void MPID_SHMEM_FreeGetAddress( addr )
void *addr;
{
    MPID_TRACE_CODE_X("Freeing space at",(long)addr );
    MPID_SHMEM_sysv_free( addr );
}

/*
 * Debugging support
 */
void MPID_SHMEM_Print_internals( fp )
FILE *fp;
{
    int i;
    char *state;
    MPID_PKT_T *pkt;

    /* Print state */
    state = "Not in device";
    switch (MPID_op) {
    case 0: break;
    case 1: state = "MPID_ReadControl"; break;
    case 2: state = "MPID_GetSendPkt" ; break;
    case 3: state = "MPID_SendControl"; break;
    }
    fprintf( fp, "[%d] State is %s\n", MPID_SHMEM_rank, state );

    /* Print the MPID_lshmem */
    for (i=0; i<MPID_SHMEM_size; i++) {
	fprintf( fp, "[%d] Availlock ptr[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 MPID_lshmem.availlockPtr[i] );
	fprintf( fp, "[%d] Incominglock ptr[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 MPID_lshmem.incominglockPtr[i] );
	fprintf( fp, "[%d] Incomingpointer contents[%d] = %lx\n", 
		 MPID_SHMEM_rank, i, 
		 MPID_lshmem.incomingPtr[i]->head );
	fprintf( fp, "[%d] Incoming packet ptr[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 MPID_lshmem.incomingPtr[i] );
	fprintf( fp, "[%d] Avail packet ptr[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 MPID_lshmem.availPtr[i] );
	fprintf( fp, "[%d] Avail packet ptr head[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 MPID_lshmem.availPtr[i]->head );
	fprintf( fp, "[%d] Free packets ptr[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 FreePkts[i] );
	fprintf( fp, "[%d] Free packets tail[%d] = %lx\n", MPID_SHMEM_rank, i, 
		 FreePktsTail[i] );
	
    }
    fprintf( fp, "[%d] Read %d packets\n", MPID_SHMEM_rank, MPID_readcnt );
    fprintf( fp, "[%d] to free = %d\n", MPID_SHMEM_rank, to_free );
    fprintf( fp, "[%d] loopcnt in GetSendPkt = %d\n", MPID_SHMEM_rank, 
	     MPID_freecnt );
    fprintf( fp, "[%d] MPID_Local = %lx\n", MPID_SHMEM_rank, MPID_local );
    fprintf( fp, "[%d] *MPID_incoming = %lx\n", MPID_SHMEM_rank, *MPID_incoming );

    pkt = (MPID_PKT_T *) MPID_lshmem.availPtr[MPID_SHMEM_rank]->head;
    i   = 0;
    while (pkt && i < 10000) {
	i++;
	pkt = pkt->head.next;
    }
    fprintf( fp, "[%d] Avail packets are %d\n", MPID_SHMEM_rank, i );
}
