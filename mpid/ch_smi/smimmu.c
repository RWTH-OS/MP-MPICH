/* $Id$ */

/* Device-internal memory management of SCI and local memory for MPI_Alloc_mem /
   one-sided communication. */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#ifndef WIN32
#include <strings.h>
#endif

#include <assert.h>

#include "smi.h"

#include "mpimem.h"
#include "mpid.h"
#include "smimem.h"
#include "smiregionmngmt.h"
#include "mpiimpl.h"
#include "mmu.h"
#include "sbcnst2.h"
#include "smidef.h"
#include "adi3types.h"
#include "smidev.h"

#include "hash.h"
#include "sbcnst2.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

typedef struct _MPID_SMI_shreg_list {
	MPID_SMI_shreg				* shreg;
	int							bucket;
	struct _MPID_SMI_shreg_list	* next;
	struct _MPID_SMI_shreg_list	* prev;
	struct _MPID_SMI_shreg_list * cross;	/* to next bucket */
} MPID_SMI_shreg_list;

typedef struct _aligned_mem_ptr {
	void *alloced_ptr;
	void *aligned_ptr;
} aligned_mem_ptr_t;

static MPID_SMI_shreg		* all_shreg = NULL;
static MPID_SMI_shreg		* pool_shreg = NULL;
static MPID_SMI_shreg_list	** shreg_table = NULL;	/* hash table of shreg */
static MPID_SMI_shreg		*** shreg_table_id = NULL;
static int					* shreg_table_id_sizes = NULL;
static size_t				shreg_table_size = 0;
static int					num_procs = 0;
static MPID_SMI_LOCK_T		mmu_mutex;


static int insert_shreg (MPID_SMI_shreg *);
static MPID_SMI_shreg * create_shreg (size_t, int);
static int remove_shreg (MPID_SMI_shreg *);

static MPID_hash_table_t aligned_ptr_hash;
static MPID_SBHeader aligned_ptr_allocator;

#define MPID_SMI_ID_TO_ADAPTER(id)	((id) & 0xff)
#define MPID_SMI_ID_TO_SGMT(id)		(((unsigned)(id)) >> 8)
#define MPID_SMI_CREATE_ID(adpt,sgmt)	(((sgmt) << 8) | 0xff & (adpt))
#define MPID_SMI_GET_SEARCH_ID(shid) (MPID_SMI_ID_TO_SGMT (shid) % 65536)

size_t make_hash_key(void *data)
{
	return (size_t)((aligned_mem_ptr_t *)data)->aligned_ptr;
}

/*
 * MPID_SMI_Alloc_mem	- allocates good memory for use with SCI
 *
 * input parameters:
 *	size		size of memory to allocate
 *	info		pointer to info structure
 *
 * return value:
 *	allocated memory or NULL on error
 */

void *MPID_SMI_Alloc_mem (size, info)
	size_t 		size;
	MPID_Info 	* info;
{
	MPID_Info	* infoptr;
	int			shared = MAY_BE_SHARED;
	int         alignment = NO_ALIGNMENT;

	for (infoptr = info; infoptr; infoptr = infoptr->next) {
		if (infoptr->key) {
			if (!strcmp (infoptr->key, MPI_INFO_KEY_TYPE)) {
				if (!strcmp (infoptr->value, MPI_INFO_VALUE_TYPE_DEFAULT)) {
					shared = MAY_BE_SHARED;
					continue;
				}
				if (!strcmp (infoptr->value, MPI_INFO_VALUE_TYPE_SHARED)) {
					shared = MUST_BE_SHARED;
					continue;
				}
				if (!strcmp (infoptr->value, MPI_INFO_VALUE_TYPE_PRIVATE)) {
					shared = MUST_BE_PRIVATE;
					continue;
				}
			}
			if (!strcmp (infoptr->key, MPI_INFO_KEY_ALIGN)) {
				alignment = (infoptr->value) ? atoi(infoptr->value) : AUTO_ALIGNMENT;
				continue;
			}
		}
	}

	return MPID_SMI_Alloc_mem_internal (size, shared, alignment);
}


/*
 * MPID_SMI_Alloc_mem_internal	- is the internal version of MPID_SMI_Alloc_mem
 *
 * input parameters:
 *	size	size of memory to allocate
 *	shared	if set to: 
 *          MUST_BE_SHARED the memory *must be* in a shared region or 
 * 					NULL is returned
 *
 * return value:
 *	pointer to allocated memory or NULL on error
 */

