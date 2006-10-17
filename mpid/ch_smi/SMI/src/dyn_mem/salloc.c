/* $Id$ */

#include "dyn_mem.h"
#include "env/smidebug.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define SMI_SAL_MIN_BLOCK 128


void *_smi_dynmem_alloc (void *region_base, uint reqsz)
{
	
    DSECTION ("_smi_dynmem_alloc");
	uint tmp = 0;
	smi_memmgr_t *mgr_node = NULL, *free_list = NULL, *node = NULL;
	
	/* DSETON */
	if (reqsz == 0)
		return NULL;
	
	mgr_node = (smi_memmgr_t *)region_base;
	free_list = mgr_node + 1;
	
	/* block size needs to be power of 2 */
	tmp = SMI_SAL_MIN_BLOCK;
	while (tmp < reqsz)
		tmp <<= 1;
	reqsz = tmp;
	
	DNOTICEI("Allocating block, size:", reqsz);
	node = _smi_dynmem_memtree (mgr_node, free_list, reqsz);
	if( node == 0 ) {
		DPROBLEM("Unable to allocate requested amount of memory");
		return NULL;
	}
	
#if 0
	/* Check capacity of free_list - add capacity, if necessary. */
	if (free_list->b_size < MIN_FREELIST_ENTRIES) {
		if (mgr_node->sb_avail < FREELIST_SZ) {
			DERROR("insufficient memory for dynamic memory management");
			return NULL;
		}
		
		freenode = _smi_dynmem_memtree (mgr_node, free_list, FREELIST_SZ); 
		free_list = _smi_free_list_init((smi_memmgr_t *)freenode->b_addr, free_list);
	}
#endif
	
	DNOTICEI("MMU allocation done - remaining memory is", mgr_node->sb_avail);
	return (void *)node->b_addr;
}

/*
  INPUT parameters:
  reqsz: size in byte of requested memory space, must be a power of 2
  node: pointer to root node from which on to search downwards
  free_list: pointer to start of list of free blocks (blocks are taken from there if one node is to be split)

  RETURN value:
  pointer to block responsible for managing memory, NULL if something went wrong
*/

