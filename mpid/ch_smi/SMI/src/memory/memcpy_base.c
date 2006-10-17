/* $Id$ */

#include <pthread.h>

#include "env/smidebug.h"
#include "regions/address_to_region.h"
#include "memcpy_base.h"
#include "regions/idstack.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* exported globals */
size_t _smi_dma_offset_alignment, _smi_dma_size_alignment, _smi_dma_maxsize;

void *(*_smi_sci_memcpy)(void *dest, const void *src, size_t len);

/* local globals */
static pthread_mutex_t mcListMutex;
static pthread_mutex_t mcHandleMutex;

static int mcInitialized = FALSE;
static int mcDMAReadEnabled = FALSE;

static char *mcAlignBuffer;
static smi_memcpy_handle_t mcHandleRoot;

static int iPageSize;


void _smi_mc_handle_lock(void)
{
    SMI_LOCK(&mcHandleMutex); 
}

void _smi_mc_handle_unlock(void)
{
    SMI_UNLOCK(&mcHandleMutex);  
}

int _smi_mc_is_initialized(void) 
{
    return(mcInitialized);
}

int _smi_mc_dma_enabled(void)
{
    return(mcDMAReadEnabled);
}

char* _smi_mc_get_align_buffer(void)
{
    if (mcInitialized == TRUE)
		return(mcAlignBuffer);
    else
		return(NULL);
}

static void mc_handle_push(smi_memcpy_handle h)
{
    h->pNext = mcHandleRoot.pNext;
    mcHandleRoot.pNext = h;
}

static smi_memcpy_handle mc_handle_pop(void)
{
    smi_memcpy_handle h;
    
    h = mcHandleRoot.pNext;
    if (h != NULL)
		mcHandleRoot.pNext = h->pNext;
    
    return(h);
}

smi_memcpy_handle _smi_mc_create_handle(smi_memcpy_handle *pHandle)
{ 
	REMDSECTION("smi_mc_create_handle");
	smi_memcpy_handle h;
  
	if ( (h = mc_handle_pop()) == NULL) {
		DNOTICE("Allocating memory for memcpy-handle");
		h = (smi_memcpy_handle) malloc(sizeof(smi_memcpy_handle_t));
		ASSERT_A( (h!=NULL), "no memory left for operation", SMI_ERR_NOMEM);
	}
  
	memset(h,0,sizeof(smi_memcpy_handle_t));
	h->otid = SMI_MEMCPY_HANDLE_OTID;
	h->dma_used = FALSE;
	h->pHandle = pHandle;

	*pHandle = h;
	DSECTLEAVE; return(h);
}

void _smi_mc_destroy_handle(smi_memcpy_handle h)
{
	REMDSECTION("smi_mc_destroy_handle");
	sci_error_t sciError;

#ifndef NO_SISCI
	if (h->dma_used == TRUE) 
		rs_SCIRemoveDMAQueue(h->dq,0,&sciError);
#endif
	if (h->dma_thread_args != NULL)
		free (h->dma_thread_args);

	*h->pHandle = NULL;
	memset(h, 0, sizeof(smi_memcpy_handle_t));
  
	/* reuse handle later, instead of freeing memory */
	mc_handle_push(h);
}


