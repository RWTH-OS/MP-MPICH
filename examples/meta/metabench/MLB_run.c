#include "MLB_common.h"
#include "MLB_calc.h"
#include "MLB_comm.h"
#include "MLB_topol.h"

void MLB_usage()
{
	printf("usage: MLB [-pc|--problem-class ABCWS] [-ff|--from-file]\n\n");
	printf("\t-ff|--from-file\t\t\t read information about processes from files "\
								"<path_to_MLB>/mh1.txt and mh2.txt\n");
	printf("\t-pc|--problem-class [ABCWS]\t set problem class to one of A,B,C,W,S\n");								
}


double MLB_Run(int n, int np, int max_steps, int delta, int res_start, int inter_flag, MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM)
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

    if( res_start && (steps+1 >= res_start) )
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

#if 0
      if(world_rank == 0) // && steps % delta == 0)
	printf("%d %f\n", steps, global_res);
#endif

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

  if(res_start) return ((double)(steps + 1));
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
  int i;
  int problem_class = S;
  int world_rank, world_size;
  int local_rank, local_size;
  int from_file = 0;
  MPI_Comm MLB_LOCAL_COMM, MLB_INTER_COMM;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  /* handle programm parameters */
  for (i = 1; i < argc; i++)
  {
  	if ((strcmp(argv[i], "-pc")==0)||(strcmp(argv[i], "--problem-class")==0))
  	{
		if (argv[++i])
			switch(argv[i][0])
			{
				case 'A': problem_class = A; break;
				case 'B': problem_class = B; break;
				case 'C': problem_class = C; break;
				case 'W': problem_class = W; break;
				case 'S': problem_class = S; break;
				default: if(world_rank == 0) 
							printf("Not existing problem class '%c'. Using default (%d) instead.\n", argv[i][0], problem_class);
			}
		else
			if(world_rank == 0)
				printf("Missing problem class. Using default (%d) instead.\n", problem_class);
  	}
  	else if ((strcmp(argv[i], "-ff")==0)||(strcmp(argv[i], "--from-file")==0))
  	{
  		from_file = 1;
  	}
  	else if ((strcmp(argv[i], "-h")==0)||(strcmp(argv[i], "--help")==0))
  	{
  		if(world_rank == 0) 
	  		MLB_usage();
		MPI_Finalize();
  		return 0;
  	}
  	else
  		if(world_rank == 0) 
			printf("Ignoring unknown argument '%s'.\n", argv[i]);
  }

  MLB_Init_comm(&MLB_LOCAL_COMM, &MLB_INTER_COMM, from_file);

  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);
  MPI_Comm_size(MLB_LOCAL_COMM, &local_size);

  {
    int n, np;
    int i, j, k;

    double power_ratio = 1.0;
    double local_load  = 1.0;
    double remote_load = 1.0;

    int steps = 0;
    int delta = 1;

    int res_start = 0;
    int max_steps = 0;

    int res_flag = 1;
    int inter_flag = 1;

    double time;
    double transpar_time;
    double best_loc_time;
    double best_opt_time;

    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    /******************************************/
    {
      if(world_rank == 0)
      {
	printf("\nMLB: Performing a transparent run...\n");
	fflush(stdout);
      }

      local_load  = 1.0;
      remote_load = 1.0;

      n  = MLB_Class_size[problem_class] + 2;
      np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);

      for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
      {
	if(i == local_rank) k++;
      }

      np = k + 2;

      steps = (int) MLB_Run(n, np, max_steps = 0, delta = 1, res_start = 1, inter_flag = 1, MLB_LOCAL_COMM, MLB_INTER_COMM);
	
      time = MLB_Run(n, np, max_steps = steps, delta = 1, res_start = 0, inter_flag = 1, MLB_LOCAL_COMM, MLB_INTER_COMM);

      if(world_rank == 0)
      {
	printf("MLB: %f seconds for %d steps\n", time, steps);
      }     

      transpar_time = time;
    }
    /******************************************/

    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    /******************************************/
    {
      if(world_rank == 0)
      {
	printf("\nMLB: Measuring load imbalance...\n");
	fflush(stdout);
      }

      local_load  = 2.0;
      remote_load = 2.0;

      n  = MLB_Class_size[problem_class] + 2;
      np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);

      for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
      {
	if(i == local_rank) k++;
      }

      np = k + 2;