static int numrun=-1;

void *MPID_SMI_Alloc_mem_internal (size, shared, alignment)
	size_t 	size;
	int		shared, alignment;
{
	MPID_SMI_shreg	* shregptr;
	aligned_mem_ptr_t *aligned_ptr;
	void *			mem = NULL, *aligned_mem = NULL;
	size_t          alloc_size = size;
	int             align_size = 0, stat_value;

	MPID_STAT_ENTRY(alloc_mem);
	numrun++;
	stat_value = size >> 10;

	/* If alignment is desired, make sure we allocate enough memory to be able
	   to return a correctly aligned address. */
	if (alignment != NO_ALIGNMENT) {
		align_size = (alignment == AUTO_ALIGNMENT) ? MPID_SMI_PAGESIZE : 
						alignment;
		alloc_size = size + align_size;
	} 
	
	if ((shared == MUST_BE_PRIVATE) 
					|| ((shared != MUST_BE_SHARED) 
					&& (alloc_size < MPID_SMI_cfg.ALLOC_MINSIZE))) {
		mem = malloc (alloc_size);
		MPID_STAT_PROBE(alloc_priv, stat_value)
	} else if (alloc_size > MPID_SMI_cfg.ALLOC_POOLSIZE) {
		/* create new shared region */
		shregptr = create_shreg (alloc_size, /* ispool= */ 0);
		if (shregptr) 
			mem = shregptr->base;
		MPID_STAT_PROBE(alloc_shared_reg, stat_value )
	} 
	if (!mem) {
		/* allocate in existing shared region */
		for (shregptr = pool_shreg; shregptr; shregptr = shregptr->next_pool) {
			mem = MPID_SMI_shmalloc (alloc_size, shregptr->id);
			if (mem) {
				MPID_SMI_Local_mem_use (mem);
				MPID_STAT_PROBE(alloc_shared_pool, stat_value )
				break;
			}
		}
		if (!mem) {
			/* Create new pool. Because SMI uses a buddy memory management, 
			   and uses some memory of the pool for internal data, the pool needs to
			   be considerably bigger than the chunk to allocate to let this be successful. */
			if (3*alloc_size < MPID_SMI_cfg.ALLOC_POOLSIZE) 
				shregptr = create_shreg (MPID_SMI_cfg.ALLOC_POOLSIZE, 1);
			else
				shregptr = create_shreg (alloc_size, 0);
			if (shregptr) {
				if (shregptr->ispool) {
					mem = MPID_SMI_shmalloc (alloc_size, shregptr->id);
					MPID_SMI_Local_mem_use (mem);
					MPID_STAT_PROBE(alloc_shared_pool, stat_value )
				} else {
					mem = shregptr->base;
					MPID_STAT_PROBE(alloc_shared_reg, stat_value )
				}
			} else {
				mem = NULL;
			}
		}
	}
	if (!mem) {
		MPID_STAT_PROBE(alloc_failed, stat_value )
		if (shared != MUST_BE_SHARED) {
			/* We should at least try to allocate properly aligned memory 
			   here - preferably page-aligned which will increase DMA 
			   performance. 
			   This is done by allocating bigger chunks and returning 
			   the aligned ptrs from with in the chunk. The original chunk 
			   addresses is stored in a hash table for the free() 
			   operation lateron. */ 
			mem = malloc (alloc_size);
		}
	}

	/* take care of the alignment */
	if (mem && align_size && ((size_t)mem % align_size)) {
		aligned_mem = (char *)mem + align_size - ((size_t)mem % align_size);

		aligned_ptr = MPID_SBalloc (aligned_ptr_allocator);
		aligned_ptr->alloced_ptr = mem;
		aligned_ptr->aligned_ptr = aligned_mem;
		MPID_hash_store (aligned_ptr_hash, aligned_ptr);
	} else {
		aligned_mem = mem;
	}
	

	MPID_STAT_EXIT(alloc_mem);
	return aligned_mem;
}


/*
 * MPID_SMI_Free_mem		- frees memory allocated by MPID_SMI_Alloc_mem
 *
 * input parameters:
 *	ptr		pointer to memory to free
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	1: on success
 *	0: on error
 */
