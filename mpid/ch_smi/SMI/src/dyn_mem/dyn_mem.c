/* $Id$ */

#include "smi.h"
#include "env/smidebug.h"
#include "dyn_mem.h"
#include "switch_consistency/switch_to_replication.h"
#include "synchronization/mutex.h"
#include "proc_node_numbers/proc_to_node.h"
#include "synchronization/store_barrier.h"
#include "regions/idstack.h"
#include "message_passing/lowlevelmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define TEST_LOCAL_SYNC 0

/*********************************************************************************/
/* This array contains for each shared region a pointer to a 'dyn_mem_seg' data  */
/* structure that was initialized to be used for this purpose. 'max_dyn_seg'     */
/* states the maximum identifier of a shared regions that is in use and has been */
/* initialized up to this point in time for dynamic memory allocation.           */
/*********************************************************************************/

static dyn_mem_seg_t** dyn_mem_seg;
static int max_dyn_seg = -1;

/*********************************************************************************/
/* This function initializes the mutex for a shared region that is used for      */
/* dynamic memory allocation. This is used to guarantee mutual exclusion between */
/* different processes that dynamically allocate pieces of shared within this    */
/* region for the memory manager's data structures. It requires that all other   */
/* stuff regarding this mode of usage of the specified shared region is already  */
/* initialized (with SMI_init_shregMMU). This function can be called without the */
/* function Imalloc working already, because inside SMI_Mutex_init only CMalloc  */
/* is required.                                                                  */
/* Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (the region id is invalid or   */
/* this region has not yet been initialized to be used with a memory manager),   */
/* and any error, SMI_Mutex_init might produce.                                  */
/*********************************************************************************/
smi_error_t _smi_allocate_MMU_lock(int region_id)
{
   DSECTION("_smi_allocate_MMU_lock");
   smi_error_t error;
#if 0
   static int mmu_lock_prank = 0;
#endif

   DSECTENTRYPOINT;

   ASSERT_R(region_id <= max_dyn_seg, "illegal region ID", SMI_ERR_PARAM);
   ASSERT_R(dyn_mem_seg[region_id] != NULL, "invalid region ptr", SMI_ERR_PARAM);   

   /* Distribute the location of the locks for MMU region across the existing
      internal regions. */
#if 0
   /* Round-robin: This is not the perfect way to do it, but better than 
      location all mutexes on a single node. */
   error = SMI_Mutex_init_with_locality(&dyn_mem_seg[region_id]->lock_id, mmu_lock_prank);
   mmu_lock_prank = (mmu_lock_prank + 1)%_smi_nbr_procs;
#else 
   /* Usually, the owner of the first/only segment will do most of the allocations. */
   error = SMI_Mutex_init_with_locality(&dyn_mem_seg[region_id]->lock_id, 
					dyn_mem_seg[region_id]->shreg->seg[0]->owner);
#endif

   ASSERT_R(error == SMI_SUCCESS, "Could not init mutex",error);


   DSECTLEAVE; return(SMI_SUCCESS);
}


/*********************************************************************************/
/* Determines the region number of the segment into which the specified address  */
/* falls.                                                                        */
/* Possible Errors: SMI_ERR_PARAM (if the address is not contained in any region)*/  
/*********************************************************************************/ 
smi_error_t _smi_address_to_MMUregion(char* address, int* region_id)
{
    dyn_mem_seg_t* s;
    boolean        found = false;

    DSECTION ("_smi_address_to_MMUregion");
    
    *region_id = 0;
    
    /* XXX: implemented with memtree for better performance. */
    while (*region_id <= max_dyn_seg && found == false) {
	s = dyn_mem_seg[*region_id];
	if (s != NULL) {
	    if ( (size_t)address >= (size_t)(s->adr)
		 && (size_t)address < (size_t)(s->adr)+s->size) {
		found = true;
	    } else {
		(*region_id)++;
	    }
	} else {
	    (*region_id)++;
	}
    }
    
    if (found == false) {
	DWARNING ("address does not belong to any MMUregion");
	return(SMI_ERR_PARAM);
    }
    
    return(SMI_SUCCESS);
}

