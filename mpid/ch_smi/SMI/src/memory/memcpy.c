/* $Id$ */

#include <string.h>
#include "env/smidebug.h"
#include "env/error_count.h"
#include "regions/address_to_region.h"
#include "regions/region_layout.h"
#include "memcpy.h"

/* if the copy function for remote memory should be determined by querying the
   write-combining, set this to 1 */
#define QUERY_WC 1

extern int* _smi_sci_rank;

smi_error_t SMI_Memcpy (void *dest, void *src, size_t size, int flags)
{
    DSECTION("SMI_Memcpy");
    int iErrInitial=0;
    int do_verify, do_barrier, do_force_dma, do_align;
    smi_error_t RetVal = SMI_SUCCESS;
    
    DSECTENTRYPOINT;
    
#ifdef HAVE_SCIMEMWRITE
    /* The most important thing about this shortcut is that it is fast. Therefore,
       we omit all fancy checks. */
    if (flags & SMI_MEMCPY_FAST) {
	sci_error_t sisci_err;
	SCIMemWrite (src, dest, size, 0 /* flags */, &sisci_err);
	return (sisci_err == SCI_ERR_OK) ? SMI_SUCCESS : SMI_ERR_TRANSFER;
    }
#endif
    
    if (_smi_mc_is_initialized() == FALSE) {
	DERROR("usage of memcpy without initialization!");
	SMI_Abort(-1);
    }
    
    DNOTICEP("Source:",src);
    DNOTICEP("Destination:",dest);
    DNOTICEI("Size: ",size);
    
    if((flags & ~SMI_MEMCPY_FLAGS_ALL) != 0) {
	DSECTLEAVE;
	return(SMI_ERR_PARAM);
    }
    
    if((flags & SMI_MEMCPY_MTFLAGS_ALL) == 0)
	flags += _smi_detect_memtransfertype(dest,src);
    
    DNOTICEI("Flags:",flags);
    
    do_verify = ((flags & SMI_MEMCPY_NOVERIFY) == 0);
    do_barrier = ((flags & SMI_MEMCPY_NOBARRIER) == 0);
    do_force_dma = ((flags & SMI_MEMCPY_FORCE_DMA) != 0);
    do_align = ((flags & SMI_MEMCPY_ALIGN) != 0);
    flags = flags & ~(SMI_MEMCPY_NOVERIFY|SMI_MEMCPY_NOBARRIER|SMI_MEMCPY_FORCE_DMA|SMI_MEMCPY_ALIGN);
    
    DNOTICEI("Flags after flagclear:", flags);
    
    switch (flags) {
    case SMI_MEMCPY_LP_LP:
    case SMI_MEMCPY_LP_LS:
	DNOTICE("Copying with memcpy()");
#ifndef SPARC
#if 0
	/* different copy routines give best results for different sizes */
	/* XXX experimental - need to distinguish different OS, too ? */
	if ((size < _smi_Cachelinesize) 
	    || ((size > _smi_1stLevel_Cachesize) && (size < _smi_2ndLevel_Cachesize)))
	    memcpy(dest, src, size);
	else
	    _smi_mmx_memcpy(dest, src, size);
#else
	memcpy(dest, src, size);
#endif
#else
	memcpy(dest, src, size);
#endif /* SPARC */
	DSECTLEAVE;
	  return(SMI_SUCCESS);

    case SMI_MEMCPY_RS_LP:
    case SMI_MEMCPY_RS_LS:
    case SMI_MEMCPY_LP_RS:
    case SMI_MEMCPY_LS_RS:
	DNOTICE("Copying with sci_memcpy()");
	do {
	    DNOTICEP("_smi_sci_memcpy:",_smi_sci_memcpy);
	    _smi_sci_memcpy(dest, src, size);
	    
#ifndef NO_SISCI
	    if (do_align) {
		if (size % _smi_StreambufSize) {
		    memcpy(((char *)dest)+size, _smi_mc_get_align_buffer(), 
			   _smi_StreambufSize-(size%_smi_StreambufSize));
		}
	    }
	    if (do_verify) {
		RetVal = SMI_Check_transfer(do_barrier ? SMI_CHECK_FAST : 0);
	    } else {
		if (do_barrier) {
		    _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1);
		}
	    }
#else 
	    RetVal = SMI_SUCCESS;
#endif /* NO_SISCI */
	    
	    
	} while (RetVal != SMI_SUCCESS);
	
	DSECTLEAVE;
	return(RetVal);
    }
    
    DWARNING("No valid Flag");
    DSECTLEAVE;
    return(SMI_ERR_PARAM);
}

