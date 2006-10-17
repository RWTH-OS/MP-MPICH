/* $Id$ */

/*********************************************************************************/
/*** The Mutex-Algorithms are base on six papers:                              ***/
/*** 1.) Leslie Lamport: A Fast Mutual Exclusion Algorithm                     ***/
/***     ACM Trans. Computer Systems, Vol 5, No. 1, 1987, pp. 1-11             ***/
/***     International Journal of Parallel Programming, June 1989, pp. 161-180 ***/
/*** 2.) Xiaodong Zhang, Yong Yan, Robert Castaneda:                           ***/
/***     Evaluating and Designing Software Mutual Exclusion Algorithm on       ***/
/***     Shared-Memory Multiprocessors, IEEE Parallel & Distributed Technology,***/ 
/***     1996, pp 25-42                                                        ***/
/*** 3.) Maged M. Micheal, Micheal L. Scott:                                   ***/
/***     Fast Mutual Exclusion, Even Width Contention                          ***/
/***     University of Rochester, NY 14627-0226, June 1993                     ***/
/*** 4.) Jae-Heon Yang, Jams H. Anderson:                                      ***/
/***     Fast, Scalable Synchronization with minimal Hardware                  ***/
/***     Proc. 12th Ann. ACM Symp. Principles of Distributed Computing,        ***/
/***     ACM Press, 1993, pp. 872-886                                          ***/
/*** 5.) Zhang, Yan, Castaneda:                                                ***/
/***     Execution Complexities and Performance of Software Mutual Exlusion    ***/
/***     Algorithms on Shared-Memory Multiporcessors                           ***/
/***     The University of Texas at San Antonio                                ***/
/*** 6.) Zhang, Castaneda:                                                     ***/
/***     Spin-Lock Synchronization on the Butterfly and KSR1                   ***/
/***     The University of Texas at San Antonio 			       ***/
/*** 7.) Martin Schulz, direkt use of sisci                                    ***/
/*********************************************************************************/

#define DEBUG_EXTERN_REC
#include "env/smidebug.h"

#ifndef SVM

#include "sync.h"
#include "mutex.h"
#include "dyn_mem/dyn_mem.h"
#include "message_passing/lowlevelmp.h"
#include "store_barrier.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define TEST_LOCAL_SYNC 0

/*** The variable all_mutex allotes a number to a mutex-identifier.            ***/
/*** A mutex-identifier contains all necessary informations to use a mutex-    ***/
/*** algorithm.                                                                ***/
static mutex_id **all_mutex;

/*** This variable counts the mutexs, which are initialized.                   ***/
static int mutex_counter;


void _smi_mutex_module_init()
{	
   mutex_counter = 0;
   all_mutex = NULL;
}

void _smi_mutex_module_finalize()
{
   if (all_mutex != NULL)
      free(all_mutex);

   mutex_counter = 0;
   all_mutex = NULL;
}


static void _smi_local_or_global_store_barrier(int proc)
{
	if (proc != -1)
       {
	 if(_smi_machine_rank[proc] == _smi_my_machine_rank)
	    _smi_local_store_barrier();
	 else
	    _smi_store_barrier();
       }
      else
	 _smi_store_barrier();
}

static void _smi_local_or_global_load_barrier(int proc)
{
	if (proc != -1)
    {
		 if(_smi_machine_rank[proc] != _smi_my_machine_rank)
				_smi_load_barrier();
    }
    else
		 _smi_load_barrier();
}

  

