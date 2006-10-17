/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

#ifndef MPIR_ROUTER_H
#define MPIR_ROUTER_H

#include <mpi.h>

#include <stdio.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h> 
#endif

#ifdef WIN32
#include "pthread.h"
#else
#include <pthread.h>
#endif
#include "rdebug.h"
#include "intqueue.h"
#include "router_config.h"

/* prototype of the router function */
int MPIR_Router (char*, int, int, char *);

extern  int MPID_router_byte_order;

/* router can be compiled standalone */
/*#define WITH_MPI*/

/* this is the minimal size of messages to send over multiple sockets */
#define SPLIT_SIZE 1500
/* Port */
#define TCP_PORTBASE 2500

#define SWAP2(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); } 

/* you can define the standard exchange order here! */
#define L_ENDIAN 0
#define B_ENDIAN 1 
#define DEF_EXCHANGE_ORDER L_ENDIAN

/* initial size of send/receive buffers */
#define INIT_ROUTER_BUFFER_SIZE (1024*512)
/* max number of routers on a host */
#define MAX_ROUTER_NUM 20
/* max number of threads in a router (should equal MAX_CONN_CONT */  
#define MAX_THREAD_NUM 20

/* XXX more than 1 Thread per connection is not yet tested -> use multiple
   sockets with each connection instead */
#define THREADS_PER_CONNECTION 1
/* max number of outstanding MPI_Isend for the threads */
#define ISEND_NUM_DEFAULT  11

/* Messages are split to packets of size PIPELINE_BLOCKSIZE and sent through a pipeline */
/* XXX only works with threaded router because send_message may block and stop switching
   between import and export messages. see: Pipeline receive loop in export_msgs() */
/*#define META_PIPELINE */
#define PIPELINE_BLOCKSIZE (1024*8)

/* states of the router */
#define MPI_APP_RUNNING 0
#define MPI_APP_FINALIZE 1


/* MPI message TAGs - used to recognize different types  of messages that are sent 
   to the router via MPI. However, the mode entry in the Meta_Header serves a 
   similar purpose and is used for inter-router messages (which aren't MPI messages);
   to not interfere with application messages, messages with these tags may only be sent via the
   internal communicators MPI_COMM_HOST and MPI_COMM_ALL, which are not meant to be used
   by MPI application programmers! */
#define MPIR_MPIMSG_TAG               1
#define MPIR_ROUTMSG_TAG              2
#define MPIR_SEPARATE_MSG_TAG         3
#define MPIR_SEPARATE_META_HEADER_TAG 4
#define MPIR_CANCEL_CONFIRM_TAG       5
#define MPIR_UNKNOWN_TAG              6

/* Align to the next 32-Bit value. SPARC likes that. */
#define DWORD_ALIGN(x) { \
   if ((x) & 3) \
        x = ((((x) >> 2)+1) << 2); \
}

/* host_to_exch_int_struct changes an array of integers from
   host to exchange order, if necessary */
void host_to_exch_int_struct(void * , int );


/*  Meta_Header preceeds the MPI message the gateway ADI sends to the router
    
    The tag, src and context_id, type and count are the values of the original message.
    The sendmode is required to provide the desired behaviour (blocking, nonblocking, ...)
    of a send-operation. This is done via a special two-way protocol between the 
    Gateway-device on the one host and the Tunnel-device on the other host  */

typedef enum _MPI_Sendmode { BLOCKING, NONBLOCKING, CANCEL, CANCEL_CONFIRM } MPI_Sendmode;
typedef enum _MPIR_GW_mode      { MPI_MSG = 2, ROUTER_MSG = 3 } MPIR_GW_mode;
typedef enum _MPIR_Router_cmd   { SYNC, RNDV, FINALIZE } MPIR_Router_cmd;

/* args to inter-Router SYNC command  */
#define SYNC_REQUEST 0
#define SYNC_OK      1
#define SYNC_NOTYET  2

/* SGWMessage is the header which gets prepended to each
   MPI message which is transmitted between hosts */

/* the following structs are shared between hosts with
   different byteorder, i.e. the members have to be swapped
   to a defined order before they are sent. This is done
   assuming the structs containing _only_ integers.
   So don't change this! */
   
typedef struct _GW_MPI_msg {
    int src_comm_lrank;
    int dest_grank;
    int tag;         
    int context_id;
    MPI_Sendmode mode;
    unsigned int count;            /* byte-size of the original msg (appended to this struct) */
    int msgrep;      
    unsigned int msgid;                 /* id for cancelling */
} GW_MPI_msg;

typedef struct _GW_Router_msg {
    MPIR_Router_cmd command;
    int src;
    int dest;
    int arg;
} GW_Router_msg;

typedef struct _Meta_Header {
    MPIR_GW_mode mode;
    union {
	GW_MPI_msg     MPI;
	GW_Router_msg  Rout;
    } msg;
    unsigned char dummychar;
} Meta_Header;
/* defined in mpid/ch_gateway/gatewaypriv.c */
extern Meta_Header  *meta_msg;
extern size_t       meta_msgsize;


/* SThreadArgs contains the thread-specific parameters for
   the threads handling the import/export of messages */
struct SThreadArgs_import {
    int thread_id;
    int connection;

    IntQueue availQ;
    IntQueue pendingQ;

    MPI_Status *snd_status;
    MPI_Request *snd_request;

    char **router_msg; /* array of pointers to buffers for incoming messages */
    int *bufsize;      /* array for actual length of buffers (may be changing during execution) */
};

struct SThreadArgs_export {
    int thread_id;
    int connection;

    int my_comm_host_rank;
    int send_context;

    char **router_msg; /* array of pointers to buffers for outgoing messages */
    int *bufsize;      /* array for actual length of buffers (may be changing during execution) */
};

typedef int THost;
typedef int TRouter;

#endif
