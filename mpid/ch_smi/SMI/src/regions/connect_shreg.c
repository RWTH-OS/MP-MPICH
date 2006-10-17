/* $Id$ */

/* connect to a delayed created region */

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "utility/general.h"
#include "memory/shmem.h"

#include "connect_shreg.h"
#include "region_layout.h"

#ifdef NO_SISCI
smi_error_t SMI_Connect_shreg (int region_id, void** address)
{
  DSECTION("SMI_Connect_shreg");
  DPROBLEM("not implemented in SMP version!");
  return(SMI_ERR_NOTIMPL);
}
#else

smi_error_t SMI_Connect_shreg (int region_id, void** address)
{
    region_t* region = NULL;
    shseg_t* shseg;
    smi_error_t error;
    sci_error_t sci_error; 

    DSECTION ("SMI_Connect_shreg");
    DSECTENTRYPOINT;

    region = _smi_get_region(region_id);
    ASSERT_R((region != NULL), "Could not find region", SMI_ERR_PARAM);
    ASSERT_R((region->no_segments == 1), "Wrong region type (consists of more than one segment)", SMI_ERR_PARAM);

    shseg = region->seg[0];  
    /* is connection required? */
    if (!(shseg->flags & SHREG_DELAYED)) {
	DWARNING ("Region is already connected");
	DSECTLEAVE;
	return (SMI_SUCCESS);
    }

    /* ok, then connect to the related segment */
    do {
	rs_SCIConnectSegment(shseg->fd, &(shseg->segment), _smi_sci_rank[shseg->owner], shseg->id, 
			     shseg->adapter, NULL, NULL, SCI_INFINITE_TIMEOUT, shseg->connect_flag, &sci_error);
    } while ( sci_error != SCI_ERR_OK );
    DNOTICEI("delay-connected to new SCI segment, ID", shseg->id);
    shseg->flags &= ~SHREG_DELAYED;
    
    /* map the segment */
    error = _smi_map_sci_shared_segment(shseg);
    ASSERT_R((error == SMI_SUCCESS), "Mapping of delayed segment failed", SMI_ERR_MAPFAILED);

    region->addresses[0] = *address = shseg->address;
    
    DSECTLEAVE;
    return (error);
}
#endif
