/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "node_size.h"


/*********************************************************************************/
/*** This function delivers the total number of nodes, on which processes      ***/
/*** reside for execution.                                                     ***/
/*** Only possible error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/ 
smi_error_t SMI_Node_size(int* size)
{
  DSECTION("SMI_Node_size");

  DSECTENTRYPOINT;

  ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);

  *size = _smi_nbr_machines;
  
  DSECTLEAVE
    return(SMI_SUCCESS);
}

  
