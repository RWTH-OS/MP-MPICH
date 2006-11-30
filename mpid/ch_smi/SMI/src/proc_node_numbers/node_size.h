/* $Id$ */

#ifndef _NODE_SIZE_H_
#define _NODE_SIZE_H_


#include "env/general_definitions.h"


#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************/
/*** This function delivers the total number of nodes, on which processes      ***/
/*** reside for execution.                                                     ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/
smi_error_t SMI_Node_size(int*);


#ifdef __cplusplus
}
#endif

#endif
