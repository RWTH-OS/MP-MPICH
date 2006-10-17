/* $Id$ */

#ifndef _2345235_SHMEMDEF_H
#define _2345235_SHMEMDEF_H

/*
   Sending and receiving packets

   Packets are sent and received on connections.  In order to simultaneously
   provide a good fit with conventional message-passing systems and with 
   other more direct systems (e.g., sockets), I've defined a set of
   connection macros that are here translated into Chameleon message-passing
   calls or into other, data-channel transfers.  These are somewhat 
   complicated by the need to provide access to non-blocking operations

   This file is designed for use with the portable shared memory code from
   p2p.

   In addition, we provide a simple way to log the "channel" operations
   If MPID_TRACE_FILE is set, we write information on the operation (both
   start and end) to the given file.  In order to simplify the code, we
   use the macro MPID_TRACE_CODE(name,channel).  Other implementations
   of channel.h are encouraged to provide the trace calls; note that as macros,
   they can be completely removed at compile time for more 
   performance-critical systems.

 */
/* Do we need stdio here, or has it already been included? */
#include "mpid_common.h"
#include "shmempackets.h"
#include "shmemsysv.h"
#include "p2p.h"

#define MPID_SHMEM_MAX_PKTS (4*MPID_MAX_PROCS)

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

/*
   Notes on the shared data.

   Some of the data may be pointers to shared memory where the POINTERS
   should reside in local memory (for increased efficiency).

   In particularly, the structure MPID_SHMEM_globmem is allocated out of
   shared memory and contains various thinks like the locks.  However, we
   don't want to find the ADDRESS of a lock by looking through some 
   shared memory.  
   Note also that we want all changable data to be on separate cache lines.

   Thus, we want to replace 
     VOLATILE MPID_PKT_T *(a[MPID_MAX_PROCS]);
   by
     VOLATILE MPID_PKT_PTR_T (a[MPID_MAX_PROCS])
   where
      typedef union { MPID_PTK_T *pkt ; PAD } MPID_PKT_PTR_T ;
   where the PAD is char pad[sizeof-cachline].

   In addition, we want to put data that is guarded together into the
   same cache-line.  However, we may not want to put the locks on the same
   cache-line, since other processes may be attempting to acquire the
   locks.

   Note that there are two structures.  A Queue, for messages (required
   message ordering), and a Stack, for available message packets.

   Finally, note that while the array of message queues and stacks itself is
   in shared memory, their locations in the shared memory do not change (for
   example, the location of the queue data structure for process 12 does
   not move).  Because of that, we do not want to access the elements of
   these structures by first dereferencing MPID_shmem (the pointer to the
   general structure), rather, we want to keep a local COPY of the locations
   in MPID_shmem and use those instead.  It is true that on some systems,
   the cache architecture will do this for us, but my making this explicit,
   we avoid any performance surprises (at least about this).  The local
   copy is kept in MPID_lshmem;
 */

/* 
   For many systems, it is important to align data structures on 
   cache lines, and to insure that separate structures are in
   different cache lines.  Currently, the largest cache line that we've seen
   is 128 bytes, so we pick that as the default.
 */
#ifndef MPID_CACHE_LINE_SIZE
#define MPID_CACHE_LINE_SIZE 128
#define MPID_CACHE_LINE_LOG_SIZE 7
#endif
/* 
   The shared datastructures.  These are padded to be on different
   cachelines.  The queue is aranged so that the head and tail pointers
   are on the same cacheline .
   It might be useful to put the locks for the datastructure in the same
   structure.  Then again, if one process is doing while(!head), this
   could slow down another process that is trying to lock the queue or
   stack.
 */
typedef struct {
    VOLATILE MPID_PKT_T *head;
    VOLATILE MPID_PKT_T *tail;
    char                pad[MPID_CACHE_LINE_SIZE - 2 * sizeof(MPID_PKT_T *)];
    } MPID_SHMEM_Queue;

typedef struct {
    VOLATILE MPID_PKT_T *head;
    char                pad[MPID_CACHE_LINE_SIZE - 1 * sizeof(MPID_PKT_T *)];
    } MPID_SHMEM_Stack;

typedef struct {
    int          size;           /* Size of barrier */
    VOLATILE int phase;          /* Used to indicate the phase of this 
				    barrier; only process 0 can change */
    VOLATILE int cnt1, cnt2;     /* Used to perform counts */
    } MPID_SHMEM_Barrier_t;

