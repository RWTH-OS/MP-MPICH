/* $Id$ */

/*********************************************************************************/
/*********************************************************************************/
/***                                                                           ***/
/*** This module implements progress counter as a mechanism of synchronization.***/
/*** A progress counter (PC) lets processes express their own progress in some ***/
/*** computation process to others. It has a "at least" interpretation, i.e.   ***/
/*** a progress value of another process means that this reached at least the  ***/
/*** seen progress but is maybe already ahead of this state. There are several ***/
/*** functions to enforce a process to wait until others have made at least a  ***/
/*** specified progress.                                                       ***/
/***                                                                           ***/
/*********************************************************************************/
/*********************************************************************************/

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "progress.h"
#include "env/general_definitions.h"
#include "env/safety.h"
#include "message_passing/lowlevelmp.h"
#include "store_barrier.h"
#include "mutex.h"

#include "barrier.h"
#include "dyn_mem/dyn_mem.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/* A Progress counter is an integer value for each process that is visible to all */
/* other processes. To remove remote polling when waiting for progress of others, */
/* the PC of each process is replicated to each other process' local memory.      */
/* First index states the process to that the memory is local, second index the   */
/* process id whose PC in stored therein.                                         */
typedef struct
{
  volatile int** counters;
  int own_counter;
} smi_pc_t;



static smi_pc_t* pc        = NULL;
static int       max_pc_id = -1;

#ifdef WIN32
#pragma optimize("", off)
#endif


/*********************************************************************************/
/* Set-up a new progress counter. It's id is returned. This is a collective call!*/
/* Possible errors: SMI_ERR_NOINIT and SMI_ERR_NOMEM                             */
/*********************************************************************************/
smi_error_t SMI_Init_PC(int* id)
{
  smi_error_t   error;
  int       i;
  smi_pc_t* tmp;

  DSECTION("SMI_Init_PC");
  DSECTENTRYPOINT;
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  /* enlarge table of PCs */
  ALLOCATE (tmp, smi_pc_t*, (max_pc_id+2)*sizeof(smi_pc_t));
  for ( i=0; i <= max_pc_id; i++)
      tmp[i] = pc[i];
  if (max_pc_id != -1)
      free(pc);
  pc = tmp;
  
  max_pc_id++;
  *id = max_pc_id;
  
  /* allocate memory for PC counters */
  ALLOCATE(pc[*id].counters, volatile int**, _smi_nbr_procs*sizeof(int*));
  for(i = 0; i < _smi_nbr_procs; i++) {
      error = SMI_Cmalloc(_smi_nbr_procs*sizeof(int)*INTS_PER_STREAM, i|INTERNAL,
			  (void **)&(pc[*id].counters[i]));
      ASSERT_R((error==SMI_SUCCESS),"Could not allocate memory",error);
  }
  
  /* Set initial value and sync, to ensure that no process uses the PC before */
  /* all processes have initialized it. */
  
  for(i = 0; i < _smi_nbr_procs; i++) {
    SMI_Check_transfer_proc(i,0);
    do {
      SEC_SET(pc[*id].counters[i][SEC_INDEX(_smi_my_proc_rank)],0);
    }
    while( SMI_Check_transfer_proc(i,0) != SMI_SUCCESS);
  }
  pc[*id].own_counter = 0;

  _smi_ll_barrier();
  
  DSECTLEAVE;
  return(SMI_SUCCESS);
}



/*********************************************************************************/
/* Reset the specified pc to zero. This is a collective function call !          */
/* possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id wrong)                     */
/*********************************************************************************/
smi_error_t SMI_Reset_PC(int id)
{
  DSECTION("SMI_Reset_PC");
  int i;
 
  DSECTENTRYPOINT;
  
  if (id & NOCONSISTENCY)
    id -= NOCONSISTENCY;
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  ASSERT_R(( (id>=0)&&(id<=max_pc_id) ), "ID is invalid", SMI_ERR_PARAM);

  /* First, we wait until all process have entered this function. This is   */
  /* necessary to ensure that no process still needs one of the PC counters */
  /* that are reset here.                                                   */
  _smi_ll_barrier();

  /* now, reset */
  for(i=0;i<_smi_nbr_procs;i++)
    SEC_SET(pc[id].counters[i][SEC_INDEX(_smi_my_proc_rank)],0);
  pc[id].own_counter = 0;
  _smi_ll_barrier();
  
  DSECTLEAVE
    return(SMI_SUCCESS);
}


/*********************************************************************************/
/* Increment the own part of the specified pc by a provided quantity. This is an */
/* individual call. Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id wrong).   */
/*********************************************************************************/
smi_error_t SMI_Increment_PC(int id, int val)
{
  DSECTION("SMI_Increment_PC");
  int i;
  int no_consistency = 0;
  
  DSECTENTRYPOINT;

  if (id & NOCONSISTENCY) {
      id -= NOCONSISTENCY;
      no_consistency = 1;
  }

  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  ASSERT_R(( (id>=0)&&(id<=max_pc_id) ), "ID is invalid", SMI_ERR_PARAM);
  
  if (no_consistency == 0)
      _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1);

  DNOTICEI("preinc: pc[id].own_counter",pc[id].own_counter);

  (pc[id].own_counter) += val;
  for (i = 0; i < _smi_nbr_procs; i++) {
      SEC_SET(pc[id].counters[i][SEC_INDEX(_smi_my_proc_rank)], pc[id].own_counter);
  } 

  DNOTICEI("postinc: pc[id].own_counter",pc[id].own_counter);

  DSECTLEAVE;
  return(SMI_SUCCESS);
}


