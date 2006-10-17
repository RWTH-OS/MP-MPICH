/* $Id$ */

#define _DEBUG_EXTYERN_REC
#include "env/smidebug.h"

#include "loop_split.h"
#include "switch_consistency/switch_to_replication.h"
#include "utility/general.h"
#include "proc_node_numbers/first_proc_on_node.h"
#include "message_passing/lowlevelmp.h"

#define SMI_DO_TEST_LOADBALANCE

/*********************************************************************************/
/*** some local data types and variables                                       ***/
/*********************************************************************************/ 
typedef struct
 {
   int entire_lower_bound;     /* lower loop bound of the total index range      */
   int entire_upper_bound;     /* upper loop bound of the total index range      */
   int local_lower_bound;      /* lower loop_bound for each process              */
   int local_upper_bound;      /* upper loop_bound for each process              */
   struct timeval start_time;  /* start time of loop for each process            */
   struct timeval end_time;    /* end time of loop for each process              */
   double elapsed_time;        /* averaged local elapsed time                    */ 
 } loop_split_t;

static loop_split_t** splitting;    /* all loop splittings that are managed             */
static int no_loops = 0;            /* number of loop splittings that are currentlt     */
                             /* managed                                          */



#define WEIGHT_FACTOR 0.95







/*********************************************************************************/
/* Given a shared memory region with it's characteristic splitting, determine    */
/* the physical local part of the specified process. If there are multiple       */
/* processes on the same machine, the physical local part is split evenly among  */
/* them. Anyway, the splitting is done at an address that is a multiple of       */
/* 'align'. Not all of the region is considered, but only a number of 'size'     */
/* byte. It is assumed, that each machine hosts as most a single segment of the  */
/* entire region.                                                                */
/*********************************************************************************/
void _smi_get_local(int region_id, int align, int size, int proc_id,
	       void** local_start, int* local_size)
 {
   region_t* region;        /* region under consideration                        */
   shseg_t*  segment;       /* physical local segment of this region             */
   int       i;             /* ordinary loop counter                             */
   int       np;            /* total number of SMI-processes on this machine     */
   void*     seg_low_adr;   /* corrected start-address of the segment with       */
                            /* respect to allignment                             */
   int       seg_size;      /* actual size of the physical local segment with    */
                            /* respect to allignment                             */
   int       delta;         /* ammount of correction due to allignment           */
   int       share;         /* part (size) of the corrected local segment per    */
                            /* SMI-process on this machine                       */



   /* serch for the region's data structure */
   _smi_id_to_region_struct(region_id, &region);

   
   /* searchs for a segment of the region that is located on this process's */
   /* machine; it is assumed that each machine holds at most one segment    */
   i=0;
   while (region->seg[i]->machine != _smi_machine_rank[proc_id] && i+1<region->no_segments)
      i++;
   segment = region->seg[i];


   if (segment->machine == _smi_machine_rank[proc_id])
    {
      /* determine start address of segment with respect to allignment and size */
      /* of physical local segment for splitting purposes with respect to       */
      /* allignment and maximum total size considered                           */
      
      seg_low_adr = segment->address;
      delta = (int)((size_t)(segment->address) -
	       (size_t)(region->addresses[0])) % align;
      seg_low_adr = (void*)((size_t)(seg_low_adr) - (size_t)delta);

      seg_size = delta + imin(size - (int)((size_t)seg_low_adr -
				      (size_t)(region->addresses[0])),
			      (int)segment->size);
      /* for the case that this segment is to high to contain */
      /* any local constitution:                              */
      if (seg_size<0) seg_size = 0;
      seg_size -= seg_size % align;

      
      /* the process determines it's local share */
      
      np = _smi_no_processes_on_machine(_smi_machine_rank[proc_id]);
      share = align * (int)((seg_size/align)/np);
      *local_start = (void*)((size_t)seg_low_adr
			     + (size_t)(share*_smi_local_rank(proc_id)));
      if (_smi_my_proc_rank == _smi_last_proc_on_node(_smi_machine_rank[proc_id]))
	 *local_size = seg_size - ((np-1) * share);
      else
	 *local_size = share;
    }
   else
    {
      /* the process does not physically own */
      /* any part of the shared region       */
      
      *local_start = NULL;
      *local_size = 0;
    }
   
   return;
 }
  
  



  
  

