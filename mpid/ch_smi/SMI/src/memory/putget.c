/* $Id$ */

/* Remote memory put/get via DMA for RDMA region (without mapping the memory) */

#include "memcpy_base.h"
#include "memcpy.h"

#include "regions/address_to_region.h"
#include "env/smidebug.h"


smi_error_t SMI_Iput (int dest_region_id, int offset, void *src, size_t size, smi_memcpy_handle* pHandle)
{
    DSECTION("SMI_Iput");
    memtype_t mtLocalRegion = _smi_detect_memtype(src);
    memtype_t mtRemoteRegion = _smi_detect_memtype_rdma(dest_region_id);
    smi_memcpy_handle h;
    size_t is_misaligned;
    char *reg_addr;
    smi_error_t error;
    
#ifndef DOLPHIN_SISCI
    DPROBLEM("DMA only supported by Dolphin's SISCI");
    DSECTLEAVE; return SMI_ERR_NOTIMPL;
#else
    
    if ((mtLocalRegion != mt_lsown) && (mtLocalRegion != mt_ls)) {
	DPROBLEM("src must be an local segment");
	DSECTLEAVE; return SMI_ERR_PARAM;
    }
    
    ASSERT_R((mtRemoteRegion == mt_rs),"dest must be an remote segment",SMI_ERR_PARAM);
    
    /* Check alignments. If the operation is peformed on a mapped region (which thus has
       an address, we can use SMI_Imemcpy() in case of a misalignemt. */
    if ((is_misaligned = size % _smi_dma_size_alignment) != 0) {
	DPROBLEMI("misaligned size for DMA:", size);
    } else {
	if ((is_misaligned = (size_t)src % _smi_dma_offset_alignment) != 0) {
	    DPROBLEMP("misaligned source address for DMA:", src);
	} else {
	    if ((is_misaligned = (size_t)offset % _smi_dma_offset_alignment) != 0) {
		DPROBLEMI("misaligned destination offset for DMA:", offset);
	    }
	}
    }
    if (is_misaligned) {
	reg_addr = _smi_get_region_address(dest_region_id);
	if (reg_addr == NULL) {
	    /* no way to transfer data */
	    DPROBLEM ("destination region is not mapped -> no way to work around misalignment"); 
	    DSECTLEAVE; return (SMI_ERR_BADALIGN);
	} else {
	    DPROBLEM ("destination region is mapped -> using SMI_Imemcpy to copy with misalignment"); 
	    DSECTLEAVE; return SMI_Imemcpy (reg_addr + offset, src, size, 0, pHandle);
	}
    }
    
    _smi_mc_handle_lock();
    h = *pHandle;
    
    if ( (h!=NULL) && (h->otid == SMI_MEMCPY_HANDLE_OTID) ) {
	if (*(h->pHandle) != *pHandle) {
	    DPROBLEM("Not a valid handle");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return SMI_ERR_PARAM;
	}
	DNOTICE("The Handle already exists!");
	if ( (h->dma_used == TRUE) && (h->dma_entries > 0)) {
	    DNOTICE("The handle contains an nonposted queue");
	}
	else {
	    DPROBLEM("The handle is in use");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return SMI_ERR_PARAM;
	}
    }
    else {
	h = _smi_mc_create_handle(pHandle);
	if (h == NULL) { 
	    _smi_mc_handle_unlock();
	    DPROBLEM ("Could not get memory for memcpy-handle");
	    DSECTLEAVE; return SMI_ERR_NOMEM;
	}
	h->mc_flags = 0;
    }

    error = _smi_dma_transfer(dest_region_id, offset, src, size, 0, h);
    
    _smi_mc_handle_unlock();  
    DSECTLEAVE; return error;
#endif
}