/*
   This is the global area of memory; when this structure is allocated,
   we have the initial shared memory
 */
typedef struct {
    /* locks may need to be aligned, so keep at front (MPID_SHMEM_sysv_malloc provides
       16-byte alignment for each allocated block).       */
    p2p_lock_t availlock[MPID_MAX_PROCS];    /* locks on avail list */
    p2p_lock_t incominglock[MPID_MAX_PROCS]; /* locks on incoming list */
    p2p_lock_t globlock;
    MPID_SHMEM_Queue    incoming[MPID_MAX_PROCS];     /* Incoming messages */
    MPID_SHMEM_Stack    avail[MPID_MAX_PROCS];        /* Avail pkts */

    VOLATILE MPID_PKT_T pool[MPID_SHMEM_MAX_PKTS];    /* Preallocated pkts */

    /* We put globid last because it may otherwise upset the cache alignment
       of the arrays */
    VOLATILE int        globid;           /* Used to get my id in the world */
    MPID_SHMEM_Barrier_t barrier;         /* Used for barriers */
    } MPID_SHMEM_globmem;	

/* 
   This is a LOCAL copy of the ADDRESSES of objects in MPID_shmem.
   We can use
    MPID_lshmem.incomingPtr[src]->head
   instead of
    MPID_shmem->incoming[src].head
   The advantage of this is that the dereference (->) is not done on the
   same address (MPID_shmem) that all other processors must also use.
 */
typedef struct {
    /* locks may need to be aligned, so keep at front (MPID_SHMEM_sysv_malloc provides
       16-byte alignment for each allocated block).       */
    p2p_lock_t *availlockPtr[MPID_MAX_PROCS];    /* locks on avail list */
    p2p_lock_t *incominglockPtr[MPID_MAX_PROCS]; /* locks on incoming list */
    MPID_SHMEM_Queue    *incomingPtr[MPID_MAX_PROCS];  /* Incoming messages */
    MPID_SHMEM_Stack    *availPtr[MPID_MAX_PROCS];     /* Avail pkts */
#ifdef FOO
    MPID_SHMEM_Queue    *my_incoming;                  /* My incoming queue */
    MPID_SHMEM_Stack    *my_avail;                     /* My avail stack */
#endif
    } MPID_SHMEM_lglobmem;	



#ifdef MPID_CACHE_LINE_SIZE
#define MPID_SHMEM_HEADER_ALIGNMENT (2*MPID_CACHE_LINE_SIZE)
#define MPID_SHMEM_HEADER_LOG_ALIGN (MPID_CACHE_LINE_LOG_SIZE+1)
#else
#define MPID_SHMEM_HEADER_LOG_ALIGN 6
#define MPID_SHMEM_HEADER_ALIGNMENT (1 << MPID_SHMEM_HEADER_LOG_ALIGN)
#endif

/* MPID_SHMEM_HEADER_ALIGNMENT is assumed below to be bigger than sizeof(p2p_lock_t) +
   sizeof(MPID_SHMEM_shmem_header_t *), so do not reduce MPID_SHMEM_HEADER_LOG_ALIGN below 4 */
union _MPID_SHMEM_shmem_header_t {
  struct {
    union _MPID_SHMEM_shmem_header_t *ptr;	/* next block if on free list */
    unsigned size;	                        /* size of this block */
  } s;
  char align[MPID_SHMEM_HEADER_ALIGNMENT];  /* Align to MPID_SHMEM_HEADER_ALIGNMENT byte boundary */
};

typedef union _MPID_SHMEM_shmem_header_t MPID_SHMEM_shmem_header_t;







extern MPID_SHMEM_globmem  *MPID_shmem;
extern MPID_SHMEM_lglobmem  MPID_lshmem;
extern MPID_PKT_T           *MPID_local;      /* Local pointer to arrived
                                      packets; it is only
						 accessed by the owner */
extern VOLATILE MPID_PKT_T **MPID_incoming;   /* pointer to my incoming 
						 queue HEAD (really?) */

extern MPID_SHMEM_shmem_header_t **MPID_SHMEM_shared_memory;

extern int MPID_SHMEM_is_master;
extern int MPID_SHMEM_do_fork;

#endif /* _2345235_SHMEMDEF_H */
