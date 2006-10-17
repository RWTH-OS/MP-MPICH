/* $Id$ */

/********************************************************************************/
/*** This file maps FORTRAN calls to SMI functions to the corresponding C     ***/
/*** functions. All functions dealing with memory allocation/releas are not   ***/
/*** contained, because FORTRAN does not support pointers. Furthermore        ***/
/*** SMI_Init/Finalize is not contained.                                      ***/ 
/********************************************************************************/

#include "smi.h"
#include "general_definitions.h"
#include "fortran_binding.h"
#include "finalize.h"
#include "smi_init.h"
#include "proc_node_numbers/proc_rank.h"
#include "proc_node_numbers/proc_size.h"
#include "proc_node_numbers/node_rank.h"
#include "proc_node_numbers/node_size.h"
#include "proc_node_numbers/proc_to_node.h"
#include "regions/create_shreg.h"
#include "regions/free_shreg.h"
#include "regions/address_to_region.h"
#include "synchronization/barrier.h"
#include "synchronization/mutex.h"
#include "switch_consistency/switch_to_sharing.h"
#include "switch_consistency/switch_to_replication.h"
#include "switch_consistency/ensure_consistency.h"
#include "dyn_mem/dyn_mem.h"
#include "page_size.h"
#include "utility/smi_time.h"
#include "loop_scheduling/loop_interface.h"
#include "redirect_io.h"

#define YES 1
#define NO 0

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/********************************************************************************/
/********************************************************************************/
void smi_init_(lint* error)
 {
   int*    argc;
   char*** argv;
#ifndef WIN32
   int i;
#endif

#ifdef WIN32
   argc = &__argc;
   argv = &__argv;
#else
   ALLOCATE( argc, int*, sizeof(int) );
   *argc = n_arg_();
   ALLOCATE( argv, char***, sizeof(char**) );
   ALLOCATE( *argv, char**, sizeof(char*) );
   for(i=0;i<*argc;i++)
     get_arg_(&i,&(*argv[i]));
#endif

   error->h = 0;
   error->l = SMI_Init(argc, argv);
 }

  
/********************************************************************************/
/********************************************************************************/  
void smi_finalize_(lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Finalize();
 }

  
/********************************************************************************/
/********************************************************************************/  
void smi_proc_rank_(lint* rank, lint* error)
 {
#ifdef INT8
   error->h = 0;
   rank->h  = 0;
#endif
   error->l = SMI_Proc_rank(&(rank->l));
 }


  
/********************************************************************************/
/********************************************************************************/  
void smi_proc_size_(lint* size, lint* error)
 {
#ifdef INT8
   error->h = 0;
   size->h  = 0;
#endif
   error->l = SMI_Proc_size(&(size->l));
 }

  
/********************************************************************************/
/********************************************************************************/  
void smi_node_rank_(lint* rank, lint* error)
 {
#ifdef INT8
   error->h = 0;
   rank->h  = 0;
#endif
   error->l = SMI_Node_rank(&(rank->l));
 }


  
/********************************************************************************/
/********************************************************************************/  
void smi_node_size_(lint* size, lint* error)
 {
#ifdef INT8
   error->h = 0;
   size->h  = 0;
#endif
   error->l = SMI_Node_size(&(size->l));
 }

  
/********************************************************************************/
/********************************************************************************/  
void smi_proc_to_node_(lint* proc, lint* node, lint* error)
 {
#ifdef INT8
   error->h = 0;
   node->h  = 0;
#endif
   error->l = SMI_Proc_to_node(proc->l, &(node->l));
 }

  
/********************************************************************************/
/********************************************************************************/  
void smi_adr_to_region_(int* adr, lint* region_id, lint* error)
 {
#ifdef INT8
   error->h      = 0;
   region_id->h  = 0;
#endif
   error->l = SMI_Adr_to_region((char*)adr, &(region_id->l));
 }


/********************************************************************************/
/********************************************************************************/  
void smi_page_size_(lint* pz, lint* error)
 {
#ifdef INT8
   error->h = 0;
   pz->h    = 0;
#endif
   error->l = SMI_Page_size(&(pz->l));
 }

  
/********************************************************************************/
/* XXX structs/union not supported by F77 -> workaround required                */
/********************************************************************************/
void smi_create_shreg_(lint *region_type, int* region_info, lint* id, int* address, lint* error)
{
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Create_shreg(region_type->l, (smi_region_info_t *)region_info, &(id->l), 
			       (char**)address);
   
 }

  
/********************************************************************************/
/********************************************************************************/
void smi_free_shreg_(lint* id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Free_shreg(id->l);
 }


/********************************************************************************/
/********************************************************************************/
void smi_mutex_init_(lint* id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Mutex_init(&(id->l));   
 }


/********************************************************************************/
/********************************************************************************/
void smi_mutex_init_with_locality_(lint* id, lint* prank, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_MUTEX_INIT(&(id->l), 1, prank->l);   
 }

  
/********************************************************************************/
/********************************************************************************/
void smi_mutex_lock_(lint* id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Mutex_lock(id->l);     
 }


/********************************************************************************/
/********************************************************************************/
void smi_mutex_unlock_(lint* id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Mutex_unlock(id->l);      
 }

  
