/* $Id$ */

/* complete wrapper for automatic tracing via MPE */

#ifdef MPI_BUILD_PROFILING
#undef MPI_BUILD_PROFILING
#endif
#include <stdio.h>
#include <malloc.h>

#include "mpi.h"
#include "mpe.h"

#include "mpelog_colors.h"

#undef MPI_BUILD_PROFILING
#include "mpiimpl.h"
#include "reqalloc.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int MPI_Allgather_stateid_0,MPI_Allgather_ncalls_0=0;
static int MPI_Allgatherv_stateid_0,MPI_Allgatherv_ncalls_0=0;
static int MPI_Allreduce_stateid_0,MPI_Allreduce_ncalls_0=0;
static int MPI_Alltoall_stateid_0,MPI_Alltoall_ncalls_0=0;
static int MPI_Alltoallv_stateid_0,MPI_Alltoallv_ncalls_0=0;
static int MPI_Barrier_stateid_0,MPI_Barrier_ncalls_0=0;
static int MPI_Bcast_stateid_0,MPI_Bcast_ncalls_0=0;
static int MPI_Gather_stateid_0,MPI_Gather_ncalls_0=0;
static int MPI_Gatherv_stateid_0,MPI_Gatherv_ncalls_0=0;
static int MPI_Op_create_stateid_0,MPI_Op_create_ncalls_0=0;
static int MPI_Op_free_stateid_0,MPI_Op_free_ncalls_0=0;
static int MPI_Reduce_scatter_stateid_0,MPI_Reduce_scatter_ncalls_0=0;
static int MPI_Reduce_stateid_0,MPI_Reduce_ncalls_0=0;
static int MPI_Scan_stateid_0,MPI_Scan_ncalls_0=0;
static int MPI_Scatter_stateid_0,MPI_Scatter_ncalls_0=0;
static int MPI_Scatterv_stateid_0,MPI_Scatterv_ncalls_0=0;
static int MPI_Attr_delete_stateid_0,MPI_Attr_delete_ncalls_0=0;
static int MPI_Attr_get_stateid_0,MPI_Attr_get_ncalls_0=0;
static int MPI_Attr_put_stateid_0,MPI_Attr_put_ncalls_0=0;
static int MPI_Comm_compare_stateid_0,MPI_Comm_compare_ncalls_0=0;
static int MPI_Comm_create_stateid_0,MPI_Comm_create_ncalls_0=0;
static int MPI_Comm_dup_stateid_0,MPI_Comm_dup_ncalls_0=0;
static int MPI_Comm_free_stateid_0,MPI_Comm_free_ncalls_0=0;
static int MPI_Comm_group_stateid_0,MPI_Comm_group_ncalls_0=0;
static int MPI_Comm_rank_stateid_0,MPI_Comm_rank_ncalls_0=0;
static int MPI_Comm_remote_group_stateid_0,MPI_Comm_remote_group_ncalls_0=0;
static int MPI_Comm_remote_size_stateid_0,MPI_Comm_remote_size_ncalls_0=0;
static int MPI_Comm_size_stateid_0,MPI_Comm_size_ncalls_0=0;
static int MPI_Comm_split_stateid_0,MPI_Comm_split_ncalls_0=0;
static int MPI_Comm_test_inter_stateid_0,MPI_Comm_test_inter_ncalls_0=0;
static int MPI_Group_compare_stateid_0,MPI_Group_compare_ncalls_0=0;
static int MPI_Group_difference_stateid_0,MPI_Group_difference_ncalls_0=0;
static int MPI_Group_excl_stateid_0,MPI_Group_excl_ncalls_0=0;
static int MPI_Group_free_stateid_0,MPI_Group_free_ncalls_0=0;
static int MPI_Group_incl_stateid_0,MPI_Group_incl_ncalls_0=0;
static int MPI_Group_intersection_stateid_0,MPI_Group_intersection_ncalls_0=0;
static int MPI_Group_rank_stateid_0,MPI_Group_rank_ncalls_0=0;
static int MPI_Group_range_excl_stateid_0,MPI_Group_range_excl_ncalls_0=0;
static int MPI_Group_range_incl_stateid_0,MPI_Group_range_incl_ncalls_0=0;
static int MPI_Group_size_stateid_0,MPI_Group_size_ncalls_0=0;
static int MPI_Group_translate_ranks_stateid_0,MPI_Group_translate_ranks_ncalls_0=0;
static int MPI_Group_union_stateid_0,MPI_Group_union_ncalls_0=0;
static int MPI_Intercomm_create_stateid_0,MPI_Intercomm_create_ncalls_0=0;
static int MPI_Intercomm_merge_stateid_0,MPI_Intercomm_merge_ncalls_0=0;
static int MPI_Keyval_create_stateid_0,MPI_Keyval_create_ncalls_0=0;
static int MPI_Keyval_free_stateid_0,MPI_Keyval_free_ncalls_0=0;
static int MPI_Abort_stateid_0,MPI_Abort_ncalls_0=0;
static int MPI_Error_class_stateid_0,MPI_Error_class_ncalls_0=0;
static int MPI_Errhandler_create_stateid_0,MPI_Errhandler_create_ncalls_0=0;
static int MPI_Errhandler_free_stateid_0,MPI_Errhandler_free_ncalls_0=0;
static int MPI_Errhandler_get_stateid_0,MPI_Errhandler_get_ncalls_0=0;
static int MPI_Error_string_stateid_0,MPI_Error_string_ncalls_0=0;
static int MPI_Errhandler_set_stateid_0,MPI_Errhandler_set_ncalls_0=0;
static int MPI_Get_processor_name_stateid_0,MPI_Get_processor_name_ncalls_0=0;
static int MPI_Initialized_stateid_0,MPI_Initialized_ncalls_0=0;
static int MPI_Wtick_stateid_0,MPI_Wtick_ncalls_0=0;
static int MPI_Wtime_stateid_0,MPI_Wtime_ncalls_0=0;
static int MPI_Address_stateid_0,MPI_Address_ncalls_0=0;
static int MPI_Bsend_stateid_0,MPI_Bsend_ncalls_0=0;
static int MPI_Bsend_init_stateid_0,MPI_Bsend_init_ncalls_0=0;
static int MPI_Buffer_attach_stateid_0,MPI_Buffer_attach_ncalls_0=0;
static int MPI_Buffer_detach_stateid_0,MPI_Buffer_detach_ncalls_0=0;
static int MPI_Cancel_stateid_0,MPI_Cancel_ncalls_0=0;
static int MPI_Request_free_stateid_0,MPI_Request_free_ncalls_0=0;
static int MPI_Recv_init_stateid_0,MPI_Recv_init_ncalls_0=0;
static int MPI_Send_init_stateid_0,MPI_Send_init_ncalls_0=0;
static int MPI_Get_elements_stateid_0,MPI_Get_elements_ncalls_0=0;
static int MPI_Get_count_stateid_0,MPI_Get_count_ncalls_0=0;
static int MPI_Ibsend_stateid_0,MPI_Ibsend_ncalls_0=0;
static int MPI_Iprobe_stateid_0,MPI_Iprobe_ncalls_0=0;
static int MPI_Irecv_stateid_0,MPI_Irecv_ncalls_0=0;
static int MPI_Irsend_stateid_0,MPI_Irsend_ncalls_0=0;
static int MPI_Isend_stateid_0,MPI_Isend_ncalls_0=0;
static int MPI_Issend_stateid_0,MPI_Issend_ncalls_0=0;
static int MPI_Pack_stateid_0,MPI_Pack_ncalls_0=0;
static int MPI_Pack_size_stateid_0,MPI_Pack_size_ncalls_0=0;
static int MPI_Probe_stateid_0,MPI_Probe_ncalls_0=0;
static int MPI_Recv_stateid_0,MPI_Recv_ncalls_0=0;
static int MPI_Rsend_stateid_0,MPI_Rsend_ncalls_0=0;
static int MPI_Rsend_init_stateid_0,MPI_Rsend_init_ncalls_0=0;
static int MPI_Send_stateid_0,MPI_Send_ncalls_0=0;
static int MPI_Sendrecv_stateid_0,MPI_Sendrecv_ncalls_0=0;
static int MPI_Sendrecv_replace_stateid_0,MPI_Sendrecv_replace_ncalls_0=0;
static int MPI_Ssend_stateid_0,MPI_Ssend_ncalls_0=0;
static int MPI_Ssend_init_stateid_0,MPI_Ssend_init_ncalls_0=0;
static int MPI_Start_stateid_0,MPI_Start_ncalls_0=0;
static int MPI_Startall_stateid_0,MPI_Startall_ncalls_0=0;
static int MPI_Test_stateid_0,MPI_Test_ncalls_0=0;
static int MPI_Testall_stateid_0,MPI_Testall_ncalls_0=0;
static int MPI_Testany_stateid_0,MPI_Testany_ncalls_0=0;
static int MPI_Test_cancelled_stateid_0,MPI_Test_cancelled_ncalls_0=0;
static int MPI_Testsome_stateid_0,MPI_Testsome_ncalls_0=0;
static int MPI_Type_commit_stateid_0,MPI_Type_commit_ncalls_0=0;
static int MPI_Type_contiguous_stateid_0,MPI_Type_contiguous_ncalls_0=0;
static int MPI_Type_extent_stateid_0,MPI_Type_extent_ncalls_0=0;
static int MPI_Type_free_stateid_0,MPI_Type_free_ncalls_0=0;
static int MPI_Type_hindexed_stateid_0,MPI_Type_hindexed_ncalls_0=0;
static int MPI_Type_hvector_stateid_0,MPI_Type_hvector_ncalls_0=0;
static int MPI_Type_indexed_stateid_0,MPI_Type_indexed_ncalls_0=0;
static int MPI_Type_lb_stateid_0,MPI_Type_lb_ncalls_0=0;
static int MPI_Type_size_stateid_0,MPI_Type_size_ncalls_0=0;
static int MPI_Type_struct_stateid_0,MPI_Type_struct_ncalls_0=0;
static int MPI_Type_ub_stateid_0,MPI_Type_ub_ncalls_0=0;
static int MPI_Type_vector_stateid_0,MPI_Type_vector_ncalls_0=0;
static int MPI_Unpack_stateid_0,MPI_Unpack_ncalls_0=0;
static int MPI_Wait_stateid_0,MPI_Wait_ncalls_0=0;
static int MPI_Waitall_stateid_0,MPI_Waitall_ncalls_0=0;
static int MPI_Waitany_stateid_0,MPI_Waitany_ncalls_0=0;
static int MPI_Waitsome_stateid_0,MPI_Waitsome_ncalls_0=0;
static int MPI_Cart_coords_stateid_0,MPI_Cart_coords_ncalls_0=0;
static int MPI_Cart_create_stateid_0,MPI_Cart_create_ncalls_0=0;
static int MPI_Cart_get_stateid_0,MPI_Cart_get_ncalls_0=0;
static int MPI_Cart_map_stateid_0,MPI_Cart_map_ncalls_0=0;
static int MPI_Cart_rank_stateid_0,MPI_Cart_rank_ncalls_0=0;
static int MPI_Cart_shift_stateid_0,MPI_Cart_shift_ncalls_0=0;
static int MPI_Cart_sub_stateid_0,MPI_Cart_sub_ncalls_0=0;
static int MPI_Cartdim_get_stateid_0,MPI_Cartdim_get_ncalls_0=0;
static int MPI_Dims_create_stateid_0,MPI_Dims_create_ncalls_0=0;
static int MPI_Graph_create_stateid_0,MPI_Graph_create_ncalls_0=0;
static int MPI_Graph_get_stateid_0,MPI_Graph_get_ncalls_0=0;
static int MPI_Graph_map_stateid_0,MPI_Graph_map_ncalls_0=0;
static int MPI_Graph_neighbors_stateid_0,MPI_Graph_neighbors_ncalls_0=0;
static int MPI_Graph_neighbors_count_stateid_0,MPI_Graph_neighbors_count_ncalls_0=0;
static int MPI_Graphdims_get_stateid_0,MPI_Graphdims_get_ncalls_0=0;
static int MPI_Topo_test_stateid_0,MPI_Topo_test_ncalls_0=0;

static int WSOCK_CtrlMgsAvail_id_0;
static int WSOCK_RecvAnyCtrl_id_0;
static int WSOCK_WaitSend_id_0;
static int WSOCK_TestSend_id_0;
static int WSOCK_StartRequest_id_0;
static int WSOCK_TestRequest_id_0;
static int WSOCK_Send_id_0;
static int SHMEM_ReadCtrl_id_0;
static int WSOCK_RecvFromChannel_id_0;

static int procid_0;
static char logFileName_0[256];
static int finishing=0;



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

/*
    MPI_Allgather - prototyping replacement for MPI_Allgather
    Log the beginning and ending of the time spent in MPI_Allgather calls.
*/

  ++MPI_Allgather_ncalls_0;
  MPE_Log_event( MPI_Allgather_stateid_0*2,
	         MPI_Allgather_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );

  MPE_Log_event( MPI_Allgather_stateid_0*2+1,
	         MPI_Allgather_ncalls_0, (char *)0 );


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

/*
    MPI_Allgatherv - prototyping replacement for MPI_Allgatherv
    Log the beginning and ending of the time spent in MPI_Allgatherv calls.
*/

  ++MPI_Allgatherv_ncalls_0;
  MPE_Log_event( MPI_Allgatherv_stateid_0*2,
	         MPI_Allgatherv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm );

  MPE_Log_event( MPI_Allgatherv_stateid_0*2+1,
	         MPI_Allgatherv_ncalls_0, (char *)0 );


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


/*
    MPI_Allreduce - prototyping replacement for MPI_Allreduce
    Log the beginning and ending of the time spent in MPI_Allreduce calls.
*/

  ++MPI_Allreduce_ncalls_0;
  MPE_Log_event( MPI_Allreduce_stateid_0*2,
	         MPI_Allreduce_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );

  MPE_Log_event( MPI_Allreduce_stateid_0*2+1,
	         MPI_Allreduce_ncalls_0, (char *)0 );


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

