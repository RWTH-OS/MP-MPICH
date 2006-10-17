/* $Id$ */

#ifndef __LOG_WRAP_H__
#define __LOG_WRAP_H__

#ifdef MPI_BUILD_PROFILING
#undef MPI_BUILD_PROFILING
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"
#include "mpe.h"
#include "mpeconf.h"

/* Enable memory tracing.  This requires MPICH's mpid/util/tr2.c codes */
#if defined(MPIR_MEMDEBUG)
#define malloc(a)    MPID_trmalloc((size_t)(a),__LINE__,__FILE__)
#define free(a)      MPID_trfree(a,__LINE__,__FILE__)
#endif

typedef struct {
    int n_calls,      /* Number of times this state used */
        is_active,    /* Allows each state to be selectively switched off */
        id,           /* Id number of state */
        kind_mask;    /* Indicates kind of state (message, environment) */
    char *name;       /* Pointer to name */
    char *color;      /* Color (or B&W representation) */
} MPE_State;

/* Global trace control */
extern int _mpe_trace_on;

/* Kind_mask values */
#define MPE_KIND_MSG 0x1
#define MPE_KIND_TOPO 0x2
#define MPE_KIND_COLL 0x4
#define MPE_KIND_DATATYPE 0x8
#define MPE_KIND_ENV 0x10
#define MPE_KIND_COMM_INFO 0x20
#define MPE_KIND_COMM 0x40
#define MPE_KIND_ATTR 0x80
#define MPE_KIND_GROUP 0x100
#define MPE_KIND_MSG_INIT 0x200
#define MPE_KIND_FILE 0x400
#define MPE_KIND_ADIO 0x800
/* More as needed */

/* Number of MPI routines; increase to allow user extensions */
#define MPE_MAX_STATES 220
extern MPE_State _mpe_states[MPE_MAX_STATES];

