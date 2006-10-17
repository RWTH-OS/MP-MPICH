/* $Id$ */

#define _DEBUG_EXTERN_REC
#include "env/smidebug.h"

#include "setup_comm.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


static MPI_Group mpi_group;            /* group of the MPI_COMM_WORLD communicator  */
static MPI_Group mpi_reordered_group;  /* group of the reordered MPI_COMM_WORLD     */
                                       /* communicator, in which processes, which   */
                                       /* are executed on the same machine possess  */
                                       /* consecutive process ranks                 */
static MPI_Group* mpi_machine_group;   /* element i of this array points to the     */
                                       /* group of processes residing on machine i  */


  
/********************************************************************************/
/*** Generate the communicator 'SMI_COMM_WORLD', which includes all           ***/
/*** processes, but with this reordering.                                     ***/
/*** The only errors that can occure are 1xxx and 2xxx errors.                ***/
/********************************************************************************/
smi_error_t _smi_build_reordered_communicator()
 {
#ifdef MPISTART   
  DSECTION("_smi_build_reordered_communicator");
  int     i;                            /* loop counter                        */
  int*    reordered_mpi_ranks;          /* new ranks of all processes, index   */
  /* by their old mpi rank               */
  int*    inverted_reordered_mpi_ranks; /* corresponding inverted table        */

  DSECTENTRYPOINT;

   /* get the communication group with all processes of */
   /* the communicator MPI_COMM_WORLD                   */
   
  mpi_error = MPI_Comm_group(MPI_COMM_WORLD, &mpi_group);
  ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Comm_group failed",1000+mpi_error);
   
   /* _smi_allocate some memory for temporal variables */
   
   ALLOCATE( reordered_mpi_ranks, int*, _smi_nbr_procs * sizeof(int) );
   ALLOCATE( inverted_reordered_mpi_ranks, int*, _smi_nbr_procs * sizeof(int) );

   /* gather all reordered SMI-internal process ranks at each process */
   
   reordered_mpi_ranks[_smi_mpi_rank] = _smi_my_proc_rank;

   mpi_error = MPI_Allgather(&_smi_my_proc_rank, 1, MPI_INT, reordered_mpi_ranks,
			     1, MPI_INT, MPI_COMM_WORLD);
   ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Allgather failed",1000+mpi_error);
  
   /* determine the inverse of these reordered SMI process ranks */

   for(i=0;i<_smi_nbr_procs;i++)
      inverted_reordered_mpi_ranks[reordered_mpi_ranks[i]] = i;


   /* and build the new communicator */
   
   mpi_error = MPI_Group_incl(mpi_group, _smi_nbr_procs,
			      inverted_reordered_mpi_ranks,
			      &mpi_reordered_group);
   ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Group_incl failed",1000+mpi_error);
   
   mpi_error = MPI_Comm_create(MPI_COMM_WORLD, mpi_reordered_group, &SMI_COMM_WORLD);
   ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Comm_create failed",1000+mpi_error);

   free(reordered_mpi_ranks);
   free(inverted_reordered_mpi_ranks);
#endif

   DSECTENTRYPOINT;
   
#ifdef OTHERSTART
   SMI_COMM_WORLD = MPI_COMM_WORLD;
#endif
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

  
  

/********************************************************************************/
/*** Generate the communicators'MPI_COMM_MACHINE'. This communicator includes ***/
/*** all processes, which are executed on the machine of the calling process. ***/
/*** Attention: This function is a global synchronization point. Therefore,   ***/
/*** it requires, that all processes call it collectively.                    ***/
/*** The only errors that can occure are 1xxx and 2xxx errors.                ***/
/********************************************************************************/
smi_error_t _smi_build_machine_communicators()
 {
   DSECTION("_smi_build_machine_communicators");
   int machine;         /* machine under consideration (loop counter)           */
   int process;         /* process under consideration (loop counter)           */
   MPI_Comm  tmp_comm;  /* communicator that is build for each group of each    */
                        /* machine                                              */
   int* rank;           /* list of process ranks that ar executed on a specific */
                        /* machine                                              */
   int no;              /* number of processes on a specific machine            */

   
   DSECTENTRYPOINT;
   
   /*******************************************************/
   /* _smi_allocate sufficient space, to store all MPI process */
   /* groups for all machine communicators                */
   /*******************************************************/
   
   ALLOCATE( mpi_machine_group, MPI_Group*, _smi_nbr_machines * sizeof(MPI_Group) );
   ALLOCATE( rank, int*, _smi_nbr_procs * sizeof(int) );  
   
   /********************************************/
   /* build the communicators for all machines */
   /********************************************/
   
   for (machine=0;machine<_smi_nbr_machines;machine++)
    {
      /* include the ranks of all processes of the machine */
      /* under consideration in the array 'rank'           */
      
      no = 0;
      for(process=0;process<_smi_nbr_procs;process++)
       {
	 if (_smi_machine_rank[process] == machine)
	  {
	    rank[no] = process;
	    no++;
	  }
       }

      /* build a MPI-group, containing all those processes */
      
      mpi_error = MPI_Group_incl(mpi_reordered_group, no, rank,
				 &(mpi_machine_group[machine]));
      ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Group_incl failed",1000+mpi_error);

      /* build the respective communicator */
      
      mpi_error = MPI_Comm_create(SMI_COMM_WORLD, mpi_machine_group[machine], &tmp_comm);
      ASSERT_R((mpi_error==MPI_SUCCESS),"MPI_Comm_create failed",1000+mpi_error);

      /* if the machine under consideration is the one on which the calling */
      /* process is executed, copy it as the own intra-machine communicator */ 
      
      if (machine == _smi_my_machine_rank)
	MPI_COMM_MACHINE = tmp_comm;
    }
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }
 
 





