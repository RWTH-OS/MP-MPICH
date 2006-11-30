/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "proc_rank.h"
#include "first_proc_on_node.h"



/*********************************************************************************/
/*** This function delivers the rank of the calling process as it results      ***/
/*** after the reordering that was performed to guarantee that processes on    ***/
/*** the same machine have consecutive ranks.                                  ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/
smi_error_t SMI_Proc_rank(int* rank)
 {
   DSECTION("SMI_Proc_rank");

   ASSERT_R(_smi_initialized,"SMI not initialized",SMI_ERR_NOINIT);
  
   *rank = _smi_my_proc_rank;
   
   return(SMI_SUCCESS);
 }

/* return the node-relative rank of a process */
int _smi_local_proc_rank (int proc)
{
    return (proc - _smi_first_proc_on_node(_smi_machine_rank[proc]));
}
  
/*********************************************************************************/
/*** This function delivers the rank of the calling process within it's own    ***/
/*** machine solely.                                                           ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/
smi_error_t SMI_Local_proc_rank(int *rank)
{
  DSECTION("SMI_Local_proc_rank");
  
  ASSERT_R(_smi_initialized, "SMI not initialized", SMI_ERR_NOINIT); 

  *rank = _smi_local_proc_rank (_smi_my_proc_rank);
  
  return (SMI_SUCCESS);
}