#define MPE_ALLGATHER_ID 0
#define MPE_ALLGATHERV_ID 1
#define MPE_ALLREDUCE_ID 2
#define MPE_ALLTOALL_ID 3
#define MPE_ALLTOALLV_ID 4
#define MPE_BARRIER_ID 5
#define MPE_BCAST_ID 6
#define MPE_GATHER_ID 7
#define MPE_GATHERV_ID 8
#define MPE_OP_CREATE_ID 9
#define MPE_OP_FREE_ID 10
#define MPE_REDUCE_SCATTER_ID 11
#define MPE_REDUCE_ID 12
#define MPE_SCAN_ID 13
#define MPE_SCATTER_ID 14
#define MPE_SCATTERV_ID 15
#define MPE_ATTR_DELETE_ID 16
#define MPE_ATTR_GET_ID 17
#define MPE_ATTR_PUT_ID 18
#define MPE_COMM_COMPARE_ID 19
#define MPE_COMM_CREATE_ID 20
#define MPE_COMM_DUP_ID 21
#define MPE_COMM_FREE_ID 22
#define MPE_COMM_GROUP_ID 23
#define MPE_COMM_RANK_ID 24
#define MPE_COMM_REMOTE_GROUP_ID 25
#define MPE_COMM_REMOTE_SIZE_ID 26
#define MPE_COMM_SIZE_ID 27
#define MPE_COMM_SPLIT_ID 28
#define MPE_COMM_TEST_INTER_ID 29
#define MPE_GROUP_COMPARE_ID 30
#define MPE_GROUP_DIFFERENCE_ID 31
#define MPE_GROUP_EXCL_ID 32
#define MPE_GROUP_FREE_ID 33
#define MPE_GROUP_INCL_ID 34
#define MPE_GROUP_INTERSECTION_ID 35
#define MPE_GROUP_RANK_ID 36
#define MPE_GROUP_RANGE_EXCL_ID 37
#define MPE_GROUP_RANGE_INCL_ID 38
#define MPE_GROUP_SIZE_ID 39
#define MPE_GROUP_TRANSLATE_RANKS_ID 40
#define MPE_GROUP_UNION_ID 41
#define MPE_INTERCOMM_CREATE_ID 42
#define MPE_INTERCOMM_MERGE_ID 43
#define MPE_KEYVAL_CREATE_ID 44
#define MPE_KEYVAL_FREE_ID 45
#define MPE_ABORT_ID 46
#define MPE_ERROR_CLASS_ID 47
#define MPE_ERRHANDLER_CREATE_ID 48
#define MPE_ERRHANDLER_FREE_ID 49
#define MPE_ERRHANDLER_GET_ID 50
#define MPE_ERROR_STRING_ID 51
#define MPE_ERRHANDLER_SET_ID 52
#define MPE_GET_PROCESSOR_NAME_ID 53
#define MPE_INITIALIZED_ID 54
#define MPE_WTICK_ID 55
#define MPE_WTIME_ID 56
#define MPE_ADDRESS_ID 57
#define MPE_BSEND_ID 58
#define MPE_BSEND_INIT_ID 59
#define MPE_BUFFER_ATTACH_ID 60
#define MPE_BUFFER_DETACH_ID 61
#define MPE_CANCEL_ID 62
#define MPE_REQUEST_FREE_ID 63
#define MPE_RECV_INIT_ID 64
#define MPE_SEND_INIT_ID 65
#define MPE_GET_ELEMENTS_ID 66
#define MPE_GET_COUNT_ID 67
#define MPE_IBSEND_ID 68
#define MPE_IPROBE_ID 69
#define MPE_IRECV_ID 70
#define MPE_IRSEND_ID 71
#define MPE_ISEND_ID 72
#define MPE_ISSEND_ID 73
#define MPE_PACK_ID 74
#define MPE_PACK_SIZE_ID 75
#define MPE_PROBE_ID 76
#define MPE_RECV_ID 77
#define MPE_RSEND_ID 78
#define MPE_RSEND_INIT_ID 79
#define MPE_SEND_ID 80
#define MPE_SENDRECV_ID 81
#define MPE_SENDRECV_REPLACE_ID 82
#define MPE_SSEND_ID 83
#define MPE_SSEND_INIT_ID 84
#define MPE_START_ID 85
#define MPE_STARTALL_ID 86
#define MPE_TEST_ID 87
#define MPE_TESTALL_ID 88
#define MPE_TESTANY_ID 89
#define MPE_TEST_CANCELLED_ID 90
#define MPE_TESTSOME_ID 91
#define MPE_TYPE_COMMIT_ID 92
#define MPE_TYPE_CONTIGUOUS_ID 93
#define MPE_TYPE_EXTENT_ID 94
#define MPE_TYPE_FREE_ID 95
#define MPE_TYPE_HINDEXED_ID 96
#define MPE_TYPE_HVECTOR_ID 97
#define MPE_TYPE_INDEXED_ID 98
#define MPE_TYPE_LB_ID 99
#define MPE_TYPE_SIZE_ID 100
#define MPE_TYPE_STRUCT_ID 101
#define MPE_TYPE_UB_ID 102
#define MPE_TYPE_VECTOR_ID 103
#define MPE_UNPACK_ID 104
#define MPE_WAIT_ID 105
#define MPE_WAITALL_ID 106
#define MPE_WAITANY_ID 107
#define MPE_WAITSOME_ID 108
#define MPE_CART_COORDS_ID 109
#define MPE_CART_CREATE_ID 110
#define MPE_CART_GET_ID 111
#define MPE_CART_MAP_ID 112
#define MPE_CART_SHIFT_ID 113
#define MPE_CARTDIM_GET_ID 114
#define MPE_DIMS_CREATE_ID 115
#define MPE_GRAPH_CREATE_ID 116
#define MPE_GRAPH_GET_ID 117
#define MPE_GRAPH_MAP_ID 118
#define MPE_GRAPH_NEIGHBORS_ID 119
#define MPE_GRAPH_NEIGHBORS_COUNT_ID 120
#define MPE_GRAPHDIMS_GET_ID 121
#define MPE_TOPO_TEST_ID 122
#define MPE_RECV_IDLE_ID 123
#define MPE_CART_RANK_ID 124
#define MPE_CART_SUB_ID 125
/* Ids for mpi-io functions */
#define MPE_FILE_CALL_ERRHANDLER_ID 126
#define MPE_FILE_CLOSE_ID 127
#define MPE_FILE_CREATE_ERRHANDLER_ID 128
#define MPE_FILE_DELETE_ID 129
#define MPE_FILE_GET_AMODE_ID 130
#define MPE_FILE_GET_ATOMICITY_ID 131
#define MPE_FILE_GET_BYTE_OFFSET_ID 132
#define MPE_FILE_GET_ERRHANDLER_ID 133
#define MPE_FILE_GET_GROUP_ID 134
#define MPE_FILE_GET_INFO_ID 135
#define MPE_FILE_GET_POSITION_ID 136
#define MPE_FILE_GET_POSITION_SHARED_ID 137
#define MPE_FILE_GET_SIZE_ID 138
#define MPE_FILE_GET_TYPE_EXTENT_ID 139
#define MPE_FILE_GET_VIEW_ID 140
#define MPE_FILE_IREAD_ID 141
#define MPE_FILE_IREAD_AT_ID 142
#define MPE_FILE_IREAD_SHARED_ID 143
#define MPE_FILE_IWRITE_ID 144
#define MPE_FILE_IWRITE_AT_ID 145
#define MPE_FILE_IWRITE_SHARED_ID 146
#define MPE_FILE_OPEN_ID 147
#define MPE_FILE_PREALLOCATE_ID 148
#define MPE_FILE_READ_ID 149
#define MPE_FILE_READ_ALL_ID 150
#define MPE_FILE_READ_ALL_BEGIN_ID 151
#define MPE_FILE_READ_ALL_END_ID 152
#define MPE_FILE_READ_AT_ID 153
#define MPE_FILE_READ_AT_ALL_ID 154
#define MPE_FILE_READ_AT_ALL_BEGIN_ID 155
#define MPE_FILE_READ_AT_ALL_END_ID 156
#define MPE_FILE_READ_ORDERED_ID 157
#define MPE_FILE_READ_ORDERED_BEGIN_ID 158
#define MPE_FILE_READ_ORDERED_END_ID 159
#define MPE_FILE_READ_SHARED_ID 160
#define MPE_FILE_SEEK_ID 161
#define MPE_FILE_SEEK_SHARED_ID 162
#define MPE_FILE_SET_ATOMICITY_ID 163
#define MPE_FILE_SET_ERRHANDLER_ID 164
#define MPE_FILE_SET_INFO_ID 165
#define MPE_FILE_SET_SIZE_ID 166
#define MPE_FILE_SET_VIEW_ID 167
#define MPE_FILE_SYNC_ID 168
#define MPE_FILE_WRITE_ID 169
#define MPE_FILE_WRITE_ALL_ID 170
#define MPE_FILE_WRITE_ALL_BEGIN_ID 171
#define MPE_FILE_WRITE_ALL_END_ID 172
#define MPE_FILE_WRITE_AT_ID 173
#define MPE_FILE_WRITE_AT_ALL_ID 174
#define MPE_FILE_WRITE_AT_ALL_BEGIN_ID 175
#define MPE_FILE_WRITE_AT_ALL_END_ID 176
#define MPE_FILE_WRITE_ORDERED_ID 177
#define MPE_FILE_WRITE_ORDERED_BEGIN_ID 178
#define MPE_FILE_WRITE_ORDERED_END_ID 179
#define MPE_FILE_WRITE_SHARED_ID 180

