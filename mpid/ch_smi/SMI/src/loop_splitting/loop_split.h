/* $Id$ */

#ifndef _LOOP_SPLIT_H_
#define _LOOP_SPLIT_H_

#include "env/general_definitions.h"

void _smi_get_local(int, int, int, int, void**, int*);
smi_error_t SMI_Loop_split_init(int*);
smi_error_t SMI_Loop_index_range(int, int*, int*, int);
smi_error_t SMI_Determine_loop_splitting(int, int, int, int);
smi_error_t SMI_Loop_time_start(int);
smi_error_t SMI_Loop_time_stop(int);
smi_error_t SMI_Loop_balance_index_range(int);

#endif
