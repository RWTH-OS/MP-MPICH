/* $Id$ */

#ifndef _SMI_CREATE_SHREG_H_
#define _SMI_CREATE_SHREG_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

smi_error_t _smi_prepare_for_new_region(int*, region_t**);
smi_error_t _smi_create_region_data_structure(int, smi_region_info_t*, int*, char**, 
					  region_t**, device_t, unsigned int);
/* XXXX to be removed 
smi_error_t SMI_Init_reginfo(smi_region_info_t* region_desc, int size, int offset, int owner, int loc_adpt, 
			 int rmt_adpt, int sgmt_id, smi_region_callback_t cb_fcn);
smi_error_t SMI_Create_shreg(int, smi_region_info_t *, int*, void **);
*/
#ifdef __cplusplus
}
#endif


#endif