smi_error_t SMI_Imemcpy (void *dest, void *src, size_t size, int flags, smi_memcpy_handle* pHandle) {
    DSECTION("SMI_Imemcpy");
    smi_memcpy_handle h;
    /*sci_error_t sciError;*/
    /* shseg_t* segLocal; */
    shseg_t* segRemote;
    /*int DirectionFlag;*/
	size_t rest, src_disalign, dest_disalign;
    size_t offset;
    smi_error_t error;
    
    DSECTENTRYPOINT;
    
    if (_smi_mc_is_initialized() == FALSE) {
	DPROBLEM("usage of memcpy without initialization!");
	return (SMI_ERR_NOINIT);
    }
    
    ASSERT_R((_smi_all_on_one==false),"Not implemented in SMP-Mode",SMI_ERR_NOTIMPL);
    
    DNOTICE("Checking flags");
    if((flags & (~SMI_MEMCPY_FLAGS_ALL)) != 0) {
	DSECTLEAVE; return(SMI_ERR_PARAM);
    }
    
    DNOTICEP("Source:",src);
    DNOTICEP("Destination:",dest);
    DNOTICEI("Size:",size);
    
    _smi_mc_handle_lock();
    
    /* check if the handle is already in use (for enqueueing multiple transfers)
       of if we need to get a new one */
    h = *pHandle;
    if ((h != NULL) && (h->otid == SMI_MEMCPY_HANDLE_OTID) ) {
	if (*(h->pHandle) != *pHandle) {
	    DPROBLEM("Not a valid handle");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return(SMI_ERR_PARAM);
	}
	DNOTICE("The handle already exists!");
	if ( (h->dma_used == TRUE) && (h->dma_entries > 0)) {
	    DNOTICE("The handle contains an nonposted queue");
	} 
	else {
	    DPROBLEM("The handle is in use");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return(SMI_ERR_PARAM);
	}
    } 
    else {
	h = _smi_mc_create_handle(pHandle);
	if (h == NULL) { 
	    _smi_mc_handle_unlock();
	    DPROBLEM("Could not allocate memcpy-handle");
	    DSECTLEAVE; return(SMI_ERR_OTHER);
	}
	h->mc_dest  = dest;
	h->mc_src   = src;
	h->mc_size  = size;
	h->mc_flags = flags;
    } 
    
#ifndef DOLPHIN_SISCI
    DNOTICE ("DMA only supported by Dolphin's SISCI - using PIO");
    h->dma_used = FALSE;
    _smi_mc_handle_unlock();
    DSECTLEAVE;
    return SMI_Memcpy (dest, src, size, flags & (~SMI_MEMCPY_ENQUEUE));
#else
    
    if((flags & SMI_MEMCPY_MTFLAGS_ALL) == 0)
	flags += _smi_detect_memtransfertype(dest,src);
    
    if (!(flags & SMI_MEMCPY_FORCE_DMA)) {
	DNOTICE("Checking if size >= SMI_MEMCPY_DMA_MINTRANS");
	if ( size < SMI_MEMCPY_DMA_MINTRANS) {
	    h->otid = SMI_MEMCPY_HANDLE_OTID;
	    h->dma_used = FALSE;
	    _smi_mc_handle_unlock();
	    DSECTLEAVE;
	    return (SMI_Memcpy(dest, src, size, flags & (~SMI_MEMCPY_ENQUEUE)));
	}
    }
    
    DNOTICE("Checking if LS<>RS");
    if (!(flags & (SMI_MEMCPY_LS_RS|SMI_MEMCPY_RS_LS))) {
	h->dma_used = FALSE;
	_smi_mc_handle_unlock();
	DSECTLEAVE;
	return (SMI_Memcpy(dest, src, size, flags & (~SMI_MEMCPY_ENQUEUE)));
    }
    
    /* check alignments */
    rest = size % _smi_dma_size_alignment;
    if (rest != 0) {
	size -= rest;
	DWARNINGI("Misaligned size, aligning with memcpy():", rest);
	memcpy(((char*)dest) + size, ((char*)src)+size, rest);
    }
    src_disalign  = (size_t)src % _smi_dma_offset_alignment;
    dest_disalign = (size_t)dest % _smi_dma_offset_alignment;
    if (src_disalign || dest_disalign) {
      if (src_disalign != dest_disalign) {
	DWARNING("Fully misaligned offset, copying with SMI_Memcpy()");
	_smi_mc_handle_unlock();
	DSECTLEAVE; return SMI_Memcpy (dest, src, size, flags);
      } else {
	DWARNINGI("Misaligned src or dst offset, aligning with memcpy():", rest);
	memcpy(((char*)dest), ((char*)src), src_disalign);
	size -= src_disalign;
	src  = (char*)src + src_disalign;
	dest = (char*)dest + src_disalign;
      }
    }
    
    if (flags & SMI_MEMCPY_LS_RS) {
	if (!_smi_mt_is_localseg(src) || !_smi_mt_is_remoteseg(dest)) {
	    if (!h->dma_used || h->dma_entries == 0)
		_smi_mc_destroy_handle(h);
	    DPROBLEM("DMA-transfer between local private and remote SCI-mem is not possible");
	    _smi_mc_handle_unlock();
	    DSECTLEAVE;	return(SMI_ERR_NOTIMPL);
	}   
	
	DNOTICE("Using DMA-WRITE funktionality");
	
	segRemote = _smi_addr_to_shseg(dest);
	offset = (size_t)dest - (size_t)(segRemote->address) + segRemote->offset;
	
	error = _smi_dma_transfer(_smi_address_to_region_id(dest), offset, src,
				  size, 0, h);
	if (error != SMI_SUCCESS) {
	    _smi_mc_handle_unlock();
	    DSECTLEAVE;	return(error);
	}
    } 
    else {
	if (flags & SMI_MEMCPY_RS_LS) {   
	    if (_smi_mc_dma_enabled() == FALSE) {
		if (!((h->dma_used == TRUE) && (h->dma_entries>0)))
		    _smi_mc_destroy_handle(h);
		DPROBLEM("DMA-read not implemented by Dolphin`s SISCI");
		_smi_mc_handle_unlock();
		DSECTLEAVE; return(SMI_ERR_NOTIMPL);
	    }
	    else {
		if( !(_smi_mt_is_localseg(dest) && _smi_mt_is_remoteseg(src)) ) { 
		    if (!((h->dma_used==TRUE)&&(h->dma_entries>0)))
			_smi_mc_destroy_handle(h);
		    _smi_mc_handle_unlock();
		    DSECTLEAVE; return(SMI_ERR_NOTIMPL);
		}
		
		DNOTICE("Using DMA-READ funktionality");
		
		segRemote = _smi_addr_to_shseg(src);
		offset = (size_t)dest - (size_t)(segRemote->address) + segRemote->offset;
		
		error = _smi_dma_transfer(_smi_address_to_region_id(src), offset,
					  dest, size, SCI_FLAG_DMA_READ, h);
		if (error != SMI_SUCCESS) {
		    _smi_mc_handle_unlock();
		    DSECTLEAVE; return(error);
		}
	    }
	}
    }
    
    _smi_mc_handle_unlock();
    DSECTLEAVE; return(SMI_SUCCESS);
#endif 
}

