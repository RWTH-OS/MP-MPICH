        MODULE MPI2
!       This file created by the script CreateModuleSrc
        IMPLICIT NONE
        INTERFACE
 
! Fortran bindings
! Process Creation and Management
!
        SUBROUTINE MPI_CLOSE_PORT(PORT_NAME, IERROR)
        CHARACTER*(*) PORT_NAME 
        INTEGER IERROR
        END SUBROUTINE MPI_CLOSE_PORT
!
        SUBROUTINE MPI_COMM_ACCEPT(PORT_NAME, INFO, ROOT, COMM,        &
      &   NEWCOMM, IERROR) 
        CHARACTER*(*) PORT_NAME 
        INTEGER INFO, ROOT, COMM, NEWCOMM, IERROR
        END SUBROUTINE MPI_COMM_ACCEPT
!
        SUBROUTINE MPI_COMM_CONNECT(PORT_NAME, INFO, ROOT, COMM,       &
      &   NEWCOMM, IERROR) 
        CHARACTER*(*) PORT_NAME 
        INTEGER INFO, ROOT, COMM, NEWCOMM, IERROR
        END SUBROUTINE MPI_COMM_CONNECT
!
        SUBROUTINE MPI_COMM_DISCONNECT(COMM, IERROR)
        INTEGER COMM, IERROR 
        
        END SUBROUTINE MPI_COMM_DISCONNECT
!
        SUBROUTINE MPI_COMM_GET_PARENT(PARENT, IERROR) 
        INTEGER PARENT, IERROR
        END SUBROUTINE MPI_COMM_GET_PARENT
!
        SUBROUTINE MPI_COMM_JOIN(FD, INTERCOMM, IERROR)
        INTEGER FD, INTERCOMM, IERROR
        END SUBROUTINE MPI_COMM_JOIN
!
        SUBROUTINE MPI_COMM_SPAWN(COMMAND, ARGV, MAXPROCS, INFO, ROOT, &
      &   COMM, INTERCOMM, ARRAY_OF_ERRCODES, IERROR) 
        CHARACTER*(*) COMMAND, ARGV(*) 
        INTEGER INFO, MAXPROCS, ROOT, COMM, INTERCOMM,                 &
      &   ARRAY_OF_ERRCODES(*), IERROR 
        END SUBROUTINE MPI_COMM_SPAWN
!
        SUBROUTINE MPI_COMM_SPAWN_MULTIPLE(COUNT, ARRAY_OF_COMMANDS,   &
      &   ARRAY_OF_ARGV, ARRAY_OF_MAXPROCS, ARRAY_OF_INFO, ROOT, COMM, &
      &   INTERCOMM, ARRAY_OF_ERRCODES, IERROR) 
        INTEGER COUNT, ARRAY_OF_INFO(*), ARRAY_OF_MAXPROCS(*), ROOT,   &
      &   COMM, INTERCOMM, ARRAY_OF_ERRCODES(*), IERROR 
        CHARACTER*(*) ARRAY_OF_COMMANDS(*), ARRAY_OF_ARGV(COUNT, *)
        END SUBROUTINE MPI_COMM_SPAWN_MULTIPLE
!
        SUBROUTINE MPI_LOOKUP_NAME(SERVICE_NAME, INFO, PORT_NAME,      &
      &   IERROR) 
        CHARACTER*(*) SERVICE_NAME, PORT_NAME 
        INTEGER INFO, IERROR
        END SUBROUTINE MPI_LOOKUP_NAME
!
        SUBROUTINE MPI_OPEN_PORT(INFO, PORT_NAME, IERROR)
        CHARACTER*(*) PORT_NAME 
        INTEGER INFO, IERROR
        END SUBROUTINE MPI_OPEN_PORT
!
        SUBROUTINE MPI_PUBLISH_NAME(SERVICE_NAME, INFO, PORT_NAME,     &
      &   IERROR) 
        INTEGER INFO, IERROR 
        CHARACTER*(*) SERVICE_NAME, PORT_NAME
        END SUBROUTINE MPI_PUBLISH_NAME
