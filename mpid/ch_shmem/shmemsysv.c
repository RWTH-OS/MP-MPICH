/* $Id$ */

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "shmem-mpid.h"
#include "shmemdef.h"
#include "shmemsysv.h"
#include "shmemcommon.h"

enum {
    CLIENT_CONTACT = 0, SHMEM_CONFIG, SHMEM_SEG
};

struct _client_contact_msg_t {
    int rank;                          /* device rank of cantacting client */
};

struct _shmem_config_msg_t {
    int numsegs;                      /* number of attached shared memory segments*/
    int semid;                        /* ID of SYSV semaphore array */
    long sem_adm_offset;              /* offset of MPID_SHMEM_semaphore_admin into shared memory */
    long shmem_globmem_offset;        /* offset of MPID_shmem into shared memory */
};

struct _shmem_seg_msg_t {
    int id;
    long mapping_offset;
};

union _shmem_msg_type {
  struct _client_contact_msg_t client_contact_msg;
  struct _shmem_config_msg_t shmem_config_msg;
  struct _shmem_seg_msg_t shmem_seg_msg;
};

struct _MPID_SHMEM_sysv_msg_t {
    long dest;
    int src;
    int type;
    union _shmem_msg_type msg_type;
};

typedef struct _MPID_SHMEM_sysv_msg_t MPID_SHMEM_sysv_msg_t;

int MPID_SHMEM_sysv_semid;
int MPID_SHMEM_sysv_pid[MPID_MAX_PROCS];
int MPID_SHMEM_sysv_mypid;
MPID_SHMEM_sem_admin_t *MPID_SHMEM_semaphore_admin;
p2p_lock_t *MPID_SHMEM_sysv_firstlock; /* Pointer to lock */


static int MPID_SHMEM_sysv_num_shmids; /* number of attached shared memory segments ( may be
				   > 1 if we cannot get a single big segment and have to
				   get several smaller ones )*/

static int *MPID_SHMEM_sysv_shmid;   /* IDs of attached shared memory segments */
static char **MPID_SHMEM_sysv_shmat; /* addresses onto which shared memory segments are mapped */

static int MPID_SHMEM_sysv_msqid;

/* These are for the operations we perform on semaphores (via semop()). Because we use System V IPC,
   even simple things get complicated. The first entry in each array has to be set to the semaphore
   number, so the 0 here is just a dummy (we perform any operation on just one semaphore at a time).
   The second is the operation (-1 decrements the semaphore and 1 increments it. The 0 at the end
   means that we don't use any flags here. */
static struct sembuf sem_lock[1]   = { { 0, -1, 0 } };
static struct sembuf sem_unlock[1] = { { 0, 1, 0 } };

/* MPID_SHMEM_create_sysv_shmem() allocates shared memory of size at least <memsize> bytes
   and attaches to it. It returns a pointer to the shared  memory in case of success and 
   NULL otherwise. If the shared memory cannot be allocated in a single segment, it is split
   into different segments of size MPID_SHMEM_SYSV_SHM_SEGSIZE. Nonetheless, these segments are
   attached in a way that the shared memory logically forms a single block in the virtual address
   space of the process. Thus, the shared memory can be accessed with the returned address as a base
   address and some offset into it. */
