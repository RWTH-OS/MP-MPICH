/* $Id: switch_to_sharing.h,v 1.1 2004/03/19 22:14:22 joachim Exp $ */

#ifndef _SWITCH_TO_SHARING_H_
#define _SWITCH_TO_SHARING_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


smi_error_t _smi_combine_add_double(region_t*, region_t*, int, int, int);
smi_error_t _smi_combine_loop_splitting(int, int, region_t*, region_t*);
smi_error_t SMI_Switch_to_sharing(int, int, int, int);

#ifdef __cplusplus
}
#endif


#endif
