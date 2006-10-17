#ifndef _MLB_COMM_H_
#define _MLB_COMM_H_

#include "MLB_common.h"

void MLB_Inter_comm(double* sendbuf, double* recvbuf, int count, MPI_Comm MLB_INTER_COMM);
void MLB_Local_comm(double** sendbufs, double** recvbufs, int count, MPI_Comm MLB_LOCAL_COMM);
void MLB_Allreduce(double *sendbuf, double *recvbuf, MLB_Op op, MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM);
void MLB_Barrier(MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM);

#endif