void *MPID_SHMEM_sysv_create_shmem( memsize )
unsigned int memsize;
{
    int i, nsegs, failure, nbr_succ;
    char *mem;
    key_t key;

    /* To initialize the data structures, we calculate the number of shared memory segments that we would
       try to get if we cannot get everything in a single segment */
    if( memsize  &&  (memsize % MPID_SHMEM_SYSV_SHM_SEGSIZE) == 0 )
      nsegs = memsize / MPID_SHMEM_SYSV_SHM_SEGSIZE;
    else
      nsegs = memsize / MPID_SHMEM_SYSV_SHM_SEGSIZE + 1;

    /* allocate the data structures */
    if( ( MPID_SHMEM_sysv_shmid = (int *) malloc( nsegs * sizeof(int) ) ) == NULL ) {
      perror( "malloc() failed" );
      return NULL;
    }

    if( ( MPID_SHMEM_sysv_shmat = (char **) malloc( nsegs * sizeof(void *) ) ) == NULL ) {
      perror( "malloc() failed" );
      free( MPID_SHMEM_sysv_shmid );
      return NULL;
    }
    
    /* First, try to allocate the memory as a single segment */
    key = ftok( "/tmp", MPID_SHMEM_sysv_mypid + 1 );
    if( ( MPID_SHMEM_sysv_shmid[0] = shmget( key, memsize, IPC_CREAT | 0600 ) ) != -1 ) {
      /* We got all the memory we want in a single segment, now we try to attach to it */
      if( ( mem = (char *)shmat( MPID_SHMEM_sysv_shmid[0], NULL, 0 ) ) == (char *)-1 ) {
	/* If we can't attach with an arbitrary address to a shared memory segment
	   that we could get, this is probably a severe problem, so we schedule the memory
	   for removal and return an error */
	if( shmctl( MPID_SHMEM_sysv_shmid[0], IPC_RMID, NULL ) != 0 )
	  perror( "shmctl() failed" );

	return NULL;
      }
      else {
	/* This is the ideal case: a single segment and one pointer to it */
	MPID_SHMEM_sysv_shmat[0] = mem;
	MPID_SHMEM_sysv_num_shmids = 1;

	return mem;
      }
    }

    /* We couldn't get a single segment.  Try for multiple segments */

    /* First, we try to get the first segment and attach it to some arbitrary address */
    key = ftok( "/tmp", MPID_SHMEM_sysv_mypid + 1 );
    if( ( MPID_SHMEM_sysv_shmid[0] = shmget( key, MPID_SHMEM_SYSV_SHM_SEGSIZE, IPC_CREAT | 0600 ) ) == -1 )
      return NULL;

    if( ( mem = (char *)shmat( MPID_SHMEM_sysv_shmid[0], NULL, 0 ) ) == (char *)-1 ) {
      if( shmctl( MPID_SHMEM_sysv_shmid[0], IPC_RMID, NULL ) != 0 )
	perror( "shmctl() failed" );
      return NULL;
    }
    
    MPID_SHMEM_sysv_shmat[0] = mem;
    MPID_SHMEM_sysv_num_shmids = 1;

    /* Now we try to get the other segments and attach to them in a way that we have one block
       of memory in our virtual address space, so we can access the memory with a base address
       and an offset */
    /* First we try to get the segments. If anything goes wrong, we have to clean up. */
    failure = 0;
    nbr_succ = 1; /* we already have the first segment*/
    for( i = 1; i < nsegs; i++ ) {
	key = ftok( "/tmp", MPID_SHMEM_sysv_mypid + 1 + i );
	if( ( MPID_SHMEM_sysv_shmid[i] = shmget( key, MPID_SHMEM_SYSV_SHM_SEGSIZE, IPC_CREAT | 0600 ) ) == -1 ) {
	    failure = 1;
	    break;
	}
	else
	    nbr_succ++;
    }
    
    if( failure ) {
      /* detach from first segment */
      if( shmdt( MPID_SHMEM_sysv_shmat[0] ) != 0 )
	perror( "shmdt() failed" );

      /* Schedule already allocated shared memory segments for removal and exit */
      for( i = 0; i < nbr_succ; i++ )
	if( shmctl( MPID_SHMEM_sysv_shmid[i], IPC_RMID, NULL ) != 0 )
	  perror( "shmctl() failed" );

      return NULL;
    }
    
    /* We have all shared memory segments, now we attach to them. At each attachment, we must get the desired start
       address for the segment, otherwise we fail in toto, in which case we have to clean everything up.
       There are two simple ways the segments can be attached: with increasing addresses (MPID_SHMEM_sysv_shmat[0]
       is the base address) or with decreasing addresses (MPID_SHMEM_sysv_shmat[nsegs-1] is the base
       address). We allow both possibilities to make no assumptions about the order in which the OS assings
       the addresses. */

    mem = MPID_SHMEM_sysv_shmat[0];
    nbr_succ = 1; /* first segment already attached */
    failure = 0;
    for( i = 1; i < nsegs; i++ ) {
      /* shmat() should fail if the given address cannot be attached to */

      /* map the next segment above the former */
      if( ( shmat( MPID_SHMEM_sysv_shmid[i], mem + MPID_SHMEM_SYSV_SHM_SEGSIZE, 0 ) ) == (void *)-1 ) {

	/* map the next segment below the former */
	if( ( shmat( MPID_SHMEM_sysv_shmid[i], mem - MPID_SHMEM_SYSV_SHM_SEGSIZE, 0 ) ) == (void *)-1 ) {
	  fprintf( stderr, "shmat() failed: %s, errno = %d\n", strerror( errno ), errno );
	  fflush( stderr );
	  failure = 1;
	  break;
	}
	else {
	  MPID_SHMEM_sysv_shmat[i] = mem - MPID_SHMEM_SYSV_SHM_SEGSIZE;
	  nbr_succ++;
	  mem -= MPID_SHMEM_SYSV_SHM_SEGSIZE;
	}
      }
      else {
	MPID_SHMEM_sysv_shmat[i] = mem + MPID_SHMEM_SYSV_SHM_SEGSIZE;
	nbr_succ++;
	mem += MPID_SHMEM_SYSV_SHM_SEGSIZE;
      }
    }

    if( failure ) {

      /* detach from all already attached segments */
      for( i = 0; i < nbr_succ; i++ )
	if( shmdt( MPID_SHMEM_sysv_shmat[i] ) != 0 )
	  perror( "shmdt() failed" );

      /* schedule all shared memory segments for removal */
      for( i = 0; i < nsegs; i++ )
	if( shmctl( MPID_SHMEM_sysv_shmid[i], IPC_RMID, NULL ) != 0 )
	  perror( "shmctl() failed" );

      return NULL;
    }
    else
	return MPID_SHMEM_sysv_shmat[0] < MPID_SHMEM_sysv_shmat[nsegs-1] ? MPID_SHMEM_sysv_shmat[0] : MPID_SHMEM_sysv_shmat[nsegs-1];
}    

