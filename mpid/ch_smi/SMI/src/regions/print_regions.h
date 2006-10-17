/* $Id: print_regions.h,v 1.1 2004/03/19 22:14:20 joachim Exp $ */

#ifndef _PRINT_REGIONS_H_
#define _PRINT_REGIONS_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


void _smi_print_segment(shseg_t*);
void _smi_print_regions(void);

#ifdef __cplusplus
}
#endif


#endif