/*********************************************************************************/
/*** This function updates the array all_mutex. After that it calls the        ***/
/*** initialize-function from the mutex-algorithm. All users of the            ***/
/*** SMI-Library must call this function to intialize the mutex-algorithm.     ***/
/*** The third parameter specifies, at which process, the mutex' data          ***/
/*** structures are to be allocated. If it is '-1', they shall be distribued   ***/
/*** evenly within the whole system.                                           ***/
/*********************************************************************************/   
smi_error_t SMI_MUTEX_INIT(int *ID, int algorithm_type, int prank)
{
  DSECTION("SMI_MUTEX_INIT");
  smi_error_t error;
  int i;
  mutex_id **new_array, *new_mutex;
  
  DSECTENTRYPOINT;


  ASSERT_R(_smi_initialized == true, "SMI nit initialized",SMI_ERR_NOINIT);
  ASSERT_R(ID != NULL  && algorithm_type >= 0 && algorithm_type < 10, "Invalid Parameters",SMI_ERR_PARAM);

  ALLOCATE (new_array, mutex_id **, (mutex_counter+1)*sizeof(mutex_id *));
  
  /* allocate memory for the new lock */
  for (i = 0; i < mutex_counter; i++)
      new_array[i] = all_mutex[i];
  if (all_mutex != NULL)
      free(all_mutex);
  all_mutex = new_array;
  
  ALLOCATE (new_mutex, mutex_id *, sizeof(mutex_id));
  all_mutex[mutex_counter] = new_mutex;
  mutex_counter++;
  
  new_mutex->shmadr = NULL;
  new_mutex->algorithm_type = algorithm_type;
  new_mutex->home_of_data = prank;

  switch(algorithm_type) {
  case L_MUTEX:     
      DNOTICE("Using Lamport's algorithm");
      error = SMI_Lamport_init(new_mutex);
      break;
  case BL_MUTEX:  
      DNOTICE("Using Burns & Lynch's algorithm");
      error = SMI_BurnsLynch_init(new_mutex);
      break;
  case SCH_MUTEX:
      DNOTICE("Using Schulz Locks");
      error = SMI_SchulzLock_init(new_mutex);
      break; 
  default:		
      error = SMI_ERR_PARAM;
      break;
  }
  
  ASSERT_R(error == SMI_SUCCESS, "Could not init mutex", error);

  error = _smi_ll_barrier();
  ASSERT_R(error == MPI_SUCCESS, "Barrier failed",1000+error);
  
  *ID = mutex_counter - 1;
  DNOTICEI("Mutex id:",*ID);

  DSECTLEAVE; return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This function calls the destroy-function from the mutex-algorithm.        ***/
/*** After that it updates the array all_mutex. All users of the               ***/
/*** SMI-Library must call this function to destroy the barrier-algorithm.     ***/
/*********************************************************************************/
smi_error_t SMI_Mutex_destroy(int ID)
{
  DSECTION("SMI_Mutex_destroy");
  smi_error_t error;

  DSECTENTRYPOINT;

  DNOTICEI("Destroying mutex with id", ID);
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  ASSERT_R(( (ID>=0)&&(ID<mutex_counter)&&(all_mutex[ID]!=NULL) ), "ID is invalid", SMI_ERR_PARAM);

  switch(all_mutex[ID]->algorithm_type) {
  case L_MUTEX:     
      error = SMI_Lamport_destroy(all_mutex[ID]);
      break;
  case BL_MUTEX:    
      error = SMI_BurnsLynch_destroy(all_mutex[ID]);
      break;
  case SCH_MUTEX:
      error = SMI_SchulzLock_destroy(all_mutex[ID]);
      break;    
  default:		
      error = SMI_ERR_PARAM;
      break;
  }
  
  ASSERT_R((error == SMI_SUCCESS),"Could not destroy mutex",error);
  
  free(all_mutex[ID]);
  all_mutex[ID] = NULL;
  
  DSECTLEAVE;
  return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This function calls the lock-function from the mutex-algorithm.           ***/
/*********************************************************************************/
smi_error_t SMI_Mutex_lock(int ID)
{
  DSECTION("SMI_Mutex_lock");
  smi_error_t error;
  int ensure_consistency = true;
  
  DSECTENTRYPOINT;
  DNOTICEI("Mutex id:",ID);

  if (ID & NOCONSISTENCY) {
      ID -= NOCONSISTENCY;
      ensure_consistency = false;
  }
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  if (_smi_nbr_procs==1)
    return(SMI_SUCCESS);
  
  ASSERT_R(( (ID>=0)&&(ID<mutex_counter)&&(all_mutex[ID]!=NULL) ), "ID is invalid", SMI_ERR_PARAM);
 
  switch(all_mutex[ID]->algorithm_type)
    {
    case L_MUTEX:     error = SMI_Lamport_lock(all_mutex[ID]);
	break;
    case BL_MUTEX:    error = SMI_BurnsLynch_lock(all_mutex[ID]);
	break;
    case SCH_MUTEX:   error = SMI_SchulzLock_lock(all_mutex[ID]);
	break;
    default:		error = SMI_ERR_PARAM;
	break;
    }
  
   if(ensure_consistency==true)
     _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);   
   	
   DSECTLEAVE
     return(error);
}
/*********************************************************************************/
/*** This function calls the lock-function from the mutex-algorithm. It it     ***/
/*** gets the loack, 'result is set to '1', otherwise, result ist set to '0'.  ***/
/*********************************************************************************/
smi_error_t SMI_Mutex_trylock(int ID, int* result)
{
  DSECTION("SMI_Mutex_trylock");
  smi_error_t error;
  
  DSECTENTRYPOINT;
  DNOTICEI("Mutex id:",ID);

  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  if (_smi_nbr_procs==1) {
    *result=1;
    return(SMI_SUCCESS);
  }
  
  ASSERT_R(( (ID>=0)&&(ID<mutex_counter)&&(all_mutex[ID]!=NULL) ), "ID is invalid", SMI_ERR_PARAM);
  
  switch(all_mutex[ID]->algorithm_type)
    {
#ifdef I_NOT_YET_IMPLEMENTED_I
    case L_MUTEX:     error = SMI_Lamport_trylock(all_mutex[ID], result);
	break;
    case SCH_MUTEX:   error = SMI_SchulzLock_trylock(all_mutex[ID]);
	break;	
#endif 
    case BL_MUTEX:    error = SMI_BurnsLynch_trylock(all_mutex[ID], result);
      break;
    default:		error = SMI_ERR_PARAM;
      break;
    }
  
  _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);
  
  DNOTICEI ("result of lock operation:", *result);
  DSECTLEAVE; return(error);
}


