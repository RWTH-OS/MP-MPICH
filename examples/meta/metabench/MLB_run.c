#include "MLB_common.h"
#include "MLB_calc.h"
#include "MLB_comm.h"
#include "MLB_topol.h"

double MLB_Run(int n, int np, int max_steps, int delta, int res_start, int inter_flag, int res_flag, MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM)
{
  int steps;

  double** old_values;
  double** new_values;

  double* sendbufs[2];
  double* recvbufs[2];

  double start_time, stop_time;
  

  MLB_Init_matrix(&old_values, n, np, MLB_LOCAL_COMM, !inter_flag);
  MLB_Init_matrix(&new_values, n, np, MLB_LOCAL_COMM, !inter_flag);
  
  sendbufs[0]=new_values[1];
  sendbufs[1]=new_values[np-2];
      
  recvbufs[0]=new_values[0];
  recvbufs[1]=new_values[np-1];

  if(inter_flag)
    MLB_Barrier(MLB_LOCAL_COMM, MLB_INTER_COMM);
  else
    MPI_Barrier(MLB_LOCAL_COMM);

  start_time = MPI_Wtime();

  for(steps=0; (!max_steps) || (steps<max_steps); steps++)
  {   
    MLB_Jacobi_iteration(old_values, new_values, n, np);
      
    MLB_Local_comm(sendbufs, recvbufs, n, MLB_LOCAL_COMM);          

    if( (inter_flag) && (steps % delta == 0) )
    {
      int local_rank;

      MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);

      if(local_rank==0) MLB_Inter_comm(sendbufs[0], recvbufs[0], n, MLB_INTER_COMM);
    }

    if( res_flag && (steps >= res_start) )
    {
      double local_res, local_max;
      double global_res, global_max;
      
      local_res = MLB_Jacobi_local_powres(new_values, n, np);
      local_max = MLB_Jacobi_local_powmax(new_values, n, np);

      if(inter_flag)
      {
	MLB_Allreduce(&local_res, &global_res, MLB_SUM, MLB_LOCAL_COMM, MLB_INTER_COMM);
	MLB_Allreduce(&local_max, &global_max, MLB_MAX, MLB_LOCAL_COMM, MLB_INTER_COMM);
      }
      else
      {
	MPI_Allreduce(&local_res, &global_res, 1, MPI_DOUBLE, MPI_SUM, MLB_LOCAL_COMM);
	MPI_Allreduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, MLB_LOCAL_COMM);
      }
      global_res = sqrt(global_res / (2*n*n));

      if( (global_max < MLB_ABORT_RESIDUAL) && (steps % delta == 0) ) break;
    }

    MLB_Jacobi_exchange(old_values, new_values, n, np);	
  }

  if(inter_flag)
    MLB_Barrier(MLB_LOCAL_COMM, MLB_INTER_COMM);
  else
    MPI_Barrier(MLB_LOCAL_COMM);

  stop_time = MPI_Wtime();

  MLB_Free_matrix(old_values);
  MLB_Free_matrix(new_values);    

  if(res_flag) return ((double)(steps + 1));
  else
  {
    double max_time;
    double elapsed_time = stop_time - start_time;

    if(inter_flag)
      MLB_Allreduce(&elapsed_time, &max_time, MLB_MAX, MLB_LOCAL_COMM, MLB_INTER_COMM);
    else
      MPI_Allreduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MLB_LOCAL_COMM);

    return max_time;
  }
}