/* free the data structures for dynmamic memory management */
smi_error_t _smi_free_shregMMU(void) {
    int i;

    for (i = max_dyn_seg; i >= 0; i--) {
	if (dyn_mem_seg[i] != NULL) {
	    /* non-collective MMU regions do not use a lock */
	    if (dyn_mem_seg[i]->lock_id >= 0)
		SMI_Mutex_destroy(dyn_mem_seg[i]->lock_id);
	    free (dyn_mem_seg[i]);
	}
    }
    free (dyn_mem_seg);

    return (SMI_SUCCESS);
}


/*********************************************************************************/
/*** This function initializes everything to use a shared region with          ***/
/*** dynamical memory allocation. It is recommended that only shared region    ***/
/*** with the distribution policy SMI_SHM_UNDIVIDED are used that way. This function   ***/
/*** has to be called collectively from all processes with the same            ***/
/*** parameter. It states a global synchronization point.                      ***/
/*** region_id & INTERNAL states that this is the initialization of one of the ***/
/*** shared region for internal purposes. Then, no lock is allocated to        ***/
/*** protect the memory manager. This will be done later on.                   ***/
/*** Possible Errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (such a region does not    ***/
/*** exists or is already initialized with a MMU), 1xxx, anything that         ***/
/*** SMI_Mutex_init might produce and SMI_ERR_OTHER (the MMU initialization    ***/
/*** failed, I don't know why).                                                ***/
/***                                                                           ***/
/*** Regions that are created asynchronously (LOCAL / REMOTE) are initialized  ***/
/*** asynchronously, too. This implies that for these regions, only the process***/
/*** to which the region is local may use the dyn. memory management!          ***/
/*********************************************************************************/
smi_error_t SMI_Init_shregMMU(int region_id)
{
  DSECTION("SMI_Init_shregMMU");
  size_t          power_of_two;
  int             i;
  dyn_mem_seg_t** tmp;
  region_t*       region;  
  boolean         internal = false;
  int             **bcast_buffer;
  smi_error_t 	  error;
 
  DSECTENTRYPOINT;

  DNOTICEI("region_id",region_id);
  
  ASSERT_R(_smi_initialized==true, "SMI library not initialized",SMI_ERR_NOINIT);
 
  /* check if the shared region for that a MMU is to be initialized 
     is one of the internal */ 
  if (region_id & INTERNAL) {
    region_id -= INTERNAL;
    internal = true;
  }
  ASSERT_R(region_id >= 0, "Illegal region id, must be >= 0",SMI_ERR_PARAM);
  
  if (region_id <= max_dyn_seg) {
    ASSERT_R(dyn_mem_seg[region_id] == NULL, 
	     "Illegal region id (region already initialized)",SMI_ERR_PARAM);
  }
  
  /* look-up the region's data structure */
  region = _smi_get_region(region_id);
  ASSERT_R(region != NULL, "Illegal region id (region does not exist)",SMI_ERR_PARAM);  
  DNOTICEP("region",region);

  /* enlarge `dyn_mem_seg' if necessary */
  if (region_id > max_dyn_seg) {
      ALLOCATE (tmp, dyn_mem_seg_t **, (region_id+1)*sizeof(dyn_mem_seg_t*));
      for(i = 0; i <= region_id; i++)
	  tmp[i] = NULL;
      for(i = 0; i <= max_dyn_seg; i++)
	  tmp[i] = dyn_mem_seg[i];
      free(dyn_mem_seg);
      dyn_mem_seg = tmp;
      max_dyn_seg = region_id;
  }

  /* fill-in start-address */
  ALLOCATE (dyn_mem_seg[region_id], dyn_mem_seg_t *, sizeof(dyn_mem_seg_t));
  dyn_mem_seg[region_id]->shreg = region;
  dyn_mem_seg[region_id]->adr = region->addresses[0];
  dyn_mem_seg[region_id]->lock_id = -1;

  DNOTICEI("effective region id",region_id);
  DNOTICEP("dyn_mem_seg[region_id]->adr",dyn_mem_seg[region_id]->adr);
  
  /* the amount of memory of the shared region, to be managed */
  /* dynamically is the greatest power-of-two-number, smaller */
  /* or equal to the size of the shared region, because this  */
  /* is required by the memory management algorithm,          */
  /* temporary employed                                       */
  power_of_two = 1;
  while (power_of_two<<1 <= region->size)
      power_of_two <<= 1;
  dyn_mem_seg[region_id]->size = power_of_two;
  
  /* Initialize the memory management data structures inside the segments. */
  /* This does solely the owner of the shared region.                      */
  if (_smi_my_proc_rank == region->seg[0]->owner) {
      error = _smi_dynmem_init(dyn_mem_seg[region_id]->adr, dyn_mem_seg[region_id]->size);
      ASSERT_R(error==0, "_smi_dynmem_init failed",SMI_ERR_OTHER);
      /* clean up in case of failure ? boris */
  }
  
  /* allocate a piece of memory inside the region, that is used */
  /* later on to broadcast the pointer of a Cmalloc.            */
  if (_smi_my_proc_rank == region->seg[0]->owner) {
      dyn_mem_seg[region_id]->adr_bucket = 
	  (volatile char**)_smi_dynmem_alloc(dyn_mem_seg[region_id]->adr,
					     (unsigned long)sizeof(char*));
      dyn_mem_seg[region_id]->step = 	
	(int*)_smi_dynmem_alloc(dyn_mem_seg[region_id]->adr, 
				(unsigned long)(sizeof(int)*_smi_nbr_procs));
      for(i=0;i<_smi_nbr_procs;i++)
	  dyn_mem_seg[region_id]->step[i] = 0;
  }
  
  if (region->collective) {
    /* communicate the basic elements of the dynamic memory management 
       (kind of "packing" to reduce number of broadcasts) */
    ALLOCATE (bcast_buffer, int **, 2*sizeof(int *));
    if (_smi_my_proc_rank == region->seg[0]->owner) {
	bcast_buffer[0] = (int *)dyn_mem_seg[region_id]->adr_bucket;
	bcast_buffer[1] = (int *)dyn_mem_seg[region_id]->step;
    }
    
    _smi_ll_bcast((int *)bcast_buffer, 2*sizeof(int *)/sizeof(int), region->seg[0]->owner, 
		  _smi_my_proc_rank);
    
    dyn_mem_seg[region_id]->adr_bucket = (volatile char **)bcast_buffer[0];
    dyn_mem_seg[region_id]->step = (volatile int *) bcast_buffer[1];
    free (bcast_buffer);
    
    /* Allocate a lock to protect the MMU's internal data structures, if */
    /* the corresponding shared region is not one of those for internal  */
    /* usage. The lock for these regions needs to be allocated lateron
       (classic hen-and-egg problem.  */
    /* The timing here is save, because the ll_bcast above performs a    */
    /* barrier synchronization.                                          */
    if (!internal) {
      error = SMI_Mutex_init(&dyn_mem_seg[region_id]->lock_id);
      ASSERT_R(error==0, "SMI_Mutex_init failed",error);
    } 
  } else {
      /* non-collective region, no locking required */
      dyn_mem_seg[region_id]->lock_id = -1;
  }
  
  DSECTLEAVE; return(SMI_SUCCESS);
}