/*********************************************************************************/
/*** This function calls the unlock-function from the mutex-algorithm.         ***/
/*********************************************************************************/
smi_error_t SMI_Mutex_unlock(int ID)
{
  DSECTION("SMI_Mutex_unlock");
  smi_error_t error;
  int ensure_consistency = true;
  
  DSECTENTRYPOINT;
  DNOTICEI("Mutex id:",ID);
  
  if (ID & NOCONSISTENCY) {
      ID -= NOCONSISTENCY;
      ensure_consistency = false;
  }
  
  ASSERT_R((_smi_initialized == true),"SMI is not initialized",SMI_ERR_NOINIT);
  
  if (_smi_nbr_procs==1)
    return(SMI_SUCCESS);
  
  ASSERT_R(( (ID>=0)&&(ID<mutex_counter)&&(all_mutex[ID]!=NULL) ), "ID is invalid", SMI_ERR_PARAM);

  if (ensure_consistency == true)
    _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1);
  
  switch(all_mutex[ID]->algorithm_type) {
    case L_MUTEX:     error = SMI_Lamport_unlock(all_mutex[ID]);
	break;
    case BL_MUTEX:    error = SMI_BurnsLynch_unlock(all_mutex[ID]);
	break;
    case SCH_MUTEX:   error = SMI_SchulzLock_unlock(all_mutex[ID]);
	break;	
    default:		error = SMI_ERR_PARAM;
	break;
    }
  
  DSECTLEAVE
    return(error);
}

