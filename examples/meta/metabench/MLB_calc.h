#ifndef _MLB_CALC_H_
#define _MLB_CALC_H_

#include "MLB_common.h"

void MLB_Init_matrix(double*** retmat, int n, int np, MPI_Comm MLB_LOCAL_COMM, int);
void MLB_Free_matrix(double** matrix);
void MLB_Jacobi_iteration(double** old_matrix, double** new_matrix, int n, int np);
void MLB_Jacobi_exchange(double** old_matrix, double** new_matrix, int n, int np);
double MLB_Jacobi_local_powres(double** matrix, int n, int np);
double MLB_Jacobi_local_powmax(double** matrix, int n, int np);
double MLB_Jacobi_variation(double** old_matrix, double** new_matrix, int n, int np);

#endif
