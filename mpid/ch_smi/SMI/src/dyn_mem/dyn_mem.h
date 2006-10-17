/* $Id$ */

#ifndef _SMI_DYN_MEM_H
#define _SMI_DYN_MEM_H

#include "env/general_definitions.h"
#include "regions/region_layout.h"
#include "../synchronization/syncmod/basetype.h"
#ifdef __cplusplus
extern "C" {
#endif


	/*********************************************************************************/
	/* this structure contains all necessary information about an shared region      */
	/* (that must be SMI_SHM_UNDIVIDED) that is used under dynamic memory allocation         */
	/*********************************************************************************/
	typedef struct
	{
		region_t *shreg;   
		size_t size;       /* size of the shared memory region                        */
		char *adr;         /* start address of the shared memory region               */
		int lock_id;       /* the id for the mutex that is used to guarantee mutual   */
		/* exclusion amon several processes that use the memory    */
		/* management instance of this shared region               */
		volatile char **adr_bucket;
		/* this is the bucket that is used for a Cmalloc to        */
		/* broadcast the address of the allocated piece of memory  */
		/* from the process that actually performs the malloc to   */
		/* all others                                              */
		volatile int *step;
		/* this array is used to synchronize all processes in a    */
		/* Cmalloc so that they wait for the process that actually */
		/* performs the malloc until it has placed the address of  */
		/* the allocated piece of memory in 'adr_bucket'           */
	} dyn_mem_seg_t; 


	/*********************************************************************************/
	/* This function initializes the mutex for a shared region that is used for      */
	/* dynamic memory allocation. This is used to guarantee mutual exclusion between */
	/* different processes that dynamically _smi_allocate pieces of shared within this    */
	/* region for the memory manager's data structures. It requires that all other   */
	/* stuff regarding this mode of usage of the specified shared region is already  */
	/* initialized (with SMI_init_shregMMU). This function can be called without the */
	/* function Imalloc working already, because inside SMI_Mutex_init only CMalloc  */
	/* is required.                                                                  */
	/* Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (the region id is invalid or   */
	/* this region has not yet been initialized to be used with a memory manager),   */
	/* and any error, SMI_Mutex_init might produce.                                  */
	/*********************************************************************************/
	smi_error_t _smi_allocate_MMU_lock(int);

	/*********************************************************************************/
	/* Determines the region number of the segment into which the specified address  */
	/* falls.                                                                        */
	/* Possible Errors: SMI_ERR_PARAM (if the address is not contained in any region)*/  
	/*********************************************************************************/
	smi_error_t _smi_address_to_MMUregion(char *, int*);


	/* free all allocated resources */
	smi_error_t _smi_free_shregMMU(void);


	/*********************************************************************************/
	/*** This function initializes everything to use a shared region with          ***/
	/*** dynamical memory allocation. It is recommended that only shared region    ***/
	/*** with the distribution policy SMI_SHM_UNDIVIDED are used that way. This function   ***/
	/*** has to be called collectively from all processes with the same            ***/
	/*** parameter. It states a global synchronization point.                      ***/
	/*** region_id & INTERNAL states that this is the initialization of one of the ***/
	/*** shared region for internal purposes. Then, no lock is _smi_allocated to        ***/
	/*** protect the memory manager. This will be done later on.                   ***/
	/*** Possible Errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (such a region does not    ***/
	/*** exists or is already initialized with a MMU), 1xxx, anything that         ***/
	/*** SMI_Mutex_init might produce and SMI_ERR_OTHER (the MMU initialization    ***/
	/*** failed, I don't know why).                                                ***/
	/*********************************************************************************/
	smi_error_t SMI_Init_shregMMU(int);

	/*********************************************************************************/
	/*** This function _smi_allocates a piece of memory in the specified shared memory  ***/
	/*** region. During this call, mutual exclusion has to be guaranteed for the   ***/
	/*** requested memory manager. There might be the case, that the necessary     ***/
	/*** lock to do so is not _smi_allocated at the point of time. In this case, no     ***/
	/*** protection is performed, this has to be performed by the calling instance ***/
	/*** itself. 'region_id&INTERNAL' means that the piece of memory is to be      ***/
	/*** _smi_allocated inside a internal segment, 'region_id-INTERNAL' specifies the   ***/
	/*** process in this case.                                                     ***/
	/*** Possible errors: SMI_ERR_NOINIT, SMI_ERR_BADPARAM (the region-id is       ***/
	/*** invalid or this region has not been initialized for dyn. mem. allocation),***/
	/*** anything SMI_Mutex_lock may produce, anything SMI_Mutex_unlock may        ***/
	/*** produce, SMI_ERR_NOMEM (because the requested piece is too large: there   ***/
	/*** is not enough memory left or it is too large at all for the region).      ***/
	/*********************************************************************************/
	smi_error_t SMI_Imalloc(size_t, int, void**);

	/*********************************************************************************/
	/*** This function frees the portion of memory to that 'address' points. This  ***/
	/*** function is protected by mutual exclusion.                                ***/
	/*** Possible Errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (if the address fit's in   ***/
	/*** no shared region, anything SMI_Mutex_lock and _unlock may produce.        ***/ 
	/*********************************************************************************/
	smi_error_t SMI_Ifree(void *);

	/*********************************************************************************/
	/*** same as above, but this is a collective call from all processes. Just one ***/
	/*** performs the free.                                                        ***/
	/*** Possible Errors: Same as SMI_Ifree.                                       ***/
	/*********************************************************************************/
	smi_error_t SMI_Cfree(void *);

	/*********************************************************************************/
	/*** same as above, but this is a collective call from all processes. Just one ***/
	/*** performs the malloc, but the common pointer is returned to all.           ***/
	/*** 'region_id&INTERNAL' means that the piece of memory is to be _smi_allocated    ***/
	/*** inside a internal segment, 'region_id-INTERNAL' specifies the process in  ***/
	/*** this case.                                                                ***/
	/*** Possible Error Codes: SMI_ERR_NOINIT, SMI_ERR_PARAM (in the case that the ***/
	/*** region is not valid because either such a region does not exist or no MMU ***/
	/*** is installed for it), anything Imalloc can produce.                       ***/
	/*********************************************************************************/
	smi_error_t SMI_Cmalloc(size_t, int, void **);


	/* internal functions and types */
#define FREELIST_SZ           4096
#define MIN_FREELIST_ENTRIES  16

	struct _smi_memmgr {
		size_t b_size;       /* size of this block */
		char *b_addr;        /* address of this block */
		size_t sb_avail;     /* available subblocks of this block (bitfield) */
		size_t mem_avail;    /* available memory in subblocks of this block; may be higher than
								sb_avail because of multiple blocks with the same size */
  
		struct _smi_memmgr *sb_l, *sb_r; /* left and right subblocks*/
	};
	typedef struct _smi_memmgr smi_memmgr_t;

	int _smi_dynmem_init (void *sgmt_adr, size_t sgmt_sz);
	smi_memmgr_t *_smi_free_list_init (smi_memmgr_t *base_node, smi_memmgr_t *free_list);

	void *_smi_dynmem_alloc (void *mgr_base, uint reqsz);
	smi_memmgr_t *_smi_dynmem_memtree(smi_memmgr_t *node, smi_memmgr_t *free_list, int reqsz);

	void _smi_dynmem_free (void *mgr_base, void *adr);


#ifdef __cplusplus
}
#endif


#endif



/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
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
