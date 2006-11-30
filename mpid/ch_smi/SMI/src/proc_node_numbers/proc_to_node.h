/* $Id$ */

#ifndef _PROC_TO_NODE_H_
#define _PROC_TO_NODE_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************/
/*** This function delivers the machine rank of the specified process as it    ***/
/*** results after the reordering that was performed to guarantee that         ***/
/*** processes on the same machine have consecutive ranks.                     ***/
/*** Possible Errors: EMI_ERR_NOINIT, SMI_ERR_PARAM (the proc-parameter is an  ***/
/*** invalid quantity).                                                        ***/
/*********************************************************************************/ 
smi_error_t SMI_Proc_to_node(int, int*);

#ifdef __cplusplus
}
#endif


#endif

