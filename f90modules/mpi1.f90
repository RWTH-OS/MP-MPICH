        MODULE MPI1
!       This file created by the script CreateModuleSrc
        IMPLICIT NONE
        INTERFACE
 
! Point to point communication
!
        SUBROUTINE MPI_CANCEL(REQUEST, IERROR)
        INTEGER  REQUEST, IERROR
        END SUBROUTINE MPI_CANCEL
!
        SUBROUTINE MPI_GET_COUNT(STATUS, DATATYPE, COUNT, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER  STATUS(MPI_STATUS_SIZE), DATATYPE, COUNT, IERROR
        END SUBROUTINE MPI_GET_COUNT
!
        SUBROUTINE MPI_GET_ELEMENTS(STATUS, DATATYPE, COUNT, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER  STATUS(MPI_STATUS_SIZE), DATATYPE, COUNT, IERROR
        END SUBROUTINE MPI_GET_ELEMENTS
!
        SUBROUTINE MPI_IPROBE(SOURCE, TAG, COMM, FLAG, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        LOGICAL  FLAG 
        INTEGER  SOURCE, TAG, COMM, STATUS(MPI_STATUS_SIZE), IERROR
        END SUBROUTINE MPI_IPROBE
!
        SUBROUTINE MPI_PACK_SIZE(INCOUNT, DATATYPE, COMM, SIZE,        &
      &   IERROR) 
        INTEGER INCOUNT, DATATYPE, COMM, SIZE, IERROR
        END SUBROUTINE MPI_PACK_SIZE
!
        SUBROUTINE MPI_PROBE(SOURCE, TAG, COMM, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER SOURCE, TAG, COMM, STATUS(MPI_STATUS_SIZE), IERROR
        END SUBROUTINE MPI_PROBE
!
        SUBROUTINE MPI_REQUEST_FREE(REQUEST, IERROR)
        INTEGER  REQUEST, IERROR
        END SUBROUTINE MPI_REQUEST_FREE
!
        SUBROUTINE MPI_START(REQUEST, IERROR)
        INTEGER  REQUEST, IERROR
        END SUBROUTINE MPI_START
!
        SUBROUTINE MPI_STARTALL(COUNT, ARRAY_OF_REQUESTS, IERROR)
        INTEGER  COUNT, ARRAY_OF_REQUESTS(*), IERROR
        END SUBROUTINE MPI_STARTALL
!
        SUBROUTINE MPI_TEST(REQUEST, FLAG, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        LOGICAL  FLAG 
        INTEGER  REQUEST, STATUS(MPI_STATUS_SIZE), IERROR
        END SUBROUTINE MPI_TEST
!
        SUBROUTINE MPI_TESTALL(COUNT, ARRAY_OF_REQUESTS, FLAG,         &
      &   ARRAY_OF_STATUSES, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        LOGICAL  FLAG 
        INTEGER  COUNT, ARRAY_OF_REQUESTS(*),                          &
      &   ARRAY_OF_STATUSES(MPI_STATUS_SIZE,*), IERROR 
        END SUBROUTINE MPI_TESTALL
!
        SUBROUTINE MPI_TESTANY(COUNT, ARRAY_OF_REQUESTS, INDEX, FLAG,  &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        LOGICAL  FLAG 
        INTEGER  COUNT, ARRAY_OF_REQUESTS(*), INDEX,                   &
      &   STATUS(MPI_STATUS_SIZE), IERROR 
        END SUBROUTINE MPI_TESTANY
!
        SUBROUTINE MPI_TESTSOME(INCOUNT, ARRAY_OF_REQUESTS, OUTCOUNT,  &
      &   ARRAY_OF_INDICES, ARRAY_OF_STATUSES, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER INCOUNT, ARRAY_OF_REQUESTS(*), OUTCOUNT,               &
      &   ARRAY_OF_INDICES(*), ARRAY_OF_STATUSES(MPI_STATUS_SIZE,*),   &
      &   IERROR 
        END SUBROUTINE MPI_TESTSOME
!
        SUBROUTINE MPI_TEST_CANCELLED(STATUS, FLAG, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        LOGICAL FLAG 
        INTEGER  STATUS(MPI_STATUS_SIZE), IERROR
        END SUBROUTINE MPI_TEST_CANCELLED
!
        SUBROUTINE MPI_TYPE_COMMIT(DATATYPE, IERROR)
        INTEGER  DATATYPE, IERROR
        END SUBROUTINE MPI_TYPE_COMMIT
!
        SUBROUTINE MPI_TYPE_CONTIGUOUS(COUNT, OLDTYPE, NEWTYPE,        &
      &   IERROR) 
        INTEGER  COUNT, OLDTYPE, NEWTYPE, IERROR
        END SUBROUTINE MPI_TYPE_CONTIGUOUS
!
        SUBROUTINE MPI_TYPE_EXTENT(DATATYPE, EXTENT, IERROR)
        INTEGER DATATYPE, EXTENT, IERROR
        END SUBROUTINE MPI_TYPE_EXTENT
!
        SUBROUTINE MPI_TYPE_FREE(DATATYPE, IERROR)
        INTEGER  DATATYPE, IERROR
        END SUBROUTINE MPI_TYPE_FREE
!
        SUBROUTINE MPI_TYPE_HINDEXED(COUNT, ARRAY_OF_BLOCKLENGTHS,     &
      &   ARRAY_OF_DISPLACEMENTS, OLDTYPE, NEWTYPE, IERROR) 
        INTEGER COUNT, ARRAY_OF_BLOCKLENGTHS(*),                       &
      &   ARRAY_OF_DISPLACEMENTS(*), OLDTYPE, NEWTYPE, IERROR 
        END SUBROUTINE MPI_TYPE_HINDEXED
!
        SUBROUTINE MPI_TYPE_HVECTOR(COUNT, BLOCKLENGTH, STRIDE,        &
      &   OLDTYPE, NEWTYPE, IERROR) 
        INTEGER  COUNT, BLOCKLENGTH, STRIDE, OLDTYPE, NEWTYPE, IERROR
        END SUBROUTINE MPI_TYPE_HVECTOR
!
        SUBROUTINE MPI_TYPE_INDEXED(COUNT, ARRAY_OF_BLOCKLENGTHS,      &
      &   ARRAY_OF_DISPLACEMENTS, OLDTYPE, NEWTYPE, IERROR) 
        INTEGER COUNT, ARRAY_OF_BLOCKLENGTHS(*),                       &
      &   ARRAY_OF_DISPLACEMENTS(*), OLDTYPE, NEWTYPE, IERROR 
        END SUBROUTINE MPI_TYPE_INDEXED
!
        SUBROUTINE MPI_TYPE_LB( DATATYPE, DISPLACEMENT, IERROR)
        INTEGER DATATYPE, DISPLACEMENT, IERROR
        END SUBROUTINE MPI_TYPE_LB
!
        SUBROUTINE MPI_TYPE_SIZE(DATATYPE, SIZE, IERROR)
        INTEGER DATATYPE, SIZE, IERROR
        END SUBROUTINE MPI_TYPE_SIZE
!
        SUBROUTINE MPI_TYPE_STRUCT(COUNT, ARRAY_OF_BLOCKLENGTHS,       &
      &   ARRAY_OF_DISPLACEMENTS, ARRAY_OF_TYPES, NEWTYPE, IERROR) 
        INTEGER  COUNT, ARRAY_OF_BLOCKLENGTHS(*),                      &
      &   ARRAY_OF_DISPLACEMENTS(*), ARRAY_OF_TYPES(*), NEWTYPE,       &
      &   IERROR 
        END SUBROUTINE MPI_TYPE_STRUCT
!
        SUBROUTINE MPI_TYPE_UB( DATATYPE, DISPLACEMENT, IERROR)
        INTEGER DATATYPE, DISPLACEMENT, IERROR
        END SUBROUTINE MPI_TYPE_UB
!
        SUBROUTINE MPI_TYPE_VECTOR(COUNT, BLOCKLENGTH, STRIDE,         &
      &   OLDTYPE, NEWTYPE, IERROR) 
        INTEGER  COUNT, BLOCKLENGTH, STRIDE, OLDTYPE, NEWTYPE, IERROR
        END SUBROUTINE MPI_TYPE_VECTOR
!
        SUBROUTINE MPI_WAIT(REQUEST, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER  REQUEST, STATUS(MPI_STATUS_SIZE), IERROR
        END SUBROUTINE MPI_WAIT
!
        SUBROUTINE MPI_WAITALL(COUNT, ARRAY_OF_REQUESTS,               &
      &   ARRAY_OF_STATUSES, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER  COUNT, ARRAY_OF_REQUESTS(*) 
        INTEGER  ARRAY_OF_STATUSES(MPI_STATUS_SIZE,*), IERROR
        END SUBROUTINE MPI_WAITALL
!
        SUBROUTINE MPI_WAITANY(COUNT, ARRAY_OF_REQUESTS, INDEX,        &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER  COUNT, ARRAY_OF_REQUESTS(*), INDEX,                   &
      &   STATUS(MPI_STATUS_SIZE), IERROR 
        END SUBROUTINE MPI_WAITANY
!
        SUBROUTINE MPI_WAITSOME(INCOUNT, ARRAY_OF_REQUESTS, OUTCOUNT,  &
      &   ARRAY_OF_INDICES, ARRAY_OF_STATUSES, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER INCOUNT, ARRAY_OF_REQUESTS(*), OUTCOUNT,               &
      &   ARRAY_OF_INDICES(*), ARRAY_OF_STATUSES(MPI_STATUS_SIZE,*),   &
      &   IERROR 
        END SUBROUTINE MPI_WAITSOME
 
! Collective communication
!
        SUBROUTINE MPI_BARRIER(COMM, IERROR) 
        INTEGER COMM, IERROR
        END SUBROUTINE MPI_BARRIER
!
        SUBROUTINE MPI_OP_CREATE( FUNCTION, COMMUTE, OP, IERROR) 
        EXTERNAL FUNCTION 
        LOGICAL COMMUTE 
        INTEGER OP, IERROR
        END SUBROUTINE MPI_OP_CREATE
!
        SUBROUTINE MPI_OP_FREE( OP, IERROR) 
        INTEGER OP, IERROR
        END SUBROUTINE MPI_OP_FREE
 
! Communicators
!
        SUBROUTINE MPI_ATTR_DELETE(COMM, KEYVAL, IERROR)
        INTEGER COMM, KEYVAL, IERROR
        END SUBROUTINE MPI_ATTR_DELETE
!
        SUBROUTINE MPI_ATTR_GET(COMM, KEYVAL, ATTRIBUTE_VAL, FLAG,     &
      &   IERROR) 
        INTEGER COMM, KEYVAL, ATTRIBUTE_VAL, IERROR 
        LOGICAL FLAG
        END SUBROUTINE MPI_ATTR_GET
!
        SUBROUTINE MPI_ATTR_PUT(COMM, KEYVAL, ATTRIBUTE_VAL, IERROR)
        INTEGER COMM, KEYVAL, ATTRIBUTE_VAL, IERROR
        END SUBROUTINE MPI_ATTR_PUT
!
        SUBROUTINE MPI_COMM_COMPARE(COMM1, COMM2, RESULT, IERROR)
        INTEGER COMM1, COMM2, RESULT, IERROR
        END SUBROUTINE MPI_COMM_COMPARE
!
        SUBROUTINE MPI_COMM_CREATE(COMM, GROUP, NEWCOMM, IERROR)
        INTEGER COMM, GROUP, NEWCOMM, IERROR
        END SUBROUTINE MPI_COMM_CREATE
!
        SUBROUTINE MPI_COMM_DUP(COMM, NEWCOMM, IERROR)
        INTEGER COMM, NEWCOMM, IERROR
        END SUBROUTINE MPI_COMM_DUP
!
        SUBROUTINE MPI_COMM_FREE(COMM, IERROR)
        INTEGER COMM, IERROR
        END SUBROUTINE MPI_COMM_FREE
!
        SUBROUTINE MPI_COMM_GROUP(COMM, GROUP, IERROR)
        INTEGER COMM, GROUP, IERROR
        END SUBROUTINE MPI_COMM_GROUP
!
        SUBROUTINE MPI_COMM_RANK(COMM, RANK, IERROR)
        INTEGER COMM, RANK, IERROR
        END SUBROUTINE MPI_COMM_RANK
!
        SUBROUTINE MPI_COMM_REMOTE_GROUP(COMM, GROUP, IERROR)
        INTEGER COMM, GROUP, IERROR
        END SUBROUTINE MPI_COMM_REMOTE_GROUP
!
        SUBROUTINE MPI_COMM_REMOTE_SIZE(COMM, SIZE, IERROR)
        INTEGER COMM, SIZE, IERROR
        END SUBROUTINE MPI_COMM_REMOTE_SIZE
!
        SUBROUTINE MPI_COMM_SIZE(COMM, SIZE, IERROR)
        INTEGER COMM, SIZE, IERROR
        END SUBROUTINE MPI_COMM_SIZE
!
        SUBROUTINE MPI_COMM_SPLIT(COMM, COLOR, KEY, NEWCOMM, IERROR)
        INTEGER COMM, COLOR, KEY, NEWCOMM, IERROR
        END SUBROUTINE MPI_COMM_SPLIT
!
        SUBROUTINE MPI_COMM_TEST_INTER(COMM, FLAG, IERROR)
        INTEGER COMM, IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_COMM_TEST_INTER
!
        SUBROUTINE MPI_GROUP_COMPARE(GROUP1, GROUP2, RESULT, IERROR)
        INTEGER GROUP1, GROUP2, RESULT, IERROR
        END SUBROUTINE MPI_GROUP_COMPARE
!
        SUBROUTINE MPI_GROUP_DIFFERENCE(GROUP1, GROUP2, NEWGROUP,      &
      &   IERROR) 
        INTEGER GROUP1, GROUP2, NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_DIFFERENCE
!
        SUBROUTINE MPI_GROUP_EXCL(GROUP, N, RANKS, NEWGROUP, IERROR)
        INTEGER GROUP, N, RANKS(*), NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_EXCL
!
        SUBROUTINE MPI_GROUP_FREE(GROUP, IERROR)
        INTEGER GROUP, IERROR
        END SUBROUTINE MPI_GROUP_FREE
!
        SUBROUTINE MPI_GROUP_INCL(GROUP, N, RANKS, NEWGROUP, IERROR)
        INTEGER GROUP, N, RANKS(*), NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_INCL
!
        SUBROUTINE MPI_GROUP_INTERSECTION(GROUP1, GROUP2, NEWGROUP,    &
      &   IERROR) 
        INTEGER GROUP1, GROUP2, NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_INTERSECTION
!
        SUBROUTINE MPI_GROUP_RANGE_EXCL(GROUP, N, RANGES, NEWGROUP,    &
      &   IERROR) 
        INTEGER GROUP, N, RANGES(3,*), NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_RANGE_EXCL
!
        SUBROUTINE MPI_GROUP_RANGE_INCL(GROUP, N, RANGES, NEWGROUP,    &
      &   IERROR) 
        INTEGER GROUP, N, RANGES(3,*), NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_RANGE_INCL
!
        SUBROUTINE MPI_GROUP_RANK(GROUP, RANK, IERROR)
        INTEGER GROUP, RANK, IERROR
        END SUBROUTINE MPI_GROUP_RANK
!
        SUBROUTINE MPI_GROUP_SIZE(GROUP, SIZE, IERROR)
        INTEGER GROUP, SIZE, IERROR
        END SUBROUTINE MPI_GROUP_SIZE
!
        SUBROUTINE MPI_GROUP_TRANSLATE_RANKS(GROUP1, N, RANKS1,        &
      &   GROUP2, RANKS2, IERROR) 
        INTEGER GROUP1, N, RANKS1(*), GROUP2, RANKS2(*), IERROR
        END SUBROUTINE MPI_GROUP_TRANSLATE_RANKS
!
        SUBROUTINE MPI_GROUP_UNION(GROUP1, GROUP2, NEWGROUP, IERROR)
        INTEGER GROUP1, GROUP2, NEWGROUP, IERROR
        END SUBROUTINE MPI_GROUP_UNION
!
        SUBROUTINE MPI_INTERCOMM_CREATE(LOCAL_COMM, LOCAL_LEADER,      &
      &   PEER_COMM, REMOTE_LEADER, TAG, NEWINTERCOMM, IERROR) 
        INTEGER LOCAL_COMM, LOCAL_LEADER, PEER_COMM, REMOTE_LEADER,    &
      &   TAG, NEWINTERCOMM, IERROR 
        END SUBROUTINE MPI_INTERCOMM_CREATE
!
        SUBROUTINE MPI_INTERCOMM_MERGE(INTERCOMM, HIGH, INTRACOMM,     &
      &   IERROR) 
        INTEGER INTERCOMM, INTRACOMM, IERROR 
        LOGICAL HIGH
        END SUBROUTINE MPI_INTERCOMM_MERGE
!
        SUBROUTINE MPI_KEYVAL_CREATE(COPY_FN, DELETE_FN, KEYVAL,       &
      &   EXTRA_STATE, IERROR) 
        EXTERNAL COPY_FN, DELETE_FN 
        INTEGER KEYVAL, EXTRA_STATE, IERROR
        END SUBROUTINE MPI_KEYVAL_CREATE
!
        SUBROUTINE MPI_KEYVAL_FREE(KEYVAL, IERROR)
        INTEGER KEYVAL, IERROR
        END SUBROUTINE MPI_KEYVAL_FREE
 
! Topology
!
        SUBROUTINE MPI_CARTDIM_GET(COMM, NDIMS, IERROR)
        INTEGER COMM, NDIMS, IERROR
        END SUBROUTINE MPI_CARTDIM_GET
!
        SUBROUTINE MPI_CART_COORDS(COMM, RANK, MAXDIMS, COORDS,        &
      &   IERROR) 
        INTEGER COMM, RANK, MAXDIMS, COORDS(*), IERROR
        END SUBROUTINE MPI_CART_COORDS
!
        SUBROUTINE MPI_CART_CREATE(COMM_OLD, NDIMS, DIMS, PERIODS,     &
      &   REORDER, COMM_CART, IERROR) 
        INTEGER COMM_OLD, NDIMS, DIMS(*), COMM_CART, IERROR 
        LOGICAL PERIODS(*), REORDER
        END SUBROUTINE MPI_CART_CREATE
!
        SUBROUTINE MPI_CART_GET(COMM, MAXDIMS, DIMS, PERIODS, COORDS,  &
      &   IERROR) 
        INTEGER COMM, MAXDIMS, DIMS(*), COORDS(*), IERROR 
        LOGICAL PERIODS(*)
        END SUBROUTINE MPI_CART_GET
!
        SUBROUTINE MPI_CART_MAP(COMM, NDIMS, DIMS, PERIODS, NEWRANK,   &
      &   IERROR) 
        INTEGER COMM, NDIMS, DIMS(*), NEWRANK, IERROR 
        LOGICAL PERIODS(*)
        END SUBROUTINE MPI_CART_MAP
!
        SUBROUTINE MPI_CART_RANK(COMM, COORDS, RANK, IERROR)
        INTEGER COMM, COORDS(*), RANK, IERROR
        END SUBROUTINE MPI_CART_RANK
!
        SUBROUTINE MPI_CART_SHIFT(COMM, DIRECTION, DISP, RANK_SOURCE,  &
      &   RANK_DEST, IERROR) 
        INTEGER COMM, DIRECTION, DISP, RANK_SOURCE, RANK_DEST, IERROR
        END SUBROUTINE MPI_CART_SHIFT
!
        SUBROUTINE MPI_CART_SUB(COMM, REMAIN_DIMS, NEWCOMM, IERROR)
        INTEGER COMM, NEWCOMM, IERROR 
        LOGICAL REMAIN_DIMS(*)
        END SUBROUTINE MPI_CART_SUB
!
        SUBROUTINE MPI_DIMS_CREATE(NNODES, NDIMS, DIMS, IERROR)
        INTEGER NNODES, NDIMS, DIMS(*), IERROR
        END SUBROUTINE MPI_DIMS_CREATE
!
        SUBROUTINE MPI_GRAPHDIMS_GET(COMM, NNODES, NEDGES, IERROR)
        INTEGER COMM, NNODES, NEDGES, IERROR
        END SUBROUTINE MPI_GRAPHDIMS_GET
!
        SUBROUTINE MPI_GRAPH_CREATE(COMM_OLD, NNODES, INDEX, EDGES,    &
      &   REORDER, COMM_GRAPH, IERROR) 
        INTEGER COMM_OLD, NNODES, INDEX(*), EDGES(*), COMM_GRAPH,      &
      &   IERROR 
        LOGICAL REORDER
        END SUBROUTINE MPI_GRAPH_CREATE
!
        SUBROUTINE MPI_GRAPH_GET(COMM, MAXINDEX, MAXEDGES, INDEX,      &
      &   EDGES, IERROR) 
        INTEGER COMM, MAXINDEX, MAXEDGES, INDEX(*), EDGES(*), IERROR
        END SUBROUTINE MPI_GRAPH_GET
!
        SUBROUTINE MPI_GRAPH_MAP(COMM, NNODES, INDEX, EDGES, NEWRANK,  &
      &   IERROR) 
        INTEGER COMM, NNODES, INDEX(*), EDGES(*), NEWRANK, IERROR
        END SUBROUTINE MPI_GRAPH_MAP
!
        SUBROUTINE MPI_GRAPH_NEIGHBORS(COMM, RANK, MAXNEIGHBORS,       &
      &   NEIGHBORS, IERROR) 
        INTEGER COMM, RANK, MAXNEIGHBORS, NEIGHBORS(*), IERROR
        END SUBROUTINE MPI_GRAPH_NEIGHBORS
!
        SUBROUTINE MPI_GRAPH_NEIGHBORS_COUNT(COMM, RANK, NNEIGHBORS,   &
      &   IERROR) 
        INTEGER COMM, RANK, NNEIGHBORS, IERROR
        END SUBROUTINE MPI_GRAPH_NEIGHBORS_COUNT
!
        SUBROUTINE MPI_TOPO_TEST(COMM, STATUS, IERROR)
        INTEGER COMM, STATUS, IERROR
        END SUBROUTINE MPI_TOPO_TEST
 
! Environmental Inquiry
!
        SUBROUTINE MPI_ABORT(COMM, ERRORCODE, IERROR)
        INTEGER COMM, ERRORCODE, IERROR
        END SUBROUTINE MPI_ABORT
!
        SUBROUTINE MPI_ERRHANDLER_CREATE(FUNCTION, ERRHANDLER, IERROR)
        EXTERNAL FUNCTION 
        INTEGER ERRHANDLER, IERROR
        END SUBROUTINE MPI_ERRHANDLER_CREATE
!
        SUBROUTINE MPI_ERRHANDLER_FREE(ERRHANDLER, IERROR)
        INTEGER ERRHANDLER, IERROR
        END SUBROUTINE MPI_ERRHANDLER_FREE
!
        SUBROUTINE MPI_ERRHANDLER_GET(COMM, ERRHANDLER, IERROR)
        INTEGER COMM, ERRHANDLER, IERROR
        END SUBROUTINE MPI_ERRHANDLER_GET
!
        SUBROUTINE MPI_ERRHANDLER_SET(COMM, ERRHANDLER, IERROR)
        INTEGER COMM, ERRHANDLER, IERROR
        END SUBROUTINE MPI_ERRHANDLER_SET
!
        SUBROUTINE MPI_ERROR_CLASS(ERRORCODE, ERRORCLASS, IERROR)
        INTEGER ERRORCODE, ERRORCLASS, IERROR
        END SUBROUTINE MPI_ERROR_CLASS
!
        SUBROUTINE MPI_ERROR_STRING(ERRORCODE, STRING, RESULTLEN,      &
      &   IERROR) 
        INTEGER ERRORCODE, RESULTLEN, IERROR 
        CHARACTER*(*) STRING
        END SUBROUTINE MPI_ERROR_STRING
!
        SUBROUTINE MPI_FINALIZE(IERROR)
        INTEGER IERROR
        END SUBROUTINE MPI_FINALIZE
!
        SUBROUTINE MPI_GET_PROCESSOR_NAME( NAME, RESULTLEN, IERROR)
        CHARACTER*(*) NAME
        INTEGER RESULTLEN,IERROR
        END SUBROUTINE MPI_GET_PROCESSOR_NAME
!
        SUBROUTINE MPI_INIT(IERROR)
        INTEGER IERROR
        END SUBROUTINE MPI_INIT
!
        SUBROUTINE MPI_INITIALIZED(FLAG, IERROR)
        LOGICAL FLAG 
        INTEGER IERROR
        END SUBROUTINE MPI_INITIALIZED
 
! Profiling
!
        SUBROUTINE MPI_PCONTROL(level)
        INTEGER LEVEL
        END SUBROUTINE MPI_PCONTROL
        FUNCTION MPI_WTIME()
            DOUBLE PRECISION MPI_WTIME
        END FUNCTION MPI_WTIME
!
        FUNCTION MPI_WTICK()
            DOUBLE PRECISION MPI_WTICK
        END FUNCTION MPI_WTICK
        END INTERFACE
        END MODULE MPI1