int MPID_SMI_Free_mem (ptr)
	void * ptr;
{
	MPID_SMI_shreg	* shreg;
	aligned_mem_ptr_t *aligned_ptr;
	void *orig_ptr = ptr;
	int is_unique, reg_id;

	MPID_STAT_ENTRY(free_mem);
			
	/* get the original ptr address in case it was an aligned allocation */
	if ((aligned_ptr = MPID_hash_key_remove (aligned_ptr_hash, (size_t)ptr, &is_unique)) != NULL) {
		assert (is_unique != 0);
		orig_ptr = aligned_ptr->alloced_ptr;
		MPID_SBfree (aligned_ptr_allocator, aligned_ptr);
	}

	shreg = MPID_SMI_Get_shreg (orig_ptr);
	if (shreg) {
		/* User free's  shared memory  */
		if (shreg->islocal) {
			if (shreg->ispool) {
				MPID_SMI_shfree (orig_ptr);
				/* Only decrement the usage counter - do not deallocate the segment! */
				MPID_SMI_Local_mem_release (NULL, shreg->id, MPID_SMI_RSRC_CACHE);
			} else {
				/* XXX we should really try to cache local SCI segments here! This would
				   require a different hashing for retrieving available segments. */
				MPID_SMI_Local_mem_release (NULL, shreg->id, MPID_SMI_RSRC_DESTROY);

				MPID_SMI_LOCK (&mmu_mutex);
				remove_shreg (shreg);
				MPID_SMI_UNLOCK (&mmu_mutex);
			}
		} else {
			MPID_STAT_EXIT(free_mem);
			return 0;
		}
	} else {
		/* Although this memory was allocated via malloc(), it might have been
		   registered afterwards. In this case, we need to de-register before free'ing. 
		   We need to look up the SMI region via the aligned ptr as this is how it was
		   registered by SMI.
		   Also, we need to do this in a loop to free *all*  registered memory with 
		   the same starting address - if we wouldn't do this, the region management 
		   will get confused afterwards by multiple regions with the same address, 
		   but different region ids. */
		while (SMI_Adr_to_region (ptr, &reg_id) == SMI_SUCCESS)
			MPID_SMI_Local_mem_release (NULL, reg_id, MPID_SMI_RSRC_DESTROY);
		FREE (orig_ptr);
	}

	MPID_STAT_EXIT(free_mem);
	return 1;
}


/*
 * MPID_SMI_Get_shreg	- returns the shared region which belongs to a given
 *							address
 *
 * input parameters:
 *	ptr		address
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	pointer to a shared region structure or NULL on error
 */
MPID_SMI_shreg *MPID_SMI_Get_shreg (ptr)
	void	* ptr;
{
	MPID_SMI_shreg_list	* shregptr;

	if ((size_t)ptr / MPID_SMI_SHREG_PAGE_SIZE > shreg_table_size) {
		return NULL;
	}
	for (shregptr=shreg_table[(size_t) ptr / MPID_SMI_SHREG_PAGE_SIZE];
					shregptr; shregptr=shregptr->next) {
		if ((size_t)ptr >= (size_t) shregptr->shreg->base && 
						(size_t)ptr < (size_t) shregptr->shreg->base 
											+ shregptr->shreg->size) 
			return shregptr->shreg;
	}
	return NULL;
}


/*
 * MPID_SMI_Get_shreg_by_id	- returns the shared region which belongs to a 
 *								given id (shared id)
 *
 * input parameters:
 *	rank	rank of process to which it does belong
 *	id		shared id (shreg->shid)
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	pointer to a shared region structure or NULL on error
 */
MPID_SMI_shreg *MPID_SMI_Get_shreg_by_id (rank, id)
	int	rank;
	int	id;
{
	int				i;
	int				sid;
	MPID_SMI_shreg	** table;
	MPID_SMI_shreg	* shregptr;

	if (rank < 0 || rank >= num_procs) 
		return NULL;

	sid = MPID_SMI_GET_SEARCH_ID (id);
	if (sid >= shreg_table_id_sizes[rank]) 
		return NULL;
	
	table = shreg_table_id[rank];
	if (!table) 
		return NULL;

	for (shregptr = table[sid]; shregptr; shregptr = shregptr->next_id) {
		if (shregptr->shid == id) 
			return shregptr;
	}
	
	return NULL;
}


