/* $Id$ */

#include "env/smidebug.h"
#include "memtree.h"
#include "region_layout.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* securely get the region for a given region id */
region_t *_smi_get_region (int shreg_id) {
    region_t *RetVal;

    SMI_LOCK(&_smi_mis_lock);

    RetVal = (shreg_id >= 0 && shreg_id < _smi_mis.no_regions 
	&& _smi_mis.region[shreg_id]->id == shreg_id) ?
	_smi_mis.region[shreg_id] : NULL;
    
    SMI_UNLOCK(&_smi_mis_lock);
    
    return(RetVal);
}

/*********************************************************************************/
/* Returns the region number of the segment into which the specified address     */
/* falls. Only searches in the user region, not the internal regions !           */
/* Possible return value: NULL (if the address is not contained in any region)   */  
/*********************************************************************************/ 
region_t *_smi_address_to_region(char* address)
{
  DSECTION("_smi_address_to_region");
  region_t *reg = NULL;
  memtree_memarea_t memarea;
  int region_id;
  
  DSECTENTRYPOINT;
  DNOTICEP("Searching region if for address =",address);
  
  memarea.pStart = address;
  memarea.iSize = 1;
  region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);

  SMI_LOCK(&_smi_mis_lock);

  /* do not search the address in the internal regions */
  if (region_id >= _smi_nbr_machines) {
      DNOTICEI ("Region id is ", region_id);
      reg = _smi_mis.region[region_id];
  } else {
      DNOTICE ("address does not belong to any SMI region");
  }

  SMI_UNLOCK(&_smi_mis_lock);

  DSECTLEAVE;
  return(reg);
}

int _smi_address_to_region_id(char* address)
{
  DSECTION("_smi_address_to_region");
  memtree_memarea_t memarea;
  int region_id;

  DSECTENTRYPOINT;
  DNOTICEP("Searching region if for address =",address);

  memarea.pStart = address;
  memarea.iSize = 1;
  region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);

  DSECTLEAVE;
  return(region_id);
}



static region_t *_smi_address_to_region_internal(char* address) 
{
  DSECTION("_smi_address_to_region_internal");
  region_t *reg = NULL;
  memtree_memarea_t memarea;
  int region_id = 0;
  
  DSECTENTRYPOINT;
  DNOTICEP("Searching internal region if for address =",address);

  memarea.pStart = address;
  memarea.iSize = 1;
  region_id = _smi_memtree_getsegid(_smi_memtree, &memarea);

  SMI_LOCK(&_smi_mis_lock);
  
  /* do only search the address in the internal regions */
  if (region_id > 0 && region_id < _smi_nbr_machines) {
      DNOTICEI ("Internal region id is ", region_id);
      reg = _smi_mis.region[region_id];
  } else {
      DNOTICE ("address does not belong to an internal SMI region");
  }
  
  SMI_UNLOCK(&_smi_mis_lock);
  
  return(reg);
}


int _smi_address_to_node(char* address)
{
  DSECTION ("_smi_address_to_node"); 
  region_t *reg;
  int s;
  
  DSECTENTRYPOINT;

  reg = _smi_address_to_region(address);
  ASSERT_R((reg != NULL), "Address bis not located on any node", -1);

  /* accelerate the easy case */
  if (reg->no_segments == 1) {
      DSECTLEAVE;
      return (reg->seg[0]->machine);
  }

  /* we *have* to find it now! */
  for (s = 0; s < reg->no_segments; s++) {
    if ((size_t)address >= (size_t)(reg->seg[s]->address)
	&& (size_t)address < (size_t)(reg->seg[s]->address)+reg->seg[s]->size) {
      break;
    }
  }
  
  DSECTLEAVE;
  return (reg->seg[s]->machine);
}

size_t _smi_get_region_size (int region_id) 
{
    DSECTION("_smi_get_region_size");
    size_t RetVal;
    
    DSECTENTRYPOINT;
  
    SMI_LOCK(&_smi_mis_lock);
    
    if (region_id < 0 || region_id >= _smi_mis.no_regions || _smi_mis.region[region_id] == NULL) {
	DWARNING ("Illegal region id");
	RetVal = 0;
    }
    else {
	if (_smi_mis.region[region_id]->type != SMI_SHM_FRAGMENTED) {
	    RetVal = _smi_mis.region[region_id]->size;
	} else {
	    /* For a fragmented region, only the size of one of the (identical sized)
	       segments makes sense. */
	    RetVal = _smi_mis.region[region_id]->seg[0]->size;
	}
    }
    
    SMI_UNLOCK(&_smi_mis_lock);
    DSECTLEAVE; return(RetVal);
}


/**********************************************************************/
/* This functions allocates a structure of type 'rlayout_t' and       */
/* returnes in it the layout of a shared memory region. I.e. the      */
/* number of segments of that it is composed, their sizes (in Byte),  */
/* their start address and their physical machine location. In case   */
/* that SMI has not been initialized before, this function returnes   */
/* SMI_ERR_NOINIT. In the case that a region with the specified ID    */
/* does not exists or is an internal one, SMI_ERR_PARAM is returned.  */
/**********************************************************************/
smi_error_t SMI_Region_layout(int region_id, smi_rlayout_t** r)
 {
   size_t i;
   region_t* region;
   
   DSECTION("SMI_Region_layout");
   DSECTENTRYPOINT;

   ASSERT_R((_smi_initialized==true),"SMI is not initialized",SMI_ERR_NOINIT);
   
   region = _smi_get_region (region_id);
   ASSERT_R((region != NULL),"Could not find region",SMI_ERR_PARAM);

   /* check, whether it is an address of an internal region */
   for (i=0;i<(size_t)_smi_nbr_machines;i++)
     if (region_id == _smi_int_shreg_id[i])
       ASSERT_R((0),"Region is an internal region",SMI_ERR_PARAM);
   
   /* assemble the return structure */
   ALLOCATE (*r,  smi_rlayout_t *, sizeof(smi_rlayout_t));
   (*r)->nsegments = region->no_segments;
   (*r)->size      = region->size;
   (*r)->adr       = region->addresses[0];
   ALLOCATE((*r)->seg_size, size_t*, (*r)->nsegments * sizeof(int));
   ALLOCATE((*r)->seg_adr, char**, (*r)->nsegments * sizeof(char*));
   ALLOCATE((*r)->seg_machine, int *, (*r)->nsegments * sizeof(int));

   for(i=0;i<(*r)->nsegments;i++) {
      (*r)->seg_size[i]    = region->seg[i]->size; 
      (*r)->seg_adr[i]     = region->seg[i]->address;       
      (*r)->seg_machine[i] = region->seg[i]->machine; 
   }
   
   DSECTLEAVE;
   return(SMI_SUCCESS);
 }


