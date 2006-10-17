/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_smpi_h
#define _gmpi_smpi_h

#include "gm.h"
#include "gm_stbar.h"
#include "gmpi_packets.h"

/* MAXIMUM OF LOCAL PROCESSES */
#define SMPI_MAX_NUMLOCALNODES 16
/* the maximum number of nodes in the job */
#define SMPI_MAX_NUMNODES 8192

#define SMPI_INITIAL_SEND_FIFO 1024
#define SMPI_MAX_DELAYED_SEND_QUEUE_SIZE 4*1024*1024

#if GM_CPU_alpha

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a) ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)

#elif (GM_CPU_sparc64 || GM_CPU_sparc)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a) ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)

#elif GM_CPU_ia64

#define SMPI_CACHE_LINE_SIZE 128
#define SMPI_ALIGN(a) ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)

#else

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a) ((a + SMPI_CACHE_LINE_SIZE + 3) & 0xFFFFFFFC)

#endif

#define SMPI_MAX_INT ((unsigned int)(-1))

#define SMPI_LENGTH_QUEUE (1 << 20) /* 1024 KB */

/* Macros for flow control and rqueues management */
#define SMPI_TOTALIN(sender,receiver) \
smpi_shmem->rqueues_params[sender].params[receiver].msgs_total_in
#define SMPI_TOTALOUT(sender,receiver) \
smpi_shmem->rqueues_flow_out[receiver][sender].msgs_total_out
#define SMPI_CURRENT(sender,receiver) \
smpi_shmem->rqueues_params[receiver].params[sender].current
#define SMPI_NEXT(sender,receiver) \
smpi_shmem->rqueues_params[sender].params[receiver].next
#define SMPI_FIRST(sender,receiver) \
smpi_shmem->rqueues_limits[receiver][sender].first
#define SMPI_LAST(sender,receiver) \
smpi_shmem->rqueues_limits[receiver][sender].last


/* packets definition */
#define SMPI_PKT_BASIC \
  GMPI_DEBUG_CHECKSUM_SMALL \
  gm_u16_t mode;             /* Contains MPID_Pkt_t */             \
  gm_u16_t lrank;            /* Local rank in sending context */   \
  unsigned int context_id;   /* Context_id */                      \
  int tag;                   /* tag is full sizeof(int) */         \
  int len;                   /* Length of DATA */
    
/* This is the minimal message packet */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
} SMPI_PKT_HEAD_T;

/* Short messages are sent eagerly (unless Ssend) */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
} SMPI_PKT_SHORT_T;

/* Long messages are sent rendez-vous if not directcopy */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
  void * recv_id;
} SMPI_PKT_CONT_GET_T;

/* GET_request packet */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
  void * address;      /* Location of data ON SENDER */
} SMPI_PKT_GET_T;

/* RNDV_request packet */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
  void * recv_id;
  GMPI_DEBUG_CHECKSUM_LARGE
} SMPI_PKT_RNDV_T;

/* Cancel packet */
typedef struct {
  SMPI_PKT_BASIC
  void * send_id;
  int cancel;        /* set to 1 if msg was cancelled - 
			0 otherwise */
} SMPI_PKT_ANTI_SEND_T;

/* all the differents types of packets */
typedef union _SMPI_PKT_T {
  SMPI_PKT_HEAD_T          head;
  SMPI_PKT_SHORT_T         short_pkt;
  SMPI_PKT_CONT_GET_T      cont_pkt;
  SMPI_PKT_GET_T           get_pkt;
  SMPI_PKT_RNDV_T          rndv_pkt;
  SMPI_PKT_ANTI_SEND_T     cancel_pkt;
  char                     pad[SMPI_CACHE_LINE_SIZE];
} SMPI_PKT_T;

/* send_requests fifo, to stack the send in case of starvation of buffer
   in the shared area memory */
struct smpi_send_fifo_req {
  void *data;
  struct _MPIR_SHANDLE *shandle;
  struct smpi_send_fifo_req *next;
  int len;
  int grank;
  int is_data;
};

/* management informations */
struct smpi_var {
  void * mmap_ptr;
  struct smpi_send_fifo_req * send_fifo_head;
  struct smpi_send_fifo_req * send_fifo_tail;
  struct gm_lookaside * send_fifo_lookaside;
  unsigned int send_fifo_queued;
  unsigned int malloc_send_buf_allocated;
  unsigned int my_local_id;
  unsigned int num_local_nodes;
  unsigned int local_nodes[SMPI_MAX_NUMNODES];
  int available_queue_length;
  int pending;
  int fd;
};

/* the shared area itself */
struct shared_mem {
  volatile int pid[SMPI_MAX_NUMLOCALNODES]; /* use for initial synchro */
  volatile int board_id[SMPI_MAX_NUMLOCALNODES];
  volatile int port_id[SMPI_MAX_NUMLOCALNODES];
  char pad1[SMPI_CACHE_LINE_SIZE];

  /* receive queues descriptors */
  volatile struct {
    volatile struct {
      volatile unsigned int current;
      volatile unsigned int next;
      volatile unsigned int msgs_total_in;
    } params[SMPI_MAX_NUMLOCALNODES];
    char pad[SMPI_CACHE_LINE_SIZE];
  } rqueues_params[SMPI_MAX_NUMLOCALNODES];

  /* rqueues flow control */
  volatile struct {
    volatile unsigned int msgs_total_out;
    char pad[SMPI_CACHE_LINE_SIZE-4];
  } rqueues_flow_out[SMPI_MAX_NUMLOCALNODES][SMPI_MAX_NUMLOCALNODES];
  
  volatile struct {
    volatile unsigned int first;
    volatile unsigned int last;
  } rqueues_limits[SMPI_MAX_NUMLOCALNODES][SMPI_MAX_NUMLOCALNODES];
  char pad2[SMPI_CACHE_LINE_SIZE];
  void *pad3;
  
  /* the receives queues */
  volatile char pool;
};

extern struct smpi_var smpi;
extern struct shared_mem * smpi_shmem;
extern int errno;


MPID_Device *MPID_SMP_InitMsgPass(int *, char ***, int, int);
extern int SMPI_Print_mode(FILE *, SMPI_PKT_T *);
extern int SMPI_Print_packet(FILE *, SMPI_PKT_T *);

extern void smpi_queue_send(void *, MPIR_SHANDLE *, int, int);
extern void smpi_post_send_bufferinplace(void *, int, int, int, int,
				  int, MPIR_SHANDLE *);
extern void smpi_post_send_queued(void *, MPIR_SHANDLE *shandle, int, int);
extern void smpi_do_get(int from, void *, void *, unsigned int);
extern void smpi_post_send_done_get(int, void *);
extern void smpi_post_send_rndv(void *, int, int, int, int, int,
				MPIR_SHANDLE  *);
extern void smpi_post_send_ok_to_send(int, MPIR_RHANDLE *);
extern int smpi_net_lookup(MPID_Device *, int);
extern int MPID_SMP_Check_incoming(MPID_Device *, MPID_BLOCKING_TYPE);
extern void smpi_init(void);
extern void smpi_finish(void);
extern void smpi_complete_send(unsigned int, unsigned int, unsigned int);
extern void smpi_complete_recv(unsigned int, unsigned int, unsigned int);
extern unsigned int smpi_able_to_send(int, int);
extern void smpi_malloc_assert(void *, char *, char *);

MPID_Protocol *MPID_SMP_Short_setup(void);
MPID_Protocol *MPID_SMP_Rndv_setup(void);

#endif /* _gmpi_smpi_h */