/*
 * MPID_SMI_Shreg_connect		- connects to a remote shared region 
 * MPID_SMI_Shreg_tryconnect	- connects only if not already connected
 *
 * input parameters:
 *	rank	- rank of process to connect to
 *	id		- exchangable shared region id (shreg->shid)
 *
 * return value:
 *	1: on success
 *	0: on error
 */
int MPID_SMI_Shreg_connect (rank, id)
	int	rank;
	int	id;
{
	MPID_SMI_shreg		* shreg;
	smi_region_info_t	shreg_info;
	int err;
	
	MPID_SMI_LOCK (&mmu_mutex);
	ZALLOCATE (shreg, MPID_SMI_shreg *, sizeof (MPID_SMI_shreg));

#if 0
	shreg_info.size    = 0;
	shreg_info.adapter = SMI_ADPT_DEFAULT;
	shreg_info.offset  = 0;
	shreg_info.rmt_adapter = MPID_SMI_ID_TO_ADAPTER (id);
	shreg_info.sgmt_id = MPID_SMI_ID_TO_SGMT (id);
	shreg_info.owner = rank;

	/* XXX Currently, the application aborts if this connect fails. But this may happen
	   due to ressource shortage and should be handled via ressource-mngmt. */
	SMIcall (SMI_Create_shreg (SMI_SHM_REMOTE, &shreg_info, &shreg->id, (char **) &shreg->base));
#endif
	/* XXX Change to active resource management: do not map the remote memory
	   here (which is statically), but do it 'on demand'. */
	err = MPID_SMI_Rmt_mem_map (rank, MPID_SMI_ID_TO_SGMT(id), 0, 0, MPID_SMI_ID_TO_ADAPTER (id),
								&shreg->base,  &shreg->id, 0);
	MPID_ASSERT (err == MPI_SUCCESS, "Could not map remote segment for one-sied communication.");
		
	SMIcall (SMI_Query (SMI_Q_SMI_REGION_SIZE, shreg->id, &shreg->size));

	shreg->refcount = 1;
	shreg->shid = id;
	shreg->owner = rank;
	shreg->islocal = 0;
	shreg->ispool = 0;

	if (!insert_shreg (shreg)) {
		FREE (shreg);
		MPID_SMI_UNLOCK (&mmu_mutex);
		return 0;		/* fatal error */
	}
	
	MPID_SMI_UNLOCK (&mmu_mutex);
	return 1;
}

int MPID_SMI_Shreg_tryconnect (rank, id)
	int	rank;
	int	id;
{
	MPID_SMI_shreg	* shreg;

	if (shreg = MPID_SMI_Get_shreg_by_id (rank, id)) {
		if (shreg->islocal) 
			return 0;

		MPID_SMI_LOCK (&mmu_mutex);
		shreg->refcount++;
		MPID_SMI_UNLOCK (&mmu_mutex);

		return 1;
	}
	return MPID_SMI_Shreg_connect (rank, id);
}



/*
 * MPID_SMI_Shreg_disconnect	- disconnects a shareg region
 *
 * input parameters:
 *	shreg		- pointer to shared region struct
 *
 * return value:
 *	1: on success
 *	0: on error
 */	
int MPID_SMI_Shreg_disconnect (shreg)
	MPID_SMI_shreg	* shreg;
{
	if (!shreg) 
		return 0;

	if (shreg->islocal) 
		return 0;
	
	MPID_SMI_LOCK (&mmu_mutex);
	shreg->refcount--;
	if (shreg->refcount == 0) {
		MPID_SMI_Rmt_mem_release (NULL, shreg->id, MPID_SMI_RSRC_DESTROY);
		remove_shreg (shreg);
	}
	MPID_SMI_UNLOCK (&mmu_mutex);

	return 1;
}



/*
 * functions to initialize and finalize
 *
 * return value:
 *	init:		1: on success
 *				0: on error
 *	shutdown:	<none>
 */