void MPID_SHMEM_sysv_shmem_cleanup( void )
{
    /* Define a dummy argument value (some systems (e.g., LINUX Redhat)
       require the fourth argument to be declared and not 0) */
#   if defined(SEMCTL_ARG_UNION)
    union semun arg;
    arg.val = 0;
#   else
    int arg = 0;
#   endif
    int i;
  
  /* remove semaphore array */
  semctl( MPID_SHMEM_sysv_semid, 0, IPC_RMID, arg );

  /* remove message queue */
  msgctl( MPID_SHMEM_sysv_msqid, IPC_RMID, NULL );

  /* detach from all attached segments */
  for( i = 0; i < MPID_SHMEM_sysv_num_shmids; i++ )
    if( shmdt( MPID_SHMEM_sysv_shmat[i] ) != 0 )
      perror( "shmdt() failed" );
  
  if( MPID_SHMEM_rank == 0 ) {
    /* schedule shared segments for removal */
    for( i = 0; i < MPID_SHMEM_sysv_num_shmids; i++ )
      if( shmctl( MPID_SHMEM_sysv_shmid[i], IPC_RMID, NULL ) != 0 )
	perror( "shmctl() failed" );
  }

  /* free local data structures */
  free( MPID_SHMEM_sysv_shmat );
  free( MPID_SHMEM_sysv_shmid );
}

/* MPID_SHMEM_sysv_init_semset() allocates a semaphore set of <nbr_sems> semaphores with <key> as the key
   and initializes the semaphores with 1. On success the semaphore set identifier is returned, -1 otherwise. */

int MPID_SHMEM_sysv_init_semset( nbr_sems, id )
int nbr_sems;
int id;
{
    int i, semid;
    union semun arg;
    key_t key;

    arg.val = 1;

    /* Create a set of <nbr_sems> semaphores */
    key = ftok( "/tmp", id );
    if( ( semid = semget( key, nbr_sems, IPC_CREAT | 0600 ) ) < 0 ) {
      perror( "semget() failed" );
      
      return -1;
    }

    /* initialize all semaphores with value 1 */
    for( i = 0; i < MPID_SHMEM_SYSV_MAX_SEMS; i++ ) {
      if( semctl( semid, i, SETVAL, arg ) == -1 ) {
	perror("semctl() failed" );
	
	if( semctl( semid, 0, IPC_RMID, NULL ) != 0 )
	  perror( "semctl() failed" );

	return -1;
      }
    }

    return semid ;
}