/*********************************************************************************/
/*** This functions discribe the mutex algorithm from Leslie Lamport.          ***/
/*** When the system has no contention, a process enters the critical section  ***/
/*** by O(1) statements. In worst case the process has to visit N variables.   ***/            
/*********************************************************************************/ 
smi_error_t SMI_Lamport_init(mutex_id *ID)
{
    REMDSECTION ("SMI_Lamport_init");
   smi_error_t error;
   int i;

   if (ID->home_of_data == -1)
     ID->home_of_data = 0;
   error = SMI_Cmalloc((_smi_nbr_procs+2)*sizeof(int), ID->home_of_data|INTERNAL, (void **)&ID->shmadr);

   ASSERT_R( error == SMI_SUCCESS, "not enough shared memory", error);
   
   if (_smi_my_proc_rank == 0) {
   /* Initialize variables */
      ID->shmadr[0] = ID->shmadr[1] = -1;
      for(i=2; i<_smi_nbr_procs+2; i++)
         ID->shmadr[i] = 0;
   }
   return(SMI_SUCCESS);
}

smi_error_t SMI_Lamport_destroy(mutex_id *ID)
{
   smi_error_t error;

   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   error = SMI_Cfree((char *) ID->shmadr);
   return(error);
}

smi_error_t SMI_Lamport_lock(mutex_id *ID)
{
   volatile int *x, *y;
   volatile int *request;
   int j;
  
   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   x = &ID->shmadr[0];
   y = &ID->shmadr[1];
   request = &ID->shmadr[2];
   for(;;)
   {  
      for(request[_smi_my_proc_rank]=1, *x=_smi_my_proc_rank; *y != -1; request[_smi_my_proc_rank]=1, *x=_smi_my_proc_rank)
      {
         request[_smi_my_proc_rank] = 0;
		 while (*y != -1) ; /* Spinning */
      }
      *y = _smi_my_proc_rank;
	  _smi_local_or_global_store_barrier(ID->home_of_data);
      if (*x != _smi_my_proc_rank)
      {
         request[_smi_my_proc_rank] = 0;
		 for(j=0; j<_smi_nbr_procs; j++)
			while (request[j] != 0) ; /* Spinning */
		 if (*y != _smi_my_proc_rank)
			while (*y != -1) ; /* Spinning */
		 else return(SMI_SUCCESS);
      }	else return(SMI_SUCCESS);
   }
}

smi_error_t SMI_Lamport_unlock(mutex_id *ID)
{
   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   ID->shmadr[1] = -1;
   ID->shmadr[_smi_my_proc_rank+2] = 0;
   
   return(SMI_SUCCESS);
}

/*********************************************************************************/
/*** This functions discribe the mutex algorithm from Burns and Lynch.         ***/
/*** The process has to visite every time N-1 variables.                       ***/
/*********************************************************************************/ 
smi_error_t SMI_BurnsLynch_init(mutex_id *ID)
{
  REMDSECTION("SMI_BurnsLynch_init");
  smi_error_t error;
  int i;
  
  DSECTENTRYPOINT;

  if (ID->home_of_data == -1)
    ID->home_of_data = 0;

  /* is this amount of memroy really stream-size dependant ? */
  DNOTICEI("INTS_PER_STREAM=",INTS_PER_STREAM);

  error = SMI_Cmalloc(_smi_nbr_procs*INTS_PER_STREAM*sizeof(int), ID->home_of_data|INTERNAL,
		      (void **)&ID->shmadr);
  ASSERT_R( error == SMI_SUCCESS, "not enough shared memory", error);

#if TEST_LOCAL_SYNC
  if (ID->home_of_data &LOCAL_ONLY)
      ID->home_of_data = _smi_first_proc_on_node(_smi_my_machine_rank);  
#endif

  if (_smi_my_proc_rank == 0) {
      /* Initialize variables */
      for (i = 0; i < _smi_nbr_procs; i++)
	ID->shmadr[SEC_INDEX(i)] = DEP_FALSE(i);
    }

  DSECTLEAVE; return(SMI_SUCCESS);
}