int _smi_memcpy_init(void)
{
	DSECTION("_smi_memcpy_init");
	int wc;  
	/* sci_error_t error; */

	if (mcInitialized == TRUE)
		return(1);
  
	mcInitialized = TRUE;
  
	memset(&mcHandleRoot, 0, sizeof(smi_memcpy_handle_t));
	mcHandleRoot.pNext = NULL;
  
	SMI_INIT_LOCK(&mcListMutex);
	SMI_INIT_LOCK(&mcHandleMutex);

#ifndef NO_SISCI
	ALLOCATE (mcAlignBuffer, char *, _smi_StreambufSize);
#endif /* NO_SISCI */

#ifdef NO_SISCI
	/* SMP-only version, no remote memory */
	_smi_sci_memcpy = memcpy;
#else
	/* we have remote memory via SCI */
#ifdef SPARC
	_smi_sci_memcpy = bcopy;
#elif defined ALPHA
	_smi_sci_memcpy = memcpy;
#elif defined X86  
	/* determine the correct function by seeing if write-combining is enabled */
	SMI_Query(SMI_Q_SYS_WRITECOMBINING, 0, &wc);
	switch (wc) {
    
		/* recent performance measurements have shown this function to be extremely slow
		   as a default, we should use the copy function(s) provided by SISCI, Boris */
#if 0
	case SMI_WC_ENABLED:
		_smi_sci_memcpy = _smi_wc_memcpy;
		break;
#endif
	case SMI_WC_UNKNOWN:
		_smi_sci_memcpy = _smi_sisci_memcpy;
		break;
		/* Remember, 64-bit assembly code for Windows cannot use the older MMX, 3D Now! and 
		   x87 instruction extensions; they have been superseded by SSE/SSE2. */
#if !(defined(WIN32) && defined(_M_AMD64))
	case SMI_WC_DISABLED:
		_smi_sci_memcpy = _smi_mmx_memcpy;
		break;
#endif
	default:
		_smi_sci_memcpy = _smi_sisci_memcpy;
		break;
	}
#else
	/* default */
	_smi_sci_memcpy = _smi_sisci_memcpy;
#endif /* SPARC */
#endif /* NO_SISCI */

#if 0
	/* testing if DMA-read is enabled */
	/* XXX This testing method is not reliable - need to do a *real* test. */
	SCIEnqueueDMATransfer(NULL, NULL, NULL, 0, 0, 0, SCI_FLAG_DMA_READ, &error);
	mcDMAReadEnabled = (error != SCI_ERR_FLAG_NOT_IMPLEMENTED);
#else
	mcDMAReadEnabled = FALSE;
#endif
 
	SMI_Query (SMI_Q_SYS_PAGESIZE, 0, &iPageSize);
	SMI_Query (SMI_Q_SCI_DMA_OFFSET_ALIGN, 0, &_smi_dma_offset_alignment);
	SMI_Query (SMI_Q_SCI_DMA_SIZE_ALIGN, 0, &_smi_dma_size_alignment);
	SMI_Query (SMI_Q_SCI_DMA_MAXSIZE, 0, &_smi_dma_maxsize);

	return(0);
}

int _smi_memcpy_finalize(void)
{
    smi_memcpy_handle h;
	if (mcInitialized == FALSE)
		return(1);

	free (mcAlignBuffer);
  
	SMI_DESTROY_LOCK(&mcListMutex);
	SMI_DESTROY_LOCK(&mcHandleMutex);

    while( (h = mc_handle_pop()) != NULL)
		free(h);
    
	mcInitialized = FALSE;
  
	return(0);
}


/* This function detects the type of the memory which is mapped to pMem.
   There may be multiple regions for one adress, but this is not relevant
   as they are all of the same type. */
memtype_t _smi_detect_memtype(void* pMem)
{
	shseg_t* pTemp = _smi_addr_to_shseg(pMem);
  
	if (pTemp == NULL)
		return(mt_lp);

	if (_smi_all_on_one == TRUE) {
#ifdef WIN32
		return(mt_lswin32);
#else
		return(mt_lsunix);
#endif
	}
  
	if(pTemp->owner == _smi_my_proc_rank)
		return(mt_lsown);
  
	if(pTemp->machine == _smi_my_machine_rank)
		return(mt_ls);
  
	return(mt_rs);
}

memtype_t _smi_detect_memtype_rdma(int regid)
{
	shseg_t* pTemp = _smi_regid_to_shseg(regid);
  
	if (pTemp == NULL)
		return(mt_lp);

	if (_smi_all_on_one == TRUE) {
#ifdef WIN32
		return(mt_lswin32);
#else
		return(mt_lsunix);
#endif
	}
  
	if(pTemp->owner == _smi_my_proc_rank)
		return(mt_lsown);
  
	if(pTemp->machine == _smi_my_machine_rank)
		return(mt_ls);
  
	return(mt_rs);
}


/* this function returns TRUE if pMem is in the adress-space of an sci-segment created by
   this process */
int _smi_mt_is_localseg(void *pMem) {
    memtype_t mt = _smi_detect_memtype(pMem);

    return (mt == mt_lsown);
}


/* this function returns TRUE if pMem is in the adress-space of an sci-segment on a
   remote node */
int _smi_mt_is_remoteseg(void *pMem) {
    memtype_t mt = _smi_detect_memtype(pMem);

    return(mt == mt_rs);
}