int MPID_SMI_MMU_init (void)
{
	SMIcall (SMI_Proc_size (&num_procs));

	/* create hash table and id table*/
	shreg_table_size = MPID_SMI_SHREG_TABLE_SIZE / MPID_SMI_SHREG_PAGE_SIZE;
	ZALLOCATE (shreg_table, MPID_SMI_shreg_list	**, sizeof(void *) * shreg_table_size);
	ZALLOCATE (shreg_table_id, MPID_SMI_shreg ***, num_procs * sizeof (void *));
	ZALLOCATE (shreg_table_id_sizes, int *, num_procs * sizeof (int));

	/* create mmu_mutex for thread safety */
	MPID_SMI_INIT_LOCK (&mmu_mutex);

	/* for aligned memory allocation */
	aligned_ptr_hash = MPID_hash_init (119, sizeof(void *), make_hash_key, MPID_UTIL_THREADSAFE);
	aligned_ptr_allocator = MPID_SBinit (sizeof(aligned_mem_ptr_t), 128, 64);

	return 1;
}

void MPID_SMI_MMU_shutdown (void)
{
	aligned_mem_ptr_t *aligned_ptr;
	MPID_SMI_shreg_list	* listptr,
						* listptr2;
	MPID_SMI_shreg 		* shregptr,
						* shregptr2;
	int					i;

	
	while ((aligned_ptr = (	aligned_mem_ptr_t *)MPID_hash_empty(aligned_ptr_hash)) != NULL)
		MPID_SMI_Free_mem (aligned_ptr->alloced_ptr);
	MPID_hash_destroy (aligned_ptr_hash);
	MPID_SBdestroy (aligned_ptr_allocator);
	
	/* free hash table */	
	for (i=0; i<shreg_table_size; i++) {
		for (listptr=shreg_table[i]; listptr; ) {
			listptr2 = listptr->next;
			FREE (listptr);
			listptr = listptr2;
		}
	}
	FREE (shreg_table);
	
	/* free all shared regions */
	for (shregptr=all_shreg; shregptr; ) {
		MPID_SMI_Rmt_mem_release (NULL, shregptr->id, MPID_SMI_RSRC_DESTROY);
		shregptr2 = shregptr->next;
		FREE (shregptr);
		shregptr = shregptr2;
	}

	/* free id table */
	for (i=0; i<num_procs; i++) {
		FREE (shreg_table_id[i]);
	}
	FREE (shreg_table_id);
	FREE (shreg_table_id_sizes);
	
	/* reset global vars */
	all_shreg = NULL;
	pool_shreg = NULL;
	shreg_table = NULL;
	shreg_table_id = NULL;
	shreg_table_id_sizes = NULL;
	shreg_table_size = 0;
	num_procs = 0;
	
	/* destroy mmu_mutex */
	MPID_SMI_DESTROY_LOCK (&mmu_mutex);

	return;
}


/*
 * MPID_SMI_Addr_to_offset	- maps address to an offset to send
 *
 * input parameters:
 *  ptr		- address to map
 *
 * output parameters:
 *	id		- shared region id
 *	offset	- offset insit this region
 *
 * return value:
 *	1: on success
 *	0: on error
 */
int MPID_SMI_Addr_to_offset (ptr, id, offset)
	void	* ptr;
	int		* id;
	size_t	* offset;
{
	MPID_SMI_shreg	* shreg;

	shreg = MPID_SMI_Get_shreg (ptr);
	if (!shreg) 
		return 0;

	*id = shreg->shid;
	*offset = (size_t) ptr - (size_t) shreg->base;
	return 1;
}

/* MPID_SMI_Offset_to_addr	- maps offset to address (reverse function of
 *								MPID_SMI_Addr_to_offset
 *
 * input parameters:
 *	id		- id of shared region
 *	offset	- offset to map
 *
 * output parameters:
 *	<none>
 *
 * return value:
 *	address on success
 *	NULL	on error
 *
 * remark:
 *	the region must be already connected
 */
void * MPID_SMI_Offset_to_addr (rank, id, offset)
	int		rank;
	int		id;
	size_t	offset;
{
	MPID_SMI_shreg	* shreg;

	shreg = MPID_SMI_Get_shreg_by_id (rank, id);
	if (!shreg) 
		return NULL;

	return (void *) ((size_t) shreg->base + offset);
}


/* 
 * MPID_SMI_Create_shreg	- creates a shared region; this is needed by
 *								the mutex algorithms
 *
 * input parameters:
 *	size	size of shared region
 *
 * return value:
 *	a shared region structure on success
 *	NULL on error
 */