/*********************************************************************************/
/* Get a pc-counter vaule (returned in pc_val) of the specified pc and the       */
/* specified process. This is an individual call. Possible errors:               */
/* SMI_ERR_NOINIT, SMI_ERR_PARAM (pcid or proc_id wrong).                        */
/*********************************************************************************/
smi_error_t SMI_Get_PC(int pcid, int proc_id, int* pc_val)
{
  DSECTION("SMI_Get_PC");
  int no_consistency = 0;

  DSECTENTRYPOINT;
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT); 
  
  if(pcid & NOCONSISTENCY)
    {
      pcid -= NOCONSISTENCY;
      no_consistency = 1;
    }
  
  ASSERT_R(( (pcid>=0)&&(pcid<=max_pc_id)&&(proc_id<_smi_nbr_procs) ), "ID is invalid", SMI_ERR_PARAM);
  
  *pc_val = *((volatile int*)&(pc[pcid].counters[_smi_my_proc_rank][SEC_INDEX(proc_id)]));
  
  if(no_consistency==0)
    _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);
  
  DSECTLEAVE;
  return(SMI_SUCCESS);
}



/*********************************************************************************/
/* Waiting untill the progress counter of a specified pc (id) and of a specified */
/* process reaches a specified value. If val=SMI_OWNPC is specified, it is waited*/
/* until the own pc values has been reached. This is a individual call. Possible */
/* errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong).                */
/*********************************************************************************/
smi_error_t SMI_Wait_individual_PC(int id, int proc_rank, int val)
{
  DSECTION("SMI_Wait_individual_PC");
  int no_consistency = 0;
  
  DSECTENTRYPOINT;
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  if(id & NOCONSISTENCY)
    {
      id -= NOCONSISTENCY;
      no_consistency = 1;
    }
  
  ASSERT_R(( (id>=0)&&(id<=max_pc_id) ), "ID is invalid", SMI_ERR_PARAM);
  
  if (val == SMI_OWNPC)
    val = pc[id].own_counter;
  
  ASSERT_R((proc_rank!=_smi_my_proc_rank),"waiting for myself",SMI_ERR_PARAM);
  
  while ((*((volatile int*)&(pc[id].counters[_smi_my_proc_rank][SEC_INDEX(proc_rank)]))) < val); 
  
  if(no_consistency==0)
    _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);

  DSECTLEAVE;
  return(SMI_SUCCESS);
}



/*********************************************************************************/
/* Waiting untill the progress counter of a specified pc (id) and of a set of    */
/* specified processes reaches a specified value. If val=SMI_OWNPC is specified, */
/* it is waited until the own pc values has been reached. This is a individual   */
/* call. Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong)  */
/*********************************************************************************/
smi_error_t SMI_Wait_individual_set_PC(int id, int* proc_ranks, int nbr_ranks, int val)
{
  DSECTION("SMI_Wait_individual_set_PC");
  smi_error_t error;
  int i;

  DSECTENTRYPOINT;
  
  for (i=0; i<nbr_ranks; i++) {
      if (_smi_my_proc_rank != proc_ranks[i]) {
	  error = SMI_Wait_individual_PC(id, proc_ranks[i], val);
	  ASSERT_R((error==SMI_SUCCESS),"SMI_Wait_individual_PC failed",error);
      }
  }

  DSECTLEAVE;
  return(SMI_SUCCESS);
}



/*********************************************************************************/
/* Waiting untill the progress counters of a specified pc (id) and of all        */
/* processs reach a specified value. If val=SMI_OWNPC is specified, it is waited */
/* until the own pc values have been reached. This is a individual call. Possible*/
/* errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong).                */
/*********************************************************************************/
smi_error_t SMI_Wait_collective_PC(int id, int val)
{
  DSECTION("SMI_Wait_collective_PC");
  int i;
  int no_consistency = 0;
  
  DSECTENTRYPOINT;
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  if(id & NOCONSISTENCY) {
    id -= NOCONSISTENCY;
    no_consistency = 1;
  }
  
  ASSERT_R(( (id>=0)&&(id<=max_pc_id) ), "ID is invalid", SMI_ERR_PARAM);

  if (val == SMI_OWNPC)
    val = pc[id].own_counter;
  
  ASSERT_R((val <= pc[id].own_counter),"Cannot wait for progress further than my progress",SMI_ERR_PARAM);

  for(i = 0; i < _smi_nbr_procs; i++) {
      while ((*((volatile int*)&(pc[id].counters[_smi_my_proc_rank][SEC_INDEX(i)]))) < val);  
  }
  
  if (no_consistency==0)
    _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);

  DSECTLEAVE;
  return(SMI_SUCCESS);
}

#ifdef WIN32
#pragma optimize("", on)
#endif


