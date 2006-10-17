/* $Id: redirect_io.c,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#define _DEBUG_EXTERN_REC
#include "smidebug.h"

#include "redirect_io.h"
#include "finalize.h"



static int smi_already_redirected = 0;


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
			int in, void* inparam)
{
  DSECTION(" SMI_Redirect_IO");
  char filename[256];
  
  DSECTENTRYPOINT;

  ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);
  
  /* It is just allowed to call this function once. */
  /* If it is called multiple times, we just return */
  if (smi_already_redirected != 0) {
    DNOTICE("Output is already redirected");
    DSECTLEAVE
      return(SMI_SUCCESS);
  }  
  
  if (err == SMI_IO_FILE)
    {
      sprintf(filename,"%s.%i",errparam,_smi_my_proc_rank);
      fclose(stderr);
      fopen(filename, "w");
      smi_already_redirected = 1;
    }

  if (out == SMI_IO_FILE)
    {
      sprintf(filename,"%s.%i",outparam,_smi_my_proc_rank);
      fclose(stdout);
      fopen(filename, "w");
      smi_already_redirected = 1;
    }
  
  if (in == SMI_IO_FILE)
    {
      fclose(stdin);
      fopen((const char*)inparam, "r");
      smi_already_redirected = 1;
    }
  
  DSECTLEAVE
    return(SMI_SUCCESS);
}


/*********************************************************************************/
/*** This function is called from SMI_Finalize to ensure that everything goes  ***/
/*** all right with redirected output.                                         ***/
/*********************************************************************************/
void _smi_close_redirected_io()
{
  fflush(stderr);
  fflush(stdout);
  if (smi_already_redirected != 0) {
    fclose(stderr); 
    fclose(stdout);
  }
}

