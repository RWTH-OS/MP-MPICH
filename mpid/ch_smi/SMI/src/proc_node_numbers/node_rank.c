/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "node_rank.h"




/*********************************************************************************/
/*** This function delivers the machine rank of the calling process as it      ***/
/*** results after the reordering that was performed to guarantee that         ***/
/*** processes on the same machine have consecutive ranks.                     ***/
/*** Possible Errors: SMI_ERR_NOINIT.                                          ***/
/*********************************************************************************/ 
smi_error_t SMI_Node_rank(int* node)
 {
   DSECTION("SMI_Node_rank");
   
   DSECTENTRYPOINT;
   
   ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);
   
   *node = _smi_machine_rank[_smi_my_proc_rank];
  
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  
