/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "address_to_region.h"
#include "memtree.h"


/**********************************************************************/
/* return the region id that corresponds to the given address. This   */
/* functions return SMI_ERR_PARAM in the case that the address does   */
/* not belong to any user.allocated shared memory region and          */
/* SMI_ERR_NOINIT in the case the SMI_Init has not been called before */
/**********************************************************************/
smi_error_t SMI_Adr_to_region(void* address, int* region_id)
{
    memtree_memarea_t memarea;
    int retval = SMI_SUCCESS;
    
    DSECTION("SMI_Adr_to_region");    
    DSECTENTRYPOINT;
    
    ASSERT_R(_smi_initialized==true, "SMI is not initialized",SMI_ERR_NOINIT);

    SMI_LOCK(&_smi_region_lock);
    DNOTICEP("Looking up region id for address", address);
    memarea.pStart = address;
    memarea.iSize = 1;
    *region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);

    if (*region_id >= 0) {
	/* check, whether it is an address of an internal region */
	/* internal regions have id's [0 .. _smi_nbr_machines-1 ) */
	if (*region_id >= 0 && *region_id < _smi_nbr_machines) {
	    retval = SMI_ERR_PARAM;
	    DNOTICE("Region is an internal region");
	} else {
	    DNOTICEI ("Address is located in region with id", *region_id);
	} 
    } else {
	DNOTICE("Address is not contained in any SMI region");
	retval = SMI_ERR_PARAM;
    }    
    
    DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return retval;
}

smi_error_t SMI_Range_to_region(void* address, size_t len, int* region_id)
{
    memtree_memarea_t memarea;    
    int retval = SMI_SUCCESS;
   
    DSECTION("SMI_Range_to_region");
    DSECTENTRYPOINT;
    
    ASSERT_R(_smi_initialized==true, "SMI is not initialized",SMI_ERR_NOINIT);

    SMI_LOCK(&_smi_region_lock);

    DNOTICEP("Looking up region id for address", address);
    DNOTICEP("                            size", len);
    memarea.pStart = address;
    memarea.iSize = len;
    *region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);

    if (*region_id >= 0) {
	/* check, whether it is an address of an internal region */
	/* internal regions have id's [0 .. _smi_nbr_machines-1 ) */
	if (*region_id >= 0 && *region_id < _smi_nbr_machines) {
	    retval = SMI_ERR_PARAM;
	    DNOTICE("Region is an internal region");
	} else {
	    DNOTICEI ("Range is located in region with id", *region_id);
	}
    } else {
	DNOTICE("Range is not contained in any SMI region");
	retval = SMI_ERR_PARAM;
    }    
    
    DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return retval;
}


void *_smi_get_region_address(int region_id) {
    int sgmt;
    void *RetVal;
    
    DSECTION("_smi_get_region_address");
    DSECTENTRYPOINT;
    
    SMI_LOCK(&_smi_mis_lock);
    if (region_id < 0 || region_id >= _smi_mis.no_regions || _smi_mis.region[region_id] == NULL) {
	DWARNING ("Illegal region id");
	SMI_UNLOCK(&_smi_mis_lock);
	DSECTLEAVE; return (NULL);
    }
    
    if (_smi_mis.region[region_id]->no_segments == 1) {
	RetVal = (_smi_mis.region[region_id]->addresses)[0];
	SMI_UNLOCK(&_smi_mis_lock);
	DSECTLEAVE; return (RetVal);
    }
    
    for (sgmt = 0; sgmt < _smi_mis.region[region_id]->no_segments; sgmt++) {
	if (_smi_mis.region[region_id]->seg[sgmt]->owner == _smi_my_proc_rank) {
	    RetVal = _smi_mis.region[region_id]->seg[sgmt]->address;
	    SMI_UNLOCK(&_smi_mis_lock);
	    DSECTLEAVE; return (RetVal);
	}
    }
    
    SMI_UNLOCK(&_smi_mis_lock);
    DPROBLEMI ("could not determine address for region, id =", region_id);
    DSECTLEAVE; return (NULL);
}


smi_error_t _smi_shseg_to_region(shseg_t* shseg, int* region_id)
 {
   memtree_memarea_t memarea;    
   boolean found = false;

   REMDSECTION("SMI_Shseg_to_region");
   DSECTENTRYPOINT;

   ASSERT_R((_smi_initialized == true),"SMI not initialized",SMI_ERR_NOINIT);

   /* search for the shseg in all (!) regions */   
   memarea.pStart = shseg->address;
   memarea.iSize  = shseg->size;
   *region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);
   if (*region_id >= 0)
     found = true;
   ASSERT_R(( found == true ),"Could not find shseg",SMI_ERR_PARAM);
  
   /* check, whether it is an shseg of an internal region */
   if (*region_id >= 0 && *region_id < _smi_nbr_machines) {
       DWARNING ("Shseg is part of an internal region");
       DSECTLEAVE; return (SMI_ERR_OTHER);
   }

   DSECTLEAVE; return (SMI_SUCCESS);
}


static int _smi_addr_to_node (char *addr) 
{
    int id;

    /* XXX this does not work! */
    return SMI_Adr_to_region (addr, &id);
}

shseg_t *_smi_addr_to_shseg(void *addr) 
{
    region_t* r;
    shseg_t *s = NULL;
    int region_id, j;
    memtree_memarea_t memarea;
    
    DSECTION("_smi_addr_to_shseg");
    DSECTENTRYPOINT;
    
    /* search for the address in all regions */
    memarea.pStart = addr;
    memarea.iSize = 1;
    region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);
    
    if (region_id >= 0) {
	SMI_LOCK(&_smi_mis_lock);
	r = _smi_mis.region[region_id];

	if (r->no_segments > 1) {
	    for (j = 0; j < r->no_segments; j++) {
		if (((size_t)addr >= (size_t)r->seg[j]->address) 
		    && ((size_t)addr < (size_t)r->seg[j]->address + r->seg[j]->size)) {
		    s = r->seg[j];
		}
	    }
	} else {
	    s = r->seg[0];
	}
	SMI_UNLOCK(&_smi_mis_lock);
    }

    DSECTLEAVE; return s;
}

shseg_t *_smi_range_to_shseg(char *addr, size_t len) 
{
    region_t* r;
    shseg_t *s = NULL;
    int region_id, j;
    memtree_memarea_t memarea;
    
    DSECTION("_smi_range_to_shseg");
    DSECTENTRYPOINT;
    
    /* search for the address in all regions */
    memarea.pStart = addr;
    memarea.iSize  = len;
    region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);
    
    if (region_id >= 0) {
	SMI_LOCK(&_smi_mis_lock);
	r = _smi_mis.region[region_id];

	if (r->no_segments > 1) {
	    for (j = 0; j < r->no_segments; j++) {
		if ((addr >= r->seg[j]->address) && (addr < r->seg[j]->address + r->seg[j]->size)) {
		    s = r->seg[j];
		}
	    }
	} else {
	    s = r->seg[0];
	}
	SMI_UNLOCK(&_smi_mis_lock);
    }

    DSECTLEAVE; return s;
}

shseg_t* _smi_regid_to_shseg(int regid) 
{
    shseg_t* RetVal;
    
    SMI_LOCK(&_smi_mis_lock);
    RetVal = (IS_VALID_ID(regid)) ? _smi_mis.region[regid]->seg[0] : NULL;
    SMI_UNLOCK(&_smi_mis_lock);
    
    return(RetVal);
}