/* Ids for ADIO functions */
/* XXX do we really need different IDs for all the different filesystems? 
   Using the same ones for UFS and NFS. */

/* PVFS */
#define MPE_PVFS_CLOSE_ID 200
#define MPE_PVFS_READDONE_ID 201
#define MPE_PVFS_WRITEDONE_ID 202
#define MPE_PVFS_FCNTL_ID 203
#define MPE_PVFS_FLUSH_ID 204
#define MPE_PVFS_SETINFO_ID 205
#define MPE_PVFS_IREADCONTIG_ID 206
#define MPE_PVFS_IREADSTRIDED_ID 207
#define MPE_PVFS_IWRITECONTIG_ID 208
#define MPE_PVFS_IWRITESTRIDED_ID 209
#define MPE_PVFS_OPEN_ID 210
#define MPE_PVFS_READSTRIDEDCOLL_ID 211
#define MPE_PVFS_READCONTIG_ID 212
#define MPE_PVFS_READSTRIDED_ID 213
#define MPE_PVFS_RESIZE_ID 214
#define MPE_PVFS_SEEKINDIVIDUAL_ID 215
#define MPE_PVFS_READCOMPLETE_ID 216
#define MPE_PVFS_WRITECOMPLETE_ID 217
#define MPE_PVFS_WRITESTRIDEDCOLL_ID 218
#define MPE_PVFS_WRITECONTIG_ID 219
#define MPE_PVFS_WRITESTRIDED_ID 220
/* NFS */
#define MPE_NFS_CLOSE_ID 221
#define MPE_NFS_READDONE_ID 222
#define MPE_NFS_WRITEDONE_ID 223
#define MPE_NFS_FCNTL_ID 224
#define MPE_NFS_FLUSH_ID 225
#define MPE_NFS_SETINFO_ID 226
#define MPE_NFS_IREADCONTIG_ID 227
#define MPE_NFS_IREADSTRIDED_ID 228
#define MPE_NFS_IWRITECONTIG_ID 229
#define MPE_NFS_IWRITESTRIDED_ID 230
#define MPE_NFS_OPEN_ID 231
#define MPE_NFS_READSTRIDEDCOLL_ID 232
#define MPE_NFS_READCONTIG_ID 233
#define MPE_NFS_READSTRIDED_ID 234
#define MPE_NFS_RESIZE_ID 235
#define MPE_NFS_SEEKINDIVIDUAL_ID 236
#define MPE_NFS_READCOMPLETE_ID 237
#define MPE_NFS_WRITECOMPLETE_ID 238
#define MPE_NFS_WRITESTRIDEDCOLL_ID 239
#define MPE_NFS_WRITECONTIG_ID 240
#define MPE_NFS_WRITESTRIDED_ID 241
#define MPE_NFS_GET_SHARED_FP_ID 242
#define MPE_NFS_SET_SHARED_FP_ID 243
/* UFS */
#define MPE_UFS_CLOSE_ID 221
#define MPE_UFS_READDONE_ID 222
#define MPE_UFS_WRITEDONE_ID 223
#define MPE_UFS_FCNTL_ID 224
#define MPE_UFS_FLUSH_ID 225
#define MPE_UFS_SETINFO_ID 226
#define MPE_UFS_IREADCONTIG_ID 227
#define MPE_UFS_IREADSTRIDED_ID 228
#define MPE_UFS_IWRITECONTIG_ID 229
#define MPE_UFS_IWRITESTRIDED_ID 230
#define MPE_UFS_OPEN_ID 231
#define MPE_UFS_READSTRIDEDCOLL_ID 232
#define MPE_UFS_READCONTIG_ID 233
#define MPE_UFS_READSTRIDED_ID 234
#define MPE_UFS_RESIZE_ID 235
#define MPE_UFS_SEEKINDIVIDUAL_ID 236
#define MPE_UFS_READCOMPLETE_ID 237
#define MPE_UFS_WRITECOMPLETE_ID 238
#define MPE_UFS_WRITESTRIDEDCOLL_ID 239
#define MPE_UFS_WRITECONTIG_ID 240
#define MPE_UFS_WRITESTRIDED_ID 241
#define MPE_UFS_GET_SHARED_FP_ID 242
#define MPE_UFS_SET_SHARED_FP_ID 243


