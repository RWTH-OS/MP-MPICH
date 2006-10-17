/* $Id: page_size.h,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#ifndef _PAGE_SIZE_H_
#define _PAGE_SIZE_H_

#include "general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************************/
/*** This function delivers the smalles common multiple of the page sizes      ***/
/*** within the cluster. Only possible Error: SMI_ERR_NOINIT.                  ***/
/*********************************************************************************/
smi_error_t SMI_Page_size(int*);

#ifdef __cplusplus
}
#endif


#endif