smi_memmgr_t *_smi_dynmem_memtree(smi_memmgr_t *node, smi_memmgr_t *free_list, int reqsz)
{
    DSECTION("_smi_dynmem_memtree");
    size_t current_sz;
    smi_memmgr_t *subnode = NULL, *foundnode = NULL;
    
    DSECTENTRYPOINT;
    
	/* sanity checks */
    if (node == NULL) {
		DNOTICE("node is null");
		DSECTLEAVE return(NULL);
    }
    
	if( reqsz == 0 ) {
		DNOTICE("requested memory is null");
		DSECTLEAVE return( NULL );
	}
	
	/* stop searching for memory here */
	if( node->sb_avail < reqsz ) {
		DNOTICE("not enough memory available");
		DSECTLEAVE return(NULL);
	}
	
    /* Is the current node a leaf?; the pointers to the child nodes should either be
	   both equal or unequal to NULL (otherwise the MMU data structures are corrupt anyway), so
	   it suffices to test one of them */
    if( node->sb_l == NULL ) { 
		if( (node->b_size == (unsigned int)reqsz) && (node->sb_avail == (unsigned int)reqsz) ) {
			/* perfect fit */
			DNOTICEI("found matching node - avail:", node->sb_avail);
			node->sb_avail = 0;
			node->mem_avail = 0;
			
			DSECTLEAVE return node;
		} 
		else {
			ASSERT_R(free_list != NULL,"free list root missing!",NULL);
			DNOTICEP("free_list->sb_l", free_list->sb_l);
			DNOTICEI("free_list->b_size", free_list->b_size);
			
			/* Block needs to be split in two halfs. */
			DNOTICE("found bigger node (leaf), splitting");
			current_sz = node->b_size >> 1; 
			node->sb_avail &= ~(node->b_size); 
			
			/* check if splitting is possible (there enough nodes) */
			ASSERT_R(free_list->sb_l != NULL, "no node in freelist(1)!", NULL);
			ASSERT_R(free_list->sb_l->sb_l != NULL, "no node in freelist(2)!", NULL);
			
			/* generate new left son */
			subnode = free_list->sb_l;  /* first block in list of free blocks */
			node->sb_l = subnode;
			free_list->sb_l = subnode->sb_l; /* new first block in list of free blocks */
			subnode->sb_l = NULL;            /* new node is a leaf */
			subnode->sb_r = NULL;
			subnode->b_addr = node->b_addr;    /* left son gets first half of block */
			subnode->b_size = current_sz;    
			subnode->sb_avail = current_sz;  /* this half is initially fully available */
			subnode->mem_avail = current_sz;
			
			/* generate new right son */
			subnode = free_list->sb_l;  /* fist block in list of free blocks */
			node->sb_r = subnode; 
			free_list->sb_l = subnode->sb_l; /* new first block in list of free blocks */
			subnode->sb_l = NULL;            /* new node is a leaf */
			subnode->sb_r = NULL;
			subnode->b_addr = node->b_addr + current_sz; /* right son gets second half of block */
			subnode->b_size = current_sz;    
			subnode->sb_avail = current_sz;  /* this half is initially fully available */
			subnode->mem_avail = current_sz;
			
			/* we have two free entries less available */
			free_list->b_size -= 2; 
			DNOTICEP("free_list->sb_l", free_list->sb_l);
			DNOTICEI("free_list->b_size", free_list->b_size);
			
			/* continue search with new, smaller block; because the requested size is a power of 2 and
			   less than node->b_size, this search must somewhen lead to a fitting memory block, so it suffices
			   to continue the search in the left branch. */
			foundnode = _smi_dynmem_memtree (node->sb_l, free_list, reqsz);
		}
    } 
	else {
		
		/* Current node is not a leaf continue on the left or on the right. */
		DNOTICE("found bigger node ...");
		
		if (node->sb_l) {
			/* go left down if a block of the requested size is available there */
			if ((node->sb_l->sb_avail & reqsz) > 0) {
				DNOTICE("... moving left");
				foundnode = _smi_dynmem_memtree (node->sb_l, free_list, reqsz);
			}
			/* go right down if a block of the requested size is available there */
			else if ((node->sb_r->sb_avail & reqsz) > 0)  {
				DNOTICE("... moving right");
				foundnode = _smi_dynmem_memtree (node->sb_r, free_list, reqsz);
			}
			/* We should have found a block now if there was one with the requested size available in one of the subtrees.
			   Otherwise, we must continue our search. But, there is no guarantee that one of these searches succeeds. */
			
			/* go left down if it's enough memory there available (some bigger block must be split, though) */
			if( (foundnode == NULL ) && ((node->sb_l->sb_avail > reqsz) > 0) ) {
				DNOTICE("... moving left");
				foundnode = _smi_dynmem_memtree (node->sb_l, free_list, reqsz);
			}
			/* go right down if it's enough memory there available (some bigger block must be split, though) */
			if( (foundnode == NULL ) && ((node->sb_r->sb_avail > reqsz) > 0) )  {
				DNOTICE("... moving right");
				foundnode = _smi_dynmem_memtree (node->sb_r, free_list, reqsz);
			}
			
		}
    }
	
	if( foundnode != NULL ) {
		
		/* There has been some block found below this node => update the information of this node depending on the information in the
		   subnoodes (propagates the information back to the root) */
		
		
		node->sb_avail = 0;
		node->mem_avail = 0;
		if (node->sb_l != NULL) {
			node->sb_avail |= node->sb_l->sb_avail;
			node->mem_avail += node->sb_l->mem_avail;
		}
		if (node->sb_r != NULL) {
			node->sb_avail |= node->sb_r->sb_avail;
			node->mem_avail += node->sb_r->mem_avail;
		}		
	}
	
	DNOTICEP("leaving with node:", node);
	DNOTICEI("             node->b_size:", node->b_size);     /* size of this block */
	DNOTICEP("             node->b_addr:", node->b_addr);       /* adress of this block */
	DNOTICEI("             node->sb_avail:", node->sb_avail); /* available subblocks of this block (bitfield) */
	DNOTICEI("             node->sb_avail L:", node->sb_l->sb_avail); /* available subblocks of left block (bitfield) */
	DNOTICEI("             node->sb_avail R:", node->sb_r->sb_avail); /* available subblocks of right block (bitfield) */
	DNOTICEP("       and result:", foundnode);
	
    DSECTLEAVE;
	
    return foundnode;    
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



