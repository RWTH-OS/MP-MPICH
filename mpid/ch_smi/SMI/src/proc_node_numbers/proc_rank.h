/* $Id: proc_rank.h,v 1.1 2004/03/19 22:14:19 joachim Exp $ */

#ifndef _SMI_PROC_RANK_H_
#define _SMI_PROC_RANK_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/* return the node-relative rank of a process */
int _smi_local_proc_rank (int proc);

/*********************************************************************************/
/*** This function delivers the rank of the calling process as it results      ***/
/*** after the reordering that was performed to guarantee that processes on    ***/
/*** the same machine have consecutive ranks.                                  ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/
smi_error_t SMI_Proc_rank(int*);


/*********************************************************************************/
/*** This function delivers the rank of the calling process within it's own    ***/
/*** machine solely.                                                           ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/
smi_error_t SMI_Local_proc_rank(int* rank);


#ifdef __cplusplus
}
#endif


#endif
