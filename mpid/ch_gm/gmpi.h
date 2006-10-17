/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_h
#define _gmpi_h

#include <assert.h>

#include "mpichconf.h"
#include "mpich-mpid.h"

#include "gm.h"


extern int MPID_GM_rank;
extern int MPID_GM_size;

#define GMPI_VERSION 0x10205106

#ifndef GM_API_VERSION
#define GM_API_VERSION GM_API_VERSION_1_5
#endif

#if (GM_API_VERSION >= 0x200)
#define GMPI_GM2 1
#else
#define GMPI_GM2 0
#endif


/* tunable parameters */
#if GM_DISABLE_REGISTRATION
#define GMPI_EAGER_SIZE_DEFAULT 1024*16-96
#define GMPI_MAX_PUT_LENGTH 1024*64
#define GMPI_INITIAL_DMA_SEND_BUF 256
#define GMPI_MAX_RECV_TOKENS 1000
#define GMPI_MAX_PUT_IN_PIPE 16
#define GMPI_ENABLE_REG_CACHE 0
#else
#define GMPI_EAGER_SIZE_DEFAULT 1024*16-96
#define GMPI_MAX_PUT_LENGTH 1024*1024
#define GMPI_INITIAL_DMA_SEND_BUF 256
#define GMPI_MAX_RECV_TOKENS 1000
#define GMPI_MAX_PUT_IN_PIPE 32
#define GMPI_ENABLE_REG_CACHE 1
#endif

#define GMPI_MAX_GM_PORTS 32
#define GMPI_MAX_GM_BOARDS 8
#define GMPI_INIT_TIMEOUT (3*60*1000) /* 3 minutes */
#define GMPI_MAX_DELAYED_SEND_QUEUE_SIZE 8*1024*1024
#define GMPI_MALLOC_HOOK_CHECK_SIZE 4*1024*1024

/* For unlucky Solaris users */
#define GMPI_BOUNCE_BUFFERS 100

#undef GMPI_PROGRESSION
#define GMPI_PROGRESSION 0

/* debug */
#define GMPI_DEBUG_CHECKSUM 0
#define GMPI_DEBUG_ASSERT 0
#define GMPI_DEBUG_REG_CACHE 0
#define GMPI_DEBUG_FIFO_SEND 0
#define GMPI_DEBUG_GM_SEND 0
#define GMPI_DEBUG_GM_RECV 0
#define GMPI_DEBUG_REGISTRATION 0
#define GMPI_DEBUG_DMA_MEMORY 0
#define GMPI_DEBUG_OUTPUT_FILE 0

/* GM sizes */
#define GMPI_PKT_GM_SIZE 30

/* misc */
#define GMPI_DMA_SEND_BUF 2
#define GMPI_MALLOC_SEND_BUF 4
#define GMPI_PUT_DATA 8

#define GMPI_PROGRESSION_LOCK()
#define GMPI_PROGRESSION_UNLOCK()
#define GMPI_PROGRESSION_INIT()
#define GMPI_PROGRESSION_FINISH()
#define GMPI_PROGRESSION_LOCK_DEBUG_REGISTRATION()
#define GMPI_PROGRESSION_UNLOCK_DEBUG_REGISTRATION()


#include <sys/types.h>
#include <netinet/in.h>

#include "gm_auto_config.h"
#include "mpid.h"
#include "gmpi_debug_checksum.h"
#include "req.h"
#include "dev.h"

#include <netinet/in.h>

struct gmpi_send_buf
{
  unsigned int type;
  unsigned int dest;
  unsigned int length;
  unsigned int register_length;
  void * source_ptr;
  void * target_ptr;
  MPIR_SHANDLE * shandle;
  struct gmpi_send_buf * next;
  unsigned int buffer; 
};


struct gmpi_var
{
  unsigned int my_global_node_id;
  unsigned int send_tokens;
  unsigned int max_send_tokens;
  unsigned int recv_tokens;
  unsigned int max_recv_tokens;
  unsigned int send_buf_fifo_queued;
  unsigned int send_dma_buf_pool_remaining;
  unsigned int malloc_send_buf_allocated;
  unsigned int pending_put_callback;
  unsigned int pending_send_callback;
  unsigned int unexpected_short;
  unsigned int malloc_hook_flag;
  unsigned int eager_size;
  unsigned int shmem;
  unsigned int magic;
  unsigned int mpd;
  char my_hostname[256];
  struct sockaddr_in master_addr;
  struct sockaddr_in slave_addr;
  FILE * debug_output_filedesc;
  union gm_recv_event *(*gm_receive_mode)(struct gm_port *);

  struct gmpi_send_buf * send_buf_fifo_head;
  struct gmpi_send_buf * send_buf_fifo_tail;
  struct gmpi_send_buf * send_dma_buf_free;

  unsigned int *global_node_ids;  /* Global node ids */
  unsigned int *local_node_ids;   /* Local node ids */
  char **host_names;           /* names of machines */
  char **exec_names;           /* names of executables */
  unsigned int *numa_nodes;    /* numa node numbers */
  unsigned int *mpi_pids;      /* PID of remote processes */
  unsigned int *port_ids;      /* port ids */
  unsigned int *board_ids;     /* board/unit number */
  unsigned int *pending_sends; /* pending sends (timeout) */
  unsigned int *dropped_sends; /* dropped sends (timeout) */
};


extern struct gmpi_var gmpi;
extern struct gm_port * gmpi_gm_port;

unsigned int gmpi_use_interval(gm_up_t, unsigned int);
void gmpi_unuse_interval(gm_up_t, unsigned int);
void gmpi_clear_interval(gm_up_t, unsigned int);
void gmpi_clear_all_intervals(void);
void gmpi_regcache_init(void);

void * gmpi_allocate_packet(unsigned int, struct gmpi_send_buf **);
void gmpi_send_packet(struct gmpi_send_buf *, unsigned int);
void gmpi_queue_packet_register(struct gmpi_send_buf *, void *, 
				unsigned int, unsigned int);
void gmpi_put_data(MPIR_SHANDLE *, unsigned int, unsigned int,
		   void *, void *);
void gmpi_init(int *, char ***);
void gmpi_finish(MPID_Device *);
int MPID_CH_Check_incoming(MPID_Device *, MPID_BLOCKING_TYPE);
void gmpi_abort(int);
void gmpi_malloc_assert(void *, char *, char *);
void * gmpi_dma_alloc(unsigned int);
void gmpi_dma_free(void *);


#define GMPI_DRAIN_GM_INCOMING

#if GMPI_DEBUG_ASSERT
#define gmpi_debug_assert assert
#else
#define gmpi_debug_assert(a) 
#endif


#include "gmpi_noreg.h"
#include "gmpi_debug_registration.h"
#include "gmpi_debug_regcache.h"
#include "gmpi_debug_dma_memory.h"
#include "gmpi_debug_gm_send_recv.h"
#include "gmpi_debug_fifo_send.h"

#endif /* gmpi_h */
