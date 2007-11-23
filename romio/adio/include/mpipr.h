/* This file replaces all MPI function names with their PMPI equivalents.
   PMPI versions are used by default so that the user can profile 
   application code without interference from MPI functions used by 
   MPI-IO. */

#ifndef USE_MPI_VERSIONS

#define MPI_Abort PMPI_Abort
#define MPI_Address PMPI_Address
#define MPI_Allgather PMPI_Allgather
#define MPI_Allgatherv PMPI_Allgatherv
#define MPI_Allreduce PMPI_Allreduce
#define MPI_Alltoall PMPI_Alltoall
#define MPI_Alltoallv PMPI_Alltoallv
#define MPI_Attr_delete PMPI_Attr_delete
#define MPI_Attr_get PMPI_Attr_get
#define MPI_Attr_put PMPI_Attr_put
#define MPI_Barrier PMPI_Barrier
#define MPI_Bcast PMPI_Bcast
#define MPI_Bsend PMPI_Bsend
#define MPI_Bsend_init PMPI_Bsend_init
#define MPI_Buffer_attach PMPI_Buffer_attach
#define MPI_Buffer_detach PMPI_Buffer_detach
#define MPI_Cancel PMPI_Cancel
#define MPI_Cart_coords PMPI_Cart_coords
#define MPI_Cart_create PMPI_Cart_create
#define MPI_Cart_get PMPI_Cart_get
#define MPI_Cart_map PMPI_Cart_map
#define MPI_Cart_rank PMPI_Cart_rank
#define MPI_Cart_shift PMPI_Cart_shift
#define MPI_Cart_sub PMPI_Cart_sub
#define MPI_Cartdim_get PMPI_Cartdim_get
#define MPI_Comm_compare PMPI_Comm_compare
#define MPI_Comm_create PMPI_Comm_create
#define MPI_Comm_dup PMPI_Comm_dup
#define MPI_Comm_free PMPI_Comm_free
#define MPI_Comm_group PMPI_Comm_group
#define MPI_Comm_rank PMPI_Comm_rank
#define MPI_Comm_remote_group PMPI_Comm_remote_group
#define MPI_Comm_remote_size PMPI_Comm_remote_size
#define MPI_Comm_size PMPI_Comm_size
#define MPI_Comm_split PMPI_Comm_split
#define MPI_Comm_test_inter PMPI_Comm_test_inter
#define MPI_Dims_create PMPI_Dims_create
#define MPI_Errhandler_create PMPI_Errhandler_create
#define MPI_Errhandler_free PMPI_Errhandler_free
#define MPI_Errhandler_get PMPI_Errhandler_get
#define MPI_Errhandler_set PMPI_Errhandler_set
#define MPI_Error_class PMPI_Error_class
#define MPI_Error_string PMPI_Error_string
#define MPI_Finalize PMPI_Finalize
#define MPI_Gather PMPI_Gather
#define MPI_Gatherv PMPI_Gatherv
#define MPI_Get_count PMPI_Get_count
#define MPI_Get_elements PMPI_Get_elements
#define MPI_Get_processor_name PMPI_Get_processor_name
#define MPI_Get_version PMPI_Get_version
#define MPI_Graph_create PMPI_Graph_create
#define MPI_Graph_get PMPI_Graph_get
#define MPI_Graph_map PMPI_Graph_map
#define MPI_Graph_neighbors PMPI_Graph_neighbors
#define MPI_Graph_neighbors_count PMPI_Graph_neighbors_count
#define MPI_Graphdims_get PMPI_Graphdims_get
#define MPI_Group_compare PMPI_Group_compare
#define MPI_Group_difference PMPI_Group_difference
#define MPI_Group_excl PMPI_Group_excl
#define MPI_Group_free PMPI_Group_free
#define MPI_Group_incl PMPI_Group_incl
#define MPI_Group_intersection PMPI_Group_intersection
#define MPI_Group_range_excl PMPI_Group_range_excl
#define MPI_Group_range_incl PMPI_Group_range_incl
#define MPI_Group_rank PMPI_Group_rank
#define MPI_Group_size PMPI_Group_size
#define MPI_Group_translate_ranks PMPI_Group_translate_ranks
#define MPI_Group_union PMPI_Group_union
#define MPI_Ibsend PMPI_Ibsend
#define MPI_Info_create PMPI_Info_create
#define MPI_Info_delete PMPI_Info_delete
#define MPI_Info_dup PMPI_Info_dup
#define MPI_Info_free PMPI_Info_free
#define MPI_Info_get PMPI_Info_get
#define MPI_Info_get_nkeys PMPI_Info_get_nkeys
#define MPI_Info_get_nthkey PMPI_Info_get_nthkey
#define MPI_Info_get_valuelen PMPI_Info_get_valuelen
#define MPI_Info_set PMPI_Info_set
#define MPI_Init PMPI_Init
#define MPI_Initialized PMPI_Initialized
#define MPI_Intercomm_create PMPI_Intercomm_create
#define MPI_Intercomm_merge PMPI_Intercomm_merge
#define MPI_Iprobe PMPI_Iprobe
#define MPI_Irecv PMPI_Irecv
#define MPI_Irsend PMPI_Irsend
#define MPI_Isend PMPI_Isend
#define MPI_Issend PMPI_Issend
#define MPI_Keyval_create PMPI_Keyval_create
#define MPI_Keyval_free PMPI_Keyval_free
#define MPI_Name_get PMPI_Name_get
#define MPI_Name_put PMPI_Name_put
#define MPI_Op_create PMPI_Op_create
#define MPI_Op_free PMPI_Op_free
#define MPI_Pack PMPI_Pack
#define MPI_Pack_size PMPI_Pack_size
#define MPI_Pcontrol PMPI_Pcontrol
#define MPI_Probe PMPI_Probe
#define MPI_Recv PMPI_Recv
#define MPI_Recv_init PMPI_Recv_init
#define MPI_Reduce PMPI_Reduce
#define MPI_Reduce_scatter PMPI_Reduce_scatter
#define MPI_Request_c2f PMPI_Request_c2f
#define MPI_Request_free PMPI_Request_free
#define MPI_Rsend PMPI_Rsend
#define MPI_Rsend_init PMPI_Rsend_init
#define MPI_Scan PMPI_Scan
#define MPI_Scatter PMPI_Scatter
#define MPI_Scatterv PMPI_Scatterv
#define MPI_Send PMPI_Send
#define MPI_Send_init PMPI_Send_init
#define MPI_Sendrecv PMPI_Sendrecv
#define MPI_Sendrecv_replace PMPI_Sendrecv_replace
#define MPI_Ssend PMPI_Ssend
#define MPI_Ssend_init PMPI_Ssend_init
#define MPI_Start PMPI_Start
#define MPI_Startall PMPI_Startall
#define MPI_Test PMPI_Test
#define MPI_Test_cancelled PMPI_Test_cancelled
#define MPI_Testall PMPI_Testall
#define MPI_Testany PMPI_Testany
#define MPI_Testsome PMPI_Testsome
#define MPI_Topo_status PMPI_Topo_status
#define MPI_Topo_test PMPI_Topo_test
#define MPI_Type_commit PMPI_Type_commit
#define MPI_Type_contiguous PMPI_Type_contiguous
#define MPI_Type_count PMPI_Type_count
/* #define MPI_Type_create_darray PMPI_Type_create_darray */
#define MPI_Type_create_indexed_block PMPI_Type_create_indexed_block
/* #define MPI_Type_create_subarray PMPI_Type_create_subarray */
#define MPI_Type_extent PMPI_Type_extent
#define MPI_Type_free PMPI_Type_free
#define MPI_Type_get_contents PMPI_Type_get_contents
#define MPI_Type_get_envelope PMPI_Type_get_envelope
#define MPI_Type_hindexed PMPI_Type_hindexed
#define MPI_Type_hvector PMPI_Type_hvector
#define MPI_Type_indexed PMPI_Type_indexed
#define MPI_Type_lb PMPI_Type_lb
#define MPI_Type_size PMPI_Type_size
#define MPI_Type_struct PMPI_Type_struct
#define MPI_Type_ub PMPI_Type_ub
#define MPI_Type_vector PMPI_Type_vector
#define MPI_Unpack PMPI_Unpack
#define MPI_Wait PMPI_Wait
#define MPI_Waitall PMPI_Waitall
#define MPI_Waitany PMPI_Waitany
#define MPI_Waitsome PMPI_Waitsome
#define MPI_Wtick PMPI_Wtick
#define MPI_Wtime PMPI_Wtime

