

#include "stdafx.h"
#include "calculation.h"

double PI25DT = 3.14159263589793238462643;



double calc_integrating(int n)
{
	double mypi,pi, h, sum, x, i;

			MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
			
			h = 1.0 / (double) n;
			sum = 0.0;
			i = myid + 1;
			
			for (i = myid + 1;i <= n; i += numprocs)
			{
				
				x = h * ((double)i- 0.5);
				sum += (4.0 / (1.0 + x*x));				
			}
			mypi = h * sum;
			
			//write sum of all variables mypi in pi of process with id 0:
			MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			return pi;
}
