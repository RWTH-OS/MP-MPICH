#include <stdio.h>
#include "mpi.h"
#include "timerwrap.h"

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

static statistics_t statistics[fun_dummy + 1];

static char stat_name[][24] = {
	"Application           ",
	"MPI collectives       ",
	"MPI barrier           ",
	"MPI send (blocking)   ",
	"MPI send (nonblocking)",
	"MPI recv (blocking)   ",
	"MPI recv (nonblocking)",
	"MPI completion check  ",
	"MPI onesided put      ",
	"MPI onesided get      ",
	"MPI onesided accu     ",
	"MPI onesided sync     ",
	"MPI memory management ",
	"other MPI functions   ",
	"DUMMY                 "
};

static double app_time;
static double rec_depth;

#define STAT_UPDATE(functype, t) \
if (rec_depth == 1) { \
statistics[functype].number_calls++; \
statistics[functype].acc_time += t; \
if ( t < statistics[functype].min_time) statistics[functype].min_time = t; \
if ( t > statistics[functype].max_time) statistics[functype].max_time = t; \
}

#define STAT_IN \
rec_depth++; \
if (rec_depth == 1) { \
  app_time = PMPI_Wtime() - app_time; \
  STAT_UPDATE(application, app_time); \
}

#define STAT_OUT \
if (rec_depth == 1) { \
  app_time = PMPI_Wtime(); \
} \
rec_depth--;


/* Collective ops */


/* Barrier */


/* Send blocking */


/* Receive blocking */


/* Send nonblocking */


/* Receive nonblocking */



/* wait, test, cancel */



/* Onesided put */


/* Onesided get */


/* Onesided accumulate */


/* Onesided synchronisation */


/* Memory management */



/* Init */


/* Finalize */


/* Others */


