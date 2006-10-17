/* $Id: proc_size.h,v 1.1 2004/03/19 22:14:19 joachim Exp $ */

#ifndef _SMI_PROC_SIZE_H_
#define _SMI_PROC_SIZE_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Deliver the number of processes running on the specified node. */
int _smi_procs_on_node (int node);

/*** This function delivers the total number of processes.                     ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
smi_error_t SMI_Proc_size(int*);

/*** This function delivers the total number of processes just on the machine  ***/
/*** of the calling process.                                                   ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
smi_error_t SMI_Local_proc_size(int* size);

/*** This function delivers the maximum among all SMI_Local_proc_size on all   ***/
/*** machines.                                                                 ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
smi_error_t SMI_Max_local_proc_size(int* size);



#ifdef __cplusplus
}
#endif


#endif