/********************************************************************************/
/********************************************************************************/
void smi_mutex_destroy_(lint* id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Mutex_destroy(id->l);      
 }

  
/********************************************************************************/
/********************************************************************************/
void smi_barrier_(lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Barrier();      
 }

  
/********************************************************************************/
/********************************************************************************/
void smi_init_shregmmu_(lint* region_id, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Init_shregMMU(region_id->l);
 }


/********************************************************************************/
/********************************************************************************/
void smi_imalloc_(lint* size, lint* region_id, int* address, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Imalloc(size->l, region_id->l, (char**)address);
 }
  

/********************************************************************************/
/********************************************************************************/
void smi_cmalloc_(lint* size, lint* region_id, int* address, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Cmalloc(size->l, region_id->l, (char**)address);
 }


/********************************************************************************/
/********************************************************************************/
void smi_ifree_(int* adr, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Ifree((char*)adr);
 }


/********************************************************************************/
/********************************************************************************/
void smi_cfree_(int* adr, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Cfree((char*)adr);
 }


/********************************************************************************/
/********************************************************************************/
void smi_get_timer_(lint* sec, lint* microsec, lint* error)
 {
#ifdef INT8
   error->h    = 0;
   sec->h      = 0;
   microsec->h = 0;
#endif
   error->l = SMI_Get_timer(&(sec->l), &(microsec->l));
 }


/********************************************************************************/
/********************************************************************************/
void smi_get_timespan_(lint* sec, lint* microsec, lint* error)
 {
#ifdef INT8
   error->h    = 0;
   sec->h      = 0;
   microsec->h = 0;
#endif
   error->l = SMI_Get_timespan(&(sec->l), &(microsec->l));
 }


/********************************************************************************/
/********************************************************************************/
void smi_redirect_io_(lint* err, void* errparam,
					  lint* out, void* outparam,
					  lint* in , void* inparam, 
					  lint* error)
{
#ifdef INT8
  error->h  = 0;
#endif
   error->l = SMI_Redirect_IO(err->l, errparam, out->l, outparam, in->l, inparam);
}


#if (FULL_SMI)
/***/  
/********************************************************************************/
/********************************************************************************/
void smi_switch_to_replication_(lint* id, lint* mode,
				lint* param1, lint* param2,
				lint* param3, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Switch_to_replication(id->l, mode->l,
					param1->l, param2->l, param3->l);
 }


/***/
/********************************************************************************/
/********************************************************************************/
void smi_switch_to_sharing_(lint* id, lint* comb_mode,
			    lint* comb_param1, lint* comb_param2, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Switch_to_sharing(id->l, comb_mode->l,
				    comb_param1->l, comb_param2->l);
 }

/***/
/********************************************************************************/
/********************************************************************************/
void smi_ensure_consistency_(lint* id, lint* comb_mode,
			    lint* comb_param1, lint* comb_param2, lint* error)
 {
#ifdef INT8
   error->h = 0;
#endif
   error->l = SMI_Ensure_consistency(id->l, comb_mode->l,
				    comb_param1->l, comb_param2->l);
 }

/***/
/********************************************************************************/
/********************************************************************************/
void smi_loop_init_(lint* id, lint* low, lint* high, lint* mode, lint* error) 
{
#ifdef INT8
   error->h = 0;
   id->h    = 0;
#endif
   error->l = SMI_Loop_init(&(id->l), low->l, high->l, mode->l);
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_get_iterations_(lint* id, lint* status, lint* low, lint* high, lint* error)
{
#ifdef INT8
   error->h  = 0;
   status->h = 0;
   low->h    = 0;
   high->h   = 0;
#endif
   error->l = SMI_Get_iterations(id->l, &(status->l), &(low->l), &(high->l));
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_loop_free_(lint* id, lint* error)
{
#ifdef INT8
   error->h  = 0;
#endif
   error->l = SMI_Loop_free(id->l); 
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_evaluate_speed_(double* speedarray, lint* error)
{
#ifdef INT8
   error->h  = 0;
#endif
   error->l = SMI_Evaluate_speed(speedarray);
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_use_evaluated_speed_(lint* id, lint* error)
{
#ifdef INT8
  error->h  = 0;
#endif
   error->l = SMI_Use_evaluated_speed(id->l);
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_set_loop_param_(lint* id, double* k, lint* minl, lint* minr, 
						 lint* maxl, lint* maxr, lint* error)
{
#ifdef INT8
  error->h  = 0;
#endif
   error->l = SMI_Set_loop_param(id->l, *k, minl->l, minr->l, maxl->l, maxr->l);
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_set_loop_help_param_(lint* id, lint* helpdist, lint* error)
{
#ifdef INT8
  error->h  = 0;
#endif
   error->l = SMI_Set_loop_help_param(id->l, helpdist->l);
}

/***/
/********************************************************************************/
/********************************************************************************/
void smi_loop_k_adaption_mode_(lint* id, lint* adap_mode, lint* maxcalcdist, lint* error)
{
#ifdef INT8
  error->h  = 0;
#endif
   error->l = SMI_Loop_k_adaption_mode(id->l, adap_mode->l, maxcalcdist->l);
}

#endif /* (FULL_SMI) */