void MPID_SHMEM_sysv_lock( L )
p2p_lock_t *L;
{
    sem_lock[0].sem_num = L->semnum;
    if( semop( MPID_SHMEM_sysv_semid, &sem_lock[0], 1 ) < 0 )
        p2p_error("OOPS: semop lock failed\n", MPID_SHMEM_sysv_semid );
}

void MPID_SHMEM_sysv_unlock( L )
p2p_lock_t *L;
{
    sem_unlock[0].sem_num = L->semnum;
    if (semop( MPID_SHMEM_sysv_semid, &sem_unlock[0], 1 ) < 0 )
        p2p_error("OOPS: semop unlock failed\n", MPID_SHMEM_sysv_semid );
}

int MPID_SHMEM_sysv_connect_to_clients( void )
{
    int nbr_to_contact, rank, i;
    key_t key;
    MPID_SHMEM_sysv_msg_t message;

    printf( "\tMPID_SHMEM_sysv_mypid = %d\n", MPID_SHMEM_sysv_mypid );
    fflush( stdout );

    /* Process ID of master has already been saved */
    key = ftok( "/tmp", MPID_SHMEM_sysv_mypid );

    if( ( MPID_SHMEM_sysv_msqid = msgget( key, IPC_CREAT | 0600 ) ) == -1 ) {
	perror( "msgget() failed" );
	return -1;
    }

    /* initialize PID array with 0; we rely on this when killing all other processes
     from the master process */
    for( i = 0; i < MPID_MAX_PROCS; i++ )
	MPID_SHMEM_sysv_pid[i] = 0;

    /* receive contact messages from clients */
    nbr_to_contact = MPID_SHMEM_size - 1;
    while( nbr_to_contact > 0 ) {
	if( msgrcv( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), MPID_SHMEM_sysv_mypid, 0 ) == -1 )
	    perror( "msgrcv() failed" );
	MPID_SHMEM_sysv_pid[message.msg_type.client_contact_msg.rank] = message.src;
	nbr_to_contact--;
    }
    MPID_SHMEM_sysv_pid[0] = MPID_SHMEM_sysv_mypid; /* to have it complete, don't know if it's needed */ 

    /* send shared memory configuration messages to clients */
    message.src = MPID_SHMEM_sysv_mypid;
    message.type = SHMEM_CONFIG;
    message.msg_type.shmem_config_msg.numsegs = MPID_SHMEM_sysv_num_shmids;
    message.msg_type.shmem_config_msg.semid   = MPID_SHMEM_sysv_semid;
    message.msg_type.shmem_config_msg.sem_adm_offset = (long)MPID_SHMEM_semaphore_admin - (long)MPID_SHMEM_shared_memory;
    message.msg_type.shmem_config_msg.shmem_globmem_offset = (long)MPID_shmem - (long)MPID_SHMEM_shared_memory;

    for( rank = 1; rank < MPID_SHMEM_size; rank++ ) {
	message.dest = MPID_SHMEM_sysv_pid[rank];
	if( msgsnd( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), 0 ) == -1 )
	    perror( "msgsnd() failed" );
    }

    /* send segment IDs and attachment offsets to client processes */
    message.src = MPID_SHMEM_sysv_mypid;
    message.type = SHMEM_SEG;
    for( i = 0; i < MPID_SHMEM_sysv_num_shmids; i++ ) {
	message.msg_type.shmem_seg_msg.id = MPID_SHMEM_sysv_shmid[i];
	message.msg_type.shmem_seg_msg.mapping_offset = (long)(MPID_SHMEM_sysv_shmat[i]) - (long)MPID_SHMEM_shared_memory;
	for( rank = 1; rank < MPID_SHMEM_size; rank ++ ) {
	    message.dest = MPID_SHMEM_sysv_pid[rank];
	    if( msgsnd( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), 0 ) == -1 )
		perror( "msgsnd() failed" );
	}
    }
}


