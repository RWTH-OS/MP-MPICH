/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "env/general_definitions.h"
#include "first_proc_on_node.h"




  
/*********************************************************************************/
/*** Returns the process rank of the process with the lowest rank residing on  ***/
/*** the specified node; Library internal function, it is assumed that the     ***/
/*** parameter is valid.                                                       ***/
/*********************************************************************************/ 
int _smi_first_proc_on_node(int node)
 {
   int proc = 0;
 
   while (_smi_machine_rank[proc] != node)
      proc++;
 
   return(proc);
 }

smi_error_t SMI_First_proc_on_node(int node, int* proc)
{
  REMDSECTION("SMI_First_proc_on_node");

  DSECTENTRYPOINT;

  ASSERT_R((_smi_initialized==true),"SMI not initialized",SMI_ERR_NOINIT);
  
  ASSERT_R(( (node>=0)&&(node<_smi_nbr_machines) ), "Illegal node-id",SMI_ERR_PARAM);

  *proc = 0;
  while (_smi_machine_rank[*proc]!=node)
    (*proc)++;
  
  DSECTLEAVE
    return(SMI_SUCCESS);
}
 


  
/*********************************************************************************/
/*** Returns the process rank of the process with the highest rank residing on ***/
/*** the specified node; Library internal function. No error checking, it is   ***/
/*** assumed that the  parameter is valid.                                     ***/
/*********************************************************************************/ 
int _smi_last_proc_on_node(int node)
 {
   int proc;
   int i;

   for(i=0;i<_smi_nbr_procs;i++)
      if(_smi_machine_rank[i]==node)
	 proc=i;
 
   return(proc);
 }
 