smi_error_t  SMI_Iget (void *dst, int src_region_id, int offset, size_t size, smi_memcpy_handle* pHandle)
{
    DSECTION("SMI_Iput");
    memtype_t mtLocalRegion = _smi_detect_memtype(dst);
    memtype_t mtRemoteRegion = _smi_detect_memtype_rdma(src_region_id);
    smi_memcpy_handle h;    
    char *reg_addr;
    size_t is_misaligned = 0;
    smi_error_t error;
    
#ifndef DOLPHIN_SISCI
    DPROBLEM("DMA only supported by Dolphin's SISCI");
    DSECTLEAVE;
    return SMI_ERR_NOTIMPL;
#else
    
    if ((mtLocalRegion != mt_lsown) && (mtLocalRegion != mt_ls)) {
	DPROBLEM("src must be an local segment");
	DSECTLEAVE;
	return(SMI_ERR_PARAM);
    }
    
    ASSERT_R((mtRemoteRegion == mt_rs),"dest must be an remote segment",SMI_ERR_PARAM);
    
    /* Check alignments. If the operation is peformed on a mapped region (which thus has
       an address, we can use SMI_Imemcpy() in case of a misalignemt. */
    if ((is_misaligned = size % _smi_dma_size_alignment) != 0) {
	DPROBLEMI("misaligned size:", size);
    } else {
	if ((is_misaligned = (size_t)dst % _smi_dma_offset_alignment) != 0) {
	    DPROBLEMP("misaligned destination address:", dst);
	} else {
	    if ((is_misaligned = (size_t)offset % _smi_dma_offset_alignment) != 0) {
		DPROBLEMI("misaligned source offset:", offset);
	    }
	}
    }
    if (is_misaligned) {
	reg_addr = _smi_get_region_address(src_region_id);
	if (reg_addr == NULL) {
	    /* no way to transfer data */
	    DPROBLEM ("destination region is not mapped -> no way to work around misalignment"); 
	    DSECTLEAVE; return (SMI_ERR_BADALIGN);
	} else {
	    DPROBLEM ("destination region is not mapped -> using SMI_Imemcpy to cope with misalignment"); 
	    DSECTLEAVE; return SMI_Imemcpy (dst, reg_addr + offset, size, 0, pHandle);
	}
    }
    
    _smi_mc_handle_lock();
    h = *pHandle;
    
    if (_smi_mc_dma_enabled() == FALSE) {
	DNOTICE("DMA-read not implemented by Dolphin`s SISCI");
	if (!((h->dma_used==TRUE)&&(h->dma_entries>0)))
	    _smi_mc_destroy_handle(h);
	_smi_mc_handle_unlock();
	DSECTLEAVE; return SMI_ERR_NOTIMPL;
    }
    
    if ( (h!=NULL) && (h->otid == SMI_MEMCPY_HANDLE_OTID) ) {
	if (*(h->pHandle) != *pHandle) {
	    DPROBLEM("Not a valid handle");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return SMI_ERR_PARAM;
	}
	DNOTICE("The Handle already exists!");
	if ( (h->dma_used == TRUE) && (h->dma_entries > 0)) {
	    DNOTICE("The handle contains an nonposted queue");
	}
	else {
	    DPROBLEM("The handle is in use");      
	    _smi_mc_handle_unlock();
	    DSECTLEAVE; return SMI_ERR_PARAM;
	}
    }
    else {
	h = _smi_mc_create_handle(pHandle);
	if (h == NULL) { 
	  DPROBLEM ("Could not get memory for memcpy-handle");
	  _smi_mc_handle_unlock();
	  DSECTLEAVE; return SMI_ERR_NOMEM;
	}
	h->mc_flags = 0;
    }

    error = _smi_dma_transfer(src_region_id, offset, dst, size, SCI_FLAG_DMA_READ, h);
    
    _smi_mc_handle_unlock();  
    DSECTLEAVE; return error;
#endif
}

smi_error_t SMI_Put (int dest_region_id, int offset, void *src, size_t size) 
{
    DSECTION("SMI_Put");
    smi_memcpy_handle h = NULL;
    smi_error_t error;
    
    DSECTENTRYPOINT;

    error = SMI_Iput(dest_region_id, offset, src, size, &h);
    if (error == SMI_SUCCESS)
	error = SMI_Memwait(h);
    
    DSECTLEAVE; return error;
}

smi_error_t SMI_Get (void *dst, int src_region_id, int offset, size_t size)
{
    DSECTION("SMI_Get");
    smi_memcpy_handle h = NULL;
    smi_error_t error;
    
    DSECTENTRYPOINT;
    
    error = SMI_Iget(dst, src_region_id, offset,size, &h);
    if (error == SMI_SUCCESS) 
	error = SMI_Memwait(h);
    
    DSECTLEAVE;	return error;
}
