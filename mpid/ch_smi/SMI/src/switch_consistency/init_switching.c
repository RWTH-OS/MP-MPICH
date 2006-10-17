/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "env/general_definitions.h"
#include "init_switching.h"
#include "dyn_mem/dyn_mem.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

int* loop_bound_array;

  
/********************************************************************************/
/***  Initialization.                                                         ***/
/********************************************************************************/
int _smi_init_switching()
 {
   DSECTION("_smi_init_switching");
   smi_error_t error;
   
   /* To allow an exchange of loop-bounds between all processes */
   /* of splittet loops, _smi_allocate a piece of memory.            */
   /* loop_bound_array[2*i] and [2*i+1] contain the lower and   */
   /* upper lop baund of a loop-splitting of process i.         */

   DSECTENTRYPOINT;

   error = SMI_Cmalloc(2*_smi_nbr_procs*sizeof(int), 0|INTERNAL,
		       (char**)&loop_bound_array);
   ASSERT_R((error==SMI_SUCCESS),"Could not allocate memory",error);
 
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
  

  
