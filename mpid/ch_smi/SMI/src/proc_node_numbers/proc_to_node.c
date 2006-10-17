/* $Id: proc_to_node.c,v 1.1 2004/03/19 22:14:19 joachim Exp $ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "proc_to_node.h"




/*********************************************************************************/
/*** This function delivers the machine rank of the specified process as it    ***/
/*** results after the reordering that was performed to guarantee that         ***/
/*** processes on the same machine have consecutive ranks.                     ***/
/*** Possible Errors: EMI_ERR_NOINIT, SMI_ERR_PARAM (the proc-parameter is an  ***/
/*** invalid quantity).                                                        ***/
/*********************************************************************************/ 
smi_error_t SMI_Proc_to_node(int proc, int* node)
 {
   REMDSECTION("SMI_Proc_to_node");
  
   DSECTENTRYPOINT;
   
   ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);

   ASSERT_R(( (proc>=0)&&(proc<_smi_nbr_procs) ), "Illegal proc-id",SMI_ERR_PARAM);
   
   *node = _smi_machine_rank[proc];

   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  