/* this function detects the type of the memtransfer operation */
int _smi_detect_memtransfertype(void* dest, void* src)
{
	DSECTION("_smi_detect_memtransfertype");
	memtype_t mtDest;
	memtype_t mtSrc;
  
	DSECTENTRYPOINT;

	mtDest = _smi_detect_memtype(dest);
	mtSrc = _smi_detect_memtype(src);
  
	/* 
	   This makes local scisegments created by this processe, treated like scisegments
	   created by other processes on the same machine
	*/
	if (mtDest == mt_lsown) mtDest = mt_ls;
	if (mtSrc == mt_lsown) mtSrc = mt_ls;

	/* This makes unix and win32 shared mem treated like standard memory */
	if ((mtDest == mt_lsunix) || (mtDest == mt_lswin32))
		mtDest = mt_lp;
	if ((mtSrc == mt_lsunix) || (mtSrc == mt_lswin32))
		mtSrc = mt_lp;
  
	switch (mtSrc) {
	case mt_lp:
		switch (mtDest) {
		case mt_lp:
			DNOTICE("SMI_MEMCPY_LP_LP");
			DSECTLEAVE
				return(SMI_MEMCPY_LP_LP);
			break;
		case mt_ls:
			DNOTICE("SMI_MEMCPY_LP_LS");
			DSECTLEAVE
				return(SMI_MEMCPY_LP_LS);
			break;
		case mt_rs:
			DNOTICE("SMI_MEMCPY_LP_RS");
			DSECTLEAVE
				return(SMI_MEMCPY_LP_RS);
			break;
		}
		break;
	case mt_ls:
		switch (mtDest) {
		case mt_lp:
			DNOTICE("SMI_MEMCPY_LS_LP");
			DSECTLEAVE
				return(SMI_MEMCPY_LS_LP);
			break;
		case mt_ls:
			DNOTICE("SMI_MEMCPY_LS_LS");
			DSECTLEAVE
				return(SMI_MEMCPY_LS_LS);
			break;
		case mt_rs:
			DNOTICE("SMI_MEMCPY_LS_RS");
			DSECTLEAVE
				return(SMI_MEMCPY_LS_RS);
			break;
		}
		break;
	case mt_rs:
		switch (mtDest) {
		case mt_lp:
			DNOTICE("SMI_MEMCPY_RS_LP");
			DSECTLEAVE
				return(SMI_MEMCPY_RS_LP);
			break;
		case mt_ls:
			DNOTICE("SMI_MEMCPY_RS_LS");
			DSECTLEAVE
				return(SMI_MEMCPY_RS_LS);
			break;
		case mt_rs:
			DNOTICE("SMI_MEMCPY_RS_RS");
			DSECTLEAVE
				return(SMI_MEMCPY_RS_RS);
			break;
		}
		break;
	}

	DSECTLEAVE
		return (0);
}

smi_error_t _smi_memwait(smi_memcpy_handle h)
{
    DSECTION("_smi_memwait");
    sci_error_t sciError;
    
    DSECTENTRYPOINT;
    
    /* only with DMA working you will need this code  - Scali SISCI won't */
#if defined  NO_SISCI || defined SCALI_SISCI
    DSECTLEAVE; return(SMI_ERR_NOTIMPL);
#else
    ASSERT_R(_smi_all_on_one==false, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);

    DNOTICE("waiting for job");
    DNOTICEP("src:", h->mc_src);
    DNOTICEP("dest:", h->mc_dest);
    DNOTICEI("size:", h->mc_size);

    SCIWaitForDMAQueue(h->dq,SCI_INFINITE_TIMEOUT,0,&sciError);
    if (sciError != SCI_ERR_OK) {
		DPROBLEMP("SCIWaitForDMAQueue returned SISCI error", sciError);
		DSECTLEAVE; return(SMI_ERR_NOTPOSTED);
    }
    
    if (!(h->mc_flags & SMI_MEMCPY_NOVERIFY)) {
		if ( SMI_Check_transfer_proc(h->dma_rmt_proc, (h->mc_flags & SMI_MEMCPY_NOBARRIER) ? 
									 SMI_CHECK_FAST : 0) == SMI_ERR_TRANSFER) {
			smi_memcpy_handle_t retry_h;
			memset (&retry_h, 0, sizeof(smi_memcpy_handle_t));
			DWARNING ("Transfer error, repeating DMA transfer");	  
			_smi_dma_transfer(h->dma_remoteseg, h->dma_offset, h->dma_localadr,
							  h->mc_size, h->dma_direction, (smi_memcpy_handle)&retry_h);
			DSECTLEAVE; return (_smi_memwait((smi_memcpy_handle)&retry_h));	  
		}
    }
    
    DSECTLEAVE; return(SMI_SUCCESS);
#endif
}