/*********************************************************************************/
/*** This function allocates a piece of memory in the specified shared memory  ***/
/*** region. During this call, mutual exclusion has to be guaranteed for the   ***/
/*** requested memory manager. There might be the case, that the necessary     ***/
/*** lock to do so is not allocated at the point of time. In this case, no     ***/
/*** protection is performed, this has to be performed by the calling instance ***/
/*** itself. 'region_id&INTERNAL' means that the piece of memory is to be      ***/
/*** allocated inside a internal segment, 'region_id-INTERNAL' specifies the   ***/
/*** process in this case.                                                     ***/
/*** Possible errors: SMI_ERR_NOINIT, SMI_ERR_BADPARAM (the region-id is       ***/
/*** invalid or this region has not been initialized for dyn. mem. allocation),***/
/*** anything SMI_Mutex_lock may produce, anything SMI_Mutex_unlock may        ***/
/*** produce, SMI_ERR_NOMEM (because the requested piece is too large: there   ***/
/*** is not enough memory left or it is too large at all for the region).      ***/
/*********************************************************************************/
smi_error_t SMI_Imalloc(size_t size, int region_id, void** address) {
    DSECTION ("SMI_Imalloc");
#if TEST_LOCAL_SYNC
    region_t *region; 
#endif
    int node;
    boolean internal = false;
    smi_error_t error;

	smi_memmgr_t *tst_node = NULL;

    
    DSECTENTRYPOINT;   
    DNOTICEI ("Region:", region_id);
    ASSERT_R(_smi_initialized, "SMI not initialized",SMI_ERR_NOINIT);
   
#if TEST_LOCAL_SYNC 
    if (region_id & LOCAL_ONLY) {
	/* 
	   If ored with LOCAL_ONLY, the internal memoryregion is used, that can
	   only be accessed by the processes on the local node.
	   Such memory areas can be used to perform nodewide barrier/mutex
	   synchronization.
	   
	   LOCAL_ONLY ovverides INTERNAL
	*/
	region_id = _smi_int_smp_shreg_id;
	internal = false;
    }
    else
#endif
	if (region_id & INTERNAL) {
	    /* this is a malloc to one of the internal shared regions. The   
	       '-region_id' specifies the process in whose internal shared memory 
	       region the memory is to be allocated. Convert 'region_id' 
	       correspondingly.                                                   */
	    
	    region_id -= INTERNAL;
	    error = SMI_Proc_to_node(region_id, &node);
	    ASSERT_R((error==SMI_SUCCESS),"SMI_Proc_to_node failed",SMI_ERR_PARAM);
	    
	    region_id = _smi_int_shreg_id[node];
	    internal = true;
	    DNOTICEI ("Effective region id of internal region:", region_id);
	} 
    
    ASSERT_R(region_id <= max_dyn_seg, "Invalid region id",SMI_ERR_PARAM);
    ASSERT_R(dyn_mem_seg[region_id] != NULL, "Invalid region id",SMI_ERR_PARAM);
#if 0
    region = _smi_get_region(region_id);
    ASSERT_R(region->collective || (region->seg[0]->owner == _smi_my_proc_rank), 
	     "Must be owner of a non-collective region to use dynamic memory with it", SMI_ERR_OTHER);
#endif
   	/* DSETON */
    /* Lock memory manager's data structures, if a lock exists */
    if (dyn_mem_seg[region_id]->lock_id >= 0) {
	error = SMI_Mutex_lock(dyn_mem_seg[region_id]->lock_id);
	ASSERT_R(error==SMI_SUCCESS, "SMI_Mutex_lock failed", error);
    }

    
    /* call the malloc function with the data structures of
       the specified memory segment   */
    
    tst_node = ((smi_memmgr_t *)((void*)dyn_mem_seg[region_id]->adr));
    if (tst_node) {
	DNOTICEI ("in region",region_id);
	DNOTICEP ("checking node : ",tst_node);
	if (tst_node->b_size)
	    DNOTICEI ("b_size is: ",tst_node->b_size);
	if (tst_node->b_addr)
	    DNOTICEP ("b_addr is: ",tst_node->b_addr);
	if (tst_node->sb_avail)
	    DNOTICEI ("sb_avail is: ",tst_node->sb_avail);
	if (tst_node->sb_l)
	    DNOTICEI ("sb_avail L is: ",tst_node->sb_l->sb_avail);
	if (tst_node->sb_r) {
	    DNOTICEI ("sb_avail R is: ",tst_node->sb_r->sb_avail);
	    DNOTICEP ("sb_r is: ",tst_node->sb_r);
	}
    }
    
    *address = _smi_dynmem_alloc((void*)dyn_mem_seg[region_id]->adr, (unsigned long)size);
    if (*address == NULL) { 
	DPROBLEMI ("Could not allocate requested amount of memory:", size);
	
	if (dyn_mem_seg[region_id]->lock_id >= 0)
	    error = SMI_Mutex_unlock(dyn_mem_seg[region_id]->lock_id);
	/* DSETOFF */
	DSECTLEAVE; return(SMI_ERR_NOMEM);
    }
    
    /* Unlock memory managers data structures, if a lock exists */
    if (dyn_mem_seg[region_id]->lock_id >= 0) {
	error = SMI_Mutex_unlock(dyn_mem_seg[region_id]->lock_id);
	ASSERT_R(error==SMI_SUCCESS, "SMI_Mutex_unlock failed",error);
    }
    
    DNOTICEI ("Successfully allocated memory, size:", size);
    DNOTICEP ("                             offset:", 
	      (char *)*address - (size_t)dyn_mem_seg[region_id]->adr);
    ASSERT_A((((char *)*address - (size_t)dyn_mem_seg[region_id]->adr) != 0), 
	     "OFFSET ZERO ERROR", SMI_ERR_OTHER);

    DSECTLEAVE; return(SMI_SUCCESS);
}


  
/*********************************************************************************/
/*** This function frees the portion of memory to that 'address' points. This  ***/
/*** function is protected by mutual exclusion.                                ***/
/*** Possible Errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (if the address fits in    ***/
/*** no shared region, anything SMI_Mutex_lock and _unlock may produce.        ***/ 
/*********************************************************************************/ 
smi_error_t SMI_Ifree(void* address)
{
    int region_id;
    smi_error_t error;
    smi_memmgr_t* tst_node;
    DSECTION ("SMI_Ifree");
    
    DSECTENTRYPOINT;
    ASSERT_R(_smi_initialized==true, "SMI not initialized",SMI_ERR_NOINIT);
    
    /* search the shared region into which the address fits */
    error = _smi_address_to_MMUregion(address, &region_id);
    ASSERT_R (error == SMI_SUCCESS, "Tried to free non-SMI memory", SMI_ERR_PARAM);
    DNOTICEI ("Region:", region_id);
    
    /* Lock memory managers data structures, if a lock exists */
    if (dyn_mem_seg[region_id]->lock_id >= 0) {
	error = SMI_Mutex_lock(dyn_mem_seg[region_id]->lock_id);
	ASSERT_R(error == SMI_SUCCESS, "SMI_Mutex_lock failed",error);
    }
    
    tst_node = ((smi_memmgr_t *)((void*)dyn_mem_seg[region_id]->adr));
    if (tst_node) {
	DNOTICEP ("checking node : ",tst_node);
	if (tst_node->b_size)
	    DNOTICEI ("b_size is: ",tst_node->b_size);
	if (tst_node->b_addr)
	    DNOTICEP ("b_addr is: ",tst_node->b_addr);
	if (tst_node->sb_avail)
	    DNOTICEI ("sb_avail is: ",tst_node->sb_avail);
	if (tst_node->sb_l)
	    DNOTICEI ("sb_avail L is: ",tst_node->sb_l->sb_avail);
	if (tst_node->sb_r)
	    DNOTICEI ("sb_avail R is: ",tst_node->sb_r->sb_avail);
    }
    
    /* call the free function with the memory management */
    /* data structures of this segment                   */
    _smi_dynmem_free((void*)(dyn_mem_seg[region_id]->adr), address);
    
    /* Unlock memory managers data structures, if a lock exists */
    if (dyn_mem_seg[region_id]->lock_id >= 0) {
	error = SMI_Mutex_unlock(dyn_mem_seg[region_id]->lock_id);
	ASSERT_R(error == SMI_SUCCESS, "SMI_Mutex_unlock failed",error);  
    }
    
    DNOTICEP ("Freed memory at offset:", (char *)address - (size_t)dyn_mem_seg[region_id]->adr);
    DSECTLEAVE; return(SMI_SUCCESS);
}