/*********************************************************************************/
/*** Allocate and initialize the data structures for a new loop whose          ***/
/*** splitting is to be managed. This function has to be called for all        ***/
/*** processes collectively                                                    ***/
/*********************************************************************************/ 
smi_error_t SMI_Loop_split_init(int* loop_id)
 {
   DSECTION("SMI_Loop_split_init");
   loop_split_t** tmp_splitting;
   int i;

   DSECTENTRYPOINT;

   ASSERT_R((_smi_initialized==true),"SMI is not initialized",SMI_ERR_NOINIT);
   
   /* enlarge list of loops for which the splitting is maintained by on entry */
   
   ALLOCATE( tmp_splitting, loop_split_t**, (no_loops+1) * sizeof(loop_split_t*) );
   for(i=0;i<no_loops;i++)
      tmp_splitting[i] = splitting[i];
   if (no_loops>0) free(splitting);
   splitting = tmp_splitting;

   ALLOCATE( splitting[no_loops], loop_split_t*, sizeof(loop_split_t) );
   
   *loop_id = no_loops;

   /* some initializations */
   
   splitting[no_loops]->elapsed_time = 0.0;


   
   no_loops++;
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  

  


/*********************************************************************************/
/*** Fill in or request the loop's index range: local or global.               ***/
/*********************************************************************************/ 
smi_error_t SMI_Loop_index_range(int loop_id, int* lower, int* upper, int mode)
 {
   /* parameter validation ckeck */
   
   if (loop_id<0 || loop_id > no_loops-1)
    {
      return(SMI_ERR_PARAM);
    }


   if (mode == SMI_LOOP_SET_GLOBAL)
    {
      splitting[loop_id]->entire_lower_bound = *lower;
      splitting[loop_id]->entire_upper_bound = *upper;
    }
   else if (mode == SMI_LOOP_GET_GLOBAL)
    {
      *lower = splitting[loop_id]->entire_lower_bound;
      *upper = splitting[loop_id]->entire_upper_bound;
    }
   else if (mode == SMI_LOOP_SET_LOCAL)
    {
      splitting[loop_id]->local_lower_bound = *lower;
      splitting[loop_id]->local_upper_bound = *upper;
    }
   else if (mode == SMI_LOOP_GET_LOCAL)
    {
      *lower = splitting[loop_id]->local_lower_bound;
      *upper = splitting[loop_id]->local_upper_bound;
    }
   else
    {
      return(SMI_ERR_PARAM);
    }

   
   return(SMI_SUCCESS);
 }



/*********************************************************************************/
/*** Determines each processes' local iteration sub-space                      ***/  
/*********************************************************************************/ 
smi_error_t SMI_Determine_loop_splitting(int loop_id, int mode, int param1, int param2)
 {
   int       total_iterations;
   int       local_iterations;
   int lower;
   int upper;
   void* local_start;
   int local_size;
   region_t* region;
   
   
   
   /* parameter validation ckeck */
   
   if (loop_id<0 || loop_id > no_loops-1)
    {
      return(SMI_ERR_PARAM);
    }
   
   lower = splitting[loop_id]->entire_lower_bound;
   upper = splitting[loop_id]->entire_upper_bound;
   

   if (mode == SMI_SPLIT_REGULAR)
    {
      total_iterations = upper - lower + 1;
      local_iterations = total_iterations / _smi_nbr_procs;
      splitting[loop_id]->local_lower_bound = lower
	 + _smi_my_proc_rank * local_iterations;
      if (_smi_my_proc_rank == _smi_nbr_procs-1)
	 splitting[loop_id]->local_upper_bound = upper;
      else
	 splitting[loop_id]->local_upper_bound = lower
	    + (_smi_my_proc_rank+1) * local_iterations - 1;   
    }

   
   if (mode == SMI_SPLIT_OWNER)
    {
      _smi_get_local(param2, param1, (upper-lower+1) * param1, _smi_my_proc_rank,
		&local_start, &local_size);
      if (local_start != NULL)
       {
	 _smi_id_to_region_struct(param2, &region);

	 splitting[loop_id]->local_lower_bound = lower +
	    (int)((size_t)local_start - (size_t)(region->addresses[0])) / param1;
	 splitting[loop_id]->local_upper_bound = splitting[loop_id]->local_lower_bound
	    + local_size / param1 - 1;
       }
      else
       {
	 splitting[loop_id]->local_lower_bound = 0;
	 splitting[loop_id]->local_upper_bound = -1;
       }
    }
      
   
   return(SMI_SUCCESS);
 }


  
  
  
/*********************************************************************************/
/*** Start the time to monitor the execution time for a processes local part   ***/
/*** of the specified loop. This data is stored and exploited for future load  ***/
/*** balancing purposes.                                                       ***/
/*********************************************************************************/ 
smi_error_t SMI_Loop_time_start(int loop_id)
 {

   /* parameter validation check */
   
   if (loop_id<0 || loop_id > no_loops-1)
    {
      return(SMI_ERR_PARAM);
    }

   
   gettimeofday(&(splitting[loop_id]->start_time), NULL);

   
   return(SMI_SUCCESS);
 }


  
  
/*********************************************************************************/
/*** Stop the time to monitor the execution time for a processes local part    ***/
/*** of the specified loop. This data is stored and exploited for future load  ***/
/*** balancing purposes.                                                       ***/
/*********************************************************************************/ 
smi_error_t SMI_Loop_time_stop(int loop_id)
 {

   /* parameter validation check */
   
   if (loop_id<0 || loop_id > no_loops-1)
    {
      return(SMI_ERR_PARAM);
    }

   
   gettimeofday(&(splitting[loop_id]->end_time), NULL);

   
   return(SMI_SUCCESS);
 }



  
/*********************************************************************************/
/*** Based on load measurements of the parallel portions of the splitted loop  ***/
/*** and the previous splitting, an enhanced splitting is calculated           ***/
/*** concerning load balancing.                                                ***/ 
/*********************************************************************************/   
smi_error_t SMI_Loop_balance_index_range(int loop_id)
{
  DSECTION("SMI_Loop_balance_index_range");
  double        local_elapsed_time; /* elapsed time during the last period      */
  loop_split_t* loop;               /* pointer to the data of the considered    */
  /* loop                                     */
  double* elapsed_time;
  int* work;
  int local_work;
  double ips;
  double time_to_balance;
  double max_time, avr_time;
  int i, i_max;
  smi_error_t mpi_error;

  DSECTENTRYPOINT;
  
  /* parameter validation check */
  
  if (loop_id<0 || loop_id > no_loops-1)
    {
      DSECTLEAVE
	return(SMI_ERR_PARAM);
    }
  

   /* maintain the elapsed time of this loop locally */
  
  loop = splitting[loop_id];
  local_elapsed_time = (double)(loop->end_time.tv_sec - loop->start_time.tv_sec)
    + 0.000001 * (double)(loop->end_time.tv_usec - loop->start_time.tv_usec);
  if (loop->elapsed_time == 0.0)
    loop->elapsed_time = local_elapsed_time;
  else
    loop->elapsed_time = WEIGHT_FACTOR * loop->elapsed_time +
      (1.0 - WEIGHT_FACTOR) * local_elapsed_time;
  
  
  /* collect elapsed times and the amount of work of all processes */
  
  local_work = loop->local_upper_bound - loop->local_lower_bound + 1;
  ALLOCATE( work, int*, _smi_nbr_procs * sizeof(int) );
  work[_smi_my_proc_rank] = local_work;
  mpi_error = _smi_ll_allgather(&local_work, 1 , work, _smi_my_proc_rank);
  ASSERT_R((mpi_error==MPI_SUCCESS),"_smi_ll_allgather failed",1000+mpi_error);
  
  ALLOCATE( elapsed_time, double*, _smi_nbr_procs * sizeof(double) );
  elapsed_time[_smi_my_proc_rank] = local_elapsed_time;
  mpi_error = _smi_ll_allgather((int*)&local_elapsed_time, sizeof(double)/sizeof(int), (int*)elapsed_time, _smi_my_proc_rank);
  ASSERT_R((mpi_error==MPI_SUCCESS),"_smi_ll_allgather failed",1000+mpi_error);
  
  /* perform load_balance to the left */
  
  if(_smi_my_proc_rank!=0)
    {
      ips = (double)(work[_smi_my_proc_rank] + work[_smi_my_proc_rank-1])
	/ (elapsed_time[_smi_my_proc_rank] + elapsed_time[_smi_my_proc_rank-1]);
      time_to_balance = (elapsed_time[_smi_my_proc_rank]
			 - elapsed_time[_smi_my_proc_rank-1]) / 2.0;
      loop->local_lower_bound += (int)(ips*time_to_balance*0.1);
    }
  
  
  /* perform load_balance to the right */
   
  if(_smi_my_proc_rank!=_smi_nbr_procs-1)
    {
      ips = (double)(work[_smi_my_proc_rank] + work[_smi_my_proc_rank+1])
	/ (elapsed_time[_smi_my_proc_rank] + elapsed_time[_smi_my_proc_rank+1]);
      time_to_balance = (elapsed_time[_smi_my_proc_rank]
			 - elapsed_time[_smi_my_proc_rank+1]) / 2.0;
      loop->local_upper_bound -= (int)(ips*time_to_balance*0.1);
    }  
  
#ifdef SMI_DO_TEST_LOADBALANCE
  /* report about load balance situation and loop-splitting */
#ifdef _DEBUG
  if (D_DO_DEBUG==TRUE) {
    fprintf(stderr,"Load Balance Info: Proc. %i has elapsed time %.5f sec.\n",
	    _smi_my_proc_rank, local_elapsed_time);
    fprintf(stderr,"Load Balance Info: Proc. %i has range %i .. %i\n",
	    _smi_my_proc_rank,loop->local_lower_bound,loop->local_upper_bound);
    
    avr_time = 0.0;
    max_time = 0.0;
    for(i=0;i<_smi_nbr_procs;i++)
      {
	avr_time += elapsed_time[i];
	if(elapsed_time[i]>max_time)
	  {
	    i_max = i;
	    max_time = elapsed_time[i];
	  }
      }
    avr_time /= _smi_nbr_procs;
    if(_smi_my_proc_rank==0)
      fprintf(stderr,"LUB=%.6f\n",(max_time-avr_time)/avr_time);
  }
#endif /* _DEBUG */
#endif /* SMI_DO_TEST_LOADBALANCE */
  
  free(work);free(elapsed_time);
 
  DSECTLEAVE
    return(SMI_SUCCESS);
}