smi_error_t SMI_BurnsLynch_destroy(mutex_id *ID)
{
   smi_error_t error;

   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   error = SMI_Cfree((char *) ID->shmadr);
   return(error);
}

smi_error_t SMI_BurnsLynch_lock(mutex_id *ID)
{
   volatile int *shmadr = ID->shmadr;
   int t; 

   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   
   SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_TRUE(_smi_my_proc_rank));
   _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, ID->home_of_data);

   t=0;
   while (t < _smi_my_proc_rank) {
       if (_smi_SECRead(&(SEC_LOCATION(shmadr,t)),DEP_FALSE(t),DEP_TRUE(t)) != DEP_FALSE(t)) {
	   SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_FALSE(_smi_my_proc_rank));
	   /* Spinning */
	   while (_smi_SECRead(&(SEC_LOCATION(shmadr,t)),DEP_FALSE(t),DEP_TRUE(t)) != DEP_FALSE(t));

	   t = 0;
	   SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_TRUE(_smi_my_proc_rank));
	   _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, ID->home_of_data);
       } else 
	   t++;
   }

   for(t = _smi_my_proc_rank+1; t < _smi_nbr_procs; t++)
       /* Spinning */
       while (_smi_SECRead(&(SEC_LOCATION(shmadr,t)),DEP_FALSE(t),DEP_TRUE(t)) != DEP_FALSE(t)); 
   
   return(SMI_SUCCESS);
}


smi_error_t SMI_BurnsLynch_trylock(mutex_id *ID, int* result)
{
   volatile int *shmadr = ID->shmadr;
   int t = 0;
    
   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   
   SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_TRUE(_smi_my_proc_rank));
   _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, ID->home_of_data);


   while (t < _smi_my_proc_rank)
   {
     if (_smi_SECRead(&(SEC_LOCATION(shmadr,t)),DEP_FALSE(t),DEP_TRUE(t)) != DEP_FALSE(t))
       {
	SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_FALSE(_smi_my_proc_rank));
	*result = 0;
	return(SMI_SUCCESS);
       } else t++;
   }

   for(t=_smi_my_proc_rank+1; t<_smi_nbr_procs; t++)
     if (_smi_SECRead(&(SEC_LOCATION(shmadr,t)),DEP_FALSE(t),DEP_TRUE(t)) != DEP_FALSE(t)) 
       {
	 SEC_SET(SEC_LOCATION(shmadr,_smi_my_proc_rank),DEP_FALSE(_smi_my_proc_rank));
	 *result = 0;
	 return(SMI_SUCCESS);
       }

   *result = 1;
   return(SMI_SUCCESS);
 }


smi_error_t SMI_BurnsLynch_unlock(mutex_id *ID)
{
   if (ID->shmadr == NULL)
      return(SMI_ERR_PARAM);
   SEC_SET(ID->shmadr[SEC_INDEX(_smi_my_proc_rank)],DEP_FALSE(_smi_my_proc_rank));
  
   return(SMI_SUCCESS);
}



smi_error_t SMI_Location_of_mutex(int mutex_id, int* location)
{
  DSECTION("SMI_Location_of_mutex");
  
  DSECTENTRYPOINT;
  
  ASSERT_R(_smi_initialized == true, "SMI is not initialized",SMI_ERR_NOINIT);
  ASSERT_R( (mutex_id>=0)&&(mutex_id<mutex_counter)&&(all_mutex[mutex_id]!=NULL), "ID is invalid", SMI_ERR_PARAM);

  *location = all_mutex[mutex_id]->home_of_data;
  
  DSECTLEAVE; return(SMI_SUCCESS);
}