#include "requests.h"

static request_list *requests_head_0, *requests_tail_0, *requests_avail_0=0;
static int procid_0;
static char logFileName_0[256];

/* This is used for the multiple-completion test/wait functions */
#define MPE_MAX_REQUESTS 1024
static MPI_Request req[MPE_MAX_REQUESTS];

/* Function prototypes */
void MPE_Add_send_req ( int, MPI_Datatype, int, int, MPI_Request, int );
void MPE_Add_recv_req ( int, MPI_Datatype, int, int, MPI_Request, int );
void MPE_Cancel_req ( MPI_Request );
void MPE_Remove_req ( MPI_Request );
void MPE_Start_req ( MPI_Request, MPE_State * );
void MPE_ProcessWaitTest ( MPI_Request, MPI_Status *, char *, MPE_State * );

/*
   Temporary MPE log definitions (eventually will replace with more
   permanent changes)
   Note that these include a communicator as well as the state (pointer
   to predefined state structure).  Use MPI_COMM_NULL for no communicator
 */
#define MPE_Log_state_begin( comm, state ) \
    MPE_Log_event( state->id, state->n_calls, (char *)0 )
#define MPE_Log_state_end( comm, state ) \
    MPE_Log_event( state->id + 1, state->n_calls, (char *)0 )

/* To use these, declare
     register MPE_State *state;
   and call around routine */
#define MPE_LOG_STATE_BEGIN(name,comm) \
  if (_mpe_trace_on) {\
      state = &_mpe_states[name];\
      if (state->is_active) {\
      	  state->n_calls++;\
          MPE_Log_state_begin( comm, state );\
      }\
  }
#define MPE_LOG_STATE_END(comm) \
  if (_mpe_trace_on && state->is_active) {\
      	  state->n_calls++;\
          MPE_Log_state_end( comm, state );\
      }
#define MPE_LOG_DO(call) \
    if (_mpe_trace_on && state->is_active) { call ; }



/*
  level = 1 turns on tracing, 
  level = 0 turns it off.

  Still to do: in some cases, must log communicator operations even if
  logging is off.
 */
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES) || \
    defined(PCONTROL_NEEDS_CONST)
#ifdef HAVE_NO_C_CONST
int MPI_Pcontrol( int level, ... );
#else
int MPI_Pcontrol( const int level, ... );
#endif
#else
#ifdef HAVE_NO_C_CONST
int MPI_Pcontrol( level );
int level;
#else
int MPI_Pcontrol(const int level, ...);
#endif
#endif

#endif