MPID_SMI_shreg * MPID_SMI_Create_shreg (size)
	size_t	size;
{
	return create_shreg (size, 0);
}


/*
 * MPID_SMI_Destroy_shreg 	- destroys a shared region; the contrary function
 *								to MPID_SMI_Create_shreg
 *
 * input parameters:
 *	shreg		the shared region to be destroyed
 *
 * return value:
 *	1: on success
 *	0: on error
 */
int MPID_SMI_Destroy_shreg (shreg)
	MPID_SMI_shreg	* shreg;
{
	if (!shreg) 
		return 0;

	MPID_SMI_Local_mem_release (NULL, shreg->id, MPID_SMI_RSRC_DESTROY);

	if (!remove_shreg (shreg)) 
		return 0;
	return 1;
}


/* local functions */
static int insert_shreg (shreg)
	 MPID_SMI_shreg	* shreg;
{
	size_t 				oldsize;
	int					i;
	MPID_SMI_shreg_list	* shreglist,
						* oldlist,
						** newtable;
	MPID_SMI_shreg		** newidtable,
						** table;
	int					sid;

	/* thread safety is garanteed by calling functions */
	/* adjust size of hash table */
	if ((((size_t) shreg->base + shreg->size) / MPID_SMI_SHREG_PAGE_SIZE 
				+ 1) >= shreg_table_size) {
		oldsize = shreg_table_size;
		shreg_table_size = ((size_t) shreg->base + shreg->size) / 
							MPID_SMI_SHREG_PAGE_SIZE + 1;
		newtable = realloc (shreg_table, sizeof (void *) * 
									shreg_table_size);
		if (!newtable)
			return 0;		/* fatal error */
		shreg_table = newtable;
		memset (shreg_table + oldsize, 0, sizeof (void *) * (
						shreg_table_size - oldsize));
	}
	
	/* fill hash table with new shreg */
	oldlist = NULL;
	for (i = (size_t) shreg->base / MPID_SMI_SHREG_PAGE_SIZE;  
						i * MPID_SMI_SHREG_PAGE_SIZE < (size_t)
						shreg->base + shreg->size; i++) { 
		ZALLOCATE (shreglist, MPID_SMI_shreg_list *, sizeof (MPID_SMI_shreg_list));
		shreglist->shreg = shreg;
		shreglist->bucket = i;
		shreglist->prev = NULL;
		shreglist->next = shreg_table [i];
		if (shreg_table[i])
			shreg_table[i]->prev = shreglist;
		shreg_table[i] = shreglist;
		if (oldlist)
			oldlist->cross = shreglist;
		else
			shreg->list = (void *)shreglist;
		oldlist = shreglist;
	}
	if (oldlist) /* to ensure, that the loop has been entered at least once */
		shreglist->cross = NULL;

	/* insert shreg into pool */
	/* inserting it at the front speeds up the process a little, but causes
		a somewhat higher fragmentation */
	if (shreg->ispool) {
		shreg->next_pool = pool_shreg;
		shreg->prev_pool = NULL;
		if (pool_shreg)
			pool_shreg->prev_pool = shreg;
		pool_shreg = shreg;
	} else {
		shreg->next_pool = NULL;
		shreg->prev_pool = NULL;
	}

	/* insert shreg into linked list */
	shreg->next = all_shreg;
	if (all_shreg)
		all_shreg->prev = shreg;
	all_shreg = shreg;
	shreg->prev = NULL;

	/* insert into id table */
	sid = MPID_SMI_GET_SEARCH_ID (shreg->shid);
	if (!shreg_table_id[shreg->owner]) {
		/* the size must be sid+1, but align it to a multiple of 32 */
		shreg_table_id_sizes[shreg->owner] = ((((sid + 1) + 31) / 32) * 32);
		ZALLOCATE (shreg_table_id[shreg->owner], MPID_SMI_shreg **, 
				   sizeof(void *) * shreg_table_id_sizes[shreg->owner]);
		table = shreg_table_id[shreg->owner];
	} else if (sid >= shreg_table_id_sizes[shreg->owner]) {
		oldsize = shreg_table_id_sizes[shreg->owner];
		/* the new size must be sid+1, but align it to a multiple of 32 */
		shreg_table_id_sizes[shreg->owner] = ((((sid + 1) + 31) / 32) * 32);
		newidtable = realloc (shreg_table_id[shreg->owner], 
					 	sizeof(void *) * shreg_table_id_sizes[shreg->owner]);
		if (!newidtable) 
			return 0;
		shreg_table_id[shreg->owner] = newidtable;
		table = newidtable;
		memset (table+oldsize, 0, sizeof(void *) *
						(shreg_table_id_sizes[shreg->owner] - oldsize));
	} else {
		table = shreg_table_id[shreg->owner];
	}
	shreg->next_id = table[sid];
	table[sid] = shreg;
	if (shreg->next_id)
		shreg->next_id->prev_id = shreg;
	shreg->prev_id = NULL;

	return 1;
}