#if 0
      steps = (int) MLB_Run(n, np, max_steps = 0, delta = 1, res_start = 1 , 0, MLB_LOCAL_COMM, MLB_INTER_COMM);
#endif	
      
      time = MLB_Run(n, np, max_steps = steps, delta = 1, res_start = 0, 0, MLB_LOCAL_COMM, MLB_INTER_COMM);

      if(local_rank == 0)
      {
	double remote_time;
	MPI_Status status;
	
	MPI_Sendrecv(&time, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, &remote_time, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);
	
	printf("MLB: %f seconds for %d steps (remote time: %f)\n", time, steps, remote_time);
	fflush(stdout);

	if(remote_time < time) best_loc_time = remote_time;
	else best_loc_time = time;	  
	
	power_ratio  = (remote_time / time) / (remote_time / time + 1);
	
	printf("MLB: power_ratio: %f\n", power_ratio);
	fflush(stdout);
      }
      
      MPI_Bcast(&best_loc_time, 1, MPI_DOUBLE, 0, MLB_LOCAL_COMM);
      MPI_Bcast(&power_ratio, 1, MPI_DOUBLE, 0, MLB_LOCAL_COMM);

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
      if(world_rank == 0)
      {
	printf("\nMLB: Determining the convergence table...\n");     
	fflush(stdout);
      }   

      local_load  = 2.0 * power_ratio;
      remote_load = 2.0 * (1.0 - power_ratio);

      n  = MLB_Class_size[problem_class] + 2;
      np = (int)((local_load * MLB_Class_size[problem_class]) + 0.5);
      
      for(i=0, j=0, k=0; j<np; i=(i+1)%local_size, j++)
      {
	if(i == local_rank) k++;
      }
      
      np = k + 2;

      if(world_rank == 0)
      {
	printf("Delta\tSteps\tTime\n");
	fflush(stdout);
      }   
    
      {
	int break_count = MLB_TABLE_BREAK_COUNT;
	double last_time;
      
	delta = 1;

	while(1)
	{
	  if(world_rank == 0)
	  {
	    printf(" %d", delta);
	    fflush(stdout);
	  }

	  steps = (int) MLB_Run(n, np, max_steps = 0, delta, res_start = steps, 1, MLB_LOCAL_COMM, MLB_INTER_COMM);

	  if(world_rank == 0)
	  {
	    printf("\t%d", steps);
	    fflush(stdout);
	  }

	  if(delta==1)
	  {
	    time = transpar_time;
	    best_opt_time = time;
	  }

	  last_time = time;

	  time = MLB_Run(n, np, max_steps = steps, delta, res_start = 0, 1, MLB_LOCAL_COMM, MLB_INTER_COMM);

	  if(world_rank == 0)
	  {
	    printf("\t%f\n", time);
	    fflush(stdout);
	  }

	  if(time < best_opt_time) best_opt_time = time;
	
	  if(delta >= MLB_TABLE_MAX_COUNT) break;

	  if(last_time < time)
	  {
	    break_count--;
	    if(!break_count) break;
	  }
	  else break_count = MLB_TABLE_BREAK_COUNT;

	  delta++;
	}
      }
    }
    /******************************************/

    if(world_rank == 0)
    {
      printf("\nMLB: Speedup for transparent coupling: S = %f / %f = %.3f\n", best_loc_time,  transpar_time, best_loc_time/ transpar_time);
      printf("MLB: Speedup with optimized algorithm: S = %f / %f = %.3f\n\n", best_loc_time, best_opt_time, best_loc_time/best_opt_time);
      fflush(stdout);
    }
  }


  MPI_Finalize();

  return 0;
}