smi_error_t _smi_dma_transfer(int remote_region_id, size_t offset, void *localadr, 
						  size_t size, int direction, smi_memcpy_handle h)
{
    DSECTION("_smi_dma_transfer");
    shseg_t *segLocal = _smi_range_to_shseg(localadr, size);
    shseg_t *segRemote = _smi_regid_to_shseg(remote_region_id);
    size_t local_offset, remote_offset; 
#if USE_DMA_THREAD
	int retval;
#endif
    size_t iQueueSize;
	int iQueueOversize = 0;
    sci_error_t sciError;

    DSECTENTRYPOINT;
    
#if defined NO_SISCI || defined SCALI_SISCI
    DSECTLEAVE; return SMI_ERR_NOTIMPL;
#else
    h->dma_remoteseg = remote_region_id;
    h->dma_rmt_proc  = segRemote->owner;
    h->dma_offset    = offset;
    h->dma_localadr  = localadr;
    h->dma_direction = direction;
    h->mc_size = size;

    DNOTICEP("local adress :", localadr);
    DNOTICEI("rmt region id:", remote_region_id);
    DNOTICEI("rmt rank:    :", h->dma_rmt_proc);
    DNOTICEI("size         :", h->mc_size);

    if (size == 0) {
		DSECTLEAVE; return (SMI_SUCCESS);
    }

    /* make the queue big enough for all possible scenarios */
    iQueueSize = (size > _smi_dma_maxsize) ? _smi_dma_maxsize/iPageSize : size/iPageSize;
    iQueueSize = iQueueSize*2 + 4;
    
    if (h->dma_used == FALSE) {
		DNOTICEI("creating new DMA queue, nbr entries:", iQueueSize);
		rs_SCICreateDMAQueue(segLocal->fd, &h->dq, 0, iQueueSize, 0, &sciError);
		RETURN_IF_FAIL("Could not create DMAQueue",sciError, SMI_ERR_OTHER);
		h->dma_used = TRUE;
		h->otid = SMI_MEMCPY_HANDLE_OTID; 
		h->dma_entries = 0;
    } 
#if 0
    /* XXX it is ok to use a handle with an existing, but empty DMA queue - 
       required for re-transmissions! */
    else {
		if (h->dma_entries <= 0) {
			DPROBLEM("handle claims dma-usage but has invalid count of dma_entries");
			DSECTLEAVE; return (SMI_ERR_PARAM);
		}
    }
#endif

    local_offset = (size_t)localadr - (size_t)(segLocal->address) + segLocal->offset;
    remote_offset = offset;
    DNOTICEI("DMA local offset :",local_offset);
    DNOTICEI("DMA remote offset:",remote_offset);

    DNOTICEI("dma_maxsize =", _smi_dma_maxsize);
    
    
#if USE_DMA_THREAD
    /* To avoid blocking the caller in case we need to split up a DMA transfer,
       we start a thread to perform the transfer and thus are able to return immeadeletly. */
    if (h->mc_size <= _smi_dma_maxsize) 
#endif
		{
			dma_thread_args_t thrd_args;
			thrd_args.mc_h = h;
			thrd_args.loc_offset = local_offset;
			thrd_args.rmt_offset = remote_offset;
			thrd_args.loc_smi_sgmt = segLocal;
			thrd_args.rmt_smi_sgmt = segRemote;
			thrd_args.direction = direction;
			thrd_args.dma_queue_size = iQueueSize;
			thrd_args.dma_thread_id   = 0;

			h->dma_thread_args = NULL;
			_smi_dma_thread (&thrd_args);
		}
#if USE_DMA_THREAD
    else {
		dma_thread_args_t *thrd_args;
		ALLOCATE (thrd_args, dma_thread_args_t *, sizeof(dma_thread_args_t));
		h->dma_thread_args = thrd_args;

		thrd_args->mc_h = h;
		thrd_args->loc_offset = local_offset;
		thrd_args->rmt_offset = remote_offset;
		thrd_args->loc_smi_sgmt = segLocal;
		thrd_args->rmt_smi_sgmt = segRemote;
		thrd_args->direction = direction;
		thrd_args->dma_queue_size = iQueueSize;

		retval = rs_pthread_create (&thrd_args->dma_thread_id, NULL, _smi_dma_thread, thrd_args);
		ASSERT_A (retval == 0, "could not create DMA transfer thread.", -1);
    }
#endif

    DSECTLEAVE; return (SMI_SUCCESS);
#endif
}