/*********************************************************************************/
/*** same as above, but this is a collective call from all processes. Just one ***/
/*** performs the free.                                                        ***/
/*** Possible Errors: Same as SMI_Ifree.                                       ***/
/*********************************************************************************/
smi_error_t SMI_Cfree(void* address) 
 {
   DSECTION("SMI_Cfree"); 
   smi_error_t error;
   region_t*       region; 
   int region_id;
   
   DSECTENTRYPOINT;
   ASSERT_R(_smi_initialized==true,"SMI not initialized",SMI_ERR_NOINIT);

   error = _smi_address_to_MMUregion(address, &region_id);
   ASSERT_R(error == SMI_SUCCESS, "Invalid address",error);
   region = _smi_get_region(region_id);
   ASSERT_R(region != NULL, "Illegal region id (region does not exist)",SMI_ERR_PARAM);  

   if (_smi_my_proc_rank == region->seg[0]->owner) {
     error = SMI_Ifree(address); 
     ASSERT_R(error == SMI_SUCCESS, "SMI_Ifree failed",error);
   }
   
   DSECTLEAVE;  return(SMI_SUCCESS); 
 }


    
/*********************************************************************************/
/*** same as above, but this is a collective call from all processes. Just one ***/
/*** performs the malloc, but the common pointer is returned to all.           ***/
/*** 'region_id&INTERNAL' means that the piece of memory is to be allocated    ***/
/*** inside a internal segment, 'region_id-INTERNAL' specifies the process in  ***/
/*** this case.                                                                ***/
/*** Possible Error Codes: SMI_ERR_NOINIT, SMI_ERR_PARAM (in the case that the ***/
/*** region is not valid because either such a region does not exist or no MMU ***/
/*** is installed for it), anything Imalloc can produce.                       ***/
/*********************************************************************************/ 
smi_error_t SMI_Cmalloc(size_t size, int region_id, void** address)
 {
   DSECTION("SMI_Cmalloc");
   smi_error_t error;
   region_t* region;
   boolean internal = false;
   int proc_id, node;
   size_t offset;
#ifndef SMI_NONFIXED_MODE
   int tmp;
#endif

   DSECTENTRYPOINT;
   ASSERT_R(_smi_initialized==true, "SMI not initialized",SMI_ERR_NOINIT);

   DNOTICEI("region_id =", region_id);
   DNOTICEI("size requested =", size);
   
#if TEST_LOCAL_SYNC
   if (region_id & LOCAL_ONLY) {
       /* 
	  If ored with LOCAL_ONLY, the internal memoryregion is used, that can
	  only be accessed by the processes on the local node.
	  Such memory areas can be used to perform nodewide barrier/mutex
	  synchronization.
	  
	  LOCAL_ONLY ovverides INTERNAL
       */
       proc_id = _smi_first_proc_on_node(_smi_my_machine_rank);
       region_id = _smi_int_smp_shreg_id;
       internal = false;
   }
   else
#endif
       if (region_id & INTERNAL) {
	   /* this is a malloc to one of the internal shared regions. The        */
	   /* '-region_id' specifies the process in whose internal shared memory */
	   /* region the memory is to be allocated. Convert 'region_id'          */
	   /* correspondingly.                                                   */
	   
	   region_id -= INTERNAL;
	   proc_id = region_id;
	   error = SMI_Proc_to_node(region_id, &node);
	   ASSERT_R(error == SMI_SUCCESS, "SMI_Proc_to_node failed (illegal region id)",SMI_ERR_PARAM);
	   
	   region_id = _smi_int_shreg_id[node];
	   internal = true;
	   DNOTICE("region is an internal region");
       }

   /* look-up the region's data structure */
   region = _smi_get_region(region_id);
   ASSERT_R(region != NULL, "Illegal region id (region does not exist)",SMI_ERR_PARAM);  
   ASSERT_R(region->collective, "Cmalloc not possible for non-collective segments!", SMI_ERR_OTHER);

   /* the owner of the region actually performs the malloc,
      because he can do it most fast                        */
   if (_smi_my_proc_rank == region->seg[0]->owner) {
       error = internal ? SMI_Imalloc(size, proc_id|INTERNAL, address) :
	   SMI_Imalloc(size, region_id, address);
       
       ASSERT_R( error == SMI_SUCCESS, "SMI_Imalloc failed",error);
       
#ifdef SMI_NONFIXED_MODE
      offset = *(char **)address - region->addresses[0];
      _smi_ll_bcast((int*) &offset,sizeof(size_t)/sizeof(int), region->seg[0]->owner, _smi_my_proc_rank);
#else
      *(dyn_mem_seg[region_id]->adr_bucket) = *address;
      _smi_range_store_barrier(dyn_mem_seg[region_id]->adr_bucket,4,-1);
      
      DNOTICE("signalize that address has been posted");
      (dyn_mem_seg[region_id]->step[_smi_my_proc_rank])++;
      _smi_range_store_barrier(&(dyn_mem_seg[region_id]->step[_smi_my_proc_rank]),4,-1);
      
      DNOTICE("wait for other processes to have read the address");
      for(i=0;i<_smi_nbr_procs;i++)
	  while (  dyn_mem_seg[region_id]->step[i] 
		   < dyn_mem_seg[region_id]->step[_smi_my_proc_rank]) ;
#endif 
   } else {
       /* Distribute the returned address to everybody. This is a problem, */
       /* because: one needs a lock to block all processes but the one     */
       /* that actually performs the malloc until the return address has   */
       /* been determined and stored in                                    */
       /* 'dyn_mem_seg[region_id]->adr_bucket'. However, this function is  */
       /* already required to create a lock. Furthermore, the allocating   */
       /* process itself is not allowed to leave this function and enter   */
       /* is again, overwriting 'dyn_mem_seg[region_id]->adr_bucket',      */
       /* before all processes have got it.                                */
       /*                                                                  */
       /* The solution: We perform the synchronization by hand. Each       */
       /* shared region possesses a shared array (inside the region        */
       /* itself) step[0...no_machine-1] that is initially set to 0...0.   */
       /* The allocating process increments its corresponding element,     */
       /* when the address has been determined. All other processes wait   */
       /* till this happens, read the address and increment their element  */
       /* of this array. The allocating processes only leaves this         */
       /* function, when it sees all processes' element incremented,       */
       /* meaning that they have read the address.                         */
       

#ifdef SMI_NONFIXED_MODE
       _smi_ll_bcast((int*)&offset,sizeof(size_t)/sizeof(int), region->seg[0]->owner, _smi_my_proc_rank);
       *address = region->addresses[0] + offset;
#else
       DNOTICE("waiting for action");
       while ((volatile int)(dyn_mem_seg[region_id]->step[region->seg[0]->owner])
	      <= dyn_mem_seg[region_id]->step[_smi_my_proc_rank]) ;
       
       
       /* This is a dirty workaround. We have to make sure that thre read of */
       /* the address did not fail, but how ? If it fails, we saw often that */
       /* '0' has been read instead, so we test for '0'                      */
       *address = NULL;
       DNOTICE("getting address");
       do {
	   *(volatile char**)address = *(dyn_mem_seg[region_id]->adr_bucket);
       } while (*address == NULL);
       
       tmp = (dyn_mem_seg[region_id]->step[_smi_my_proc_rank]) + 1;
       
       do {
	   dyn_mem_seg[region_id]->step[_smi_my_proc_rank] = tmp;
	   _smi_local_store_barrier();
       }	while (dyn_mem_seg[region_id]->step[_smi_my_proc_rank] != tmp);
       
#endif 
   }
   
   DSECTLEAVE; return(SMI_SUCCESS);
 }






