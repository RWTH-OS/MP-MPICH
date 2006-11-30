/* $Id$ */

#ifndef _REDIRECT_IO_H_
#define _REDIRECT_IO_H_

#include "general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************************/
/*** This function allows a user to redirect stderr, stdout and stdin. stderr  ***/
/*** and stdout can be redirected to a file. In this case, 'err' and/or' 'out' ***/
/*** have to be SMI_IO_FILE and 'errparam' and/or 'outparam' the coresponding  ***/
/*** filename. To distinduish between output from different processes, a       ***/
/*** '.proc_rank' suffix is appended to the name. Alternatively, one or both   ***/
/*** can be redirected to a frontend. In this case, SMI_IO_FRONTEND has to be  ***/
/*** specified. This option is currently available only under Windows NT. In a ***/
/*** UNIX environment, this optionn is simply ignored. stdin can be redirected ***/
/*** in that all processes read from a single file, whose filename has to be   ***/
/*** specified in 'inparam'. If one or several of the streams shall remain     ***/
/*** unchanged, SMI_IO_ASIS can be specified.  		                           ***/
/*** Possible errors:														   ***/
/*** SMI_ERR_NOINIT, SMI_ERR_OTHER (in the case that the redirection to a      ***/
/*** frontend fails), or 2xxx in case that a open to a file fails.             ***/
/*********************************************************************************/
smi_error_t SMI_Redirect_IO(int err, void* errparam, 
						int out, void* outparam, 
						int in, void* inparam);


/*********************************************************************************/
/*** This function is called from SMI_Finalize to ensure that everything goes  ***/
/*** all right with redirected output.                                         ***/
/*********************************************************************************/
void _smi_close_redirected_io(void);


#ifdef __cplusplus
}
#endif


#endif