static int remove_shreg (shreg)
	MPID_SMI_shreg	* shreg;
{
	MPID_SMI_shreg_list	* list,
						* list2;
	MPID_SMI_shreg		** table;
	int					i;
	int					sid;

	/* thread safety is garanteed by calling functions */
	/* remove from linked list */
	if (shreg->prev)
		shreg->prev->next = shreg->next;
	else
		all_shreg = shreg->next;
	if (shreg->next)
		shreg->next->prev = shreg->prev;

	/* remove from table */
	list = (MPID_SMI_shreg_list *) shreg->list;
	while (list) {
		if (list->next)
			list->next->prev = list->prev;
		if (list->prev)
			list->prev->next = list->next;
		else
			shreg_table[list->bucket] = list->next;
		list2 = list->cross;
		FREE (list);
		list = list2;
	}

	/* remove from pool */
	if (shreg->ispool) {
		if (shreg->next_pool)
			shreg->next_pool->prev_pool = shreg->prev_pool;
		if (shreg->prev_pool)
			shreg->prev_pool->next_pool = shreg->next_pool;
		else
			pool_shreg = shreg->next_pool;
	}

	/* remove from id table */
	if (shreg->next_id)
		shreg->next_id->prev_id = shreg->prev_id;
	if (shreg->prev_id) {
		shreg->prev_id->next_id = shreg->next_id;
	} else {
		sid = MPID_SMI_GET_SEARCH_ID (shreg->id);
		table = shreg_table_id[shreg->owner];
		if (table) 
			table[sid] = shreg->next;
	}

	FREE (shreg);
	return 1;
}


static MPID_SMI_shreg *create_shreg (size, ispool)
	size_t	size;
	int		ispool;
{
	MPID_SMI_shreg		* shreg;
	size_t				sgmnt_size;
	int					i;
	int					sgmtid, adptnr;

	ZALLOCATE (shreg, MPID_SMI_shreg *, sizeof (MPID_SMI_shreg));

	/* create shared region */
	sgmnt_size = ispool ? MPID_SMI_cfg.ALLOC_POOLSIZE : size;
	if (MPID_SMI_Local_mem_create (&sgmnt_size, size, &shreg->base, &shreg->id, &sgmtid)
		!= MPI_SUCCESS) {
		FREE (shreg);
		return NULL;
	}

	if (sgmnt_size == size) 
		ispool = 0;

	shreg->size    = sgmnt_size;
	shreg->ispool  = ispool;
	shreg->islocal = 1;
	shreg->owner   = MPID_SMI_myid;

	MPID_SMI_LOCK (&mmu_mutex);

	SMIcall (SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, shreg->id, &adptnr));
	shreg->shid = MPID_SMI_CREATE_ID (adptnr, sgmtid);

	/* Establish dynamic memory management for a pool area. */
	if (ispool) 
		SMIcall (SMI_Init_shregMMU (shreg->id));

	/* insert shared region into local lists */
	if (!insert_shreg (shreg)) {
		FREE (shreg);
		MPID_SMI_UNLOCK (&mmu_mutex);
		return NULL;		/* fatal error */
	}

	/* Finally, we 'release' the local memory if this is a memory pool (but cache it), to set its usage
	   counter to 0 if no memory from it is in use. */
	if (ispool)
	    MPID_SMI_Local_mem_release (NULL, shreg->id, MPID_SMI_RSRC_CACHE);

	MPID_SMI_UNLOCK (&mmu_mutex);

	return shreg;
}
	










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