smi_error_t SMI_Memwait (smi_memcpy_handle h)
{
    DSECTION("SMI_Memwait");
    int RetVal = SMI_SUCCESS;
#ifndef NO_SISCI
    sci_error_t sciError;
#endif
    
    DSECTENTRYPOINT;
    
    if (_smi_mc_is_initialized() == FALSE) {
	DERROR("usage of memcpy without initialization!");
	SMI_Abort(-1);
    }
    
    _smi_mc_handle_lock();
    
    if ((h == NULL) || (h->otid != SMI_MEMCPY_HANDLE_OTID)) {
	DNOTICEP("h:",h);
	if (h != NULL) {
	    DNOTICEI("otid:",h->otid);
	}
	DNOTICE("not a valid handle");
	_smi_mc_handle_unlock();
	DSECTLEAVE; return(SMI_ERR_PARAM);
    }
    if (h->dma_used == FALSE) {
	DNOTICE("not using dma, thus no need to wait");
	_smi_mc_destroy_handle(h);
	_smi_mc_handle_unlock();
	DSECTLEAVE; return(SMI_SUCCESS);
    }
    
#ifdef DOLPHIN_SISCI
    /* only with DMA working you will need this code  - Scali SISCI won't */
  ASSERT_R(_smi_all_on_one==false, "Not implemented in SMP-Mode", SMI_ERR_NOTIMPL);
 
  DNOTICE("waiting for dma-transfer to complete");
  _smi_mc_handle_unlock();
  
  if (h->dma_err == SMI_SUCCESS) {
#if USE_DMA_THREAD
      if (h->dma_thread_args != NULL) {
	  rs_pthread_join (((dma_thread_args_t *)h->dma_thread_args)->dma_thread_id, NULL);
      }
#endif 
      SCIWaitForDMAQueue (h->dq, SCI_INFINITE_TIMEOUT, 0, &sciError);
      if (sciError != SCI_ERR_OK) {
	  _smi_mc_destroy_handle(h);
	  DWARNING("Queue not posted");
	  DSECTLEAVE; return(SMI_ERR_NOTPOSTED);
      }
  } else {
      smi_error_t dma_err = h->dma_err;

      _smi_mc_destroy_handle(h);
      DPROBLEMI ("DMA aborted, posting of DMA returned SMI error ", dma_err);
      DSECTLEAVE; return (dma_err);
  }

  _smi_mc_handle_lock();
  
  /* Handle wurde zwischenzeitlich von einem anderen Thread entfernt
     die Memcpy Operation wurde beendet. Das Neusenden im Transferfehlerfall
     wird vom anderen Thread erledigt. */
  if (h == NULL) {
      _smi_mc_handle_unlock();
      DNOTICE("not a valid handle");
      DSECTLEAVE; return (SMI_SUCCESS);
  }
  
  if (!(h->mc_flags & SMI_MEMCPY_NOVERIFY)) {
      if ( SMI_Check_transfer_proc(h->dma_rmt_proc, (h->mc_flags & SMI_MEMCPY_NOBARRIER) ? 
				   SMI_CHECK_FAST : 0) != SMI_SUCCESS) {
	  smi_memcpy_handle_t retry_h;
	  memset (&retry_h, 0, sizeof(smi_memcpy_handle_t));
	  DWARNING ("Transfer error, repeating DMA transfer");	  
	  _smi_dma_transfer(h->dma_remoteseg, h->dma_offset, h->dma_localadr,
			    h->mc_size, h->dma_direction, &retry_h);
	  RetVal = _smi_memwait(&retry_h);
      }
  }
  
  _smi_mc_destroy_handle(h);
  _smi_mc_handle_unlock();
#endif
  DSECTLEAVE; return(RetVal);
}

