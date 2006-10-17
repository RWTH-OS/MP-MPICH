/* $Id$ */

#include "env/general_definitions.h"
#include "utility/general.h"
#include "copy.h"


#if defined(WIN32) && defined(_M_IX86)
#define MOVE8(s, d) { __asm { mov eax, dword ptr [s] } \
					  __asm { fild qword ptr [eax] } \
					  __asm { mov eax, dword ptr [d] } \
					  __asm { fistp qword ptr [eax] } }
#else
#define MOVE8(s, d) { *d = *s; }
#endif

 


/********************************************************************************/
/*** copies the data of the specified shared region into each processes'      ***/
/*** local region; do this in chunks of  8 byte; if mode & ONE_PER_NODE, than ***/
/*** the local region is a shared region of all processes of this node ->     ***/
/*** split work accordingly.                                                  ***/
/********************************************************************************/
smi_error_t _smi_copy_from_to(void* from, void* to, size_t byte, int mode)
 {
   volatile double* f;
   volatile double* t;
   size_t i;
   size_t iter_pre            = 8 - (((size_t)from) % 8);
   size_t iter_8			  = (byte-iter_pre) / 8;
   size_t iter_rest           = byte-iter_pre-8*iter_8;
   volatile double* from_chunk = (volatile double*)(((char*)from)+iter_pre);
   volatile double* to_chunk   = (volatile double*)(((char*)to)+iter_pre);
   int procs_on_node          = _smi_no_processes_on_machine(_smi_my_machine_rank);
   int rank_on_machine        = _smi_local_rank(_smi_my_proc_rank);



   if (!(mode & SMI_REP_ONE_PER_NODE) || ((mode & SMI_REP_ONE_PER_NODE) && rank_on_machine==0))
      for(i=0;i<iter_pre;i++)
	 ((char*)to)[i]=((char*)from)[i];

   if (!(mode & SMI_REP_ONE_PER_NODE))
   {
      for(i=0;i<iter_8;i++)
	  {
		  f	= &(from_chunk[i]);
		  t	= &(to_chunk[i]);	
		  MOVE8(f, t);
	  }
   }
   else
   {
      for(i=rank_on_machine;i<iter_8;i+=rank_on_machine)
	  {
		  f	= &(from_chunk[i]);
		  t	= &(to_chunk[i]);	
		  MOVE8(f, t);	
	  }
   }
	 
   
   if (!(mode & SMI_REP_ONE_PER_NODE) || ((mode & SMI_REP_ONE_PER_NODE) && rank_on_machine==0))
      for(i=iter_pre+8*iter_8;i<iter_pre+8*iter_8+iter_rest;i++)
	 ((char*)to)[i]=((char*)from)[i];


   return(SMI_SUCCESS);
 }


