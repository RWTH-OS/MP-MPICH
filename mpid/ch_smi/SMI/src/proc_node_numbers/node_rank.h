/* $Id: node_rank.h,v 1.1 2004/03/19 22:14:18 joachim Exp $ */

#ifndef _NODE_RANK_H_
#define _NODE_RANK_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************************/
/*** This function delivers the machine rank of the calling process as it      ***/
/*** results after the reordering that was performed to guarantee that         ***/
/*** processes on the same machine have consecutive ranks.                     ***/
/*** Possible Errors: SMI_ERR_NOINIT.                                          ***/
/*********************************************************************************/ 
smi_error_t SMI_Node_rank(int*);

#ifdef __cplusplus
}
#endif



#endif