smi_error_t SMI_Memtest (smi_memcpy_handle h)
{
    REMDSECTION("SMI_Memtest");
    smi_error_t RetVal = SMI_ERR_PARAM;
#ifndef NO_SISCI
    sci_dma_queue_state_t questate;
    /*sci_error_t sciError;*/
#endif /* NO_SISCI */
    
    DSECTENTRYPOINT;
    
    if (_smi_mc_is_initialized() == FALSE) {
	DERROR("usage of memcpy without initialization!");
	SMI_Abort(-1);
    }
    
    _smi_mc_handle_lock();
    
    if ((h==NULL) || (h->otid != SMI_MEMCPY_HANDLE_OTID)) {
	_smi_mc_handle_unlock();
	DSECTLEAVE;
	return(SMI_ERR_PARAM);
    }
    if(h->dma_used == FALSE) {
	_smi_mc_handle_unlock();
	DSECTLEAVE;
	return(SMI_SUCCESS);
    }
    
#ifdef DOLPHIN_SISCI
    /* only with DMA working you will need this code  - Scali SISCI won't */
    ASSERT_R((_smi_all_on_one==false),"Not implemented in SMP-Mode",SMI_ERR_NOTIMPL);
    
    questate = SCIDMAQueueState(h->dq);
    
    switch(questate) {
    case SCI_DMAQUEUE_POSTED:
	RetVal = SMI_ERR_PENDING;
	break;
	
    case SCI_DMAQUEUE_DONE:
	_smi_mc_destroy_handle(h);
	RetVal = SMI_SUCCESS;
    break;
    
    case SCI_DMAQUEUE_ABORTED:
	RetVal = SMI_ERR_TRANSFER;
	break;
	
    case SCI_DMAQUEUE_ERROR:

/* shouldn't test return imediately? */
#if 0
	_smi_dma_transfer(h->dma_remoteseg, h->dma_offset, h->dma_localadr,
			  h->mc_size, h->dma_direction, h);
	RetVal = _smi_memwait(h);
#else
	RetVal = SMI_ERR_TRANSFER;	
#endif	
	_smi_mc_destroy_handle(h);
	break;
	
    case SCI_DMAQUEUE_GATHER:
	RetVal = SMI_ERR_NOTPOSTED;
	break;
	
    case SCI_DMAQUEUE_IDLE:
    default:
	DERROR ("internal failure of DMA transfer"); 
	SMI_Abort(-1);
    }
    
    _smi_mc_handle_unlock();
#endif
    
    DSECTLEAVE;
    return(RetVal);
}