!
        SUBROUTINE MPI_UNPUBLISH_NAME(SERVICE_NAME, INFO, PORT_NAME,   &
      &   IERROR) 
        INTEGER INFO, IERROR 
        CHARACTER*(*) SERVICE_NAME, PORT_NAME
        END SUBROUTINE MPI_UNPUBLISH_NAME
 
! One-Sided Communications
!
        SUBROUTINE MPI_WIN_COMPLETE(WIN, IERROR)
        INTEGER WIN,  IERROR
        END SUBROUTINE MPI_WIN_COMPLETE
!
        SUBROUTINE MPI_WIN_FENCE(ASSERT, WIN, IERROR)
        INTEGER ASSERT, WIN, IERROR
        END SUBROUTINE MPI_WIN_FENCE
!
        SUBROUTINE MPI_WIN_FREE(WIN, IERROR)
        INTEGER WIN, IERROR
        END SUBROUTINE MPI_WIN_FREE
!
        SUBROUTINE MPI_WIN_GET_GROUP(WIN, GROUP, IERROR)
        INTEGER WIN, GROUP, IERROR
        END SUBROUTINE MPI_WIN_GET_GROUP
!
        SUBROUTINE MPI_WIN_LOCK(LOCK_TYPE, RANK, ASSERT, WIN, IERROR)
        INTEGER LOCK_TYPE, RANK, ASSERT, WIN, IERROR
        END SUBROUTINE MPI_WIN_LOCK
!
        SUBROUTINE MPI_WIN_POST(GROUP, ASSERT, WIN, IERROR)
        INTEGER GROUP, ASSERT, WIN, IERROR
        END SUBROUTINE MPI_WIN_POST
!
        SUBROUTINE MPI_WIN_START(GROUP, ASSERT, WIN, IERROR)
        INTEGER GROUP, ASSERT, WIN, IERROR
        END SUBROUTINE MPI_WIN_START
!
        SUBROUTINE MPI_WIN_TEST(WIN, FLAG, IERROR)
        INTEGER WIN, IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_WIN_TEST
!
        SUBROUTINE MPI_WIN_UNLOCK(RANK, WIN, IERROR)
        INTEGER RANK, WIN, IERROR
        END SUBROUTINE MPI_WIN_UNLOCK
!
        SUBROUTINE MPI_WIN_WAIT(WIN, IERROR)
        INTEGER WIN,  IERROR
        END SUBROUTINE MPI_WIN_WAIT
 
! Extended Collective Operations
 
! External Interfaces
!
        SUBROUTINE MPI_ADD_ERROR_CLASS(ERRORCLASS, IERROR)
        INTEGER ERRORCLASS, IERROR
        END SUBROUTINE MPI_ADD_ERROR_CLASS
!
        SUBROUTINE MPI_ADD_ERROR_CODE(ERRORCLASS, ERRORCODE, IERROR)
        INTEGER ERRORCLASS, ERRORCODE, IERROR
        END SUBROUTINE MPI_ADD_ERROR_CODE
!
        SUBROUTINE MPI_ADD_ERROR_STRING(ERRORCODE, STRING, IERROR)
        INTEGER ERRORCODE, IERROR 
        CHARACTER*(*) STRING
        END SUBROUTINE MPI_ADD_ERROR_STRING
!
        SUBROUTINE MPI_COMM_CALL_ERRHANDLER(COMM, ERRORCODE, IERROR)
        INTEGER COMM, ERRORCODE, IERROR
        END SUBROUTINE MPI_COMM_CALL_ERRHANDLER
!
        SUBROUTINE MPI_COMM_CREATE_KEYVAL(COMM_COPY_ATTR_FN,           &
      &   COMM_DELETE_ATTR_FN, COMM_KEYVAL, EXTRA_STATE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        EXTERNAL COMM_COPY_ATTR_FN, COMM_DELETE_ATTR_FN
        INTEGER COMM_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) EXTRA_STATE
        END SUBROUTINE MPI_COMM_CREATE_KEYVAL
!
        SUBROUTINE MPI_COMM_DELETE_ATTR(COMM, COMM_KEYVAL, IERROR)
        INTEGER COMM, COMM_KEYVAL, IERROR
        END SUBROUTINE   MPI_COMM_DELETE_ATTR