int MPID_SHMEM_sysv_connect_to_master( master_pid )
     int master_pid;
{
    int order;
    key_t key;
    int i;
    MPID_SHMEM_sysv_msg_t message;
    long sem_adm_offset, shmem_globmem_offset;
    long *map_offsets;
    char *address;

    key = ftok( "/tmp", master_pid );
    if( ( MPID_SHMEM_sysv_msqid = msgget( key, IPC_CREAT | 0600 ) ) == -1 ) {
	perror( "msgget() failed" );
	return -1;
    }

    /* Process ID of client process not saved yet */
    MPID_SHMEM_sysv_mypid = getpid();

    /* send contact message to master process */
    message.dest = (long)master_pid;
    message.src  = MPID_SHMEM_sysv_mypid;
    message.type = CLIENT_CONTACT;
    message.msg_type.client_contact_msg.rank = MPID_SHMEM_rank;
    
    if( msgsnd( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), 0 ) == -1 )
	perror( "msgsnd() failed" );

    /* receive shared memory configuration message from master process */
    if( msgrcv( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), MPID_SHMEM_sysv_mypid, 0 ) == -1 )
	perror( "msgrcv() failed" );

    MPID_SHMEM_sysv_num_shmids = message.msg_type.shmem_config_msg.numsegs;
    MPID_SHMEM_sysv_semid = message.msg_type.shmem_config_msg.semid;
    sem_adm_offset = message.msg_type.shmem_config_msg.sem_adm_offset;
    shmem_globmem_offset = message.msg_type.shmem_config_msg.shmem_globmem_offset;

    
    /* receive segment IDs and attachment offsets from master process */ 
    MPID_SHMEM_sysv_shmid = (int *)malloc( MPID_SHMEM_sysv_num_shmids * sizeof(int) );
    MPID_SHMEM_sysv_shmat = (char **)malloc( MPID_SHMEM_sysv_num_shmids * sizeof(char *) );
    map_offsets = (long *)malloc( MPID_SHMEM_sysv_num_shmids * sizeof(long) );
    
    for( i = 0; i < MPID_SHMEM_sysv_num_shmids; i++ ) {
	if( msgrcv( MPID_SHMEM_sysv_msqid, &message, sizeof(message) - sizeof(long), MPID_SHMEM_sysv_mypid, 0 ) == -1 )
	    perror( "msgrcv() failed" );
	MPID_SHMEM_sysv_shmid[i] = message.msg_type.shmem_seg_msg.id;
	map_offsets[i] = message.msg_type.shmem_seg_msg.mapping_offset;
    }
    
    /* mapping of the shared memory segments */

    /* attach first segment at an arbitrary address */
    if( ( MPID_SHMEM_sysv_shmat[0] = (char *)shmat( MPID_SHMEM_sysv_shmid[0], NULL, 0 ) ) == (char *)-1 ) {
	perror( "shmat() failed" );
	exit( EXIT_FAILURE );
    }

    /* attach the rest of the segments */
    for( i = 1; i < MPID_SHMEM_sysv_num_shmids; i++ ) {
	if( ( MPID_SHMEM_sysv_shmat[i] = (char *)shmat( MPID_SHMEM_sysv_shmid[i], MPID_SHMEM_sysv_shmat[0] + map_offsets[i], 0 ) ) == (char *)-1 ) {
	    perror( "shmat() failed" );
	    exit( EXIT_FAILURE );
	}
    }
	     
    /* base pointer */
    if( MPID_SHMEM_sysv_num_shmids > 1 )
	MPID_SHMEM_shared_memory = (MPID_SHMEM_shmem_header_t **)( ( map_offsets[1] > 0 ) ? MPID_SHMEM_sysv_shmat[0] : MPID_SHMEM_sysv_shmat[MPID_SHMEM_sysv_num_shmids-1] ) ;
    else
	MPID_SHMEM_shared_memory = (MPID_SHMEM_shmem_header_t **)( MPID_SHMEM_sysv_shmat[0] );

    /* pointer into shared mem */
    MPID_SHMEM_semaphore_admin = (MPID_SHMEM_sem_admin_t *)( (char *)MPID_SHMEM_shared_memory + sem_adm_offset );
    MPID_shmem = (MPID_SHMEM_globmem *)( (char *)MPID_SHMEM_shared_memory + shmem_globmem_offset );
    MPID_SHMEM_sysv_firstlock = (p2p_lock_t *)( ((MPID_SHMEM_shmem_header_t **)MPID_shmem) + 1 );
}

