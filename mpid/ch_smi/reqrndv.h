/* $Id$ 
 
   ch-smi specific defintion for rndv-information type in handles. */

#ifndef _MPID_REQRNDV_H
#define _MPID_REQRNDV_H

#include <sys/types.h>
#ifndef WIN32
#include <pthread.h>
#endif

#include "smitypes.h"
#ifndef WIN32
#include "smi_conf.h"
#endif
/* XXX This is a redundant define from smidev.h, but we can't include this here. */
#if defined MPID_USE_DEVTHREADS && !defined WIN32
#define RNDV_LOCK_T pthread_mutex_t
#else
#define RNDV_LOCK_T int
#endif

typedef enum { 
    /* default rendez-vous mode */
    RNDV_SYNC,                        /* synchronous rendez-vous via PIO */
    /* special send modes */
    RNDV_ASYNC_SEND_PIO,              /* asynchronous, using PIO from user buffer */
    RNDV_ASYNC_SEND_DMA,              /* asynchronous, DMA from intermediate buffer */
    RNDV_ASYNC_SEND_DMAZC,            /* asynchronous, DMA from registered user buffer
                                         (de-registering required when send is complete) */
    RNDV_ASYNC_SEND_DMAZC_PERS,       /* asynchronous, DMA from registered user buffer 
                                         (persistent) */
    RNDV_SYNC_SEND_DMA,               /* synchronous rendez-vous via DMA (mainly as fallback from PIO) */
    RNDV_SYNC_SEND_DMAZC,             /* synchronous, DMA from registered user buffer
                                         (de-registering required when send is complete) */
    RNDV_SYNC_SEND_DMAZC_PERS,        /* synchronous, DMA from registered user buffer 
                                         (persistent) */
    /* special receive modes */
    RNDV_ASYNC_RECV,                  /* asynchronous, DMA/PIO recv into intermediate buffer */
    RNDV_ASYNC_RECV_ZC,               /* asynchronous, DMA (PIO) recv into registered user buffer 
                                         (de-registering required when recv is complete) */
    RNDV_ASYNC_RECV_ZC_PERS,          /* asynchronous, DMA (PIO) recv into registered user buffer 
                                         (persistent) */
    RNDV_SYNC_RECV_ZC,                /* synchronous, DMA (PIO) recv into registered user buffer 
                                         (de-registering required when recv is complete) */
    RNDV_SYNC_RECV_ZC_PERS            /* synchronous, DMA (PIO) recv into registered user buffer 
                                         (persistent) */
} MPID_SMI_rndv_mode_t;


typedef struct _rndv_handle_info {
  MPID_SMI_rndv_mode_t mode;
  unsigned int         flags;

  void                *dest_addr;  
  unsigned int         len_local;
  int                  smi_regid_src;
  int                  smi_regid_dest;

  RNDV_LOCK_T         *complete_lock;

  void                *dma_outbuf_addr;
  int                  dma_outbuf_len;
  volatile int         dma_precopy_busy;

  longlong_t           sync_delay;

  /* the original finish function */
  int (*finish)(void * /* will be interpreted as 'union MPIR_HANDLE *' */);
} MPID_SMI_RNDV_info;

typedef MPID_SMI_RNDV_info* MPID_RNDV_T;

#endif
