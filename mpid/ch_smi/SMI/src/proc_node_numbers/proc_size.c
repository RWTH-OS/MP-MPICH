/* $Id: proc_size.c,v 1.1 2004/03/19 22:14:19 joachim Exp $ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "proc_size.h"




/*********************************************************************************/
/*** This function delivers the total number of processes.                     ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/ 
smi_error_t SMI_Proc_size(int* size)
 {
   DSECTION("SMI_Proc_size");
   
   DSECTENTRYPOINT;

   ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);

   *size = _smi_nbr_procs;

   DSECTLEAVE
     return(SMI_SUCCESS);
 }


int _smi_procs_on_node (int node)
{
   int i, procs = 0;

   for(i =0 ; i < _smi_nbr_procs; i++)
       if (_smi_machine_rank[i] == node)
	   procs++;
   
   return (procs);
}

/*********************************************************************************/
/*** This function delivers the total number of processes just on the machine  ***/
/*** of the calling process.                                                   ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/ 
smi_error_t SMI_Local_proc_size(int* size)
 {
   DSECTION("SMI_Local_proc_size");
   DSECTENTRYPOINT;
    
   ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);
   
   *size = _smi_procs_on_node(_smi_my_machine_rank);

   DSECTLEAVE;  return(SMI_SUCCESS);
 }

  


/*********************************************************************************/
/*** This function delivers the maximum among all SMI_Local_proc_size on all   ***/
/*** machines.                                                                 ***/
/*** Only possible Error: SMI_ERR_NOINIT.                                      ***/
/*********************************************************************************/ 
smi_error_t SMI_Max_local_proc_size(int* size)
 {
   DSECTION("SMI_Max_local_proc_size");
   int i,j,tmp;

   DSECTENTRYPOINT;
   
   ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);
  
   *size = 0;
   for(i=0;i<_smi_nbr_machines;i++)
     {
       tmp=0;
       for(j=0;j<_smi_nbr_procs;j++)
	 if (_smi_machine_rank[j]==i)
	   tmp++;
       if(tmp>*size)
	 *size=tmp;
     }
  
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  