/*
    MPI_Alltoall - prototyping replacement for MPI_Alltoall
    Log the beginning and ending of the time spent in MPI_Alltoall calls.
*/

  ++MPI_Alltoall_ncalls_0;
  MPE_Log_event( MPI_Alltoall_stateid_0*2,
	         MPI_Alltoall_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm );

  MPE_Log_event( MPI_Alltoall_stateid_0*2+1,
	         MPI_Alltoall_ncalls_0, (char *)0 );


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
  int   returnVal, i;
  int   typesize, commsize;

/*
    MPI_Alltoallv - prototyping replacement for MPI_Alltoallv
    Log the beginning and ending of the time spent in MPI_Alltoallv calls.
*/

  ++MPI_Alltoallv_ncalls_0;
  PMPI_Type_size(sendtype,&typesize);
  PMPI_Comm_size(comm, &commsize);

  MPE_Log_event( MPI_Alltoallv_stateid_0*2,
	         MPI_Alltoallv_ncalls_0, (char *)0 );
  
  for (i = 0; i < commsize; i++)
      MPE_Log_send(i,0,typesize*sendcnts[i]);
  returnVal = PMPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm );
  for (i = 0; i < commsize; i++)
      MPE_Log_receive(i,0,typesize*recvcnts[i]);

  MPE_Log_event( MPI_Alltoallv_stateid_0*2+1,
	         MPI_Alltoallv_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Barrier( comm )
MPI_Comm comm;
{
  int   returnVal;

/*
    MPI_Barrier - prototyping replacement for MPI_Barrier
    Log the beginning and ending of the time spent in MPI_Barrier calls.
*/

  ++MPI_Barrier_ncalls_0;
  MPE_Log_event( MPI_Barrier_stateid_0*2,
	         MPI_Barrier_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Barrier( comm );

  MPE_Log_event( MPI_Barrier_stateid_0*2+1,
	         MPI_Barrier_ncalls_0, (char *)0 );


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

/*
    MPI_Bcast - prototyping replacement for MPI_Bcast
    Log the beginning and ending of the time spent in MPI_Bcast calls.
*/

  ++MPI_Bcast_ncalls_0;
  MPE_Log_event( MPI_Bcast_stateid_0*2,
	         MPI_Bcast_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Bcast( buffer, count, datatype, root, comm );

  MPE_Log_event( MPI_Bcast_stateid_0*2+1,
	         MPI_Bcast_ncalls_0, (char *)0 );


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

/*
    MPI_Gather - prototyping replacement for MPI_Gather
    Log the beginning and ending of the time spent in MPI_Gather calls.
*/

  ++MPI_Gather_ncalls_0;
  MPE_Log_event( MPI_Gather_stateid_0*2,
	         MPI_Gather_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm );

  MPE_Log_event( MPI_Gather_stateid_0*2+1,
	         MPI_Gather_ncalls_0, (char *)0 );


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

/*
    MPI_Gatherv - prototyping replacement for MPI_Gatherv
    Log the beginning and ending of the time spent in MPI_Gatherv calls.
*/

  ++MPI_Gatherv_ncalls_0;
  MPE_Log_event( MPI_Gatherv_stateid_0*2,
	         MPI_Gatherv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Gatherv( sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm );

  MPE_Log_event( MPI_Gatherv_stateid_0*2+1,
	         MPI_Gatherv_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Op_create( function, commute, op )
MPI_User_function * function;
int commute;
MPI_Op * op;
{
  int  returnVal;

/*
    MPI_Op_create - prototyping replacement for MPI_Op_create
    Log the beginning and ending of the time spent in MPI_Op_create calls.
*/

  ++MPI_Op_create_ncalls_0;
  MPE_Log_event( MPI_Op_create_stateid_0*2,
	         MPI_Op_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Op_create( function, commute, op );

  MPE_Log_event( MPI_Op_create_stateid_0*2+1,
	         MPI_Op_create_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Op_free( op )
MPI_Op * op;
{
  int  returnVal;

/*
    MPI_Op_free - prototyping replacement for MPI_Op_free
    Log the beginning and ending of the time spent in MPI_Op_free calls.
*/

  ++MPI_Op_free_ncalls_0;
  MPE_Log_event( MPI_Op_free_stateid_0*2,
	         MPI_Op_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Op_free( op );

  MPE_Log_event( MPI_Op_free_stateid_0*2+1,
	         MPI_Op_free_ncalls_0, (char *)0 );


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

/*
    MPI_Reduce_scatter - prototyping replacement for MPI_Reduce_scatter
    Log the beginning and ending of the time spent in MPI_Reduce_scatter calls.
*/

  ++MPI_Reduce_scatter_ncalls_0;
  MPE_Log_event( MPI_Reduce_scatter_stateid_0*2,
	         MPI_Reduce_scatter_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op, comm );

  MPE_Log_event( MPI_Reduce_scatter_stateid_0*2+1,
	         MPI_Reduce_scatter_ncalls_0, (char *)0 );


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

/*
    MPI_Reduce - prototyping replacement for MPI_Reduce
    Log the beginning and ending of the time spent in MPI_Reduce calls.
*/

  ++MPI_Reduce_ncalls_0;
  MPE_Log_event( MPI_Reduce_stateid_0*2,
	         MPI_Reduce_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );

  MPE_Log_event( MPI_Reduce_stateid_0*2+1,
	         MPI_Reduce_ncalls_0, (char *)0 );


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

/*
    MPI_Scan - prototyping replacement for MPI_Scan
    Log the beginning and ending of the time spent in MPI_Scan calls.
*/

  ++MPI_Scan_ncalls_0;
  MPE_Log_event( MPI_Scan_stateid_0*2,
	         MPI_Scan_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Scan( sendbuf, recvbuf, count, datatype, op, comm );

  MPE_Log_event( MPI_Scan_stateid_0*2+1,
	         MPI_Scan_ncalls_0, (char *)0 );


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

/*
    MPI_Scatter - prototyping replacement for MPI_Scatter
    Log the beginning and ending of the time spent in MPI_Scatter calls.
*/

  ++MPI_Scatter_ncalls_0;
  MPE_Log_event( MPI_Scatter_stateid_0*2,
	         MPI_Scatter_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Scatter( sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  MPE_Log_event( MPI_Scatter_stateid_0*2+1,
	         MPI_Scatter_ncalls_0, (char *)0 );


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

/*
    MPI_Scatterv - prototyping replacement for MPI_Scatterv
    Log the beginning and ending of the time spent in MPI_Scatterv calls.
*/

  ++MPI_Scatterv_ncalls_0;
  MPE_Log_event( MPI_Scatterv_stateid_0*2,
	         MPI_Scatterv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Scatterv( sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm );

  MPE_Log_event( MPI_Scatterv_stateid_0*2+1,
	         MPI_Scatterv_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Attr_delete( comm, keyval )
MPI_Comm comm;
int keyval;
{
  int   returnVal;

/*
    MPI_Attr_delete - prototyping replacement for MPI_Attr_delete
    Log the beginning and ending of the time spent in MPI_Attr_delete calls.
*/

  ++MPI_Attr_delete_ncalls_0;
  MPE_Log_event( MPI_Attr_delete_stateid_0*2,
	         MPI_Attr_delete_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Attr_delete( comm, keyval );

  MPE_Log_event( MPI_Attr_delete_stateid_0*2+1,
	         MPI_Attr_delete_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Attr_get( comm, keyval, attr_value, flag )
MPI_Comm comm;
int keyval;
void * attr_value;
int * flag;
{
  int   returnVal;

/*
    MPI_Attr_get - prototyping replacement for MPI_Attr_get
    Log the beginning and ending of the time spent in MPI_Attr_get calls.
*/

  ++MPI_Attr_get_ncalls_0;
  MPE_Log_event( MPI_Attr_get_stateid_0*2,
	         MPI_Attr_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Attr_get( comm, keyval, attr_value, flag );

  MPE_Log_event( MPI_Attr_get_stateid_0*2+1,
	         MPI_Attr_get_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Attr_put( comm, keyval, attr_value )
MPI_Comm comm;
int keyval;
void * attr_value;
{
  int   returnVal;

/*
    MPI_Attr_put - prototyping replacement for MPI_Attr_put
    Log the beginning and ending of the time spent in MPI_Attr_put calls.
*/

  ++MPI_Attr_put_ncalls_0;
  MPE_Log_event( MPI_Attr_put_stateid_0*2,
	         MPI_Attr_put_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Attr_put( comm, keyval, attr_value );

  MPE_Log_event( MPI_Attr_put_stateid_0*2+1,
	         MPI_Attr_put_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_compare( comm1, comm2, result )
MPI_Comm comm1;
MPI_Comm comm2;
int * result;
{
  int   returnVal;

/*
    MPI_Comm_compare - prototyping replacement for MPI_Comm_compare
    Log the beginning and ending of the time spent in MPI_Comm_compare calls.
*/

  ++MPI_Comm_compare_ncalls_0;
  MPE_Log_event( MPI_Comm_compare_stateid_0*2,
	         MPI_Comm_compare_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_compare( comm1, comm2, result );

  MPE_Log_event( MPI_Comm_compare_stateid_0*2+1,
	         MPI_Comm_compare_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_create( comm, group, comm_out )
MPI_Comm comm;
MPI_Group group;
MPI_Comm * comm_out;
{
  int   returnVal;

/*
    MPI_Comm_create - prototyping replacement for MPI_Comm_create
    Log the beginning and ending of the time spent in MPI_Comm_create calls.
*/

  ++MPI_Comm_create_ncalls_0;
  MPE_Log_event( MPI_Comm_create_stateid_0*2,
	         MPI_Comm_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_create( comm, group, comm_out );

  MPE_Log_event( MPI_Comm_create_stateid_0*2+1,
	         MPI_Comm_create_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_dup( comm, comm_out )
MPI_Comm comm;
MPI_Comm * comm_out;
{
  int   returnVal;

/*
    MPI_Comm_dup - prototyping replacement for MPI_Comm_dup
    Log the beginning and ending of the time spent in MPI_Comm_dup calls.
*/

  ++MPI_Comm_dup_ncalls_0;
  MPE_Log_event( MPI_Comm_dup_stateid_0*2,
	         MPI_Comm_dup_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_dup( comm, comm_out );

  MPE_Log_event( MPI_Comm_dup_stateid_0*2+1,
	         MPI_Comm_dup_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_free( comm )
MPI_Comm * comm;
{
  int   returnVal;

/*
    MPI_Comm_free - prototyping replacement for MPI_Comm_free
    Log the beginning and ending of the time spent in MPI_Comm_free calls.
*/

  ++MPI_Comm_free_ncalls_0;
  MPE_Log_event( MPI_Comm_free_stateid_0*2,
	         MPI_Comm_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_free( comm );

  MPE_Log_event( MPI_Comm_free_stateid_0*2+1,
	         MPI_Comm_free_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;

/*
    MPI_Comm_group - prototyping replacement for MPI_Comm_group
    Log the beginning and ending of the time spent in MPI_Comm_group calls.
*/

  ++MPI_Comm_group_ncalls_0;
  MPE_Log_event( MPI_Comm_group_stateid_0*2,
	         MPI_Comm_group_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_group( comm, group );

  MPE_Log_event( MPI_Comm_group_stateid_0*2+1,
	         MPI_Comm_group_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_rank( comm, rank )
MPI_Comm comm;
int * rank;
{
  int   returnVal;

/*
    MPI_Comm_rank - prototyping replacement for MPI_Comm_rank
    Log the beginning and ending of the time spent in MPI_Comm_rank calls.
*/

  ++MPI_Comm_rank_ncalls_0;
  MPE_Log_event( MPI_Comm_rank_stateid_0*2,
	         MPI_Comm_rank_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_rank( comm, rank );

  MPE_Log_event( MPI_Comm_rank_stateid_0*2+1,
	         MPI_Comm_rank_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_remote_group( comm, group )
MPI_Comm comm;
MPI_Group * group;
{
  int   returnVal;

/*
    MPI_Comm_remote_group - prototyping replacement for MPI_Comm_remote_group
    Log the beginning and ending of the time spent in MPI_Comm_remote_group calls.
*/

  ++MPI_Comm_remote_group_ncalls_0;
  MPE_Log_event( MPI_Comm_remote_group_stateid_0*2,
	         MPI_Comm_remote_group_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_remote_group( comm, group );

  MPE_Log_event( MPI_Comm_remote_group_stateid_0*2+1,
	         MPI_Comm_remote_group_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_remote_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;

/*
    MPI_Comm_remote_size - prototyping replacement for MPI_Comm_remote_size
    Log the beginning and ending of the time spent in MPI_Comm_remote_size calls.
*/

  ++MPI_Comm_remote_size_ncalls_0;
  MPE_Log_event( MPI_Comm_remote_size_stateid_0*2,
	         MPI_Comm_remote_size_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_remote_size( comm, size );

  MPE_Log_event( MPI_Comm_remote_size_stateid_0*2+1,
	         MPI_Comm_remote_size_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;

/*
    MPI_Comm_size - prototyping replacement for MPI_Comm_size
    Log the beginning and ending of the time spent in MPI_Comm_size calls.
*/

  ++MPI_Comm_size_ncalls_0;
  MPE_Log_event( MPI_Comm_size_stateid_0*2,
	         MPI_Comm_size_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_size( comm, size );

  MPE_Log_event( MPI_Comm_size_stateid_0*2+1,
	         MPI_Comm_size_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_split( comm, color, key, comm_out )
MPI_Comm comm;
int color;
int key;
MPI_Comm * comm_out;
{
  int   returnVal;

/*
    MPI_Comm_split - prototyping replacement for MPI_Comm_split
    Log the beginning and ending of the time spent in MPI_Comm_split calls.
*/

  ++MPI_Comm_split_ncalls_0;
  MPE_Log_event( MPI_Comm_split_stateid_0*2,
	         MPI_Comm_split_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_split( comm, color, key, comm_out );

  MPE_Log_event( MPI_Comm_split_stateid_0*2+1,
	         MPI_Comm_split_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Comm_test_inter( comm, flag )
MPI_Comm comm;
int * flag;
{
  int   returnVal;

/*
    MPI_Comm_test_inter - prototyping replacement for MPI_Comm_test_inter
    Log the beginning and ending of the time spent in MPI_Comm_test_inter calls.
*/

  ++MPI_Comm_test_inter_ncalls_0;
  MPE_Log_event( MPI_Comm_test_inter_stateid_0*2,
	         MPI_Comm_test_inter_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Comm_test_inter( comm, flag );

  MPE_Log_event( MPI_Comm_test_inter_stateid_0*2+1,
	         MPI_Comm_test_inter_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_compare( group1, group2, result )
MPI_Group group1;
MPI_Group group2;
int * result;
{
  int   returnVal;

/*
    MPI_Group_compare - prototyping replacement for MPI_Group_compare
    Log the beginning and ending of the time spent in MPI_Group_compare calls.
*/

  ++MPI_Group_compare_ncalls_0;
  MPE_Log_event( MPI_Group_compare_stateid_0*2,
	         MPI_Group_compare_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_compare( group1, group2, result );

  MPE_Log_event( MPI_Group_compare_stateid_0*2+1,
	         MPI_Group_compare_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_difference( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;

/*
    MPI_Group_difference - prototyping replacement for MPI_Group_difference
    Log the beginning and ending of the time spent in MPI_Group_difference calls.
*/

  ++MPI_Group_difference_ncalls_0;
  MPE_Log_event( MPI_Group_difference_stateid_0*2,
	         MPI_Group_difference_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_difference( group1, group2, group_out );

  MPE_Log_event( MPI_Group_difference_stateid_0*2+1,
	         MPI_Group_difference_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_excl( group, n, ranks, newgroup )
MPI_Group group;
int n;
int * ranks;
MPI_Group * newgroup;
{
  int   returnVal;

/*
    MPI_Group_excl - prototyping replacement for MPI_Group_excl
    Log the beginning and ending of the time spent in MPI_Group_excl calls.
*/

  ++MPI_Group_excl_ncalls_0;
  MPE_Log_event( MPI_Group_excl_stateid_0*2,
	         MPI_Group_excl_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_excl( group, n, ranks, newgroup );

  MPE_Log_event( MPI_Group_excl_stateid_0*2+1,
	         MPI_Group_excl_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_free( group )
MPI_Group * group;
{
  int   returnVal;

/*
    MPI_Group_free - prototyping replacement for MPI_Group_free
    Log the beginning and ending of the time spent in MPI_Group_free calls.
*/

  ++MPI_Group_free_ncalls_0;
  MPE_Log_event( MPI_Group_free_stateid_0*2,
	         MPI_Group_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_free( group );

  MPE_Log_event( MPI_Group_free_stateid_0*2+1,
	         MPI_Group_free_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_incl( group, n, ranks, group_out )
MPI_Group group;
int n;
int * ranks;
MPI_Group * group_out;
{
  int   returnVal;

/*
    MPI_Group_incl - prototyping replacement for MPI_Group_incl
    Log the beginning and ending of the time spent in MPI_Group_incl calls.
*/

  ++MPI_Group_incl_ncalls_0;
  MPE_Log_event( MPI_Group_incl_stateid_0*2,
	         MPI_Group_incl_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_incl( group, n, ranks, group_out );

  MPE_Log_event( MPI_Group_incl_stateid_0*2+1,
	         MPI_Group_incl_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_intersection( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;

/*
    MPI_Group_intersection - prototyping replacement for MPI_Group_intersection
    Log the beginning and ending of the time spent in MPI_Group_intersection calls.
*/

  ++MPI_Group_intersection_ncalls_0;
  MPE_Log_event( MPI_Group_intersection_stateid_0*2,
	         MPI_Group_intersection_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_intersection( group1, group2, group_out );

  MPE_Log_event( MPI_Group_intersection_stateid_0*2+1,
	         MPI_Group_intersection_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_rank( group, rank )
MPI_Group group;
int * rank;
{
  int   returnVal;

/*
    MPI_Group_rank - prototyping replacement for MPI_Group_rank
    Log the beginning and ending of the time spent in MPI_Group_rank calls.
*/

  ++MPI_Group_rank_ncalls_0;
  MPE_Log_event( MPI_Group_rank_stateid_0*2,
	         MPI_Group_rank_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_rank( group, rank );

  MPE_Log_event( MPI_Group_rank_stateid_0*2+1,
	         MPI_Group_rank_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_range_excl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;

/*
    MPI_Group_range_excl - prototyping replacement for MPI_Group_range_excl
    Log the beginning and ending of the time spent in MPI_Group_range_excl calls.
*/

  ++MPI_Group_range_excl_ncalls_0;
  MPE_Log_event( MPI_Group_range_excl_stateid_0*2,
	         MPI_Group_range_excl_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_range_excl( group, n, ranges, newgroup );

  MPE_Log_event( MPI_Group_range_excl_stateid_0*2+1,
	         MPI_Group_range_excl_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_range_incl( group, n, ranges, newgroup )
MPI_Group group;
int n;
int ranges[][3];
MPI_Group * newgroup;
{
  int   returnVal;

/*
    MPI_Group_range_incl - prototyping replacement for MPI_Group_range_incl
    Log the beginning and ending of the time spent in MPI_Group_range_incl calls.
*/

  ++MPI_Group_range_incl_ncalls_0;
  MPE_Log_event( MPI_Group_range_incl_stateid_0*2,
	         MPI_Group_range_incl_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_range_incl( group, n, ranges, newgroup );

  MPE_Log_event( MPI_Group_range_incl_stateid_0*2+1,
	         MPI_Group_range_incl_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_size( group, size )
MPI_Group group;
int * size;
{
  int   returnVal;

/*
    MPI_Group_size - prototyping replacement for MPI_Group_size
    Log the beginning and ending of the time spent in MPI_Group_size calls.
*/

  ++MPI_Group_size_ncalls_0;
  MPE_Log_event( MPI_Group_size_stateid_0*2,
	         MPI_Group_size_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_size( group, size );

  MPE_Log_event( MPI_Group_size_stateid_0*2+1,
	         MPI_Group_size_ncalls_0, (char *)0 );


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

/*
    MPI_Group_translate_ranks - prototyping replacement for MPI_Group_translate_ranks
    Log the beginning and ending of the time spent in MPI_Group_translate_ranks calls.
*/

  ++MPI_Group_translate_ranks_ncalls_0;
  MPE_Log_event( MPI_Group_translate_ranks_stateid_0*2,
	         MPI_Group_translate_ranks_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_translate_ranks( group_a, n, ranks_a, group_b, ranks_b );

  MPE_Log_event( MPI_Group_translate_ranks_stateid_0*2+1,
	         MPI_Group_translate_ranks_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Group_union( group1, group2, group_out )
MPI_Group group1;
MPI_Group group2;
MPI_Group * group_out;
{
  int   returnVal;

/*
    MPI_Group_union - prototyping replacement for MPI_Group_union
    Log the beginning and ending of the time spent in MPI_Group_union calls.
*/

  ++MPI_Group_union_ncalls_0;
  MPE_Log_event( MPI_Group_union_stateid_0*2,
	         MPI_Group_union_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Group_union( group1, group2, group_out );

  MPE_Log_event( MPI_Group_union_stateid_0*2+1,
	         MPI_Group_union_ncalls_0, (char *)0 );


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

/*
    MPI_Intercomm_create - prototyping replacement for MPI_Intercomm_create
    Log the beginning and ending of the time spent in MPI_Intercomm_create calls.
*/

  ++MPI_Intercomm_create_ncalls_0;
  MPE_Log_event( MPI_Intercomm_create_stateid_0*2,
	         MPI_Intercomm_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Intercomm_create( local_comm, local_leader, peer_comm, remote_leader, tag, comm_out );

  MPE_Log_event( MPI_Intercomm_create_stateid_0*2+1,
	         MPI_Intercomm_create_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Intercomm_merge( comm, high, comm_out )
MPI_Comm comm;
int high;
MPI_Comm * comm_out;
{
  int   returnVal;

/*
    MPI_Intercomm_merge - prototyping replacement for MPI_Intercomm_merge
    Log the beginning and ending of the time spent in MPI_Intercomm_merge calls.
*/

  ++MPI_Intercomm_merge_ncalls_0;
  MPE_Log_event( MPI_Intercomm_merge_stateid_0*2,
	         MPI_Intercomm_merge_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Intercomm_merge( comm, high, comm_out );

  MPE_Log_event( MPI_Intercomm_merge_stateid_0*2+1,
	         MPI_Intercomm_merge_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state )
MPI_Copy_function * copy_fn;
MPI_Delete_function * delete_fn;
int * keyval;
void * extra_state;
{
  int   returnVal;

/*
    MPI_Keyval_create - prototyping replacement for MPI_Keyval_create
    Log the beginning and ending of the time spent in MPI_Keyval_create calls.
*/

  ++MPI_Keyval_create_ncalls_0;
  MPE_Log_event( MPI_Keyval_create_stateid_0*2,
	         MPI_Keyval_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Keyval_create( copy_fn, delete_fn, keyval, extra_state );

  MPE_Log_event( MPI_Keyval_create_stateid_0*2+1,
	         MPI_Keyval_create_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Keyval_free( keyval )
int * keyval;
{
  int   returnVal;

/*
    MPI_Keyval_free - prototyping replacement for MPI_Keyval_free
    Log the beginning and ending of the time spent in MPI_Keyval_free calls.
*/

  ++MPI_Keyval_free_ncalls_0;
  MPE_Log_event( MPI_Keyval_free_stateid_0*2,
	         MPI_Keyval_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Keyval_free( keyval );

  MPE_Log_event( MPI_Keyval_free_stateid_0*2+1,
	         MPI_Keyval_free_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Abort( comm, errorcode )
MPI_Comm comm;
int errorcode;
{
  int  returnVal;

/*
    MPI_Abort - prototyping replacement for MPI_Abort
    Log the beginning and ending of the time spent in MPI_Abort calls.
*/

  ++MPI_Abort_ncalls_0;
  MPE_Log_event( MPI_Abort_stateid_0*2,
	         MPI_Abort_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Abort( comm, errorcode );

  MPE_Log_event( MPI_Abort_stateid_0*2+1,
	         MPI_Abort_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Error_class( errorcode, errorclass )
int errorcode;
int * errorclass;
{
  int  returnVal;

/*
    MPI_Error_class - prototyping replacement for MPI_Error_class
    Log the beginning and ending of the time spent in MPI_Error_class calls.
*/

  ++MPI_Error_class_ncalls_0;
  MPE_Log_event( MPI_Error_class_stateid_0*2,
	         MPI_Error_class_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Error_class( errorcode, errorclass );

  MPE_Log_event( MPI_Error_class_stateid_0*2+1,
	         MPI_Error_class_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Errhandler_create( function, errhandler )
MPI_Handler_function * function;
MPI_Errhandler * errhandler;
{
  int  returnVal;

/*
    MPI_Errhandler_create - prototyping replacement for MPI_Errhandler_create
    Log the beginning and ending of the time spent in MPI_Errhandler_create calls.
*/

  ++MPI_Errhandler_create_ncalls_0;
  MPE_Log_event( MPI_Errhandler_create_stateid_0*2,
	         MPI_Errhandler_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Errhandler_create( function, errhandler );

  MPE_Log_event( MPI_Errhandler_create_stateid_0*2+1,
	         MPI_Errhandler_create_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Errhandler_free( errhandler )
MPI_Errhandler * errhandler;
{
  int  returnVal;

/*
    MPI_Errhandler_free - prototyping replacement for MPI_Errhandler_free
    Log the beginning and ending of the time spent in MPI_Errhandler_free calls.
*/

  ++MPI_Errhandler_free_ncalls_0;
  MPE_Log_event( MPI_Errhandler_free_stateid_0*2,
	         MPI_Errhandler_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Errhandler_free( errhandler );

  MPE_Log_event( MPI_Errhandler_free_stateid_0*2+1,
	         MPI_Errhandler_free_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler * errhandler;
{
  int  returnVal;

/*
    MPI_Errhandler_get - prototyping replacement for MPI_Errhandler_get
    Log the beginning and ending of the time spent in MPI_Errhandler_get calls.
*/

  ++MPI_Errhandler_get_ncalls_0;
  MPE_Log_event( MPI_Errhandler_get_stateid_0*2,
	         MPI_Errhandler_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Errhandler_get( comm, errhandler );

  MPE_Log_event( MPI_Errhandler_get_stateid_0*2+1,
	         MPI_Errhandler_get_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Error_string( errorcode, string, resultlen )
int errorcode;
char * string;
int * resultlen;
{
  int  returnVal;

/*
    MPI_Error_string - prototyping replacement for MPI_Error_string
    Log the beginning and ending of the time spent in MPI_Error_string calls.
*/

  ++MPI_Error_string_ncalls_0;
  MPE_Log_event( MPI_Error_string_stateid_0*2,
	         MPI_Error_string_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Error_string( errorcode, string, resultlen );

  MPE_Log_event( MPI_Error_string_stateid_0*2+1,
	         MPI_Error_string_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Errhandler_set( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler errhandler;
{
  int  returnVal;

/*
    MPI_Errhandler_set - prototyping replacement for MPI_Errhandler_set
    Log the beginning and ending of the time spent in MPI_Errhandler_set calls.
*/

  ++MPI_Errhandler_set_ncalls_0;
  MPE_Log_event( MPI_Errhandler_set_stateid_0*2,
	         MPI_Errhandler_set_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Errhandler_set( comm, errhandler );

  MPE_Log_event( MPI_Errhandler_set_stateid_0*2+1,
	         MPI_Errhandler_set_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Finalize(  )
{
  int  returnVal;

/*
    MPI_Finalize - prototyping replacement for MPI_Finalize
*/

  finishing=1;
  if (procid_0 == 0) {
    fprintf( stderr, "Writing logfile.\n");
	
    MPE_Describe_state( MPI_Allgather_stateid_0*2,
	                            MPI_Allgather_stateid_0*2+1,
      				    "MPI_Allgather", colors[MPI_Allgather_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Allgatherv_stateid_0*2,
	                            MPI_Allgatherv_stateid_0*2+1,
      				    "MPI_Allgatherv", colors[MPI_Allgatherv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Allreduce_stateid_0*2,
	                            MPI_Allreduce_stateid_0*2+1,
      				    "MPI_Allreduce", colors[MPI_Allreduce_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Alltoall_stateid_0*2,
	                            MPI_Alltoall_stateid_0*2+1,
      				    "MPI_Alltoall", colors[MPI_Alltoall_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Alltoallv_stateid_0*2,
	                            MPI_Alltoallv_stateid_0*2+1,
      				    "MPI_Alltoallv", colors[MPI_Alltoallv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Barrier_stateid_0*2,
	                            MPI_Barrier_stateid_0*2+1,
      				    "MPI_Barrier", colors[MPI_Barrier_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Bcast_stateid_0*2,
	                            MPI_Bcast_stateid_0*2+1,
      				    "MPI_Bcast", colors[MPI_Bcast_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Gather_stateid_0*2,
	                            MPI_Gather_stateid_0*2+1,
      				    "MPI_Gather", colors[MPI_Gather_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Gatherv_stateid_0*2,
	                            MPI_Gatherv_stateid_0*2+1,
      				    "MPI_Gatherv", colors[MPI_Gatherv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Op_create_stateid_0*2,
	                            MPI_Op_create_stateid_0*2+1,
      				    "MPI_Op_create", colors[MPI_Op_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Op_free_stateid_0*2,
	                            MPI_Op_free_stateid_0*2+1,
      				    "MPI_Op_free", colors[MPI_Op_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Reduce_scatter_stateid_0*2,
	                            MPI_Reduce_scatter_stateid_0*2+1,
      				    "MPI_Reduce_scatter", colors[MPI_Reduce_scatter_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Reduce_stateid_0*2,
	                            MPI_Reduce_stateid_0*2+1,
      				    "MPI_Reduce", colors[MPI_Reduce_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Scan_stateid_0*2,
	                            MPI_Scan_stateid_0*2+1,
      				    "MPI_Scan", colors[MPI_Scan_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Scatter_stateid_0*2,
	                            MPI_Scatter_stateid_0*2+1,
      				    "MPI_Scatter", colors[MPI_Scatter_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Scatterv_stateid_0*2,
	                            MPI_Scatterv_stateid_0*2+1,
      				    "MPI_Scatterv", colors[MPI_Scatterv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Attr_delete_stateid_0*2,
	                            MPI_Attr_delete_stateid_0*2+1,
      				    "MPI_Attr_delete", colors[MPI_Attr_delete_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Attr_get_stateid_0*2,
	                            MPI_Attr_get_stateid_0*2+1,
      				    "MPI_Attr_get", colors[MPI_Attr_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Attr_put_stateid_0*2,
	                            MPI_Attr_put_stateid_0*2+1,
      				    "MPI_Attr_put", colors[MPI_Attr_put_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_compare_stateid_0*2,
	                            MPI_Comm_compare_stateid_0*2+1,
      				    "MPI_Comm_compare", colors[MPI_Comm_compare_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_create_stateid_0*2,
	                            MPI_Comm_create_stateid_0*2+1,
      				    "MPI_Comm_create", colors[MPI_Comm_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_dup_stateid_0*2,
	                            MPI_Comm_dup_stateid_0*2+1,
      				    "MPI_Comm_dup", colors[MPI_Comm_dup_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_free_stateid_0*2,
	                            MPI_Comm_free_stateid_0*2+1,
      				    "MPI_Comm_free", colors[MPI_Comm_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_group_stateid_0*2,
	                            MPI_Comm_group_stateid_0*2+1,
      				    "MPI_Comm_group", colors[MPI_Comm_group_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_rank_stateid_0*2,
	                            MPI_Comm_rank_stateid_0*2+1,
      				    "MPI_Comm_rank", colors[MPI_Comm_rank_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_remote_group_stateid_0*2,
	                            MPI_Comm_remote_group_stateid_0*2+1,
      				    "MPI_Comm_remote_group", colors[MPI_Comm_remote_group_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_remote_size_stateid_0*2,
	                            MPI_Comm_remote_size_stateid_0*2+1,
      				    "MPI_Comm_remote_size", colors[MPI_Comm_remote_size_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_size_stateid_0*2,
	                            MPI_Comm_size_stateid_0*2+1,
      				    "MPI_Comm_size", colors[MPI_Comm_size_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_split_stateid_0*2,
	                            MPI_Comm_split_stateid_0*2+1,
      				    "MPI_Comm_split", colors[MPI_Comm_split_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Comm_test_inter_stateid_0*2,
	                            MPI_Comm_test_inter_stateid_0*2+1,
      				    "MPI_Comm_test_inter", colors[MPI_Comm_test_inter_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_compare_stateid_0*2,
	                            MPI_Group_compare_stateid_0*2+1,
      				    "MPI_Group_compare", colors[MPI_Group_compare_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_difference_stateid_0*2,
	                            MPI_Group_difference_stateid_0*2+1,
      				    "MPI_Group_difference", colors[MPI_Group_difference_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_excl_stateid_0*2,
	                            MPI_Group_excl_stateid_0*2+1,
      				    "MPI_Group_excl", colors[MPI_Group_excl_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_free_stateid_0*2,
	                            MPI_Group_free_stateid_0*2+1,
      				    "MPI_Group_free", colors[MPI_Group_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_incl_stateid_0*2,
	                            MPI_Group_incl_stateid_0*2+1,
      				    "MPI_Group_incl", colors[MPI_Group_incl_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_intersection_stateid_0*2,
	                            MPI_Group_intersection_stateid_0*2+1,
      				    "MPI_Group_intersection", colors[MPI_Group_intersection_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_rank_stateid_0*2,
	                            MPI_Group_rank_stateid_0*2+1,
      				    "MPI_Group_rank", colors[MPI_Group_rank_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_range_excl_stateid_0*2,
	                            MPI_Group_range_excl_stateid_0*2+1,
      				    "MPI_Group_range_excl", colors[MPI_Group_range_excl_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_range_incl_stateid_0*2,
	                            MPI_Group_range_incl_stateid_0*2+1,
      				    "MPI_Group_range_incl", colors[MPI_Group_range_incl_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_size_stateid_0*2,
	                            MPI_Group_size_stateid_0*2+1,
      				    "MPI_Group_size", colors[MPI_Group_size_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_translate_ranks_stateid_0*2,
	                            MPI_Group_translate_ranks_stateid_0*2+1,
      				    "MPI_Group_translate_ranks", colors[MPI_Group_translate_ranks_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Group_union_stateid_0*2,
	                            MPI_Group_union_stateid_0*2+1,
      				    "MPI_Group_union", colors[MPI_Group_union_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Intercomm_create_stateid_0*2,
	                            MPI_Intercomm_create_stateid_0*2+1,
      				    "MPI_Intercomm_create", colors[MPI_Intercomm_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Intercomm_merge_stateid_0*2,
	                            MPI_Intercomm_merge_stateid_0*2+1,
      				    "MPI_Intercomm_merge", colors[MPI_Intercomm_merge_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Keyval_create_stateid_0*2,
	                            MPI_Keyval_create_stateid_0*2+1,
      				    "MPI_Keyval_create", colors[MPI_Keyval_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Keyval_free_stateid_0*2,
	                            MPI_Keyval_free_stateid_0*2+1,
      				    "MPI_Keyval_free", colors[MPI_Keyval_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Abort_stateid_0*2,
	                            MPI_Abort_stateid_0*2+1,
      				    "MPI_Abort", colors[MPI_Abort_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Error_class_stateid_0*2,
	                            MPI_Error_class_stateid_0*2+1,
      				    "MPI_Error_class", colors[MPI_Error_class_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Errhandler_create_stateid_0*2,
	                            MPI_Errhandler_create_stateid_0*2+1,
      				    "MPI_Errhandler_create", colors[MPI_Errhandler_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Errhandler_free_stateid_0*2,
	                            MPI_Errhandler_free_stateid_0*2+1,
      				    "MPI_Errhandler_free", colors[MPI_Errhandler_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Errhandler_get_stateid_0*2,
	                            MPI_Errhandler_get_stateid_0*2+1,
      				    "MPI_Errhandler_get", colors[MPI_Errhandler_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Error_string_stateid_0*2,
	                            MPI_Error_string_stateid_0*2+1,
      				    "MPI_Error_string", colors[MPI_Error_string_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Errhandler_set_stateid_0*2,
	                            MPI_Errhandler_set_stateid_0*2+1,
      				    "MPI_Errhandler_set", colors[MPI_Errhandler_set_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Get_processor_name_stateid_0*2,
	                            MPI_Get_processor_name_stateid_0*2+1,
      				    "MPI_Get_processor_name", colors[MPI_Get_processor_name_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Initialized_stateid_0*2,
	                            MPI_Initialized_stateid_0*2+1,
      				    "MPI_Initialized", colors[MPI_Initialized_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Wtick_stateid_0*2,
	                            MPI_Wtick_stateid_0*2+1,
      				    "MPI_Wtick", colors[MPI_Wtick_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Wtime_stateid_0*2,
	                            MPI_Wtime_stateid_0*2+1,
      				    "MPI_Wtime", colors[MPI_Wtime_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Address_stateid_0*2,
	                            MPI_Address_stateid_0*2+1,
      				    "MPI_Address", colors[MPI_Address_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Bsend_stateid_0*2,
	                            MPI_Bsend_stateid_0*2+1,
      				    "MPI_Bsend", colors[MPI_Bsend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Bsend_init_stateid_0*2,
	                            MPI_Bsend_init_stateid_0*2+1,
      				    "MPI_Bsend_init", colors[MPI_Bsend_init_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Buffer_attach_stateid_0*2,
	                            MPI_Buffer_attach_stateid_0*2+1,
      				    "MPI_Buffer_attach", colors[MPI_Buffer_attach_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Buffer_detach_stateid_0*2,
	                            MPI_Buffer_detach_stateid_0*2+1,
      				    "MPI_Buffer_detach", colors[MPI_Buffer_detach_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cancel_stateid_0*2,
	                            MPI_Cancel_stateid_0*2+1,
      				    "MPI_Cancel", colors[MPI_Cancel_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Request_free_stateid_0*2,
	                            MPI_Request_free_stateid_0*2+1,
      				    "MPI_Request_free", colors[MPI_Request_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Recv_init_stateid_0*2,
	                            MPI_Recv_init_stateid_0*2+1,
      				    "MPI_Recv_init", colors[MPI_Recv_init_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Send_init_stateid_0*2,
	                            MPI_Send_init_stateid_0*2+1,
      				    "MPI_Send_init", colors[MPI_Send_init_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Get_elements_stateid_0*2,
	                            MPI_Get_elements_stateid_0*2+1,
      				    "MPI_Get_elements", colors[MPI_Get_elements_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Get_count_stateid_0*2,
	                            MPI_Get_count_stateid_0*2+1,
      				    "MPI_Get_count", colors[MPI_Get_count_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Ibsend_stateid_0*2,
	                            MPI_Ibsend_stateid_0*2+1,
      				    "MPI_Ibsend", colors[MPI_Ibsend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Iprobe_stateid_0*2,
	                            MPI_Iprobe_stateid_0*2+1,
      				    "MPI_Iprobe", colors[MPI_Iprobe_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Irecv_stateid_0*2,
	                            MPI_Irecv_stateid_0*2+1,
      				    "MPI_Irecv", colors[MPI_Irecv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Irsend_stateid_0*2,
	                            MPI_Irsend_stateid_0*2+1,
      				    "MPI_Irsend", colors[MPI_Irsend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Isend_stateid_0*2,
	                            MPI_Isend_stateid_0*2+1,
      				    "MPI_Isend", colors[MPI_Isend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Issend_stateid_0*2,
	                            MPI_Issend_stateid_0*2+1,
      				    "MPI_Issend", colors[MPI_Issend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Pack_stateid_0*2,
	                            MPI_Pack_stateid_0*2+1,
      				    "MPI_Pack", colors[MPI_Pack_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Pack_size_stateid_0*2,
	                            MPI_Pack_size_stateid_0*2+1,
      				    "MPI_Pack_size", colors[MPI_Pack_size_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Probe_stateid_0*2,
	                            MPI_Probe_stateid_0*2+1,
      				    "MPI_Probe", colors[MPI_Probe_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Recv_stateid_0*2,
	                            MPI_Recv_stateid_0*2+1,
      				    "MPI_Recv", colors[MPI_Recv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Rsend_stateid_0*2,
	                            MPI_Rsend_stateid_0*2+1,
      				    "MPI_Rsend", colors[MPI_Rsend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Rsend_init_stateid_0*2,
	                            MPI_Rsend_init_stateid_0*2+1,
      				    "MPI_Rsend_init", colors[MPI_Rsend_init_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Send_stateid_0*2,
	                            MPI_Send_stateid_0*2+1,
      				    "MPI_Send", colors[MPI_Send_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Sendrecv_stateid_0*2,
	                            MPI_Sendrecv_stateid_0*2+1,
      				    "MPI_Sendrecv", colors[MPI_Sendrecv_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Sendrecv_replace_stateid_0*2,
	                            MPI_Sendrecv_replace_stateid_0*2+1,
      				    "MPI_Sendrecv_replace", colors[MPI_Sendrecv_replace_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Ssend_stateid_0*2,
	                            MPI_Ssend_stateid_0*2+1,
      				    "MPI_Ssend", colors[MPI_Ssend_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Ssend_init_stateid_0*2,
	                            MPI_Ssend_init_stateid_0*2+1,
      				    "MPI_Ssend_init", colors[MPI_Ssend_init_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Start_stateid_0*2,
	                            MPI_Start_stateid_0*2+1,
      				    "MPI_Start", colors[MPI_Start_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Startall_stateid_0*2,
	                            MPI_Startall_stateid_0*2+1,
      				    "MPI_Startall", colors[MPI_Startall_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Test_stateid_0*2,
	                            MPI_Test_stateid_0*2+1,
      				    "MPI_Test", colors[MPI_Test_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Testall_stateid_0*2,
	                            MPI_Testall_stateid_0*2+1,
      				    "MPI_Testall", colors[MPI_Testall_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Testany_stateid_0*2,
	                            MPI_Testany_stateid_0*2+1,
      				    "MPI_Testany", colors[MPI_Testany_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Test_cancelled_stateid_0*2,
	                            MPI_Test_cancelled_stateid_0*2+1,
      				    "MPI_Test_cancelled", colors[MPI_Test_cancelled_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Testsome_stateid_0*2,
	                            MPI_Testsome_stateid_0*2+1,
      				    "MPI_Testsome", colors[MPI_Testsome_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_commit_stateid_0*2,
	                            MPI_Type_commit_stateid_0*2+1,
      				    "MPI_Type_commit", colors[MPI_Type_commit_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_contiguous_stateid_0*2,
	                            MPI_Type_contiguous_stateid_0*2+1,
      				    "MPI_Type_contiguous", colors[MPI_Type_contiguous_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_extent_stateid_0*2,
	                            MPI_Type_extent_stateid_0*2+1,
      				    "MPI_Type_extent", colors[MPI_Type_extent_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_free_stateid_0*2,
	                            MPI_Type_free_stateid_0*2+1,
      				    "MPI_Type_free", colors[MPI_Type_free_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_hindexed_stateid_0*2,
	                            MPI_Type_hindexed_stateid_0*2+1,
      				    "MPI_Type_hindexed", colors[MPI_Type_hindexed_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_hvector_stateid_0*2,
	                            MPI_Type_hvector_stateid_0*2+1,
      				    "MPI_Type_hvector", colors[MPI_Type_hvector_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_indexed_stateid_0*2,
	                            MPI_Type_indexed_stateid_0*2+1,
      				    "MPI_Type_indexed", colors[MPI_Type_indexed_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_lb_stateid_0*2,
	                            MPI_Type_lb_stateid_0*2+1,
      				    "MPI_Type_lb", colors[MPI_Type_lb_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_size_stateid_0*2,
	                            MPI_Type_size_stateid_0*2+1,
      				    "MPI_Type_size", colors[MPI_Type_size_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_struct_stateid_0*2,
	                            MPI_Type_struct_stateid_0*2+1,
      				    "MPI_Type_struct", colors[MPI_Type_struct_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_ub_stateid_0*2,
	                            MPI_Type_ub_stateid_0*2+1,
      				    "MPI_Type_ub", colors[MPI_Type_ub_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Type_vector_stateid_0*2,
	                            MPI_Type_vector_stateid_0*2+1,
      				    "MPI_Type_vector", colors[MPI_Type_vector_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Unpack_stateid_0*2,
	                            MPI_Unpack_stateid_0*2+1,
      				    "MPI_Unpack", colors[MPI_Unpack_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Wait_stateid_0*2,
	                            MPI_Wait_stateid_0*2+1,
      				    "MPI_Wait", colors[MPI_Wait_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Waitall_stateid_0*2,
	                            MPI_Waitall_stateid_0*2+1,
      				    "MPI_Waitall", colors[MPI_Waitall_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Waitany_stateid_0*2,
	                            MPI_Waitany_stateid_0*2+1,
      				    "MPI_Waitany", colors[MPI_Waitany_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Waitsome_stateid_0*2,
	                            MPI_Waitsome_stateid_0*2+1,
      				    "MPI_Waitsome", colors[MPI_Waitsome_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_coords_stateid_0*2,
	                            MPI_Cart_coords_stateid_0*2+1,
      				    "MPI_Cart_coords", colors[MPI_Cart_coords_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_create_stateid_0*2,
	                            MPI_Cart_create_stateid_0*2+1,
      				    "MPI_Cart_create", colors[MPI_Cart_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_get_stateid_0*2,
	                            MPI_Cart_get_stateid_0*2+1,
      				    "MPI_Cart_get", colors[MPI_Cart_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_map_stateid_0*2,
	                            MPI_Cart_map_stateid_0*2+1,
      				    "MPI_Cart_map", colors[MPI_Cart_map_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_rank_stateid_0*2,
	                            MPI_Cart_rank_stateid_0*2+1,
      				    "MPI_Cart_rank", colors[MPI_Cart_rank_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_shift_stateid_0*2,
	                            MPI_Cart_shift_stateid_0*2+1,
      				    "MPI_Cart_shift", colors[MPI_Cart_shift_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cart_sub_stateid_0*2,
	                            MPI_Cart_sub_stateid_0*2+1,
      				    "MPI_Cart_sub", colors[MPI_Cart_sub_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Cartdim_get_stateid_0*2,
	                            MPI_Cartdim_get_stateid_0*2+1,
      				    "MPI_Cartdim_get", colors[MPI_Cartdim_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Dims_create_stateid_0*2,
	                            MPI_Dims_create_stateid_0*2+1,
      				    "MPI_Dims_create", colors[MPI_Dims_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graph_create_stateid_0*2,
	                            MPI_Graph_create_stateid_0*2+1,
      				    "MPI_Graph_create", colors[MPI_Graph_create_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graph_get_stateid_0*2,
	                            MPI_Graph_get_stateid_0*2+1,
      				    "MPI_Graph_get", colors[MPI_Graph_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graph_map_stateid_0*2,
	                            MPI_Graph_map_stateid_0*2+1,
      				    "MPI_Graph_map", colors[MPI_Graph_map_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graph_neighbors_stateid_0*2,
	                            MPI_Graph_neighbors_stateid_0*2+1,
      				    "MPI_Graph_neighbors", colors[MPI_Graph_neighbors_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graph_neighbors_count_stateid_0*2,
	                            MPI_Graph_neighbors_count_stateid_0*2+1,
      				    "MPI_Graph_neighbors_count", colors[MPI_Graph_neighbors_count_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Graphdims_get_stateid_0*2,
	                            MPI_Graphdims_get_stateid_0*2+1,
      				    "MPI_Graphdims_get", colors[MPI_Graphdims_get_stateid_0%NUMCOLORS] );
    MPE_Describe_state( MPI_Topo_test_stateid_0*2,
	                            MPI_Topo_test_stateid_0*2+1,
      				    "MPI_Topo_test", colors[MPI_Topo_test_stateid_0%NUMCOLORS] );

  }
  MPE_Finish_log( logFileName_0 );
  if (procid_0 == 0)
    fprintf( stderr, "Finished writing logfile.\n");

  
  returnVal = PMPI_Finalize(  );


  return returnVal;
}

void LogMsgAvail(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_CtrlMgsAvail_id_0*2+end,0, (char *)0 );
}

void LogAnyCtrl(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_RecvAnyCtrl_id_0*2+end,0, (char *)0 );
}

void LogWaitSend(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_WaitSend_id_0*2+end,0, (char *)0 );
}

void LogTestSend(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_TestSend_id_0*2+end,0, (char *)0 );
}

void LogStartRequest(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_StartRequest_id_0*2+end,0, (char *)0 );
}

void LogSend(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_Send_id_0*2+end,0, (char *)0 );
}


void LogRecv(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_RecvFromChannel_id_0*2+end,0, (char *)0 );
}

void LogTestRequest(int end) {
	if(finishing) return;
	MPE_Log_event( WSOCK_TestRequest_id_0*2+end,0, (char *)0 );
}

void LogReadCtrl(int end) {
	if(finishing) return;
	MPE_Log_event( SHMEM_ReadCtrl_id_0*2+end,0, (char *)0 );
}

int  MPI_Init( argc, argv )
int * argc;
char *** argv;
{
  int  returnVal;
  int stateid;

  returnVal = PMPI_Init( argc, argv );

  MPE_Init_log();
  
  PMPI_Comm_rank( MPI_COMM_WORLD, &procid_0 );
  fprintf(stderr,"%d, Init Log done\n",procid_0);
  fflush(stderr);
  stateid=1;
  MPI_Allgather_stateid_0 = stateid++;
  MPI_Allgatherv_stateid_0 = stateid++;
  MPI_Allreduce_stateid_0 = stateid++;
  MPI_Alltoall_stateid_0 = stateid++;
  MPI_Alltoallv_stateid_0 = stateid++;
  MPI_Barrier_stateid_0 = stateid++;
  MPI_Bcast_stateid_0 = stateid++;
  MPI_Gather_stateid_0 = stateid++;
  MPI_Gatherv_stateid_0 = stateid++;
  MPI_Op_create_stateid_0 = stateid++;
  MPI_Op_free_stateid_0 = stateid++;
  MPI_Reduce_scatter_stateid_0 = stateid++;
  MPI_Reduce_stateid_0 = stateid++;
  MPI_Scan_stateid_0 = stateid++;
  MPI_Scatter_stateid_0 = stateid++;
  MPI_Scatterv_stateid_0 = stateid++;
  MPI_Attr_delete_stateid_0 = stateid++;
  MPI_Attr_get_stateid_0 = stateid++;
  MPI_Attr_put_stateid_0 = stateid++;
  MPI_Comm_compare_stateid_0 = stateid++;
  MPI_Comm_create_stateid_0 = stateid++;
  MPI_Comm_dup_stateid_0 = stateid++;
  MPI_Comm_free_stateid_0 = stateid++;
  MPI_Comm_group_stateid_0 = stateid++;
  MPI_Comm_rank_stateid_0 = stateid++;
  MPI_Comm_remote_group_stateid_0 = stateid++;
  MPI_Comm_remote_size_stateid_0 = stateid++;
  MPI_Comm_size_stateid_0 = stateid++;
  MPI_Comm_split_stateid_0 = stateid++;
  MPI_Comm_test_inter_stateid_0 = stateid++;
  MPI_Group_compare_stateid_0 = stateid++;
  MPI_Group_difference_stateid_0 = stateid++;
  MPI_Group_excl_stateid_0 = stateid++;
  MPI_Group_free_stateid_0 = stateid++;
  MPI_Group_incl_stateid_0 = stateid++;
  MPI_Group_intersection_stateid_0 = stateid++;
  MPI_Group_rank_stateid_0 = stateid++;
  MPI_Group_range_excl_stateid_0 = stateid++;
  MPI_Group_range_incl_stateid_0 = stateid++;
  MPI_Group_size_stateid_0 = stateid++;
  MPI_Group_translate_ranks_stateid_0 = stateid++;
  MPI_Group_union_stateid_0 = stateid++;
  MPI_Intercomm_create_stateid_0 = stateid++;
  MPI_Intercomm_merge_stateid_0 = stateid++;
  MPI_Keyval_create_stateid_0 = stateid++;
  MPI_Keyval_free_stateid_0 = stateid++;
  MPI_Abort_stateid_0 = stateid++;
  MPI_Error_class_stateid_0 = stateid++;
  MPI_Errhandler_create_stateid_0 = stateid++;
  MPI_Errhandler_free_stateid_0 = stateid++;
  MPI_Errhandler_get_stateid_0 = stateid++;
  MPI_Error_string_stateid_0 = stateid++;
  MPI_Errhandler_set_stateid_0 = stateid++;
  MPI_Get_processor_name_stateid_0 = stateid++;
  MPI_Initialized_stateid_0 = stateid++;
  MPI_Wtick_stateid_0 = stateid++;
  MPI_Wtime_stateid_0 = stateid++;
  MPI_Address_stateid_0 = stateid++;
  MPI_Bsend_stateid_0 = stateid++;
  MPI_Bsend_init_stateid_0 = stateid++;
  MPI_Buffer_attach_stateid_0 = stateid++;
  MPI_Buffer_detach_stateid_0 = stateid++;
  MPI_Cancel_stateid_0 = stateid++;
  MPI_Request_free_stateid_0 = stateid++;
  MPI_Recv_init_stateid_0 = stateid++;
  MPI_Send_init_stateid_0 = stateid++;
  MPI_Get_elements_stateid_0 = stateid++;
  MPI_Get_count_stateid_0 = stateid++;
  MPI_Ibsend_stateid_0 = stateid++;
  MPI_Iprobe_stateid_0 = stateid++;
  MPI_Irecv_stateid_0 = stateid++;
  MPI_Irsend_stateid_0 = stateid++;
  MPI_Isend_stateid_0 = stateid++;
  MPI_Issend_stateid_0 = stateid++;
  MPI_Pack_stateid_0 = stateid++;
  MPI_Pack_size_stateid_0 = stateid++;
  MPI_Probe_stateid_0 = stateid++;
  MPI_Recv_stateid_0 = stateid++;
  MPI_Rsend_stateid_0 = stateid++;
  MPI_Rsend_init_stateid_0 = stateid++;
  MPI_Send_stateid_0 = stateid++;
  MPI_Sendrecv_stateid_0 = stateid++;
  MPI_Sendrecv_replace_stateid_0 = stateid++;
  MPI_Ssend_stateid_0 = stateid++;
  MPI_Ssend_init_stateid_0 = stateid++;
  MPI_Start_stateid_0 = stateid++;
  MPI_Startall_stateid_0 = stateid++;
  MPI_Test_stateid_0 = stateid++;
  MPI_Testall_stateid_0 = stateid++;
  MPI_Testany_stateid_0 = stateid++;
  MPI_Test_cancelled_stateid_0 = stateid++;
  MPI_Testsome_stateid_0 = stateid++;
  MPI_Type_commit_stateid_0 = stateid++;
  MPI_Type_contiguous_stateid_0 = stateid++;
  MPI_Type_extent_stateid_0 = stateid++;
  MPI_Type_free_stateid_0 = stateid++;
  MPI_Type_hindexed_stateid_0 = stateid++;
  MPI_Type_hvector_stateid_0 = stateid++;
  MPI_Type_indexed_stateid_0 = stateid++;
  MPI_Type_lb_stateid_0 = stateid++;
  MPI_Type_size_stateid_0 = stateid++;
  MPI_Type_struct_stateid_0 = stateid++;
  MPI_Type_ub_stateid_0 = stateid++;
  MPI_Type_vector_stateid_0 = stateid++;
  MPI_Unpack_stateid_0 = stateid++;
  MPI_Wait_stateid_0 = stateid++;
  MPI_Waitall_stateid_0 = stateid++;
  MPI_Waitany_stateid_0 = stateid++;
  MPI_Waitsome_stateid_0 = stateid++;
  MPI_Cart_coords_stateid_0 = stateid++;
  MPI_Cart_create_stateid_0 = stateid++;
  MPI_Cart_get_stateid_0 = stateid++;
  MPI_Cart_map_stateid_0 = stateid++;
  MPI_Cart_rank_stateid_0 = stateid++;
  MPI_Cart_shift_stateid_0 = stateid++;
  MPI_Cart_sub_stateid_0 = stateid++;
  MPI_Cartdim_get_stateid_0 = stateid++;
  MPI_Dims_create_stateid_0 = stateid++;
  MPI_Graph_create_stateid_0 = stateid++;
  MPI_Graph_get_stateid_0 = stateid++;
  MPI_Graph_map_stateid_0 = stateid++;
  MPI_Graph_neighbors_stateid_0 = stateid++;
  MPI_Graph_neighbors_count_stateid_0 = stateid++;
  MPI_Graphdims_get_stateid_0 = stateid++;
  MPI_Topo_test_stateid_0 = stateid++;
  
  WSOCK_RecvFromChannel_id_0 = stateid++;
  WSOCK_CtrlMgsAvail_id_0 = stateid++;
  WSOCK_RecvAnyCtrl_id_0= stateid++;
  WSOCK_TestRequest_id_0= stateid++;
  WSOCK_WaitSend_id_0= stateid++;
  WSOCK_TestSend_id_0= stateid++;
  WSOCK_StartRequest_id_0= stateid++;
  WSOCK_Send_id_0 = stateid++;
  SHMEM_ReadCtrl_id_0= stateid++;
  WSOCK_RecvFromChannel_id_0 = stateid++;
  
	sprintf( logFileName_0, "%s_profile", (*argv)[0] );

  MPE_Start_log();

  return returnVal;
}

int  MPI_Initialized( flag )
int * flag;
{
  int  returnVal;

/*
    MPI_Initialized - prototyping replacement for MPI_Initialized
    Log the beginning and ending of the time spent in MPI_Initialized calls.
*/

  ++MPI_Initialized_ncalls_0;
  MPE_Log_event( MPI_Initialized_stateid_0*2,
	         MPI_Initialized_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Initialized( flag );

  MPE_Log_event( MPI_Initialized_stateid_0*2+1,
	         MPI_Initialized_ncalls_0, (char *)0 );


  return returnVal;
}

double  MPI_Wtick(  )
{
  double  returnVal;

/*
    MPI_Wtick - prototyping replacement for MPI_Wtick
    Log the beginning and ending of the time spent in MPI_Wtick calls.
*/

  ++MPI_Wtick_ncalls_0;
  MPE_Log_event( MPI_Wtick_stateid_0*2,
	         MPI_Wtick_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Wtick(  );

  MPE_Log_event( MPI_Wtick_stateid_0*2+1,
	         MPI_Wtick_ncalls_0, (char *)0 );


  return returnVal;
}

double  MPI_Wtime(  )
{
  double  returnVal;

/*
    MPI_Wtime - prototyping replacement for MPI_Wtime
    Log the beginning and ending of the time spent in MPI_Wtime calls.
*/

  ++MPI_Wtime_ncalls_0;
  MPE_Log_event( MPI_Wtime_stateid_0*2,
	         MPI_Wtime_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Wtime(  );

  MPE_Log_event( MPI_Wtime_stateid_0*2+1,
	         MPI_Wtime_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Address( location, address )
void * location;
MPI_Aint * address;
{
  int  returnVal;

/*
    MPI_Address - prototyping replacement for MPI_Address
    Log the beginning and ending of the time spent in MPI_Address calls.
*/

  ++MPI_Address_ncalls_0;
  MPE_Log_event( MPI_Address_stateid_0*2,
	         MPI_Address_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Address( location, address );

  MPE_Log_event( MPI_Address_stateid_0*2+1,
	         MPI_Address_ncalls_0, (char *)0 );


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

/*
    MPI_Bsend - prototyping replacement for MPI_Bsend
    Log the beginning and ending of the time spent in MPI_Bsend calls.
*/

  ++MPI_Bsend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);
  MPE_Log_event( MPI_Bsend_stateid_0*2,
	         MPI_Bsend_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Bsend( buf, count, datatype, dest, tag, comm );

  MPE_Log_event( MPI_Bsend_stateid_0*2+1,
	         MPI_Bsend_ncalls_0, (char *)0 );


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

/*
    MPI_Bsend_init - prototyping replacement for MPI_Bsend_init
    Log the beginning and ending of the time spent in MPI_Bsend_init calls.
*/

  ++MPI_Bsend_init_ncalls_0;
  MPE_Log_event( MPI_Bsend_init_stateid_0*2,
	         MPI_Bsend_init_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Bsend_init( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Bsend_init_stateid_0*2+1,
	         MPI_Bsend_init_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Buffer_attach( buffer, size )
void * buffer;
int size;
{
  int  returnVal;

/*
    MPI_Buffer_attach - prototyping replacement for MPI_Buffer_attach
    Log the beginning and ending of the time spent in MPI_Buffer_attach calls.
*/

  ++MPI_Buffer_attach_ncalls_0;
  MPE_Log_event( MPI_Buffer_attach_stateid_0*2,
	         MPI_Buffer_attach_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Buffer_attach( buffer, size );

  MPE_Log_event( MPI_Buffer_attach_stateid_0*2+1,
	         MPI_Buffer_attach_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Buffer_detach( buffer, size )
void * buffer;
int * size;
{
  int  returnVal;

/*
    MPI_Buffer_detach - prototyping replacement for MPI_Buffer_detach
    Log the beginning and ending of the time spent in MPI_Buffer_detach calls.
*/

  ++MPI_Buffer_detach_ncalls_0;
  MPE_Log_event( MPI_Buffer_detach_stateid_0*2,
	         MPI_Buffer_detach_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Buffer_detach( buffer, size );

  MPE_Log_event( MPI_Buffer_detach_stateid_0*2+1,
	         MPI_Buffer_detach_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Cancel( request )
MPI_Request * request;
{
  int  returnVal;

/*
    MPI_Cancel - prototyping replacement for MPI_Cancel
    Log the beginning and ending of the time spent in MPI_Cancel calls.
*/

  ++MPI_Cancel_ncalls_0;
  MPE_Log_event( MPI_Cancel_stateid_0*2,
	         MPI_Cancel_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cancel( request );

  MPE_Log_event( MPI_Cancel_stateid_0*2+1,
	         MPI_Cancel_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Request_free( request )
MPI_Request * request;
{
  int  returnVal;

/*
    MPI_Request_free - prototyping replacement for MPI_Request_free
    Log the beginning and ending of the time spent in MPI_Request_free calls.
*/

  ++MPI_Request_free_ncalls_0;
  MPE_Log_event( MPI_Request_free_stateid_0*2,
	         MPI_Request_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Request_free( request );

  MPE_Log_event( MPI_Request_free_stateid_0*2+1,
	         MPI_Request_free_ncalls_0, (char *)0 );


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

/*
    MPI_Recv_init - prototyping replacement for MPI_Recv_init
    Log the beginning and ending of the time spent in MPI_Recv_init calls.
*/

  ++MPI_Recv_init_ncalls_0;
  MPE_Log_event( MPI_Recv_init_stateid_0*2,
	         MPI_Recv_init_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Recv_init( buf, count, datatype, source, tag, comm, request );

  MPE_Log_event( MPI_Recv_init_stateid_0*2+1,
	         MPI_Recv_init_ncalls_0, (char *)0 );


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

/*
    MPI_Send_init - prototyping replacement for MPI_Send_init
    Log the beginning and ending of the time spent in MPI_Send_init calls.
*/

  ++MPI_Send_init_ncalls_0;
  MPE_Log_event( MPI_Send_init_stateid_0*2,
	         MPI_Send_init_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Send_init( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Send_init_stateid_0*2+1,
	         MPI_Send_init_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Get_elements( status, datatype, elements )
MPI_Status * status;
MPI_Datatype datatype;
int * elements;
{
  int   returnVal;

/*
    MPI_Get_elements - prototyping replacement for MPI_Get_elements
    Log the beginning and ending of the time spent in MPI_Get_elements calls.
*/

  ++MPI_Get_elements_ncalls_0;
  MPE_Log_event( MPI_Get_elements_stateid_0*2,
	         MPI_Get_elements_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Get_elements( status, datatype, elements );

  MPE_Log_event( MPI_Get_elements_stateid_0*2+1,
	         MPI_Get_elements_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Get_count( status, datatype, count )
MPI_Status * status;
MPI_Datatype datatype;
int * count;
{
  int  returnVal;

/*
    MPI_Get_count - prototyping replacement for MPI_Get_count
    Log the beginning and ending of the time spent in MPI_Get_count calls.
*/

  ++MPI_Get_count_ncalls_0;
  MPE_Log_event( MPI_Get_count_stateid_0*2,
	         MPI_Get_count_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Get_count( status, datatype, count );

  MPE_Log_event( MPI_Get_count_stateid_0*2+1,
	         MPI_Get_count_ncalls_0, (char *)0 );


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

/*
    MPI_Ibsend - prototyping replacement for MPI_Ibsend
    Log the beginning and ending of the time spent in MPI_Ibsend calls.
*/

  ++MPI_Ibsend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);
  MPE_Log_event( MPI_Ibsend_stateid_0*2,
	         MPI_Ibsend_ncalls_0, (char *)0 );
 
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Ibsend( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Ibsend_stateid_0*2+1,
	         MPI_Ibsend_ncalls_0, (char *)0 );


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

/*
    MPI_Iprobe - prototyping replacement for MPI_Iprobe
    Log the beginning and ending of the time spent in MPI_Iprobe calls.
*/

  ++MPI_Iprobe_ncalls_0;
  MPE_Log_event( MPI_Iprobe_stateid_0*2,
	         MPI_Iprobe_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Iprobe( source, tag, comm, flag, status );

  MPE_Log_event( MPI_Iprobe_stateid_0*2+1,
	         MPI_Iprobe_ncalls_0, (char *)0 );


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

/*
    MPI_Irecv - prototyping replacement for MPI_Irecv
    Log the beginning and ending of the time spent in MPI_Irecv calls.
*/

  ++MPI_Irecv_ncalls_0;
  
  MPE_Log_event( MPI_Irecv_stateid_0*2,
	         MPI_Irecv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Irecv( buf, count, datatype, source, tag, comm, request );

  MPE_Log_event( MPI_Irecv_stateid_0*2+1,
	         MPI_Irecv_ncalls_0, (char *)0 );


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

/*
    MPI_Irsend - prototyping replacement for MPI_Irsend
    Log the beginning and ending of the time spent in MPI_Irsend calls.
*/

  ++MPI_Irsend_ncalls_0;

  PMPI_Type_size(datatype,&returnVal);
  MPE_Log_event( MPI_Irsend_stateid_0*2,
	         MPI_Irsend_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Irsend( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Irsend_stateid_0*2+1,
	         MPI_Irsend_ncalls_0, (char *)0 );


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

/*
    MPI_Isend - prototyping replacement for MPI_Isend
    Log the beginning and ending of the time spent in MPI_Isend calls.
*/

  ++MPI_Isend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);

  MPE_Log_event( MPI_Isend_stateid_0*2,
	         MPI_Isend_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  
  returnVal = PMPI_Isend( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Isend_stateid_0*2+1,
	         MPI_Isend_ncalls_0, (char *)0 );


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

/*
    MPI_Issend - prototyping replacement for MPI_Issend
    Log the beginning and ending of the time spent in MPI_Issend calls.
*/

  ++MPI_Issend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);
  
  MPE_Log_event( MPI_Issend_stateid_0*2,
	         MPI_Issend_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Issend( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Issend_stateid_0*2+1,
	         MPI_Issend_ncalls_0, (char *)0 );


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

/*
    MPI_Pack - prototyping replacement for MPI_Pack
    Log the beginning and ending of the time spent in MPI_Pack calls.
*/

  ++MPI_Pack_ncalls_0;
  MPE_Log_event( MPI_Pack_stateid_0*2,
	         MPI_Pack_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Pack( inbuf, incount, type, outbuf, outcount, position, comm );

  MPE_Log_event( MPI_Pack_stateid_0*2+1,
	         MPI_Pack_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Pack_size( incount, datatype, comm, size )
int incount;
MPI_Datatype datatype;
MPI_Comm comm;
int * size;
{
  int   returnVal;

/*
    MPI_Pack_size - prototyping replacement for MPI_Pack_size
    Log the beginning and ending of the time spent in MPI_Pack_size calls.
*/

  ++MPI_Pack_size_ncalls_0;
  MPE_Log_event( MPI_Pack_size_stateid_0*2,
	         MPI_Pack_size_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Pack_size( incount, datatype, comm, size );

  MPE_Log_event( MPI_Pack_size_stateid_0*2+1,
	         MPI_Pack_size_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Probe( source, tag, comm, status )
int source;
int tag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;

/*
    MPI_Probe - prototyping replacement for MPI_Probe
    Log the beginning and ending of the time spent in MPI_Probe calls.
*/

  ++MPI_Probe_ncalls_0;
  MPE_Log_event( MPI_Probe_stateid_0*2,
	         MPI_Probe_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Probe( source, tag, comm, status );

  MPE_Log_event( MPI_Probe_stateid_0*2+1,
	         MPI_Probe_ncalls_0, (char *)0 );


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
  int  returnVal,size;

/*
    MPI_Recv - prototyping replacement for MPI_Recv
    Log the beginning and ending of the time spent in MPI_Recv calls.
*/

  ++MPI_Recv_ncalls_0;
  PMPI_Type_size(datatype,&size);
  
  MPE_Log_event( MPI_Recv_stateid_0*2,
	         MPI_Recv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Recv( buf, count, datatype, source, tag, comm, status );

  MPE_Log_receive(source,tag,size*count);
  MPE_Log_event( MPI_Recv_stateid_0*2+1,
	         MPI_Recv_ncalls_0, (char *)0 );


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

/*
    MPI_Rsend - prototyping replacement for MPI_Rsend
    Log the beginning and ending of the time spent in MPI_Rsend calls.
*/

  ++MPI_Rsend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);
  MPE_Log_event( MPI_Rsend_stateid_0*2,
	         MPI_Rsend_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Rsend( buf, count, datatype, dest, tag, comm );

  MPE_Log_event( MPI_Rsend_stateid_0*2+1,
	         MPI_Rsend_ncalls_0, (char *)0 );


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

/*
    MPI_Rsend_init - prototyping replacement for MPI_Rsend_init
    Log the beginning and ending of the time spent in MPI_Rsend_init calls.
*/

  ++MPI_Rsend_init_ncalls_0;
  MPE_Log_event( MPI_Rsend_init_stateid_0*2,
	         MPI_Rsend_init_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Rsend_init( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Rsend_init_stateid_0*2+1,
	         MPI_Rsend_init_ncalls_0, (char *)0 );


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

/*
    MPI_Send - prototyping replacement for MPI_Send
    Log the beginning and ending of the time spent in MPI_Send calls.
*/

  ++MPI_Send_ncalls_0;

  PMPI_Type_size(datatype,&returnVal);
  MPE_Log_event( MPI_Send_stateid_0*2,
	         MPI_Send_ncalls_0, (char *)0 );
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Send( buf, count, datatype, dest, tag, comm );

  MPE_Log_event( MPI_Send_stateid_0*2+1,
	         MPI_Send_ncalls_0, (char *)0 );


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

/*
    MPI_Sendrecv - prototyping replacement for MPI_Sendrecv
    Log the beginning and ending of the time spent in MPI_Sendrecv calls.
*/

  ++MPI_Sendrecv_ncalls_0;
  MPE_Log_event( MPI_Sendrecv_stateid_0*2,
	         MPI_Sendrecv_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Sendrecv( sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status );

  MPE_Log_event( MPI_Sendrecv_stateid_0*2+1,
	         MPI_Sendrecv_ncalls_0, (char *)0 );


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

/*
    MPI_Sendrecv_replace - prototyping replacement for MPI_Sendrecv_replace
    Log the beginning and ending of the time spent in MPI_Sendrecv_replace calls.
*/

  ++MPI_Sendrecv_replace_ncalls_0;
  MPE_Log_event( MPI_Sendrecv_replace_stateid_0*2,
	         MPI_Sendrecv_replace_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, source, recvtag, comm, status );

  MPE_Log_event( MPI_Sendrecv_replace_stateid_0*2+1,
	         MPI_Sendrecv_replace_ncalls_0, (char *)0 );


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

/*
    MPI_Ssend - prototyping replacement for MPI_Ssend
    Log the beginning and ending of the time spent in MPI_Ssend calls.
*/

  ++MPI_Ssend_ncalls_0;
  PMPI_Type_size(datatype,&returnVal);
  

  MPE_Log_event( MPI_Ssend_stateid_0*2,
	         MPI_Ssend_ncalls_0, (char *)0 );
  
  
  MPE_Log_send(dest,tag,returnVal*count);
  returnVal = PMPI_Ssend( buf, count, datatype, dest, tag, comm );

  MPE_Log_event( MPI_Ssend_stateid_0*2+1,
	         MPI_Ssend_ncalls_0, (char *)0 );


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

/*
    MPI_Ssend_init - prototyping replacement for MPI_Ssend_init
    Log the beginning and ending of the time spent in MPI_Ssend_init calls.
*/

  ++MPI_Ssend_init_ncalls_0;
  MPE_Log_event( MPI_Ssend_init_stateid_0*2,
	         MPI_Ssend_init_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Ssend_init( buf, count, datatype, dest, tag, comm, request );

  MPE_Log_event( MPI_Ssend_init_stateid_0*2+1,
	         MPI_Ssend_init_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Start( request )
MPI_Request * request;
{
  int  returnVal;

/*
    MPI_Start - prototyping replacement for MPI_Start
    Log the beginning and ending of the time spent in MPI_Start calls.
*/

  ++MPI_Start_ncalls_0;
  MPE_Log_event( MPI_Start_stateid_0*2,
	         MPI_Start_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Start( request );

  MPE_Log_event( MPI_Start_stateid_0*2+1,
	         MPI_Start_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Startall( count, array_of_requests )
int count;
MPI_Request * array_of_requests;
{
  int  returnVal;

/*
    MPI_Startall - prototyping replacement for MPI_Startall
    Log the beginning and ending of the time spent in MPI_Startall calls.
*/

  ++MPI_Startall_ncalls_0;
  MPE_Log_event( MPI_Startall_stateid_0*2,
	         MPI_Startall_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Startall( count, array_of_requests );

  MPE_Log_event( MPI_Startall_stateid_0*2+1,
	         MPI_Startall_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Test( request, flag, status )
MPI_Request * request;
int * flag;
MPI_Status * status;
{
  int   returnVal;

/*
    MPI_Test - prototyping replacement for MPI_Test
    Log the beginning and ending of the time spent in MPI_Test calls.
*/

  ++MPI_Test_ncalls_0;
  MPE_Log_event( MPI_Test_stateid_0*2,
	         MPI_Test_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Test( request, flag, status );

  MPE_Log_event( MPI_Test_stateid_0*2+1,
	         MPI_Test_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Testall( count, array_of_requests, flag, array_of_statuses )
int count;
MPI_Request * array_of_requests;
int * flag;
MPI_Status * array_of_statuses;
{
  int  returnVal;

/*
    MPI_Testall - prototyping replacement for MPI_Testall
    Log the beginning and ending of the time spent in MPI_Testall calls.
*/

  ++MPI_Testall_ncalls_0;
  MPE_Log_event( MPI_Testall_stateid_0*2,
	         MPI_Testall_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Testall( count, array_of_requests, flag, array_of_statuses );

  MPE_Log_event( MPI_Testall_stateid_0*2+1,
	         MPI_Testall_ncalls_0, (char *)0 );


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

/*
    MPI_Testany - prototyping replacement for MPI_Testany
    Log the beginning and ending of the time spent in MPI_Testany calls.
*/

  ++MPI_Testany_ncalls_0;
  MPE_Log_event( MPI_Testany_stateid_0*2,
	         MPI_Testany_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Testany( count, array_of_requests, index, flag, status );

  MPE_Log_event( MPI_Testany_stateid_0*2+1,
	         MPI_Testany_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Test_cancelled( status, flag )
MPI_Status * status;
int * flag;
{
  int  returnVal;

/*
    MPI_Test_cancelled - prototyping replacement for MPI_Test_cancelled
    Log the beginning and ending of the time spent in MPI_Test_cancelled calls.
*/

  ++MPI_Test_cancelled_ncalls_0;
  MPE_Log_event( MPI_Test_cancelled_stateid_0*2,
	         MPI_Test_cancelled_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Test_cancelled( status, flag );

  MPE_Log_event( MPI_Test_cancelled_stateid_0*2+1,
	         MPI_Test_cancelled_ncalls_0, (char *)0 );


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

/*
    MPI_Testsome - prototyping replacement for MPI_Testsome
    Log the beginning and ending of the time spent in MPI_Testsome calls.
*/

  ++MPI_Testsome_ncalls_0;
  MPE_Log_event( MPI_Testsome_stateid_0*2,
	         MPI_Testsome_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Testsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  MPE_Log_event( MPI_Testsome_stateid_0*2+1,
	         MPI_Testsome_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Type_commit( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;

/*
    MPI_Type_commit - prototyping replacement for MPI_Type_commit
    Log the beginning and ending of the time spent in MPI_Type_commit calls.
*/

  ++MPI_Type_commit_ncalls_0;
  MPE_Log_event( MPI_Type_commit_stateid_0*2,
	         MPI_Type_commit_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_commit( datatype );

  MPE_Log_event( MPI_Type_commit_stateid_0*2+1,
	         MPI_Type_commit_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Type_contiguous( count, old_type, newtype )
int count;
MPI_Datatype old_type;
MPI_Datatype * newtype;
{
  int  returnVal;

/*
    MPI_Type_contiguous - prototyping replacement for MPI_Type_contiguous
    Log the beginning and ending of the time spent in MPI_Type_contiguous calls.
*/

  ++MPI_Type_contiguous_ncalls_0;
  MPE_Log_event( MPI_Type_contiguous_stateid_0*2,
	         MPI_Type_contiguous_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_contiguous( count, old_type, newtype );

  MPE_Log_event( MPI_Type_contiguous_stateid_0*2+1,
	         MPI_Type_contiguous_ncalls_0, (char *)0 );


  return returnVal;
}


int  MPI_Type_extent( datatype, extent )
MPI_Datatype datatype;
MPI_Aint * extent;
{
  int  returnVal;

/*
    MPI_Type_extent - prototyping replacement for MPI_Type_extent
    Log the beginning and ending of the time spent in MPI_Type_extent calls.
*/

  ++MPI_Type_extent_ncalls_0;
  MPE_Log_event( MPI_Type_extent_stateid_0*2,
	         MPI_Type_extent_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_extent( datatype, extent );

  MPE_Log_event( MPI_Type_extent_stateid_0*2+1,
	         MPI_Type_extent_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Type_free( datatype )
MPI_Datatype * datatype;
{
  int   returnVal;

/*
    MPI_Type_free - prototyping replacement for MPI_Type_free
    Log the beginning and ending of the time spent in MPI_Type_free calls.
*/

  ++MPI_Type_free_ncalls_0;
  MPE_Log_event( MPI_Type_free_stateid_0*2,
	         MPI_Type_free_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_free( datatype );

  MPE_Log_event( MPI_Type_free_stateid_0*2+1,
	         MPI_Type_free_ncalls_0, (char *)0 );


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

/*
    MPI_Type_hindexed - prototyping replacement for MPI_Type_hindexed
    Log the beginning and ending of the time spent in MPI_Type_hindexed calls.
*/

  ++MPI_Type_hindexed_ncalls_0;
  MPE_Log_event( MPI_Type_hindexed_stateid_0*2,
	         MPI_Type_hindexed_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_hindexed( count, blocklens, indices, old_type, newtype );

  MPE_Log_event( MPI_Type_hindexed_stateid_0*2+1,
	         MPI_Type_hindexed_ncalls_0, (char *)0 );


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

/*
    MPI_Type_hvector - prototyping replacement for MPI_Type_hvector
    Log the beginning and ending of the time spent in MPI_Type_hvector calls.
*/

  ++MPI_Type_hvector_ncalls_0;
  MPE_Log_event( MPI_Type_hvector_stateid_0*2,
	         MPI_Type_hvector_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_hvector( count, blocklen, stride, old_type, newtype );

  MPE_Log_event( MPI_Type_hvector_stateid_0*2+1,
	         MPI_Type_hvector_ncalls_0, (char *)0 );


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

/*
    MPI_Type_indexed - prototyping replacement for MPI_Type_indexed
    Log the beginning and ending of the time spent in MPI_Type_indexed calls.
*/

  ++MPI_Type_indexed_ncalls_0;
  MPE_Log_event( MPI_Type_indexed_stateid_0*2,
	         MPI_Type_indexed_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_indexed( count, blocklens, indices, old_type, newtype );

  MPE_Log_event( MPI_Type_indexed_stateid_0*2+1,
	         MPI_Type_indexed_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Type_lb( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;

/*
    MPI_Type_lb - prototyping replacement for MPI_Type_lb
    Log the beginning and ending of the time spent in MPI_Type_lb calls.
*/

  ++MPI_Type_lb_ncalls_0;
  MPE_Log_event( MPI_Type_lb_stateid_0*2,
	         MPI_Type_lb_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_lb( datatype, displacement );

  MPE_Log_event( MPI_Type_lb_stateid_0*2+1,
	         MPI_Type_lb_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Type_size( datatype, size )
MPI_Datatype datatype;
int * size;
{
  int   returnVal;

/*
    MPI_Type_size - prototyping replacement for MPI_Type_size
    Log the beginning and ending of the time spent in MPI_Type_size calls.
*/

  ++MPI_Type_size_ncalls_0;
  MPE_Log_event( MPI_Type_size_stateid_0*2,
	         MPI_Type_size_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_size( datatype, size );

  MPE_Log_event( MPI_Type_size_stateid_0*2+1,
	         MPI_Type_size_ncalls_0, (char *)0 );


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

/*
    MPI_Type_struct - prototyping replacement for MPI_Type_struct
    Log the beginning and ending of the time spent in MPI_Type_struct calls.
*/

  ++MPI_Type_struct_ncalls_0;
  MPE_Log_event( MPI_Type_struct_stateid_0*2,
	         MPI_Type_struct_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_struct( count, blocklens, indices, old_types, newtype );

  MPE_Log_event( MPI_Type_struct_stateid_0*2+1,
	         MPI_Type_struct_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Type_ub( datatype, displacement )
MPI_Datatype datatype;
MPI_Aint * displacement;
{
  int   returnVal;

/*
    MPI_Type_ub - prototyping replacement for MPI_Type_ub
    Log the beginning and ending of the time spent in MPI_Type_ub calls.
*/

  ++MPI_Type_ub_ncalls_0;
  MPE_Log_event( MPI_Type_ub_stateid_0*2,
	         MPI_Type_ub_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_ub( datatype, displacement );

  MPE_Log_event( MPI_Type_ub_stateid_0*2+1,
	         MPI_Type_ub_ncalls_0, (char *)0 );


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

/*
    MPI_Type_vector - prototyping replacement for MPI_Type_vector
    Log the beginning and ending of the time spent in MPI_Type_vector calls.
*/

  ++MPI_Type_vector_ncalls_0;
  MPE_Log_event( MPI_Type_vector_stateid_0*2,
	         MPI_Type_vector_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Type_vector( count, blocklen, stride, old_type, newtype );

  MPE_Log_event( MPI_Type_vector_stateid_0*2+1,
	         MPI_Type_vector_ncalls_0, (char *)0 );


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

/*
    MPI_Unpack - prototyping replacement for MPI_Unpack
    Log the beginning and ending of the time spent in MPI_Unpack calls.
*/

  ++MPI_Unpack_ncalls_0;
  MPE_Log_event( MPI_Unpack_stateid_0*2,
	         MPI_Unpack_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Unpack( inbuf, insize, position, outbuf, outcount, type, comm );

  MPE_Log_event( MPI_Unpack_stateid_0*2+1,
	         MPI_Unpack_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Wait( request, status )
MPI_Request * request;
MPI_Status * status;
{
  int   returnVal;
  MPIR_OPTYPE type;

/*
    MPI_Wait - prototyping replacement for MPI_Wait
    Log the beginning and ending of the time spent in MPI_Wait calls.
*/

  ++MPI_Wait_ncalls_0;
  type = (*request)->handle_type;
  MPE_Log_event( MPI_Wait_stateid_0*2,
	         MPI_Wait_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Wait( request, status );
  if(type == MPIR_RECV || type == MPIR_PERSISTENT_RECV)
	  MPE_Log_receive(status->MPI_SOURCE,status->MPI_TAG,status->count);

  MPE_Log_event( MPI_Wait_stateid_0*2+1,
	         MPI_Wait_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Waitall( count, array_of_requests, array_of_statuses )
int count;
MPI_Request * array_of_requests;
MPI_Status * array_of_statuses;
{
  int  returnVal;
  int i;
  MPIR_OPTYPE *type;

/*
    MPI_Waitall - prototyping replacement for MPI_Waitall
    Log the beginning and ending of the time spent in MPI_Waitall calls.
*/

  ++MPI_Waitall_ncalls_0;
  type = (MPIR_OPTYPE *)__builtin_alloca(sizeof(MPIR_OPTYPE)*count);
  for(i=0;i<count;i++) {
	  type[i]=array_of_requests[i]->handle_type;
  }
  MPE_Log_event( MPI_Waitall_stateid_0*2,
	         MPI_Waitall_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Waitall( count, array_of_requests, array_of_statuses );
  for(i=0;i<count;i++) {
	if(type[i] == MPIR_RECV || type[i] == MPIR_PERSISTENT_RECV)
	  MPE_Log_receive(array_of_statuses[i].MPI_SOURCE,array_of_statuses[i].MPI_TAG,array_of_statuses[i].count);
  }

  MPE_Log_event( MPI_Waitall_stateid_0*2+1,
	         MPI_Waitall_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Waitany( count, array_of_requests, index, status )
int count;
MPI_Request * array_of_requests;
int * index;
MPI_Status * status;
{
  int  returnVal,i;

  MPIR_OPTYPE *type;
/*
    MPI_Waitany - prototyping replacement for MPI_Waitany
    Log the beginning and ending of the time spent in MPI_Waitany calls.
*/

  ++MPI_Waitany_ncalls_0;

  type = (MPIR_OPTYPE *)__builtin_alloca(sizeof(MPIR_OPTYPE)*count);
    for(i=0;i<count;i++) {
	  type[i]=array_of_requests[i]->handle_type;
	}

  MPE_Log_event( MPI_Waitany_stateid_0*2,
	         MPI_Waitany_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Waitany( count, array_of_requests, index, status );

  if(type[*index] == MPIR_RECV ||type[*index] == MPIR_PERSISTENT_RECV)
	  MPE_Log_receive(status->MPI_SOURCE,status->MPI_TAG,status->count);
  MPE_Log_event( MPI_Waitany_stateid_0*2+1,
	         MPI_Waitany_ncalls_0, (char *)0 );


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

/*
    MPI_Waitsome - prototyping replacement for MPI_Waitsome
    Log the beginning and ending of the time spent in MPI_Waitsome calls.
*/

  ++MPI_Waitsome_ncalls_0;
  MPE_Log_event( MPI_Waitsome_stateid_0*2,
	         MPI_Waitsome_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Waitsome( incount, array_of_requests, outcount, array_of_indices, array_of_statuses );

  MPE_Log_event( MPI_Waitsome_stateid_0*2+1,
	         MPI_Waitsome_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Cart_coords( comm, rank, maxdims, coords )
MPI_Comm comm;
int rank;
int maxdims;
int * coords;
{
  int   returnVal;

/*
    MPI_Cart_coords - prototyping replacement for MPI_Cart_coords
    Log the beginning and ending of the time spent in MPI_Cart_coords calls.
*/

  ++MPI_Cart_coords_ncalls_0;
  MPE_Log_event( MPI_Cart_coords_stateid_0*2,
	         MPI_Cart_coords_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_coords( comm, rank, maxdims, coords );

  MPE_Log_event( MPI_Cart_coords_stateid_0*2+1,
	         MPI_Cart_coords_ncalls_0, (char *)0 );


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

/*
    MPI_Cart_create - prototyping replacement for MPI_Cart_create
    Log the beginning and ending of the time spent in MPI_Cart_create calls.
*/

  ++MPI_Cart_create_ncalls_0;
  MPE_Log_event( MPI_Cart_create_stateid_0*2,
	         MPI_Cart_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_create( comm_old, ndims, dims, periods, reorder, comm_cart );

  MPE_Log_event( MPI_Cart_create_stateid_0*2+1,
	         MPI_Cart_create_ncalls_0, (char *)0 );


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

/*
    MPI_Cart_get - prototyping replacement for MPI_Cart_get
    Log the beginning and ending of the time spent in MPI_Cart_get calls.
*/

  ++MPI_Cart_get_ncalls_0;
  MPE_Log_event( MPI_Cart_get_stateid_0*2,
	         MPI_Cart_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_get( comm, maxdims, dims, periods, coords );

  MPE_Log_event( MPI_Cart_get_stateid_0*2+1,
	         MPI_Cart_get_ncalls_0, (char *)0 );


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

/*
    MPI_Cart_map - prototyping replacement for MPI_Cart_map
    Log the beginning and ending of the time spent in MPI_Cart_map calls.
*/

  ++MPI_Cart_map_ncalls_0;
  MPE_Log_event( MPI_Cart_map_stateid_0*2,
	         MPI_Cart_map_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_map( comm_old, ndims, dims, periods, newrank );

  MPE_Log_event( MPI_Cart_map_stateid_0*2+1,
	         MPI_Cart_map_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Cart_rank( comm, coords, rank )
MPI_Comm comm;
int * coords;
int * rank;
{
  int   returnVal;

/*
    MPI_Cart_rank - prototyping replacement for MPI_Cart_rank
    Log the beginning and ending of the time spent in MPI_Cart_rank calls.
*/

  ++MPI_Cart_rank_ncalls_0;
  MPE_Log_event( MPI_Cart_rank_stateid_0*2,
	         MPI_Cart_rank_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_rank( comm, coords, rank );

  MPE_Log_event( MPI_Cart_rank_stateid_0*2+1,
	         MPI_Cart_rank_ncalls_0, (char *)0 );


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

/*
    MPI_Cart_shift - prototyping replacement for MPI_Cart_shift
    Log the beginning and ending of the time spent in MPI_Cart_shift calls.
*/

  ++MPI_Cart_shift_ncalls_0;
  MPE_Log_event( MPI_Cart_shift_stateid_0*2,
	         MPI_Cart_shift_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_shift( comm, direction, displ, source, dest );

  MPE_Log_event( MPI_Cart_shift_stateid_0*2+1,
	         MPI_Cart_shift_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Cart_sub( comm, remain_dims, comm_new )
MPI_Comm comm;
int * remain_dims;
MPI_Comm * comm_new;
{
  int   returnVal;

/*
    MPI_Cart_sub - prototyping replacement for MPI_Cart_sub
    Log the beginning and ending of the time spent in MPI_Cart_sub calls.
*/

  ++MPI_Cart_sub_ncalls_0;
  MPE_Log_event( MPI_Cart_sub_stateid_0*2,
	         MPI_Cart_sub_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cart_sub( comm, remain_dims, comm_new );

  MPE_Log_event( MPI_Cart_sub_stateid_0*2+1,
	         MPI_Cart_sub_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Cartdim_get( comm, ndims )
MPI_Comm comm;
int * ndims;
{
  int   returnVal;

/*
    MPI_Cartdim_get - prototyping replacement for MPI_Cartdim_get
    Log the beginning and ending of the time spent in MPI_Cartdim_get calls.
*/

  ++MPI_Cartdim_get_ncalls_0;
  MPE_Log_event( MPI_Cartdim_get_stateid_0*2,
	         MPI_Cartdim_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Cartdim_get( comm, ndims );

  MPE_Log_event( MPI_Cartdim_get_stateid_0*2+1,
	         MPI_Cartdim_get_ncalls_0, (char *)0 );


  return returnVal;
}

int  MPI_Dims_create( nnodes, ndims, dims )
int nnodes;
int ndims;
int * dims;
{
  int  returnVal;

/*
    MPI_Dims_create - prototyping replacement for MPI_Dims_create
    Log the beginning and ending of the time spent in MPI_Dims_create calls.
*/

  ++MPI_Dims_create_ncalls_0;
  MPE_Log_event( MPI_Dims_create_stateid_0*2,
	         MPI_Dims_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Dims_create( nnodes, ndims, dims );

  MPE_Log_event( MPI_Dims_create_stateid_0*2+1,
	         MPI_Dims_create_ncalls_0, (char *)0 );


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

/*
    MPI_Graph_create - prototyping replacement for MPI_Graph_create
    Log the beginning and ending of the time spent in MPI_Graph_create calls.
*/

  ++MPI_Graph_create_ncalls_0;
  MPE_Log_event( MPI_Graph_create_stateid_0*2,
	         MPI_Graph_create_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graph_create( comm_old, nnodes, index, edges, reorder, comm_graph );

  MPE_Log_event( MPI_Graph_create_stateid_0*2+1,
	         MPI_Graph_create_ncalls_0, (char *)0 );


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

/*
    MPI_Graph_get - prototyping replacement for MPI_Graph_get
    Log the beginning and ending of the time spent in MPI_Graph_get calls.
*/

  ++MPI_Graph_get_ncalls_0;
  MPE_Log_event( MPI_Graph_get_stateid_0*2,
	         MPI_Graph_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graph_get( comm, maxindex, maxedges, index, edges );

  MPE_Log_event( MPI_Graph_get_stateid_0*2+1,
	         MPI_Graph_get_ncalls_0, (char *)0 );


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

/*
    MPI_Graph_map - prototyping replacement for MPI_Graph_map
    Log the beginning and ending of the time spent in MPI_Graph_map calls.
*/

  ++MPI_Graph_map_ncalls_0;
  MPE_Log_event( MPI_Graph_map_stateid_0*2,
	         MPI_Graph_map_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graph_map( comm_old, nnodes, index, edges, newrank );

  MPE_Log_event( MPI_Graph_map_stateid_0*2+1,
	         MPI_Graph_map_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Graph_neighbors( comm, rank, maxneighbors, neighbors )
MPI_Comm comm;
int rank;
int  maxneighbors;
int * neighbors;
{
  int   returnVal;

/*
    MPI_Graph_neighbors - prototyping replacement for MPI_Graph_neighbors
    Log the beginning and ending of the time spent in MPI_Graph_neighbors calls.
*/

  ++MPI_Graph_neighbors_ncalls_0;
  MPE_Log_event( MPI_Graph_neighbors_stateid_0*2,
	         MPI_Graph_neighbors_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graph_neighbors( comm, rank, maxneighbors, neighbors );

  MPE_Log_event( MPI_Graph_neighbors_stateid_0*2+1,
	         MPI_Graph_neighbors_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Graph_neighbors_count( comm, rank, nneighbors )
MPI_Comm comm;
int rank;
int * nneighbors;
{
  int   returnVal;

/*
    MPI_Graph_neighbors_count - prototyping replacement for MPI_Graph_neighbors_count
    Log the beginning and ending of the time spent in MPI_Graph_neighbors_count calls.
*/

  ++MPI_Graph_neighbors_count_ncalls_0;
  MPE_Log_event( MPI_Graph_neighbors_count_stateid_0*2,
	         MPI_Graph_neighbors_count_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graph_neighbors_count( comm, rank, nneighbors );

  MPE_Log_event( MPI_Graph_neighbors_count_stateid_0*2+1,
	         MPI_Graph_neighbors_count_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Graphdims_get( comm, nnodes, nedges )
MPI_Comm comm;
int * nnodes;
int * nedges;
{
  int   returnVal;

/*
    MPI_Graphdims_get - prototyping replacement for MPI_Graphdims_get
    Log the beginning and ending of the time spent in MPI_Graphdims_get calls.
*/

  ++MPI_Graphdims_get_ncalls_0;
  MPE_Log_event( MPI_Graphdims_get_stateid_0*2,
	         MPI_Graphdims_get_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Graphdims_get( comm, nnodes, nedges );

  MPE_Log_event( MPI_Graphdims_get_stateid_0*2+1,
	         MPI_Graphdims_get_ncalls_0, (char *)0 );


  return returnVal;
}

int   MPI_Topo_test( comm, top_type )
MPI_Comm comm;
int * top_type;
{
  int   returnVal;

/*
    MPI_Topo_test - prototyping replacement for MPI_Topo_test
    Log the beginning and ending of the time spent in MPI_Topo_test calls.
*/

  ++MPI_Topo_test_ncalls_0;
  MPE_Log_event( MPI_Topo_test_stateid_0*2,
	         MPI_Topo_test_ncalls_0, (char *)0 );
  
  returnVal = PMPI_Topo_test( comm, top_type );

  MPE_Log_event( MPI_Topo_test_stateid_0*2+1,
	         MPI_Topo_test_ncalls_0, (char *)0 );


  return returnVal;
}
