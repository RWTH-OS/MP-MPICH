/* $Id$ */

#ifndef _SMI_ADDRESS_TO_REGION_H_
#define _SMI_ADDRESS_TO_REGION_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************/
/* return the region id that corresponds to the given address. This   */
/* functions return SMI_ERR_PARAM in the case that the address does   */
/* not belong to any user-_smi_allocated shared memory region and          */
/* SMI_ERR_NOINIT in the case the SMI_Init has not been called before */
/**********************************************************************/
smi_error_t SMI_Adr_to_region (void *address, int *region_id);
smi_error_t SMI_Range_to_region (void *address, size_t len, int *region_id);

/* internal functions */
smi_error_t _smi_shseg_to_region(shseg_t* shseg, int* region_id);
shseg_t *_smi_addr_to_shseg(void *addr);
shseg_t *_smi_range_to_shseg(char *addr, size_t len);
shseg_t* _smi_regid_to_shseg(int regid);
void *_smi_get_region_address(int region_id);


#ifdef __cplusplus
}
#endif


#endif
