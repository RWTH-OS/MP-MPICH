/* $Id$ */

/*********************************************************************************/
/*** The Barrier-Algorithms are base on two papers:                            ***/
/*** 1.) Raijv Gupta, Charles R. Hill:                                         ***/
/***     A Scalable Implementation of Barrier Synchronization Using an         ***/
/***     Adaptive Combining Tree,                                              ***/
/***     International Journal of Parallel Programming, June 1989, pp. 161-180 ***/
/*** 2.) Michael L. Scott, John M. Mellor-Crummey:                             ***/
/***     Fast, Contention-Free Combining Tree Barriers,                        ***/
/***     University of Rochester, CCR-8809615, June 1992                       ***/
/*********************************************************************************/

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#ifndef SVM

#include "barrier.h"
#include "mutex.h"

#include "dyn_mem/dyn_mem.h"
#include "store_barrier.h"
#include "progress.h"
#include "message_passing/lowlevelmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* REMOVE!!! */
static int NUMMER=0;

/*********************************************************************************/
/*** The variable all_barrier allotes a number to a barrier-identifier.        ***/
/*** A barrier_identifier contains all necessary informations to use a         ***/
/*** barrier-algorithm.                                                        ***/
/*********************************************************************************/
barrier_id* all_barrier[2];

/*********************************************************************************/
/*** This variable counts the barriers, which are initialized.                 ***/
/*********************************************************************************/
int barrier_counter;


void _smi_barrier_module_init()
{	
    barrier_counter = 0;
    all_barrier[0] = all_barrier[1] = NULL;
}

void _smi_barrier_module_finalize()
{
    int error;
    
    error = SMI_BARRIER_DESTROY(0);
    
    if (all_barrier[0] != NULL)
	free(all_barrier[0]);
    if (all_barrier[1] != NULL)
	free(all_barrier[1]);
    barrier_counter = 0;
    all_barrier[0] = all_barrier[1] = NULL; 
}