/* commented out because these could be macros themselves, as in MPICH 
#define MPI_Type_c2f PMPI_Type_c2f
#define MPI_Type_f2c PMPI_Type_f2c
#define MPI_Comm_c2f PMPI_Comm_c2f
#define MPI_Comm_f2c PMPI_Comm_f2c
#define MPI_Group_c2f PMPI_Group_c2f
#define MPI_Group_f2c PMPI_Group_f2c
#define MPI_Op_c2f PMPI_Op_c2f
#define MPI_Op_f2c PMPI_Op_f2c
#define MPI_Request_c2f PMPI_Request_c2f
#define MPI_Request_f2c PMPI_Request_f2c
#define MPI_Win_c2f PMPI_Win_c2f
#define MPI_Win_f2c PMPI_Win_f2c
#define MPI_Status_c2f PMPI_Status_c2f
#define MPI_Status_f2c PMPI_Status_f2c
*/

#ifndef MPI_INFO_SRC  /* everywhere except in info source directory */
#define MPI_Info_create PMPI_Info_create
#define MPI_Info_set PMPI_Info_set
#define MPI_Info_delete PMPI_Info_delete
#define MPI_Info_get PMPI_Info_get
#define MPI_Info_get_valuelen PMPI_Info_get_valuelen
#define MPI_Info_get_nkeys PMPI_Info_get_nkeys
#define MPI_Info_get_nthkey PMPI_Info_get_nthkey
#define MPI_Info_dup PMPI_Info_dup
#define MPI_Info_free PMPI_Info_free
#define MPI_Info_c2f PMPI_Info_c2f
#define MPI_Info_f2c PMPI_Info_f2c
#endif

#ifdef MPIO_FORTRAN_SRC   /* only in MPI-IO Fortran source directory */
#define MPI_File_c2f PMPI_File_c2f
#define MPI_File_f2c PMPI_File_f2c
#endif

#endif