!
        SUBROUTINE MPI_COMM_FREE_KEYVAL(COMM_KEYVAL, IERROR)
        INTEGER COMM_KEYVAL, IERROR
        END SUBROUTINE  MPI_COMM_FREE_KEYVAL
!
        SUBROUTINE MPI_COMM_GET_ATTR(COMM, COMM_KEYVAL, ATTRIBUTE_VAL, &
      &   FLAG, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER COMM, COMM_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        LOGICAL FLAG
        END SUBROUTINE   MPI_COMM_GET_ATTR
!
        SUBROUTINE MPI_COMM_GET_NAME(COMM, COMM_NAME, RESULTLEN,       &
      &   IERROR) 
        INTEGER COMM, RESULTLEN, IERROR
        CHARACTER*(*) COMM_NAME
        END SUBROUTINE MPI_COMM_GET_NAME
!
        SUBROUTINE MPI_COMM_SET_ATTR(COMM, COMM_KEYVAL, ATTRIBUTE_VAL, &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER COMM, COMM_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        END SUBROUTINE  MPI_COMM_SET_ATTR
!
        SUBROUTINE MPI_COMM_SET_NAME(COMM, COMM_NAME, IERROR) 
        INTEGER COMM, IERROR
        CHARACTER*(*) COMM_NAME
        END SUBROUTINE MPI_COMM_SET_NAME
!
        SUBROUTINE MPI_FILE_CALL_ERRHANDLER(FH, ERRORCODE, IERROR)
        INTEGER FH, ERRORCODE, IERROR
        END SUBROUTINE MPI_FILE_CALL_ERRHANDLER
!
        SUBROUTINE MPI_GREQUEST_COMPLETE(REQUEST, IERROR)
        INTEGER REQUEST, IERROR
        END SUBROUTINE MPI_GREQUEST_COMPLETE
!
        SUBROUTINE MPI_GREQUEST_START(QUERY_FN, FREE_FN, CANCEL_FN,    &
      &   EXTRA_STATE, REQUEST, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER REQUEST, IERROR
        EXTERNAL QUERY_FN, FREE_FN, CANCEL_FN
        INTEGER (KIND=MPI_ADDRESS_KIND) EXTRA_STATE
        END SUBROUTINE MPI_GREQUEST_START
!
        SUBROUTINE MPI_INIT_THREAD(REQUIRED, PROVIDED, IERROR)
        INTEGER REQUIRED, PROVIDED, IERROR
        END SUBROUTINE MPI_INIT_THREAD
!
        SUBROUTINE MPI_IS_THREAD_MAIN(FLAG, IERROR) 
        LOGICAL FLAG 
        INTEGER IERROR
        END SUBROUTINE MPI_IS_THREAD_MAIN
!
        SUBROUTINE MPI_QUERY_THREAD(PROVIDED, IERROR)
        INTEGER PROVIDED, IERROR
        END SUBROUTINE MPI_QUERY_THREAD
!
        SUBROUTINE MPI_STATUS_SET_CANCELLED(STATUS, FLAG, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER STATUS(MPI_STATUS_SIZE), IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_STATUS_SET_CANCELLED
!
        SUBROUTINE MPI_STATUS_SET_ELEMENTS(STATUS, DATATYPE, COUNT,    &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER STATUS(MPI_STATUS_SIZE), DATATYPE, COUNT, IERROR
        END SUBROUTINE MPI_STATUS_SET_ELEMENTS
!
        SUBROUTINE MPI_TYPE_CREATE_KEYVAL(TYPE_COPY_ATTR_FN,           &
      &   TYPE_DELETE_ATTR_FN, TYPE_KEYVAL, EXTRA_STATE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        EXTERNAL TYPE_COPY_ATTR_FN, TYPE_DELETE_ATTR_FN
        INTEGER TYPE_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) EXTRA_STATE
        END SUBROUTINE MPI_TYPE_CREATE_KEYVAL
!
        SUBROUTINE MPI_TYPE_DELETE_ATTR(TYPE, TYPE_KEYVAL, IERROR)
        INTEGER TYPE, TYPE_KEYVAL, IERROR
        END SUBROUTINE   MPI_TYPE_DELETE_ATTR
!
        SUBROUTINE MPI_TYPE_DUP(TYPE, NEWTYPE, IERROR)
        INTEGER TYPE, NEWTYPE, IERROR
        END SUBROUTINE MPI_TYPE_DUP
!
        SUBROUTINE MPI_TYPE_FREE_KEYVAL(TYPE_KEYVAL, IERROR)
        INTEGER TYPE_KEYVAL, IERROR
        END SUBROUTINE  MPI_TYPE_FREE_KEYVAL
!
        SUBROUTINE MPI_TYPE_GET_ATTR(TYPE, TYPE_KEYVAL, ATTRIBUTE_VAL, &
      &   FLAG, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER TYPE, TYPE_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        LOGICAL FLAG
        END SUBROUTINE   MPI_TYPE_GET_ATTR
!
        SUBROUTINE MPI_TYPE_GET_CONTENTS(DATATYPE, MAX_INTEGERS,       &
      &   MAX_ADDRESSES, MAX_DATATYPES, ARRAY_OF_INTEGERS,             &
      &   ARRAY_OF_ADDRESSES, ARRAY_OF_DATATYPES, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER DATATYPE, MAX_INTEGERS, MAX_ADDRESSES, MAX_DATATYPES,  &
      &   ARRAY_OF_INTEGERS(*), ARRAY_OF_DATATYPES(*), IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) ARRAY_OF_ADDRESSES(*)
        END SUBROUTINE MPI_TYPE_GET_CONTENTS
!
        SUBROUTINE MPI_TYPE_GET_ENVELOPE(DATATYPE, NUM_INTEGERS,       &
      &   NUM_ADDRESSES, NUM_DATATYPES, COMBINER, IERROR) 
        INTEGER DATATYPE, NUM_INTEGERS, NUM_ADDRESSES, NUM_DATATYPES,  &
      &   COMBINER, IERROR 
        END SUBROUTINE MPI_TYPE_GET_ENVELOPE
!
        SUBROUTINE MPI_TYPE_GET_NAME(TYPE, TYPE_NAME, RESULTLEN,       &
      &   IERROR) 
        INTEGER TYPE, RESULTLEN, IERROR
        CHARACTER*(*) TYPE_NAME
        END SUBROUTINE MPI_TYPE_GET_NAME
!
        SUBROUTINE MPI_TYPE_SET_ATTR(TYPE, TYPE_KEYVAL, ATTRIBUTE_VAL, &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER TYPE, TYPE_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        END SUBROUTINE  MPI_TYPE_SET_ATTR
!
        SUBROUTINE MPI_TYPE_SET_NAME(TYPE, TYPE_NAME, IERROR) 
        INTEGER TYPE, IERROR
        CHARACTER*(*) TYPE_NAME
        END SUBROUTINE MPI_TYPE_SET_NAME
!
        SUBROUTINE MPI_WIN_CALL_ERRHANDLER(WIN, ERRORCODE, IERROR)
        INTEGER WIN, ERRORCODE, IERROR
        END SUBROUTINE MPI_WIN_CALL_ERRHANDLER
!
        SUBROUTINE MPI_WIN_CREATE_KEYVAL(WIN_COPY_ATTR_FN,             &
      &   WIN_DELETE_ATTR_FN, WIN_KEYVAL, EXTRA_STATE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        EXTERNAL WIN_COPY_ATTR_FN, WIN_DELETE_ATTR_FN
        INTEGER WIN_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) EXTRA_STATE
        END SUBROUTINE MPI_WIN_CREATE_KEYVAL
!
        SUBROUTINE MPI_WIN_DELETE_ATTR(WIN, WIN_KEYVAL, IERROR)
        INTEGER WIN, WIN_KEYVAL, IERROR
        END SUBROUTINE   MPI_WIN_DELETE_ATTR
!
        SUBROUTINE MPI_WIN_FREE_KEYVAL(WIN_KEYVAL, IERROR)
        INTEGER WIN_KEYVAL, IERROR
        END SUBROUTINE  MPI_WIN_FREE_KEYVAL
!
        SUBROUTINE MPI_WIN_GET_ATTR(WIN, WIN_KEYVAL, ATTRIBUTE_VAL,    &
      &   FLAG, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER WIN, WIN_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        LOGICAL FLAG
        END SUBROUTINE   MPI_WIN_GET_ATTR
!
        SUBROUTINE MPI_WIN_GET_NAME(WIN, WIN_NAME, RESULTLEN, IERROR) 
        INTEGER WIN, RESULTLEN, IERROR
        CHARACTER*(*) WIN_NAME
        END SUBROUTINE MPI_WIN_GET_NAME
!
        SUBROUTINE MPI_WIN_SET_ATTR(WIN, WIN_KEYVAL, ATTRIBUTE_VAL,    &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER WIN, WIN_KEYVAL, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) ATTRIBUTE_VAL
        END SUBROUTINE  MPI_WIN_SET_ATTR
!
        SUBROUTINE MPI_WIN_SET_NAME(WIN, WIN_NAME, IERROR) 
        INTEGER WIN, IERROR
        CHARACTER*(*) WIN_NAME
        END SUBROUTINE MPI_WIN_SET_NAME
 
! Miscellany
!
        SUBROUTINE MPI_ALLOC_MEM(SIZE, INFO, BASEPTR, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER INFO, IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) SIZE, BASEPTR
        END SUBROUTINE MPI_ALLOC_MEM
!
        SUBROUTINE MPI_COMM_CREATE_ERRHANDLER(FUNCTION, ERRHANDLER,    &
      &   IERROR) 
        EXTERNAL FUNCTION 
        INTEGER ERRHANDLER, IERROR
        END SUBROUTINE MPI_COMM_CREATE_ERRHANDLER
!
        SUBROUTINE MPI_COMM_GET_ERRHANDLER(COMM, ERRHANDLER, IERROR) 
        INTEGER  COMM, ERRHANDLER, IERROR
        END SUBROUTINE MPI_COMM_GET_ERRHANDLER
!
        SUBROUTINE MPI_COMM_SET_ERRHANDLER(COMM, ERRHANDLER, IERROR) 
        INTEGER  COMM, ERRHANDLER, IERROR
        END SUBROUTINE MPI_COMM_SET_ERRHANDLER
!
        SUBROUTINE MPI_FILE_CREATE_ERRHANDLER(FUNCTION, ERRHANDLER,    &
      &   IERROR) 
        EXTERNAL FUNCTION 
        INTEGER ERRHANDLER, IERROR
        END SUBROUTINE MPI_FILE_CREATE_ERRHANDLER
!
        SUBROUTINE MPI_FILE_GET_ERRHANDLER(FILE, ERRHANDLER, IERROR) 
        INTEGER  FILE, ERRHANDLER, IERROR
        END SUBROUTINE MPI_FILE_GET_ERRHANDLER
!
        SUBROUTINE MPI_FILE_SET_ERRHANDLER(FILE, ERRHANDLER, IERROR) 
        INTEGER  FILE, ERRHANDLER, IERROR
        END SUBROUTINE MPI_FILE_SET_ERRHANDLER
!
        SUBROUTINE MPI_FINALIZED(FLAG, IERROR)
        LOGICAL FLAG
        INTEGER IERROR
        END SUBROUTINE MPI_FINALIZED
!
        SUBROUTINE MPI_INFO_CREATE(INFO, IERROR)
        INTEGER INFO, IERROR
        END SUBROUTINE MPI_INFO_CREATE
!
        SUBROUTINE MPI_INFO_DELETE(INFO, KEY, IERROR)
        INTEGER INFO, IERROR 
        CHARACTER*(*) KEY
        END SUBROUTINE MPI_INFO_DELETE
!
        SUBROUTINE MPI_INFO_DUP(INFO, NEWINFO, IERROR)
        INTEGER INFO, NEWINFO, IERROR
        END SUBROUTINE MPI_INFO_DUP
!
        SUBROUTINE MPI_INFO_FREE(INFO, IERROR) 
        INTEGER INFO, IERROR
        END SUBROUTINE MPI_INFO_FREE
!
        SUBROUTINE MPI_INFO_GET(INFO, KEY, VALUELEN, VALUE, FLAG,      &
      &   IERROR) 
        INTEGER INFO, VALUELEN, IERROR 
        CHARACTER*(*) KEY, VALUE 
        LOGICAL FLAG
        END SUBROUTINE MPI_INFO_GET
!
        SUBROUTINE MPI_INFO_GET_NKEYS(INFO, NKEYS, IERROR)
        INTEGER INFO, NKEYS, IERROR
        END SUBROUTINE MPI_INFO_GET_NKEYS
!
        SUBROUTINE MPI_INFO_GET_NTHKEY(INFO, N, KEY, IERROR)
        INTEGER INFO, N, IERROR 
        CHARACTER*(*) KEY
        END SUBROUTINE MPI_INFO_GET_NTHKEY
!
        SUBROUTINE MPI_INFO_GET_VALUELEN(INFO, KEY, VALUELEN, FLAG,    &
      &   IERROR) 
        INTEGER INFO, VALUELEN, IERROR 
        LOGICAL FLAG 
        CHARACTER*(*) KEY
        END SUBROUTINE MPI_INFO_GET_VALUELEN
!
        SUBROUTINE MPI_INFO_SET(INFO, KEY, VALUE, IERROR)
        INTEGER INFO, IERROR 
        CHARACTER*(*) KEY, VALUE
        END SUBROUTINE MPI_INFO_SET
!
        SUBROUTINE MPI_PACK_EXTERNAL_SIZE(DATAREP, INCOUNT, DATATYPE,  &
      &   SIZE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER INCOUNT, DATATYPE, IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) SIZE 
        CHARACTER*(*) DATAREP 
        END SUBROUTINE MPI_PACK_EXTERNAL_SIZE
!
        SUBROUTINE MPI_REQUEST_GET_STATUS( REQUEST, FLAG, STATUS,      &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        INTEGER REQUEST, STATUS(MPI_STATUS_SIZE), IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_REQUEST_GET_STATUS
!
        SUBROUTINE MPI_TYPE_CREATE_DARRAY(SIZE, RANK, NDIMS,           &
      &   ARRAY_OF_GSIZES, ARRAY_OF_DISTRIBS, ARRAY_OF_DARGS,          &
      &   ARRAY_OF_PSIZES, ORDER, OLDTYPE, NEWTYPE, IERROR) 
        INTEGER SIZE, RANK, NDIMS, ARRAY_OF_GSIZES(*),                 &
      &   ARRAY_OF_DISTRIBS(*), ARRAY_OF_DARGS(*), ARRAY_OF_PSIZES(*), &
      &   ORDER, OLDTYPE, NEWTYPE, IERROR 
        END SUBROUTINE MPI_TYPE_CREATE_DARRAY
!
        SUBROUTINE MPI_TYPE_CREATE_HINDEXED(COUNT,                     &
      &   ARRAY_OF_BLOCKLENGTHS, ARRAY_OF_DISPLACEMENTS, OLDTYPE,      &
      &   NEWTYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER COUNT, ARRAY_OF_BLOCKLENGTHS(*), OLDTYPE, NEWTYPE,     &
      &   IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) ARRAY_OF_DISPLACEMENTS(*)
        END SUBROUTINE MPI_TYPE_CREATE_HINDEXED
!
        SUBROUTINE MPI_TYPE_CREATE_HVECTOR(COUNT, BLOCKLENGTH, STRIDE, &
      &   OLDTYPE, NEWTYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER COUNT, BLOCKLENGTH, OLDTYPE, NEWTYPE, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) STRIDE
        END SUBROUTINE MPI_TYPE_CREATE_HVECTOR
!
        SUBROUTINE MPI_TYPE_CREATE_INDEXED_BLOCK(COUNT, BLOCKLENGTH,   &
      &   ARRAY_OF_DISPLACEMENTS, OLDTYPE, NEWTYPE, IERROR) 
        INTEGER COUNT, BLOCKLENGTH, ARRAY_OF_DISPLACEMENTS(*),         &
      &   OLDTYPE, NEWTYPE, IERROR 
        END SUBROUTINE MPI_TYPE_CREATE_INDEXED_BLOCK
!
        SUBROUTINE MPI_TYPE_CREATE_RESIZED(OLDTYPE, LB, EXTENT,        &
      &   NEWTYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER OLDTYPE,  NEWTYPE, IERROR
        INTEGER(KIND=MPI_ADDRESS_KIND) LB, EXTENT
        END SUBROUTINE MPI_TYPE_CREATE_RESIZED
!
        SUBROUTINE MPI_TYPE_CREATE_STRUCT(COUNT,                       &
      &   ARRAY_OF_BLOCKLENGTHS, ARRAY_OF_DISPLACEMENTS,               &
      &   ARRAY_OF_TYPES, NEWTYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER COUNT, ARRAY_OF_BLOCKLENGTHS(*), ARRAY_OF_TYPES(*),    &
      &   NEWTYPE, IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) ARRAY_OF_DISPLACEMENTS(*)
        END SUBROUTINE MPI_TYPE_CREATE_STRUCT
!
        SUBROUTINE MPI_TYPE_CREATE_SUBARRAY(NDIMS, ARRAY_OF_SIZES,     &
      &   ARRAY_OF_SUBSIZES, ARRAY_OF_STARTS, ORDER, OLDTYPE, NEWTYPE, &
      &   IERROR) 
        INTEGER NDIMS, ARRAY_OF_SIZES(*), ARRAY_OF_SUBSIZES(*),        &
      &   ARRAY_OF_STARTS(*), ORDER, OLDTYPE, NEWTYPE, IERROR 
        END SUBROUTINE MPI_TYPE_CREATE_SUBARRAY
!
        SUBROUTINE MPI_TYPE_GET_EXTENT(DATATYPE, LB, EXTENT, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER  DATATYPE,  IERROR
        INTEGER(KIND = MPI_ADDRESS_KIND) LB, EXTENT
        END SUBROUTINE MPI_TYPE_GET_EXTENT
!
        SUBROUTINE MPI_TYPE_GET_TRUE_EXTENT(DATATYPE, TRUE_LB,         &
      &   TRUE_EXTENT, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER DATATYPE,  IERROR
        INTEGER(KIND = MPI_ADDRESS_KIND) TRUE_LB, TRUE_EXTENT
        END SUBROUTINE MPI_TYPE_GET_TRUE_EXTENT
!
        SUBROUTINE MPI_WIN_CREATE_ERRHANDLER(FUNCTION, ERRHANDLER,     &
      &   IERROR) 
        EXTERNAL FUNCTION 
        INTEGER ERRHANDLER, IERROR
        END SUBROUTINE MPI_WIN_CREATE_ERRHANDLER
!
        SUBROUTINE MPI_WIN_GET_ERRHANDLER(WIN, ERRHANDLER, IERROR) 
        INTEGER WIN, ERRHANDLER, IERROR
        END SUBROUTINE MPI_WIN_GET_ERRHANDLER
!
        SUBROUTINE MPI_WIN_SET_ERRHANDLER(WIN, ERRHANDLER, IERROR) 
        INTEGER WIN, ERRHANDLER, IERROR
        END SUBROUTINE MPI_WIN_SET_ERRHANDLER
 
! I/O
!
        SUBROUTINE MPI_FILE_CLOSE(FH, IERROR)
        INTEGER FH, IERROR
        END SUBROUTINE MPI_FILE_CLOSE
!
        SUBROUTINE MPI_FILE_DELETE(FILENAME, INFO, IERROR)
        CHARACTER*(*) FILENAME 
        INTEGER INFO, IERROR
        END SUBROUTINE MPI_FILE_DELETE
!
        SUBROUTINE MPI_FILE_GET_AMODE(FH, AMODE, IERROR)
        INTEGER FH, AMODE, IERROR
        END SUBROUTINE MPI_FILE_GET_AMODE
!
        SUBROUTINE MPI_FILE_GET_ATOMICITY(FH, FLAG, IERROR)
        INTEGER FH, IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_FILE_GET_ATOMICITY
!
        SUBROUTINE MPI_FILE_GET_BYTE_OFFSET(FH, OFFSET, DISP, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET, DISP
        END SUBROUTINE MPI_FILE_GET_BYTE_OFFSET
!
        SUBROUTINE MPI_FILE_GET_GROUP(FH, GROUP, IERROR)
        INTEGER FH, GROUP, IERROR
        END SUBROUTINE MPI_FILE_GET_GROUP
!
        SUBROUTINE MPI_FILE_GET_INFO(FH, INFO_USED, IERROR)
        INTEGER FH, INFO_USED, IERROR
        END SUBROUTINE MPI_FILE_GET_INFO
!
        SUBROUTINE MPI_FILE_GET_POSITION(FH, OFFSET, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        END SUBROUTINE MPI_FILE_GET_POSITION
!
        SUBROUTINE MPI_FILE_GET_POSITION_SHARED(FH, OFFSET, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        END SUBROUTINE MPI_FILE_GET_POSITION_SHARED
!
        SUBROUTINE MPI_FILE_GET_SIZE(FH, SIZE, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) SIZE
        END SUBROUTINE MPI_FILE_GET_SIZE
!
        SUBROUTINE MPI_FILE_GET_TYPE_EXTENT(FH, DATATYPE, EXTENT,      &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        INTEGER FH, DATATYPE, IERROR 
        INTEGER(KIND=MPI_ADDRESS_KIND) EXTENT
        END SUBROUTINE MPI_FILE_GET_TYPE_EXTENT
!
        SUBROUTINE MPI_FILE_GET_VIEW(FH, DISP, ETYPE, FILETYPE,        &
      &   DATAREP, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, ETYPE, FILETYPE, IERROR 
        CHARACTER*(*) DATAREP
        INTEGER(KIND=MPI_OFFSET_KIND) DISP
        END SUBROUTINE MPI_FILE_GET_VIEW
!
        SUBROUTINE MPI_FILE_OPEN(COMM, FILENAME, AMODE, INFO, FH,      &
      &   IERROR) 
        CHARACTER*(*) FILENAME 
        INTEGER COMM, AMODE, INFO, FH, IERROR
        END SUBROUTINE MPI_FILE_OPEN
!
        SUBROUTINE MPI_FILE_PREALLOCATE(FH, SIZE, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) SIZE
        END SUBROUTINE MPI_FILE_PREALLOCATE
!
        SUBROUTINE MPI_FILE_SEEK(FH, OFFSET, WHENCE, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, WHENCE, IERROR
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        END SUBROUTINE MPI_FILE_SEEK
!
        SUBROUTINE MPI_FILE_SEEK_SHARED(FH, OFFSET, WHENCE, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, WHENCE, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        END SUBROUTINE MPI_FILE_SEEK_SHARED
!
        SUBROUTINE MPI_FILE_SET_ATOMICITY(FH, FLAG, IERROR)
        INTEGER FH, IERROR
        LOGICAL FLAG
        END SUBROUTINE MPI_FILE_SET_ATOMICITY
!
        SUBROUTINE MPI_FILE_SET_INFO(FH, INFO, IERROR)
        INTEGER FH, INFO, IERROR
        END SUBROUTINE MPI_FILE_SET_INFO
!
        SUBROUTINE MPI_FILE_SET_SIZE(FH, SIZE, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) SIZE
        END SUBROUTINE MPI_FILE_SET_SIZE
!
        SUBROUTINE MPI_FILE_SET_VIEW(FH, DISP, ETYPE, FILETYPE,        &
      &   DATAREP, INFO, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        INTEGER FH, ETYPE, FILETYPE, INFO, IERROR 
        CHARACTER*(*) DATAREP 
        INTEGER(KIND=MPI_OFFSET_KIND) DISP
        END SUBROUTINE MPI_FILE_SET_VIEW
!
        SUBROUTINE MPI_FILE_SYNC(FH, IERROR)
        INTEGER FH, IERROR
        END SUBROUTINE MPI_FILE_SYNC
!
        SUBROUTINE MPI_REGISTER_DATAREP(DATAREP, READ_CONVERSION_FN,   &
      &   WRITE_CONVERSION_FN, DTYPE_FILE_EXTENT_FN, EXTRA_STATE,      &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_ADDRESS_KIND
        CHARACTER*(*) DATAREP 
        EXTERNAL READ_CONVERSION_FN, WRITE_CONVERSION_FN,              &
      &   DTYPE_FILE_EXTENT_FN 
        INTEGER(KIND=MPI_ADDRESS_KIND) EXTRA_STATE 
        INTEGER IERROR
        END SUBROUTINE MPI_REGISTER_DATAREP
        END INTERFACE
        END MODULE MPI2
