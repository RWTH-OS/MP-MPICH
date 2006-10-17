#include "MLB_common.h"
#include "MLB_calc.h"

/* exported functions: */
void MLB_Init_matrix(double***, int, int, MPI_Comm, int);
void MLB_Free_matrix(double**);
void MLB_Jacobi_iteration(double**, double**, int, int);
void MLB_Jacobi_exchange(double**, double**, int, int);
double MLB_Jacobi_variation(double**, double**, int, int);

/******************************************************************************/

void MLB_Init_matrix(double*** retmat, int n, int np, MPI_Comm MLB_LOCAL_COMM, int flag)
{
  int i,j ;
  int local_rank, local_size;
  double **matrix;

  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);
  MPI_Comm_size(MLB_LOCAL_COMM, &local_size);
  
  matrix = (double**)malloc(np*sizeof(double*));
  matrix[0] = (double*)malloc(np*n*sizeof(double));
  for(i = 1; i < np; i++)
    matrix[i] = matrix[i-1]+n;
  
  for(i = 0; i < np; i++)
    for(j = 0; j < n; j++)
      matrix[i][j] = MLB_INNER_START_VALUE;
  
  for(i = 0; i < np; i++)
  {
    matrix[i][0]   = MLB_BOUNDARY_VALUE;
    matrix[i][n-1] = MLB_BOUNDARY_VALUE;
  }
  
  if(flag)
  {
    if(local_rank == 0)
    {
      for(j = 0; j < n; j++)
	matrix[0][j] = MLB_BOUNDARY_VALUE;
    }
  }

  if(local_rank == local_size-1)
  {
    for(j = 0; j < n; j++)
      matrix[np-1][j] = MLB_BOUNDARY_VALUE;
  }

  *retmat = matrix;

  return;
}

void MLB_Free_matrix(double** matrix)
{
  free(matrix[0]);
  free(matrix);
}

void MLB_Jacobi_iteration(double** old_matrix, double** new_matrix, int n, int np)
{
  int i, j;

  for(i=1; i < np-1; i++)
    for(j=1; j < n-1; j++)
      new_matrix[i][j] = 0.25 * (old_matrix[i-1][j] + old_matrix[i+1][j]
			       + old_matrix[i][j-1] + old_matrix[i][j+1]);
  return;
}

double MLB_Jacobi_local_powres(double** matrix, int n, int np)
{
  int i, j;
  double res;

  res = 0.0;

  for( i=1; i < np-1; i++ )
    for( j=1; j < n-1; j++ )
      res = res + pow( 4 * matrix[i][j] - matrix[i-1][j] - matrix[i+1][j]
		                        - matrix[i][j-1] - matrix[i][j+1], 2);
  return res;
}

double MLB_Jacobi_local_powmax(double** matrix, int n, int np)
{
  /* XXX die oo-Norm liefert ausgepraegtere Residuums-Anstiege nach einer
         Inter-Metahost-Kommunikation (delta)als die euklidische Norm!
  */
  
  int i, j;
  double max;

  max = 0.0;

  for( i=1; i < np-1; i++ )
    for( j=1; j < n-1; j++ )
      if( fabs( 4 * matrix[i][j] - matrix[i-1][j] - matrix[i+1][j]
		- matrix[i][j-1] - matrix[i][j+1]) > max) max = fabs( 4 * matrix[i][j] - matrix[i-1][j] - matrix[i+1][j]
								      - matrix[i][j-1] - matrix[i][j+1]);
  return max/4;
}

double MLB_Jacobi_variation(double** old_matrix, double** new_matrix, int n, int np)
{
  /* XXX die Abweichung old-new (Variation) liefert erst im nachsten
         Iterations-Schritt ein Anstieg des Residuums!
	 --> daher besser die oo-Norm verwenden!     
   */

  int i, j;
  double max, tmp;

  max = 0.0;
  
  for(i=1; i < np-1; i++)
  {
    for(j=1; j < n-1; j++)
    {      
      tmp = fabs(new_matrix[i][j] - old_matrix[i][j]);
      if(tmp > max) max = tmp;
    }
  }

  return max;
}

void MLB_Jacobi_exchange(double** old_matrix, double** new_matrix, int n, int np)
{
  int i, j;

  for(i=0; i < np; i++)
    for(j=0; j < n; j++)
      old_matrix[i][j] = new_matrix[i][j];

  return;
}