int main(int argc, char* argv[])
{
  int world_rank, world_size;
  int local_rank, local_size;

  MPI_Comm MLB_LOCAL_COMM, MLB_INTER_COMM;    

  MPI_Init(&argc, &argv);

  MLB_Init_comm(&MLB_LOCAL_COMM, &MLB_INTER_COMM);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);
  MPI_Comm_size(MLB_LOCAL_COMM, &local_size);

  {
    int problem_class = A;

    double power_ratio = 1.0;
    double local_load  = 1.0;
    double remote_load = 1.0;

    int sync_steps = 0;

    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

#if 0
    if(local_rank == 0)
    {
      int remote = 0;
      int local  = (world_rank+1)*2;
      MPI_Status status;
      MPI_Request request;

      int inter_size;
      int inter_rank;
      int remote_size;

      MPI_Comm_rank(MLB_INTER_COMM, &inter_rank);
      MPI_Comm_size(MLB_INTER_COMM, &inter_size);
      MPI_Comm_remote_size(MLB_INTER_COMM, &remote_size);

      printf("inter-size: %d / inter-rank: %d / remote-size: %d\n", inter_size, inter_rank, remote_size); fflush(stdout);

      MPI_Isend(&local, 1, MPI_INT, 0, MLB_INTER_TAG, MLB_INTER_COMM, &request);
      MPI_Recv(&remote, 1, MPI_INT, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);

      //      MPI_Sendrecv(&local, 1, MPI_INT, 1, MLB_INTER_TAG, &remote, 1, MPI_INT, 1, MLB_INTER_TAG, MLB_INTER_COMM, &status);
      
      printf("local: %d / remote: %d\n", local, remote); fflush(stdout);
    }
    
    MPI_Finalize();
    exit(0);
#endif

    /******************************************/
    {
      int n, np;

      if(world_rank == 0)
      {
	printf("\nMLB: Performing a transparent run...\n");
	fflush(stdout);
      }

      local_load  = 1.0;
      remote_load = 1.0;

      {	
	int i, j, k;

	n  = MLB_Class_size[problem_class] + 2;
	np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);

	for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
	{
	  if(i == local_rank) k++;
	}

	np = k + 2;
      }
    
      {
	int steps;
	double time;

	steps = (int) MLB_Run(n, np, 0, 1, sync_steps, 1, 1, MLB_LOCAL_COMM, MLB_INTER_COMM);
	
	time = MLB_Run(n, np, steps, 1, 0, 1, 0, MLB_LOCAL_COMM, MLB_INTER_COMM);

	if(world_rank == 0)
	{
	  printf("MLB: %f seconds for %d steps\n", time, steps);
	}

	sync_steps = steps - 1;
      }     
    }
    /******************************************/

    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    /******************************************/
    {
      int n, np;

      if(world_rank == 0)
      {
	printf("\nMLB: Measuring load imbalance...\n");
	fflush(stdout);
      }

      local_load  = 2.0;
      remote_load = 2.0;

      {
	int i, j, k;

	n  = MLB_Class_size[problem_class] + 2;
	np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);

	for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
	{
	  if(i == local_rank) k++;
	}

	np = k + 2;
      }

      {
	int steps = sync_steps;
	double time;

#if 0
	steps = (int) MLB_Run(n, np, 0, 1, sync_steps, 0, 1, MLB_LOCAL_COMM, MLB_INTER_COMM);
	
	time = MLB_Run(n, np, steps, 1, 0, 0, 0, MLB_LOCAL_COMM, MLB_INTER_COMM);
#else
	time = MLB_Run(n, np, steps, 1, sync_steps+1, 0, 0, MLB_LOCAL_COMM, MLB_INTER_COMM);
#endif

	if(local_rank == 0)
	{
	  double remote_time;
	  MPI_Status status;

	  MPI_Sendrecv(&time, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, &remote_time, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);

	  printf("MLB: %f seconds for %d steps (remote time: %f)\n", time, steps, remote_time);
	  fflush(stdout);

	  power_ratio  = (remote_time / time) / (remote_time / time + 1);

	  printf("MLB: power_ratio: %f\n", power_ratio);
	}

	MPI_Bcast(&power_ratio, 1, MPI_DOUBLE, 0, MLB_LOCAL_COMM);

	sync_steps = steps - 1;
      }

      MPI_Barrier(MPI_COMM_WORLD);

      if(world_rank == 0)
      {
	printf("MLB: Load ratio is %.3f to %.3f\n", 2.0 *  power_ratio, 2.0 * (1.0 - power_ratio));
	printf("MLB: Load balance: %d / %d\n",
	       (int)( 2.0 * power_ratio * MLB_Class_size[problem_class] + 0.5),
	       (int)( 2.0 * (1.0 - power_ratio) * MLB_Class_size[problem_class] + 0.5));
	fflush(stdout);
      }
    }
    /******************************************/

    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    /******************************************/
    {
      int n, np;

      if(world_rank == 0)
      {
	printf("\nMLB: Determining the convergence table...\n");     
	fflush(stdout);
      }   

      local_load  = 2.0 * power_ratio;
      remote_load = 2.0 * (1.0 - power_ratio);

      {
	int i, j, k;

	n  = MLB_Class_size[problem_class] + 2;
	np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);

	for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
	{
	  if(i == local_rank) k++;
	}

	np = k + 2;
      }

      if(world_rank == 0)
      {
	printf("Delta\tSteps\tTime\n");
	fflush(stdout);
      }   
    
      {
	int steps = sync_steps;
	int delta = 1;
	double time, last_time;
      
	while(1)
	{
	  if(world_rank == 0)
	  {
	    printf("%d", delta);
	    fflush(stdout);
	  }

	  steps = (int) MLB_Run(n, np, 0, delta, steps, 1, 1, MLB_LOCAL_COMM, MLB_INTER_COMM);

	  if(world_rank == 0)
	  {
	    printf("\t%d", steps);
	    fflush(stdout);
	  }

	  time = MLB_Run(n, np, steps, delta, 0, 1, 0, MLB_LOCAL_COMM, MLB_INTER_COMM);

	  if(world_rank == 0)
	  {
	    printf("\t%f\n", time);
	    fflush(stdout);
	  }
	
	  if(steps >= MLB_CONV_TABLE_FACTOR * sync_steps) break;

	  if(delta == 1) last_time = time;

	  //	  if(last_time < time) break;	  

	  delta++;
	}
      }
    }
    /******************************************/
  }

  MPI_Finalize();

  return 0;
}