/*********************************************************************************/
/*** This functions discribe the mutex algorithm from Schulz                   ***/
/*********************************************************************************/ 

#ifdef USE_SCHULZSYNC

smi_error_t SMI_SchulzLock_init(mutex_id *ID)
{
    DSECTION("SMI_SchulzLock_init");
    errCode_t         err;
    
    DSECTENTRYPOINT;
    
    ASSERT_R((_smi_all_on_one != TRUE), "SCHULZLOCKS require at least two nodes",SMI_ERR_NOTIMPL);

    if (ID->home_of_data == -1)
	ID->home_of_data = 0;
    
    if (_smi_my_proc_rank == ID->home_of_data)
    {
	err = syncMod_allocLock(&(ID->atomic));
	ASSERT_R((err == 0), "Error during alloc of a barrier", SMI_ERR_OTHER);
	DNOTICEI("Allocated barrier:",ID->atomic);
    }  
    
    err = syncMod_distribute(ID->home_of_data, &(ID->atomic));
    ASSERT_R((err == 0), "Error during the barrier distribution",SMI_ERR_OTHER);
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}

smi_error_t SMI_SchulzLock_destroy(mutex_id *ID)
{ 
    REMDSECTION("SMI_SchulzLock_destroy");
    errCode_t         err;
   
    DSECTENTRYPOINT;

    ASSERT_R((_smi_all_on_one != TRUE), "SCHULZLOCKS require at least two nodes",SMI_ERR_NOTIMPL);

    /* There is no free/destroy routine for Schulz' Locks? */

    DSECTLEAVE;
    return(SMI_SUCCESS);
}

smi_error_t SMI_SchulzLock_lock(mutex_id *ID)
{
    REMDSECTION("SMI_SchulzLock_lock");
    errCode_t         err;

    DSECTENTRYPOINT;

    ASSERT_R((_smi_all_on_one != TRUE), "SCHULZLOCKS require at least two nodes",SMI_ERR_NOTIMPL);

    err = syncMod_lock(ID->atomic);

    DSECTLEAVE;
    return((err==0)?SMI_SUCCESS:SMI_ERR_OTHER);
}


smi_error_t SMI_SchulzLock_trylock(mutex_id *ID, int* result)
{
    DPROBLEM("SMI_SchulzLock_trylock is not implemented");
    return(SMI_ERR_NOTIMPL);
}


smi_error_t SMI_SchulzLock_unlock(mutex_id *ID)
{
    REMDSECTION("SMI_SchulzLock_unlock");
    errCode_t         err;

    DSECTENTRYPOINT;

    ASSERT_R((_smi_all_on_one != TRUE), "SCHULZLOCKS require at least two nodes",SMI_ERR_NOTIMPL);

    err = syncMod_unlock(ID->atomic);

    DSECTLEAVE;
    return((err==0)?SMI_SUCCESS:SMI_ERR_OTHER);
}

#else /* USE_SCHULZSYNC */

smi_error_t SMI_SchulzLock_init(mutex_id *ID) {
    DPROBLEM("the lib has been compiled without SCHULZLOCKS");
    return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_SchulzLock_destroy(mutex_id *ID) { 
    DPROBLEM("the lib has been compiled without SCHULZLOCKS");
    return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_SchulzLock_lock(mutex_id *ID) {
    DPROBLEM("the lib has been compiled without SCHULZLOCKS");
    return(SMI_ERR_NOTIMPL);
}


smi_error_t SMI_SchulzLock_trylock(mutex_id *ID, int* result) {
    DPROBLEM("the lib has been compiled without SCHULZLOCKS");
    return(SMI_ERR_NOTIMPL);
}

smi_error_t SMI_SchulzLock_unlock(mutex_id *ID) {
    DPROBLEM("the lib has been compiled without SCHULZLOCKS");
    return(SMI_ERR_NOTIMPL);
}

#endif /* USE_SCHULZSYNC */


#endif /* SVM */