void *MPID_SHMEM_sysv_malloc( nbytes )
unsigned int nbytes;
{
    MPID_SHMEM_shmem_header_t *p, *prevp;
    char *address = (char *) NULL;
    unsigned int nunits;
	
    /* Start of critical region */
    MPID_SHMEM_sysv_lock( MPID_SHMEM_sysv_firstlock );

    if (*MPID_SHMEM_shared_memory) {
        /* Look for free shared memory */
	
	nunits = ((nbytes + sizeof(MPID_SHMEM_shmem_header_t) - 1) >> MPID_SHMEM_HEADER_LOG_ALIGN) + 1;
	
	prevp = *MPID_SHMEM_shared_memory;
	p = prevp->s.ptr;

	while( 1 ) {
	    if (p->s.size >= nunits) {
		/* Big enuf */
		if (p->s.size == nunits) {
		    /* exact fit */
		    
		    if (p == p->s.ptr) {
			/* No more shared memory available */
			prevp = (MPID_SHMEM_shmem_header_t *) NULL;
		    }
		    else {
			prevp->s.ptr = p->s.ptr;
		    }
		}
		else {
		    /* allocate tail end */
		    p->s.size -= nunits;
		    p += p->s.size;
		    p->s.size = nunits;
		}
		*MPID_SHMEM_shared_memory = prevp;
		address = (char *) (p + 1);
		break;
	    }

	    if (p == *MPID_SHMEM_shared_memory) {
		/* wrapped around the free list ... no fit
		 * found */
		address = (char *) NULL;
		break;
	    }

	    prevp = p;
	    p = p->s.ptr;
	}
    }
    
    /* End critical region */
    MPID_SHMEM_sysv_unlock(MPID_SHMEM_sysv_firstlock);

    return address;
}

void MPID_SHMEM_sysv_free( ap )
void *ap;
{
    MPID_SHMEM_shmem_header_t *bp, *p;
    

    if( !ap )
      return;			/* Do nothing with NULL pointers */

    

    /* Begin critical region */

    MPID_SHMEM_sysv_lock( MPID_SHMEM_sysv_firstlock );

    bp = (MPID_SHMEM_shmem_header_t *)ap - 1;	/* Point to block header */

    if( *MPID_SHMEM_shared_memory ) {
         /* there are already free region(s) in the shared memory region */

    	for( p = *MPID_SHMEM_shared_memory; !(bp > p && bp < p->s.ptr); p = p->s.ptr ) {
	  if( p >= p->s.ptr && (bp > p || bp < p->s.ptr) )
	    break;		/* Freed block at start of end of arena */
    	}
	
        /* Integrate bp in list */

    	*MPID_SHMEM_shared_memory = p;

	if( bp + bp->s.size == p->s.ptr ) {
	  /* join to upper neighbour */
	  if( p->s.ptr == *MPID_SHMEM_shared_memory )
	    *MPID_SHMEM_shared_memory = bp;
	  if( p->s.ptr == p )
	    bp->s.ptr = bp;
	  else
	    bp->s.ptr = p->s.ptr->s.ptr;

	  bp->s.size += p->s.ptr->s.size;
	}
	else
	  bp->s.ptr = p->s.ptr;

	if( p + p->s.size == bp ) {
	  /* Join to lower neighbour */
	  p->s.size += bp->s.size;
	  p->s.ptr = bp->s.ptr;
	}
	else
	  p->s.ptr = bp;

    }
    else {
      /* There wasn't a free shared memory region before */

      bp->s.ptr = bp;
      
      *MPID_SHMEM_shared_memory = bp;
    }

    /* End critical region */
    MPID_SHMEM_sysv_unlock(MPID_SHMEM_sysv_firstlock);
}