/*********************************************************************************/
/*** This function updates the array all_barrier. After that it calls the      ***/
/*** initialize-function from the barrier-algorithm. All users of the          ***/
/*** SMI-Library must call this function to intialize the barrier-algorithm.   ***/
/*********************************************************************************/
smi_error_t SMI_BARRIER_INIT(int *ID, int algorithm_type, int is_active)
{
    REMDSECTION("SMI_BARRIER_INIT");
    smi_error_t error;
    int i;
#if ALLOW_SYNCHRONIZATION_SE
    int j, iTemp;
#endif
    barrier_id* new_array[2];
    
    DSECTENTRYPOINT;
    
    ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
    ASSERT_R(( (ID != NULL)&&(algorithm_type >= 0)&&(algorithm_type <= 3) ),"Invalid parameters",SMI_ERR_PARAM);
    
    /* Create new arrays. */
    ALLOCATE (new_array[0], barrier_id *, (barrier_counter+1)*sizeof(barrier_id));
    ALLOCATE (new_array[1], barrier_id *, (barrier_counter+1)*sizeof(barrier_id));
    
    /* Rescue the old element */
    if (barrier_counter > 0) {
	for(i=0; i<barrier_counter; i++)	{
	    new_array[0][i] = all_barrier[0][i];
	    new_array[1][i] = all_barrier[1][i];
	}
	free(all_barrier[0]);
	free(all_barrier[1]);
    }
    *ID = barrier_counter;
    barrier_counter++;
    all_barrier[0] = new_array[0];
    all_barrier[1] = new_array[1];
    
    /* Initialze the new element. */
    for (i=0; i<2; i++) {
	new_array[i][*ID].algorithm_type = algorithm_type;
#if ALLOW_SYNCHRONIZATION_SET
	new_array[i][*ID].is_active = is_active;
	ALLOCATE( new_array[i][*ID].process_ranks, int*, sizeof(int) * _smi_nbr_procs );
	ASSERT_R((new_array[i][*ID].process_ranks != NULL), "no more system memory",SMI_ERR_NOMEM);
	new_array[i][*ID].nbr_processes = 0;
#endif /* ALLOW_SYNCHRONIZATION_SET */
    }
    
    /* Then the funtion calls the initialize-function from the barrier-algorithm. */
    switch(algorithm_type) {
    case PROGRESS_COUNTER_BARRIER:
	error = SMI_ProgressCounterBarrier_init(&(all_barrier[0][*ID]));
	if (error == SMI_SUCCESS)
	    error = SMI_ProgressCounterBarrier_init(&(all_barrier[1][*ID]));
	break;
    case SCHULZ_BARRIER:
	error = SMI_SchulzBarrier_init(&(all_barrier[0][*ID]));
	if (error == SMI_SUCCESS)
	    error = SMI_SchulzBarrier_init(&(all_barrier[1][*ID]));
	break;
    default:               
	error = SMI_ERR_PARAM;
    }
    ASSERT_R((error == SMI_SUCCESS),"SMI_ProgressCounterBarrier_init failed",error);
    
#if ALLOW_SYNCHRONIZATION_SET
    /* Obtain the list of processes which are intended to be part of the */
    /* set to be synchronyzed. */
    for (i=0; i<_smi_nbr_procs; i++) {
	iTemp = is_active;
	_smi_ll_bcast(&iTemp, 1, i, _smi_my_proc_rank);
	if (iTemp == TRUE) {
	    for (j=0; j<2; j++) {
		new_array[j][*ID].process_ranks[(new_array[j][*ID].nbr_processes)++] = i;
	    }  
	}
    }
#endif /* ALLOW_SYNCHRONIZATION_SET */
    
    /* XXX really necessary ? */
    _smi_ll_barrier();
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This function calls the destroy-function from the barrier-algorithm.      ***/
/*** After that it updates the array all_barrier. All users of the             ***/
/*** SMI-Library must call this function to destroy the barrier-algorithm.     ***/
/*********************************************************************************/
smi_error_t SMI_BARRIER_DESTROY(int ID)
{
    DSECTION("SMI_BARRIER_DESTROY");
    smi_error_t error;
    int i;
    
    DSECTENTRYPOINT;
    
    ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
    
    ASSERT_R(( (all_barrier[0]!=NULL) && (all_barrier[1]!=NULL) ), "Invalid Parameter", SMI_ERR_PARAM);
    
    /* The funtion calls the destroy-function from the barrier-algorithm. */
    switch(all_barrier[0][ID].algorithm_type)
    {
    case PROGRESS_COUNTER_BARRIER:
	error = SMI_ProgressCounterBarrier_destroy(&(all_barrier[0][ID]));
	if (error == SMI_SUCCESS)
	    error = SMI_ProgressCounterBarrier_destroy(&(all_barrier[1][ID]));
	break;
    case SCHULZ_BARRIER:
	error = SMI_SchulzBarrier_destroy(&(all_barrier[0][ID]));
	if (error == SMI_SUCCESS)
	    error = SMI_SchulzBarrier_destroy(&(all_barrier[1][ID]));
	break;
    default:          
	error = SMI_ERR_PARAM;
	break;
    }
    ASSERT_R((error==SMI_SUCCESS),"SMI_ProgressCounterBarrier_destroy failed",error);
    
    for(i=0; i<2; i++)
    {
	all_barrier[i][ID].algorithm_type = -1;
#if ALLOW_SYNCHRONIZATION_SET
	free(all_barrier[i][ID].process_ranks);
#endif /* ALLOW_SYNCHRONIZATION_SET */
    }
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This function calls the barrier-function from the barrier-algorithm.      ***/
/*** Problem: When process calls the barrier-function and an other process     ***/
/***          is still reinitialze the barrier, then the barrier-algorithm     ***/
/***          is not deadlock-free.                                            ***/
/*** Solve: This function swaps the pointers all_barrier[0] and all_barrier[1].***/
/*********************************************************************************/
smi_error_t SMI_BARRIER(int ID)
{
    DSECTION("SMI_BARRIER");
    smi_error_t error;
    barrier_id* help; 
    
    DSECTENTRYPOINT;
    
    ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
    
    if (_smi_nbr_procs==1)
	return(SMI_SUCCESS);
    
    _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1);
    
    ASSERT_R(( (all_barrier[0]!=NULL)&&(all_barrier[1]!=NULL) ),"Invalid Parameter",SMI_ERR_PARAM);
    
    help = &(all_barrier[0][ID]);
    switch(help->algorithm_type)
    {
    case PROGRESS_COUNTER_BARRIER: 
	error = SMI_ProgressCounterBarrier(&(all_barrier[1][ID]));
	break;
    case SCHULZ_BARRIER:
	error =  SMI_SchulzBarrier(&(all_barrier[1][ID]));
	break;
    default:               
	error = SMI_ERR_PARAM;
	break;
    } 
    ASSERT_R((error == SMI_SUCCESS),"SMI_ProgressCounterBarrier failed",error);
    
    help = all_barrier[0];
    all_barrier[0] = all_barrier[1];
    all_barrier[1] = help;
    
    _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This is a barrier, based on the progress counters with local spinning. It ***/
/*** should be the most efficient and the least critical regarding correctness.***/
/*********************************************************************************/

smi_error_t SMI_ProgressCounterBarrier_init(barrier_id* ID)
{
    smi_error_t error;
    
    error = SMI_Init_PC(&ID->progress_counter_id);
    
    return(error);
}

smi_error_t SMI_ProgressCounterBarrier(barrier_id* ID)
{
    smi_error_t error;
    
    error = SMI_Increment_PC(ID->progress_counter_id | NOCONSISTENCY, 1);
    if (error != SMI_SUCCESS) 
	return(error);
#if ALLOW_SYNCHRONIZATION_SET
    error = SMI_Wait_individual_set_PC(ID->progress_counter_id |
				       NOCONSISTENCY,
				       ID->process_ranks,
				       ID->nbr_processes,
				       SMI_OWNPC);
#else
    error = SMI_Wait_collective_PC(ID->progress_counter_id |
				   NOCONSISTENCY,
				   SMI_OWNPC);	
#endif /* ALLOW_SYNCHRONIZATION_SET */

    return(error);
}

smi_error_t SMI_ProgressCounterBarrier_destroy(barrier_id* ID)
{
    return(SMI_SUCCESS);
}


/*********************************************************************************/
/*** This is a barrier, based on the implementations of Martin Schulz wich are ***/
/*** directly based on dolphins sisci                                          ***/
/*********************************************************************************/

#ifdef USE_SCHULZSYNC

smi_error_t SMI_SchulzBarrier_init(barrier_id* ID)
{
    DSECTION("SMI_SchuluzBarrier_init");
    smi_error_t error; 
    errCode_t err;
    
    DSECTENTRYPOINT;
    
    if (_smi_my_proc_rank == 0) {
	err = syncMod_allocBarrier(&(ID->atomic));
	ASSERT_R((err==0),"Error during alloc of a barrier",SMI_ERR_OTHER);
    }
    
    err = syncMod_distribute(0,&(ID->atomic));
    ASSERT_R((err==0),"Error during the barrier distributio",SMI_ERR_OTHER);
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

smi_error_t SMI_SchulzBarrier(barrier_id* ID)
{
    DSECTION("SMI_SchulzBarrier");
    smi_error_t error;
    errCode_t err;

    DSECTENTRYPOINT;
    
#if ALLOW_SYNCHRONIZATION_SET
    err = syncMod_fixedBarrier(ID->atomic, ID->nbr_processes);
#else
    err = syncMod_fixedBarrier(ID->atomic, _smi_nbr_procs);
#endif  /* ALLOW_SYNCHRONIZATION_SET */
    ASSERT_R((err==0),"Error during barrier", SMI_ERR_OTHER);
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

smi_error_t SMI_SchulzBarrier_destroy(barrier_id* ID)
{
    /* no destruction routines included in Schluz' implementation */
    return(SMI_SUCCESS);
}

#else /* USE_SCHULZSYNC */

smi_error_t SMI_SchulzBarrier_init(barrier_id* ID)
{
    DPROBLEM("the lib has been compiled without SCHULZSYNC");
    return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_SchulzBarrier(barrier_id* ID)
{
    DPROBLEM("the lib has been compiled without SCHULZSYNC");
    return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_SchulzBarrier_destroy(barrier_id* ID)
{
    DPROBLEM("the lib has been compiled without SCHULZSYNC");
    return(SMI_ERR_NOTIMPL);
}

#endif /* USE_SCHULZSYNC */

#endif /* SVM */