#ifdef DOLPHIN_SISCI
static void *_smi_dma_thread (void *thread_arg)
{
    dma_thread_args_t *dma_args = (dma_thread_args_t *)thread_arg;
    smi_memcpy_handle h = dma_args->mc_h;
    sci_error_t SISCI_err;
    size_t remaining = h->mc_size;

    DSECTION ("dma_thread");

    while (remaining > 0) {	
		if (remaining > _smi_dma_maxsize) {	    
			h->mc_size = _smi_dma_maxsize;
			remaining -= _smi_dma_maxsize;
		} else {
			h->mc_size = remaining;
			remaining = 0;
		}

		DNOTICE("enqueueing DMA transfers");
		SCIEnqueueDMATransfer(h->dq, dma_args->loc_smi_sgmt->localseg, dma_args->rmt_smi_sgmt->segment, 
							  (unsigned int)dma_args->loc_offset, (unsigned int)dma_args->rmt_offset, (unsigned int)h->mc_size, dma_args->direction, 
							  &SISCI_err);
		if (SISCI_err != SCI_ERR_OK) {
			/* This should never happen! */
			DPROBLEMP("SCIEnqueueDMATransfer returned with SISCI error", SISCI_err);
#if 0
			/* XXX why destroy here? */
			_smi_mc_destroy_handle(h);
#endif
			h->dma_err = SMI_ERR_OTHER;
			DSECTLEAVE; return NULL;
		}
#if 0
		/* XXX It depends on the kind of the source and target region (contiguous or user-registered
		   (= page-size fragmented) memory, how many entries we really need for a transfer, and 
		   how big each single transfer of these really is. 
		   To be on the safe side, assume the worst case scenario for both cases: limit the size of
		   a single DMA enqueue to the max. size (as given by the DMA engine), and assume that 
		   the maximum of DMA queue entries has to be used. */
		h->dma_entries++;
	
		if (((h->mc_flags & SMI_MEMCPY_ENQUEUE) == 0) || (h->dma_entries >= iQueueSize) ){
#endif
			DNOTICE("posting DMA queue");
			/* Repeatly post the queue - it has to succeed. 
			   XXX Limit the amount of repeats? Introduce a delay? Locate the specific problem? */
			do {
				SCIPostDMAQueue (h->dq, NULL, NULL, 0, &SISCI_err);
				if (SISCI_err != SCI_ERR_OK) {
					DPROBLEMP("SCIPostDMAQueue returned with SISCI error", SISCI_err);
				}
			} while (SISCI_err != SCI_ERR_OK);
			h->dma_entries = 0;
#if 0
		}
#endif
	
		if (remaining > 0) {
			DNOTICE("waiting for pending queue");
			if (_smi_memwait(h) != SMI_SUCCESS) {
				DPROBLEM("unknown error");
				h->dma_err = SMI_SUCCESS;
				DSECTLEAVE; return NULL;
			}	    
			rs_SCIRemoveDMAQueue(h->dq, 0, &SISCI_err);
			rs_SCICreateDMAQueue(dma_args->loc_smi_sgmt->fd, &h->dq, 0, dma_args->dma_queue_size, 
								 0, &SISCI_err);

			/* recalculate offsets */
			dma_args->loc_offset += _smi_dma_maxsize;
			dma_args->rmt_offset += _smi_dma_maxsize;
			h->dma_offset += _smi_dma_maxsize;
			h->dma_localadr += _smi_dma_maxsize;
		}
    }
    
    h->dma_err = SMI_SUCCESS;
    DSECTLEAVE; return NULL;
}

#endif

/*
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