smi_error_t SMI_MemwaitAll (int count, smi_memcpy_handle *h, smi_error_t *status)
{
  int i;
  smi_error_t RetVal = SMI_SUCCESS;

  for (i = 0; i < count; i++) {
      status[i] = SMI_Memwait(h[i]);
      if (status[i] != SMI_SUCCESS)
	  RetVal = SMI_ERR_OTHER;
  }

  return(RetVal);
}

smi_error_t SMI_MemtestAll (int count, smi_memcpy_handle *h, smi_error_t *status) 
{
  int i;
  smi_error_t RetVal = SMI_SUCCESS;

  for (i = 0; i < count; i++) {
      status[i] = SMI_Memtest(h[i]);
  }
  
  for (i = 0; i < count; i++) {
      if((status[i] != SMI_ERR_PENDING) && (status[i] != SMI_SUCCESS))
	  return (SMI_ERR_OTHER);
      if (status[i] == SMI_ERR_PENDING)
	  RetVal = SMI_ERR_PENDING;
  }
  
  return(RetVal);
}

smi_error_t SMI_Mempost (smi_memcpy_handle h) {
  REMDSECTION("SMI_Mempost");
  sci_error_t sciError;
 
  DSECTENTRYPOINT;

  ASSERT_R((_smi_mc_is_initialized() == TRUE),"usage of memcpy without initialization!",SMI_ERR_NOINIT);
  
  if ( (h != NULL) && (h->otid == SMI_MEMCPY_HANDLE_OTID) ) {
    if ( (h->dma_used == FALSE) || (h->dma_entries <= 0)) {
      DPROBLEM("The handle points to an nonenqueued memcopy-operation");
      DSECTLEAVE; return(SMI_ERR_PARAM);
    }
  } else {
    DPROBLEM("invalid handle!");
    DSECTLEAVE; return(SMI_ERR_PARAM);   
  }
  
#ifdef DOLPHIN_SISCI
  DNOTICE("Posting queue");
  SCIPostDMAQueue(h->dq,NULL,NULL,0,&sciError);
  h->dma_entries = 0;
  
  if (sciError != SCI_ERR_OK)
    return(SMI_ERR_OTHER);
#endif
  
  DSECTLEAVE;
  return(SMI_SUCCESS);
}
