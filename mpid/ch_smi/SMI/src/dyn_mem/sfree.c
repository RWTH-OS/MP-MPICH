/* $Id$ */

#include "dyn_mem.h"
#include "env/smidebug.h"

static void _smi_dynmem_release (smi_memmgr_t *mgr_node, smi_memmgr_t *free_list,
								 smi_memmgr_t *free_node)
{
    DSECTION("_smi_dynmem_release");
	
	DSECTENTRYPOINT;
	
    /* 
	   Is the manager node a leaf? Then it has to match the adress to be free'd!
	   The pointers to the child nodes should either be both equal or both unequal
	   to NULL (otherwise the data structures are corrupt anyway), so it suffices
	   to test one of them 
	*/
    if( mgr_node->sb_l == NULL ) {
		if ((uint *)free_node != (uint *)mgr_node->b_addr) {
			DERRORP("memory address to be free'd not found:", free_node);
		} else {
			mgr_node->sb_avail = mgr_node->b_size;
			mgr_node->mem_avail = mgr_node->b_size;
			DNOTICEI("marked block as available, size", mgr_node->mem_avail);
		}
		DSECTLEAVE return;
    } else  {
		/* Continue search to the left or to the right? */
		if ((size_t)free_node < ((size_t)mgr_node->b_addr + (size_t)mgr_node->b_size/2)) {
			_smi_dynmem_release (mgr_node->sb_l, free_list, free_node);
		} else {   
			_smi_dynmem_release (mgr_node->sb_r, free_list, free_node);
		}
    }
    
	DNOTICEI("in: sb_avail",mgr_node->sb_avail);
    mgr_node->sb_avail  = mgr_node->sb_l->sb_avail  | mgr_node->sb_r->sb_avail;
	mgr_node->mem_avail = mgr_node->sb_l->mem_avail + mgr_node->sb_r->mem_avail;
	DNOTICEI("out: sb_avail",mgr_node->sb_avail);
	
	
    /* Join free neighboring blocks back into one bigger block. */
    if( (mgr_node->sb_l->b_size == mgr_node->sb_l->mem_avail) &&
		(mgr_node->sb_r->b_size == mgr_node->sb_r->mem_avail) )	{
		
		mgr_node->sb_avail = mgr_node->b_size;
		mgr_node->mem_avail = mgr_node->b_size;
		mgr_node->sb_r->sb_l = free_list->sb_l;
		mgr_node->sb_l->sb_l = mgr_node->sb_r;
		free_list->sb_l   = mgr_node->sb_l;
		
		free_list->b_size += 2;
		mgr_node->sb_l = NULL;
		mgr_node->sb_r = NULL;
		DNOTICEI("joined two block into one with size", mgr_node->sb_avail);
		DNOTICEP("free_list->sb_l", free_list->sb_l);
    }
	
	DSECTLEAVE return;  
}


void _smi_dynmem_free (void *region_base, void *adr)
{
	smi_memmgr_t *free_adr;
	smi_memmgr_t *mgr_node;
	
	DSECTION("_smi_dynmem_free");
	
	free_adr = (smi_memmgr_t *)adr;
	mgr_node = (smi_memmgr_t *)region_base;
	
	DNOTICEP("mgr_node is" ,mgr_node);
	DNOTICEI("on entry - remaining memory is", mgr_node->sb_avail);
	
	/* 
	   Is the adress to be free'd actually located located in the 
	   managed region? If not, just return. 
	*/
	if ((free_adr < mgr_node) || 
		((size_t)free_adr > (size_t)region_base + (size_t)mgr_node->b_size)) {
		DERRORP("Illegal address to be free'd:", adr);
		return;
	}
	
	_smi_dynmem_release (mgr_node, mgr_node+1, free_adr);
	DNOTICEI("MMU free'ing done - remaining memory is", mgr_node->sb_avail);
	
	return;
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



