/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "smidebug.h"
#include "page_size.h"




/*********************************************************************************/
/*** This function delivers the smalles common multiple of the page sizes      ***/
/*** within the cluster. Only possible Error: SMI_ERR_NOINIT.                  ***/
/*********************************************************************************/
smi_error_t SMI_Page_size(int* size)
 {
   DSECTION("SMI_Page_size");
   
   DSECTENTRYPOINT;
   
   ASSERT_R((_smi_initialized == true),"SMI not initialized",SMI_ERR_NOINIT);
   
   *size = _smi_page_size;
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  
