#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "papi_wrappers.h"

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* colors for groups of functions */
static int _app_col  = YELLOW; /* Application */
static int _coll_col = BLUE;   /* Collective ops */
static int _send_col = RED;    /* Send */
static int _recv_col = GREEN;  /* Receive */
static int _wait_col = ORANGE; /* wait, test, cancel */
static int _def_col  = WHITE;  /* Non communicating */

static int bITrace=FALSE;

/* Collective ops */

/* Send */

/* Receive */

/* wait, test, cancel */

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Barrier( comm )
MPI_Comm comm;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Barrier( comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Bcast( buffer, count, datatype, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Gatherv( sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Op_create( function, commute, op )
MPI_User_function * function;
int commute;
MPI_Op * op;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Op_create( function, commute, op );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Op_free( op )
MPI_Op * op;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Op_free( op );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Scan( sendbuf, recvbuf, count, datatype, op, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Scatter( sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Scatterv( sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Attr_delete( comm, keyval )
MPI_Comm comm;
int keyval;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Attr_delete( comm, keyval );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Attr_get( comm, keyval, attr_value, flag )
MPI_Comm comm;
int keyval;
void * attr_value;
int * flag;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Attr_get( comm, keyval, attr_value, flag );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Attr_put( comm, keyval, attr_value )
MPI_Comm comm;
int keyval;
void * attr_value;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Attr_put( comm, keyval, attr_value );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_compare( comm1, comm2, result )
MPI_Comm comm1;
MPI_Comm comm2;
int * result;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_compare( comm1, comm2, result );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_create( comm, group, comm_out )
MPI_Comm comm;
MPI_Group group;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_create( comm, group, comm_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_dup( comm, comm_out )
MPI_Comm comm;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_dup( comm, comm_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_free( comm )
MPI_Comm * comm;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_free( comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_group( comm, group );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_rank( comm, rank )
MPI_Comm comm;
int * rank;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_rank( comm, rank );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_remote_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_remote_group( comm, group );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_remote_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_remote_size( comm, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_size( comm, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_split( comm, color, key, comm_out )
MPI_Comm comm;
int color;
int key;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_split( comm, color, key, comm_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Comm_test_inter( comm, flag )
MPI_Comm comm;
int * flag;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Comm_test_inter( comm, flag );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_compare( group1, group2, result )
MPI_Group group1;
MPI_Group group2;
int * result;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_compare( group1, group2, result );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_difference( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_difference( group1, group2, group_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_excl( group, n, ranks, newgroup )
MPI_Group group;
int n;
int * ranks;
MPI_Group * newgroup;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_excl( group, n, ranks, newgroup );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_free( group )
MPI_Group * group;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_free( group );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_incl( group, n, ranks, group_out )
MPI_Group group;
int n;
int * ranks;
MPI_Group * group_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_incl( group, n, ranks, group_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_intersection( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_intersection( group1, group2, group_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_rank( group, rank )
MPI_Group group;
int * rank;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_rank( group, rank );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_range_excl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_range_excl( group, n, ranges, newgroup );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_range_incl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_range_incl( group, n, ranges, newgroup );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_size( group, size )
MPI_Group group;
int * size;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_size( group, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_translate_ranks( group_a, n, ranks_a, group_b, ranks_b );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Group_union( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Group_union( group1, group2, group_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Intercomm_create( local_comm, local_leader, peer_comm, remote_leader, tag, comm_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Intercomm_merge( comm, high, comm_out )
MPI_Comm comm;
int high;
MPI_Comm * comm_out;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Intercomm_merge( comm, high, comm_out );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state )
MPI_Copy_function * copy_fn;
MPI_Delete_function * delete_fn;
int * keyval;
void * extra_state;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Keyval_free( keyval )
int * keyval;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Keyval_free( keyval );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Abort( comm, errorcode )
MPI_Comm comm;
int errorcode;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Abort( comm, errorcode );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Error_class( errorcode, errorclass )
int errorcode;
int * errorclass;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Error_class( errorcode, errorclass );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Errhandler_create( function, errhandler )
MPI_Handler_function * function;
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Errhandler_create( function, errhandler );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Errhandler_free( errhandler )
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Errhandler_free( errhandler );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler * errhandler;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Errhandler_get( comm, errhandler );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Error_string( errorcode, string, resultlen )
int errorcode;
char * string;
int * resultlen;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Error_string( errorcode, string, resultlen );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Errhandler_set( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler errhandler;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Errhandler_set( comm, errhandler );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Finalize(  )
{
  int  returnVal;
  
  /* stop_perfometer(); */
  
  returnVal = PMPI_Finalize(  );


  return returnVal;
}

int  MPI_Get_processor_name( name, resultlen )
char * name;
int * resultlen;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Get_processor_name( name, resultlen );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Init( argc, argv )
int * argc;
char *** argv;
{
  int  returnVal;
  
  char szFileName[1024];
  int iRank;
  char* proclist = getenv("MPIPAPI_PROCS");
  char* pos;
  char* lasts;  

  
  returnVal = PMPI_Init( argc, argv );


  MPI_Comm_rank(MPI_COMM_WORLD, &iRank);
  if ((getenv("MPIRUNPID") == NULL ) || (getenv("MPIRUNWD") == NULL)) {
      perror("usage of papi requieres mpirunskript to export MPIRUNWD and MPIRUNPID shell variable");
      MPI_Abort(MPI_COMM_WORLD, -1);
  }
  
  if (proclist == NULL ) {
      bITrace = TRUE;
  }
  else {
    for(pos = strtok_r(proclist,":",&lasts); pos != NULL; pos = strtok_r(NULL,":",&lasts))
      if ( atoi(pos) == iRank )
        bITrace = TRUE;
  }
  sprintf(szFileName,"%s/%s.%d", getenv("MPIRUNWD"), getenv("MPIRUNPID"), iRank);
  if (bITrace) fperfometer(szFileName);
  if (bITrace) mark_perfometer(_app_col, "Application");

  MPI_Barrier(MPI_COMM_WORLD);

  return returnVal;
}

int  MPI_Initialized( flag )
int * flag;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Initialized( flag );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

double  MPI_Wtick(  )
{
  double  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Wtick(  );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

double  MPI_Wtime(  )
{
  double  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Wtime(  );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Address( location, address )
void * location;
MPI_Aint * address;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Address( location, address );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Bsend( buf, count, datatype, dest, tag, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Bsend_init( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Buffer_attach( buffer, size )
void * buffer;
int size;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Buffer_attach( buffer, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Buffer_detach( buffer, size )
void * buffer;
int * size;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Buffer_detach( buffer, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Cancel( request )
MPI_Request * request;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Cancel( request );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Request_free( request )
MPI_Request * request;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Request_free( request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Recv_init( buf, count, datatype, source, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Send_init( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Get_elements( status, datatype, elements )
MPI_Status * status;
MPI_Datatype datatype;
int * elements;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Get_elements( status, datatype, elements );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Get_count( status, datatype, count )
MPI_Status * status;
MPI_Datatype datatype;
int * count;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Get_count( status, datatype, count );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Ibsend( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Iprobe( source, tag, comm, flag, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_recv_col, "Receive");
  
  returnVal = PMPI_Irecv( buf, count, datatype, source, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Irsend( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Isend( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Issend( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Pack( inbuf, incount, type, outbuf, outcount, position, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Pack_size( incount, datatype, comm, size )
int incount;
MPI_Datatype datatype;
MPI_Comm comm;
int * size;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Pack_size( incount, datatype, comm, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Probe( source, tag, comm, status )
int source;
int tag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Probe( source, tag, comm, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_recv_col, "Receive");
  
  returnVal = PMPI_Recv( buf, count, datatype, source, tag, comm, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Rsend( buf, count, datatype, dest, tag, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Rsend_init( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Send( buf, count, datatype, dest, tag, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Sendrecv( sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  
  returnVal = PMPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, source, recvtag, comm, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_send_col, "Send");
  
  returnVal = PMPI_Ssend( buf, count, datatype, dest, tag, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Ssend_init( buf, count, datatype, dest, tag, comm, request );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Start( request )
MPI_Request * request;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Start( request );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Startall( count, array_of_requests )
int count;
MPI_Request * array_of_requests;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Startall( count, array_of_requests );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Test( request, flag, status )
MPI_Request * request;
int * flag;
MPI_Status * status;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Test( request, flag, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Testall( count, array_of_requests, flag, array_of_statuses )
int count;
MPI_Request * array_of_requests;
int * flag;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Testall( count, array_of_requests, flag, array_of_statuses );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Testany( count, array_of_requests, index, flag, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Test_cancelled( status, flag )
MPI_Status * status;
int * flag;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Test_cancelled( status, flag );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Testsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Type_commit( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_commit( datatype );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Type_contiguous( count, old_type, newtype )
int count;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_contiguous( count, old_type, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Type_extent( datatype, extent )
MPI_Datatype datatype;
MPI_Aint * extent;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_extent( datatype, extent );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Type_free( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_free( datatype );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_hindexed( count, blocklens, indices, old_type, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_hvector( count, blocklen, stride, old_type, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_indexed( count, blocklens, indices, old_type, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Type_lb( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_lb( datatype, displacement );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Type_size( datatype, size )
MPI_Datatype datatype;
int * size;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_size( datatype, size );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_struct( count, blocklens, indices, old_types, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Type_ub( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_ub( datatype, displacement );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Type_vector( count, blocklen, stride, old_type, newtype );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Unpack( inbuf, insize, position, outbuf, outcount, type, comm );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Wait( request, status )
MPI_Request * request;
MPI_Status * status;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Wait( request, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Waitall( count, array_of_requests, array_of_statuses )
int count;
MPI_Request * array_of_requests;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Waitall( count, array_of_requests, array_of_statuses );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Waitany( count, array_of_requests, index, status )
int count;
MPI_Request * array_of_requests;
int * index;
MPI_Status * status;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Waitany( count, array_of_requests, index, status );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  
  returnVal = PMPI_Waitsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Cart_coords( comm, rank, maxdims, coords )
MPI_Comm comm;
int rank;
int maxdims;
int * coords;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_coords( comm, rank, maxdims, coords );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_create( comm_old, ndims, dims, periods, reorder, comm_cart );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_get( comm, maxdims, dims, periods, coords );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_map( comm_old, ndims, dims, periods, newrank );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Cart_rank( comm, coords, rank )
MPI_Comm comm;
int * coords;
int * rank;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_rank( comm, coords, rank );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_shift( comm, direction, displ, source, dest );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Cart_sub( comm, remain_dims, comm_new )
MPI_Comm comm;
int * remain_dims;
MPI_Comm * comm_new;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cart_sub( comm, remain_dims, comm_new );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Cartdim_get( comm, ndims )
MPI_Comm comm;
int * ndims;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Cartdim_get( comm, ndims );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int  MPI_Dims_create( nnodes, ndims, dims )
int nnodes;
int ndims;
int * dims;
{
  int  returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Dims_create( nnodes, ndims, dims );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graph_create( comm_old, nnodes, index, edges, reorder, comm_graph );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graph_get( comm, maxindex, maxedges, index, edges );

  if (bITrace) mark_perfometer(_app_col, "Application");

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
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graph_map( comm_old, nnodes, index, edges, newrank );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Graph_neighbors( comm, rank, maxneighbors, neighbors )
MPI_Comm comm;
int rank;
int  maxneighbors;
int * neighbors;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graph_neighbors( comm, rank, maxneighbors, neighbors );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Graph_neighbors_count( comm, rank, nneighbors )
MPI_Comm comm;
int rank;
int * nneighbors;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graph_neighbors_count( comm, rank, nneighbors );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Graphdims_get( comm, nnodes, nedges )
MPI_Comm comm;
int * nnodes;
int * nedges;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Graphdims_get( comm, nnodes, nedges );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

int   MPI_Topo_test( comm, top_type )
MPI_Comm comm;
int * top_type;
{
  int   returnVal;
  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  
  returnVal = PMPI_Topo_test( comm, top_type );

  if (bITrace) mark_perfometer(_app_col, "Application");

  return returnVal;
}

