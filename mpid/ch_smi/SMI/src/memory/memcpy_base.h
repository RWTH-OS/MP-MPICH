/* $Id$ */

#ifndef _SMI_MEMCPY_BASE_H_
#define _SMI_MEMCPY_BASE_H_

#include "env/general_definitions.h"

/* If the SCI driver does only allow DMA between segments which are allocated using
   the same SCI descriptor (old Dolphin drivers), this needs to be set to '1' */
#define DMA_RECONNECT 0

#ifdef DISABLE_THREADS
#define USE_DMA_THREAD 0
#else
/* change this define here to enable/disable the thread for long DMA transfers. */
#define USE_DMA_THREAD 0
#endif

#define SMI_MEMCPY_HANDLE_OTID    815        /* correct OTID for an MEMCPY-Handle       */
#define SMI_MEMCPY_DMAQUEUE_SIZE  20         /* default number of entries for a DMA queue */
#define SMI_MEMCPY_DMAQUEUE_LIMIT 1170       /* maximum nbr of entries for a DMA queue */   
#define SMI_MEMCPY_DMA_MINTRANS   (4*1024)   /* minimal size for DMA-transfer (lower limit for performance) */
#define SMI_MEMCPY_DMA_MINTRANS   (4*1024)   /* minimal size for DMA-transfer */

/* This type indikates what kind of memory is mapped to a certain address */
typedef enum {mt_lp,           /* standard memory */
	      mt_lsunix,       /* unix shared memory (IPC) */
	      mt_lswin32,      /* win32 shared memory (Systemswapfile) */
	      mt_ls,           /* sci segment on this machine, _smi_allocated by another process */
	      mt_lsown,        /* sci segment on this machine, _smi_allocated by this process */
	      mt_rs            /* sci segment on remote node */
} memtype_t;

/* args for the thread which is performing long DMA transfers. */
typedef struct {
    smi_memcpy_handle mc_h;

    size_t loc_offset;
    size_t rmt_offset;
    shseg_t *loc_smi_sgmt;
    shseg_t *rmt_smi_sgmt;
    int direction;
    size_t dma_queue_size;

    SMI_THREAD_T dma_thread_id;
} dma_thread_args_t;

/* exported variables */
extern size_t _smi_dma_offset_alignment, _smi_dma_size_alignment;

/* prototypes */
int _smi_memcpy_init(void);
int _smi_memcpy_finalize(void);

int _smi_mc_is_initialized(void);
int _smi_mc_dma_enabled(void);
char* _smi_mc_get_align_buffer(void);

void _smi_mc_handle_lock(void);
void _smi_mc_handle_unlock(void);

smi_memcpy_handle _smi_mc_create_handle(smi_memcpy_handle* pHandle);
void _smi_mc_destroy_handle(smi_memcpy_handle h);

#ifndef NO_SISCI
sci_remote_segment_t _smi_mc_GetConnectWithFD(sci_desc_t fd, shseg_t* segRemote); 
int _smi_mc_SetConnectWithFD(sci_desc_t fd, sci_remote_segment_t rsegConnect, shseg_t* segRemote); 
int _smi_mc_ResetOriginalConnect(shseg_t* segRemote);
#endif

shseg_t* _smi_get_seginfo(void* pMem); 
shseg_t* _smi_get_seginfo_rdma(int regid); 

memtype_t _smi_detect_memtype(void* pMem);
memtype_t _smi_detect_memtype_rdma(int regid);

int _smi_mt_is_localseg(void *pMem);
int _smi_mt_is_remoteseg(void *pMem);
int _smi_detect_memtransfertype(void* dest, void* src);

smi_error_t _smi_dma_transfer(int remote_region_id, size_t offset, void *localadr, 
			  size_t size, int Direction, smi_memcpy_handle h);
static void *_smi_dma_thread (void *thread_arg);

/* this is the function to copy data to memory on remote nodes (via SCI)*/
extern void *(*_smi_sci_memcpy)(void *dest, const void *src, size_t len);	  

void *_smi_mmx_memcpy(void *dst, const void *src, size_t len);
void *_smi_wc_memcpy(void *dest, const void *src, size_t len);
void *_smi_sisci_memcpy(void *dest, const void *src, size_t len);

smi_error_t _smi_memwait(smi_memcpy_handle h);

#endif
