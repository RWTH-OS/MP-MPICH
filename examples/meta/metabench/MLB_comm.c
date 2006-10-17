#include "MLB_comm.h"
#include "MLB_topol.h"
#include "MLB_common.h"

/* exported functions: */
void MLB_Inter_comm(double*, double*, int, MPI_Comm);
void MLB_Local_comm(double**, double**, int, MPI_Comm);
void MLB_Allreduce(double*, double*, MLB_Op, MPI_Comm, MPI_Comm);
void MLB_Barrier(MPI_Comm, MPI_Comm);

/******************************************************************************/

void MLB_Inter_comm(double* sendbuf, double* recvbuf, int count, MPI_Comm MLB_INTER_COMM)
{
  MPI_Status status;

  MPI_Sendrecv(sendbuf, count, MPI_DOUBLE, 0, MLB_INTER_TAG, recvbuf, count, MPI_DOUBLE, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);

  return;
}

void MLB_Local_comm(double** sendbufs, double** recvbufs, int count, MPI_Comm MLB_LOCAL_COMM)
{
  int local_rank, local_size;

  MPI_Status  status[4];
  MPI_Request requests[4];
  MPI_Request *up_requests   = requests;
  MPI_Request *down_requests = requests + 2;
  
  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);
  MPI_Comm_size(MLB_LOCAL_COMM, &local_size);  

  if(local_rank > 0)
  {
    MPI_Isend(sendbufs[0], count, MPI_DOUBLE, local_rank-1, MLB_LOCAL_TAG, MLB_LOCAL_COMM, &(up_requests[0]));
    MPI_Irecv(recvbufs[0], count, MPI_DOUBLE, local_rank-1, MLB_LOCAL_TAG, MLB_LOCAL_COMM, &(up_requests[1]));
  }

  if(local_rank < local_size-1)
  {
    MPI_Isend(sendbufs[1], count, MPI_DOUBLE, local_rank+1, MLB_LOCAL_TAG, MLB_LOCAL_COMM, &(down_requests[0]));
    MPI_Irecv(recvbufs[1], count, MPI_DOUBLE, local_rank+1, MLB_LOCAL_TAG, MLB_LOCAL_COMM, &(down_requests[1]));
  }

  if( (local_rank > 0) && (local_rank < local_size-1) )
    MPI_Waitall(4, requests, status);
  else
  {
    if( (local_rank == 0) && (local_size > 1 ) )
      MPI_Waitall(2, down_requests, status);

    if( (local_rank == local_size-1) && (local_size > 1 ) )
      MPI_Waitall(2, up_requests, status);
  }

  return;
}

void MLB_Allreduce(double *sendbuf, double *recvbuf, MLB_Op op, MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM)
{
  int local_rank;
  MPI_Status status;

  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);

  if(op==MLB_MAX) MPI_Reduce(sendbuf, recvbuf, 1, MPI_DOUBLE, MPI_MAX, 0, MLB_LOCAL_COMM);
  if(op==MLB_SUM) MPI_Reduce(sendbuf, recvbuf, 1, MPI_DOUBLE, MPI_SUM, 0, MLB_LOCAL_COMM);

  if(local_rank==0)
    MPI_Sendrecv(recvbuf, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, sendbuf, 1, MPI_DOUBLE, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);

  if(op==MLB_MAX)
    if(*sendbuf > *recvbuf) *recvbuf = *sendbuf;

  if(op==MLB_SUM)
    *recvbuf = *recvbuf + *sendbuf;

  MPI_Bcast(recvbuf, 1, MPI_DOUBLE, 0, MLB_LOCAL_COMM);
}


void MLB_Barrier(MPI_Comm MLB_LOCAL_COMM, MPI_Comm MLB_INTER_COMM)
{
  int local_rank;
  MPI_Status status;
  
  MPI_Comm_rank(MLB_LOCAL_COMM, &local_rank);
  
  MPI_Barrier(MLB_LOCAL_COMM);
  if(local_rank==0) MPI_Sendrecv(NULL, 0, MPI_CHAR, 0, MLB_INTER_TAG, NULL, 0, MPI_CHAR, 0, MLB_INTER_TAG, MLB_INTER_COMM, &status);
}