int   MPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm )
void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
void * recvbuf;
int recvcount;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm )
void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
void * recvbuf;
int * recvcounts;
int * displs;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm )
void * sendbuf;
void * recvbuf;
int count;
MPI_Datatype datatype;
MPI_Op op;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm )
void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnt;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int  returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm )
void * sendbuf;
int * sendcnts;
int * sdispls;
MPI_Datatype sendtype;
void * recvbuf;
int * recvcnts;
int * rdispls;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Barrier( comm )
MPI_Comm comm;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Barrier( comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(barrier, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Bcast( buffer, count, datatype, root, comm )
void * buffer;
int count;
MPI_Datatype datatype;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Bcast( buffer, count, datatype, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm )
void * sendbuf;
int sendcnt;
MPI_Datatype sendtype;
void * recvbuf;
int recvcount;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Gatherv( sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm )
void * sendbuf;
int sendcnt;
MPI_Datatype sendtype;
void * recvbuf;
int * recvcnts;
int * displs;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Gatherv( sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Op_create( function, commute, op )
MPI_User_function * function;
int commute;
MPI_Op * op;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Op_create( function, commute, op );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Op_free( op )
MPI_Op * op;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Op_free( op );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void * sendbuf;
void * recvbuf;
int * recvcnts;
MPI_Datatype datatype;
MPI_Op op;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm )
void * sendbuf;
void * recvbuf;
int count;
MPI_Datatype datatype;
MPI_Op op;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Scan( sendbuf, recvbuf, count, datatype, op, comm )
void * sendbuf;
void * recvbuf;
int count;
MPI_Datatype datatype;
MPI_Op op;
MPI_Comm comm;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Scan( sendbuf, recvbuf, count, datatype, op, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Scatter( sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm )
void * sendbuf;
int sendcnt;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnt;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Scatter( sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Scatterv( sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm )
void * sendbuf;
int * sendcnts;
int * displs;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnt;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int   returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Scatterv( sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Attr_delete( comm, keyval )
MPI_Comm comm;
int keyval;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Attr_delete( comm, keyval );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Attr_get( comm, keyval, attr_value, flag )
MPI_Comm comm;
int keyval;
void * attr_value;
int * flag;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Attr_get( comm, keyval, attr_value, flag );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Attr_put( comm, keyval, attr_value )
MPI_Comm comm;
int keyval;
void * attr_value;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Attr_put( comm, keyval, attr_value );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_compare( comm1, comm2, result )
MPI_Comm comm1;
MPI_Comm comm2;
int * result;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_compare( comm1, comm2, result );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_create( comm, group, comm_out )
MPI_Comm comm;
MPI_Group group;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_create( comm, group, comm_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_dup( comm, comm_out )
MPI_Comm comm;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_dup( comm, comm_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_free( comm )
MPI_Comm * comm;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_free( comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_group( comm, group );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_rank( comm, rank )
MPI_Comm comm;
int * rank;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_rank( comm, rank );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_remote_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_remote_group( comm, group );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_remote_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_remote_size( comm, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_size( comm, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_split( comm, color, key, comm_out )
MPI_Comm comm;
int color;
int key;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_split( comm, color, key, comm_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Comm_test_inter( comm, flag )
MPI_Comm comm;
int * flag;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Comm_test_inter( comm, flag );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_compare( group1, group2, result )
MPI_Group group1;
MPI_Group group2;
int * result;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_compare( group1, group2, result );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_difference( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_difference( group1, group2, group_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_excl( group, n, ranks, newgroup )
MPI_Group group;
int n;
int * ranks;
MPI_Group * newgroup;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_excl( group, n, ranks, newgroup );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_free( group )
MPI_Group * group;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_free( group );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_incl( group, n, ranks, group_out )
MPI_Group group;
int n;
int * ranks;
MPI_Group * group_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_incl( group, n, ranks, group_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_intersection( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_intersection( group1, group2, group_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_rank( group, rank )
MPI_Group group;
int * rank;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_rank( group, rank );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_range_excl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_range_excl( group, n, ranges, newgroup );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_range_incl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_range_incl( group, n, ranges, newgroup );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_size( group, size )
MPI_Group group;
int * size;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_size( group, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_translate_ranks( group_a, n, ranks_a, group_b, ranks_b )
MPI_Group group_a;
int n;
int * ranks_a;
MPI_Group group_b;
int * ranks_b;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_translate_ranks( group_a, n, ranks_a, group_b, ranks_b );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Group_union( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Group_union( group1, group2, group_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Intercomm_create( local_comm, local_leader, peer_comm, remote_leader, tag, comm_out )
MPI_Comm local_comm;
int local_leader;
MPI_Comm peer_comm;
int remote_leader;
int tag;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Intercomm_create( local_comm, local_leader, peer_comm, remote_leader, tag, comm_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Intercomm_merge( comm, high, comm_out )
MPI_Comm comm;
int high;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Intercomm_merge( comm, high, comm_out );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state )
MPI_Copy_function * copy_fn;
MPI_Delete_function * delete_fn;
int * keyval;
void * extra_state;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Keyval_free( keyval )
int * keyval;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Keyval_free( keyval );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Abort( comm, errorcode )
MPI_Comm comm;
int errorcode;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Abort( comm, errorcode );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Error_class( errorcode, errorclass )
int errorcode;
int * errorclass;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Error_class( errorcode, errorclass );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Errhandler_create( function, errhandler )
MPI_Handler_function * function;
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Errhandler_create( function, errhandler );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Errhandler_free( errhandler )
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Errhandler_free( errhandler );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Errhandler_get( comm, errhandler );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Error_string( errorcode, string, resultlen )
int errorcode;
char * string;
int * resultlen;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Error_string( errorcode, string, resultlen );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Errhandler_set( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler errhandler;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Errhandler_set( comm, errhandler );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Finalize(  )
{
  int  returnVal;

  int i,p;
  int iRank;
  int iSize;
  double total_time = 0; 

  rec_depth = 1;

  app_time = PMPI_Wtime() - app_time;
  STAT_UPDATE(application, app_time);

  for (i = 0; i < fun_dummy; i++) {
	total_time  += statistics[i].acc_time;
  }

  PMPI_Comm_rank(MPI_COMM_WORLD, &iRank);
  PMPI_Comm_size(MPI_COMM_WORLD, &iSize);

  for (p=0; p<iSize; p++) {
    if(iRank == p) {
      printf("# time spent by process %d:\n", p);
      printf("# type                   calls  min [us]  max [us]  avg [us]   acc [s]  %% of total \n");

      for (i=0; i<fun_dummy; i++) {
        if (statistics[i].number_calls > 0) {
          printf("%s%8d%10.1f%10.1f%10.1f%10.3f%10.2f\n",
          statistics[i].name,
          statistics[i].number_calls,
          statistics[i].min_time * 1000000,
          statistics[i].max_time * 1000000,
          (statistics[i].acc_time / (double)statistics[i].number_calls) * 1000000,
          statistics[i].acc_time,
          statistics[i].acc_time/total_time*100);
        }
      }
      printf("\n");
      fflush(stdout);
    }
    PMPI_Barrier(MPI_COMM_WORLD);
  }
  
  returnVal = PMPI_Finalize(  );


  return returnVal;
}

int  MPI_Get_processor_name( name, resultlen )
char * name;
int * resultlen;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Get_processor_name( name, resultlen );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Init( argc, argv )
int * argc;
char *** argv;
{
  int  returnVal;
  
  int i;

  
  returnVal = PMPI_Init( argc, argv );


  for (i=0; i<fun_dummy; i++) {
    statistics[i].name = stat_name[i];   
    statistics[i].number_calls = 0;
    statistics[i].min_time = 1000000.0;
    statistics[i].max_time = 0.0;
    statistics[i].acc_time = 0.0;
  }  
  
  app_time = PMPI_Wtime();
  rec_depth = 0;

  return returnVal;
}

int  MPI_Initialized( flag )
int * flag;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Initialized( flag );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

double  MPI_Wtick(  )
{
  double  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Wtick(  );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

double  MPI_Wtime(  )
{
  double  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Wtime(  );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Address( location, address )
void * location;
MPI_Aint * address;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Address( location, address );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Bsend( buf, count, datatype, dest, tag, comm )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Bsend( buf, count, datatype, dest, tag, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_blck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Bsend_init( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Bsend_init( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Buffer_attach( buffer, size )
void * buffer;
int size;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Buffer_attach( buffer, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Buffer_detach( buffer, size )
void * buffer;
int * size;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Buffer_detach( buffer, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Cancel( request )
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cancel( request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Request_free( request )
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Request_free( request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Recv_init( buf, count, datatype, source, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int source;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Recv_init( buf, count, datatype, source, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Send_init( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Send_init( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Get_elements( status, datatype, elements )
MPI_Status * status;
MPI_Datatype datatype;
int * elements;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Get_elements( status, datatype, elements );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Get_count( status, datatype, count )
MPI_Status * status;
MPI_Datatype datatype;
int * count;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Get_count( status, datatype, count );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Ibsend( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Ibsend( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_noblck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Iprobe( source, tag, comm, flag, status )
int source;
int tag;
MPI_Comm comm;
int * flag;
MPI_Status * status;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Iprobe( source, tag, comm, flag, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Irecv( buf, count, datatype, source, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int source;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Irecv( buf, count, datatype, source, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(recv_noblck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Irsend( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Irsend( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_noblck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Isend( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Isend( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_noblck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Issend( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Issend( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_noblck, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Pack( inbuf, incount, type, outbuf, outcount, position, comm )
void * inbuf;
int incount;
MPI_Datatype type;
void * outbuf;
int outcount;
int * position;
MPI_Comm comm;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Pack( inbuf, incount, type, outbuf, outcount, position, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Pack_size( incount, datatype, comm, size )
int incount;
MPI_Datatype datatype;
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Pack_size( incount, datatype, comm, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Probe( source, tag, comm, status )
int source;
int tag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Probe( source, tag, comm, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Recv( buf, count, datatype, source, tag, comm, status )
void * buf;
int count;
MPI_Datatype datatype;
int source;
int tag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Recv( buf, count, datatype, source, tag, comm, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(recv_blck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Rsend( buf, count, datatype, dest, tag, comm )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Rsend( buf, count, datatype, dest, tag, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_blck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Rsend_init( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Rsend_init( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Send( buf, count, datatype, dest, tag, comm )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Send( buf, count, datatype, dest, tag, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_blck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Sendrecv( sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status )
void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
int dest;
int sendtag;
void * recvbuf;
int recvcount;
MPI_Datatype recvtype;
int source;
int recvtag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Sendrecv( sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, source, recvtag, comm, status )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int sendtag;
int source;
int recvtag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;

  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, source, recvtag, comm, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Ssend( buf, count, datatype, dest, tag, comm )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Ssend( buf, count, datatype, dest, tag, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_blck, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Ssend_init( buf, count, datatype, dest, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Ssend_init( buf, count, datatype, dest, tag, comm, request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Start( request )
MPI_Request * request;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Start( request );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Startall( count, array_of_requests )
int count;
MPI_Request * array_of_requests;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Startall( count, array_of_requests );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Test( request, flag, status )
MPI_Request * request;
int * flag;
MPI_Status * status;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Test( request, flag, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Testall( count, array_of_requests, flag, array_of_statuses )
int count;
MPI_Request * array_of_requests;
int * flag;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Testall( count, array_of_requests, flag, array_of_statuses );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Testany( count, array_of_requests, index, flag, status )
int count;
MPI_Request * array_of_requests;
int * index;
int * flag;
MPI_Status * status;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Testany( count, array_of_requests, index, flag, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Test_cancelled( status, flag )
MPI_Status * status;
int * flag;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Test_cancelled( status, flag );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Testsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses )
int incount;
MPI_Request * array_of_requests;
int * outcount;
int * array_of_indices;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Testsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Type_commit( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_commit( datatype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_contiguous( count, old_type, newtype )
int count;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_contiguous( count, old_type, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_extent( datatype, extent )
MPI_Datatype datatype;
MPI_Aint * extent;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_extent( datatype, extent );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Type_free( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_free( datatype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_hindexed( count, blocklens, indices, old_type, newtype )
int count;
int * blocklens;
MPI_Aint * indices;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_hindexed( count, blocklens, indices, old_type, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_hvector( count, blocklen, stride, old_type, newtype )
int count;
int blocklen;
MPI_Aint stride;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_hvector( count, blocklen, stride, old_type, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_indexed( count, blocklens, indices, old_type, newtype )
int count;
int * blocklens;
int * indices;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_indexed( count, blocklens, indices, old_type, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Type_lb( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_lb( datatype, displacement );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Type_size( datatype, size )
MPI_Datatype datatype;
int * size;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_size( datatype, size );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_struct( count, blocklens, indices, old_types, newtype )
int count;
int * blocklens;
MPI_Aint * indices;
MPI_Datatype * old_types;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_struct( count, blocklens, indices, old_types, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Type_ub( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_ub( datatype, displacement );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Type_vector( count, blocklen, stride, old_type, newtype )
int count;
int blocklen;
int stride;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Type_vector( count, blocklen, stride, old_type, newtype );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Unpack( inbuf, insize, position, outbuf, outcount, type, comm )
void * inbuf;
int insize;
int * position;
void * outbuf;
int outcount;
MPI_Datatype type;
MPI_Comm comm;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Unpack( inbuf, insize, position, outbuf, outcount, type, comm );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Wait( request, status )
MPI_Request * request;
MPI_Status * status;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Wait( request, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Waitall( count, array_of_requests, array_of_statuses )
int count;
MPI_Request * array_of_requests;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Waitall( count, array_of_requests, array_of_statuses );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Waitany( count, array_of_requests, index, status )
int count;
MPI_Request * array_of_requests;
int * index;
MPI_Status * status;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Waitany( count, array_of_requests, index, status );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Waitsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses )
int incount;
MPI_Request * array_of_requests;
int * outcount;
int * array_of_indices;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Waitsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_coords( comm, rank, maxdims, coords )
MPI_Comm comm;
int rank;
int maxdims;
int * coords;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_coords( comm, rank, maxdims, coords );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_create( comm_old, ndims, dims, periods, reorder, comm_cart )
MPI_Comm comm_old;
int ndims;
int * dims;
int * periods;
int reorder;
MPI_Comm * comm_cart;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_create( comm_old, ndims, dims, periods, reorder, comm_cart );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_get( comm, maxdims, dims, periods, coords )
MPI_Comm comm;
int maxdims;
int * dims;
int * periods;
int * coords;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_get( comm, maxdims, dims, periods, coords );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_map( comm_old, ndims, dims, periods, newrank )
MPI_Comm comm_old;
int ndims;
int * dims;
int * periods;
int * newrank;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_map( comm_old, ndims, dims, periods, newrank );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_rank( comm, coords, rank )
MPI_Comm comm;
int * coords;
int * rank;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_rank( comm, coords, rank );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_shift( comm, direction, displ, source, dest )
MPI_Comm comm;
int direction;
int displ;
int * source;
int * dest;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_shift( comm, direction, displ, source, dest );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cart_sub( comm, remain_dims, comm_new )
MPI_Comm comm;
int * remain_dims;
MPI_Comm * comm_new;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cart_sub( comm, remain_dims, comm_new );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Cartdim_get( comm, ndims )
MPI_Comm comm;
int * ndims;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Cartdim_get( comm, ndims );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int  MPI_Dims_create( nnodes, ndims, dims )
int nnodes;
int ndims;
int * dims;
{
  int  returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Dims_create( nnodes, ndims, dims );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graph_create( comm_old, nnodes, index, edges, reorder, comm_graph )
MPI_Comm comm_old;
int nnodes;
int * index;
int * edges;
int reorder;
MPI_Comm * comm_graph;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graph_create( comm_old, nnodes, index, edges, reorder, comm_graph );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graph_get( comm, maxindex, maxedges, index, edges )
MPI_Comm comm;
int maxindex;
int maxedges;
int * index;
int * edges;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graph_get( comm, maxindex, maxedges, index, edges );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graph_map( comm_old, nnodes, index, edges, newrank )
MPI_Comm comm_old;
int nnodes;
int * index;
int * edges;
int * newrank;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graph_map( comm_old, nnodes, index, edges, newrank );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graph_neighbors( comm, rank, maxneighbors, neighbors )
MPI_Comm comm;
int rank;
int  maxneighbors;
int * neighbors;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graph_neighbors( comm, rank, maxneighbors, neighbors );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graph_neighbors_count( comm, rank, nneighbors )
MPI_Comm comm;
int rank;
int * nneighbors;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graph_neighbors_count( comm, rank, nneighbors );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Graphdims_get( comm, nnodes, nedges )
MPI_Comm comm;
int * nnodes;
int * nedges;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Graphdims_get( comm, nnodes, nedges );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int   MPI_Topo_test( comm, top_type )
MPI_Comm comm;
int * top_type;
{
  int   returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Topo_test( comm, top_type );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Alloc_mem( size, info, addr )
MPI_Aint size;
MPI_Info info;
void * addr;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Alloc_mem( size, info, addr );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(memory, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Free_mem( addr )
void * addr;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Free_mem( addr );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(memory, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_create( addr, size, rank, info, comm, win )
void * addr;
MPI_Aint size;
int rank;
MPI_Info info;
MPI_Comm comm;
MPI_Win * win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_create( addr, size, rank, info, comm, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(memory, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_free( win )
MPI_Win * win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_free( win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(memory, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_get_attr( win, key, attr, nbr_values )
MPI_Win win;
int key;
void * attr;
int * nbr_values;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_get_attr( win, key, attr, nbr_values );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_get_group( win, group )
MPI_Win win;
MPI_Group * group;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_get_group( win, group );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Put( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win )
void * addr;
int count;
MPI_Datatype origin_dtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype target_dtype;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Put( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_put, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Get( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win )
void * addr;
int count;
MPI_Datatype origin_dtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype target_dtype;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Get( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_get, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Accumulate( addr, scount, sdtype, rank, displcmnt, target_count, taget_dtype, op, win )
void * addr;
int scount;
MPI_Datatype sdtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype taget_dtype;
MPI_Op op;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Accumulate( addr, scount, sdtype, rank, displcmnt, target_count, taget_dtype, op, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_accu, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_fence( assert, win )
int  assert;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_fence( assert, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_start( group, assert, win )
MPI_Group group;
int assert;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_start( group, assert, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_complete( win )
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_complete( win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_post( group, assert, win )
MPI_Group group;
int assert;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_post( group, assert, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_wait( win )
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_wait( win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_test( win, flag )
MPI_Win win;
int * flag;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_test( win, flag );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_lock( locktype, rank, assert, win )
int locktype;
int rank;
int assert;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_lock( locktype, rank, assert, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}

int MPI_Win_unlock( rank, win )
int rank;
MPI_Win win;
{
  int returnVal;
  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  
  returnVal = PMPI_Win_unlock( rank, win );

  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;

  return returnVal;
}
