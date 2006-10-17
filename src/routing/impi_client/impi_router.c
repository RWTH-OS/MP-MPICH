
#include "impi_router.h"
#include "impi_common.h"

#include <mpi.h>
#include <metaconfig.h>

#include "connection.h"
#include "mpi_router.h"
#include "conn_common.h"
#include "mpid.h"
#include "../../mpid/ch2/packets.h"
#include "rhlist.h"
#include "prof_timer.h"
#include "reqalloc.h"
#include "sendq.h"

#undef  IMPI_MODULE_NAME
#define IMPI_MODULE_NAME "Router"

int IMPI_Client(int argc, char* argv[]);
int IMPI_Host(int*);

int IMPI_Router(int argc, char* argv[])
{
  struct MPIR_COMMUNICATOR *comm_ptr;
  int all, world, local;

  MPI_Comm_rank(MPI_COMM_ALL, &all);
  MPI_Comm_rank(MPI_COMM_WORLD, &world);
  MPI_Comm_rank(MPI_COMM_LOCAL, &local);

  IMPI_Client(argc, argv);

  printf("My Ranks are:  a%d w%d l%d\n", all, world, local);

  comm_ptr = MPIR_GET_COMM_PTR(MPI_COMM_LOCAL);      
  IMPI_Host(comm_ptr->lrank_to_grank);

  return 0;
}


#define ROUTER_ABORT {PRERROR("Router Init failed -Router Process calls local MPI_Abort\n");MPI_Abort(MPI_COMM_HOST, 99); exit(-1);}
static char *IMPI_adjustbuffer(char *buf, size_t actualsize, size_t newsize);

int IMPI_Gateway_export(int *src_comm_lrank, int *dest_grank, int *tag, size_t *length, void **buffer)
{
  int gateway_flag = 0;

  int i;
  int iprobe_flag;
  size_t recv_msgcount;

  static int  my_comm_host_rank;
  static int  send_context;
  static int  procs_on_metahost;
  static int   *meta_header_sent;
  static char  *router_msg[1];
  static size_t router_msg_size[1];
  static size_t *meta_msg_i_size;
  static Meta_Header **meta_msg_i;

  static MPI_Status recv_status;
  struct MPIR_COMMUNICATOR *comm_host_ptr;

  static int firstcall=1;

  if(firstcall)
  {
    /* set up buffers */
    router_msg_size[0] = INIT_ROUTER_BUFFER_SIZE;
    router_msg[0] = (char *)malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char));
    if( router_msg[0]==NULL ) exit(-1);
    
    comm_host_ptr = MPIR_GET_COMM_PTR( MPI_COMM_HOST );
    my_comm_host_rank = comm_host_ptr->local_rank;
    send_context = comm_host_ptr->send_context;
    
    procs_on_metahost = 3;
    
    meta_msg_i = (Meta_Header **) malloc( procs_on_metahost * sizeof( Meta_Header * ) );
    meta_msg_i_size = (int *) malloc( procs_on_metahost * sizeof( int ) );
    meta_header_sent = (int *) malloc( procs_on_metahost * sizeof( int ) );

    for( i = 0; i < procs_on_metahost; i++ )
    {
      if( !(MPIR_meta_cfg.isRouter[i]) )
      {
	meta_msg_i[i] = (Meta_Header *) malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char) );
	meta_msg_i_size[i] = INIT_ROUTER_BUFFER_SIZE * sizeof(char);
	meta_header_sent[i] = 0;
      }
    }
    
    firstcall = 0;

    DBG("Check_gateway --> first call");
  } 
  
  /* receive messages from the mpi-processes of the localhost 
     and route them to the according host */
  
  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &iprobe_flag, &recv_status);
  DBG2("Iprobe is %d / Sender is %d", iprobe_flag, recv_status.MPI_SOURCE);
    
  if (!iprobe_flag)
  {
    /* No gateway message available...*/
    return 0;
  } 
  else 
  {
    if(buffer==NULL) return 1; /* <-- this is just a check-gateway call */

    /* get the size of the message to be received */
    MPI_Get_count(&recv_status, MPI_BYTE, &recv_msgcount);

    if( (recv_status.MPI_TAG == MPIR_SEPARATE_MSG_TAG) && (meta_header_sent[recv_status.MPI_SOURCE]) )
    {
      DBG2("SEPARATE_MSG_TAG %d %d",recv_msgcount,recv_msgcount-sizeof(Meta_Header));
      /* a meta header was sent and this is the message belonging to that header */
      meta_msg_i[recv_status.MPI_SOURCE] = (Meta_Header * )IMPI_adjustbuffer( (char *)(meta_msg_i[recv_status.MPI_SOURCE]),
									      meta_msg_i_size[recv_status.MPI_SOURCE],
									      recv_msgcount + sizeof( Meta_Header ) );
      if ( meta_msg_i_size[recv_status.MPI_SOURCE] < recv_msgcount + sizeof( Meta_Header ) )
	meta_msg_i_size[recv_status.MPI_SOURCE] = recv_msgcount + sizeof( Meta_Header );
    }      
    else
    {
      DBG2("No SEPARATE_MSG_TAG %d %d",recv_msgcount,recv_msgcount-sizeof(Meta_Header));
      router_msg[0] = IMPI_adjustbuffer(router_msg[0], router_msg_size[0], recv_msgcount);
      if( router_msg_size[0] < recv_msgcount ) router_msg_size[0] = recv_msgcount;
    }
    
    if( (recv_status.MPI_TAG == MPIR_SEPARATE_MSG_TAG) && (meta_header_sent[recv_status.MPI_SOURCE]) ) {
      
      /* we receive this message in the buffer for the sender process, directly after the meta header */
      MPI_Recv( meta_msg_i[recv_status.MPI_SOURCE] + 1, recv_msgcount, MPI_BYTE,
		recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &recv_status);
      DBG("Local message received");
    }
    else {
      DBG2("Going to recv the router message (%d) from %d", recv_msgcount, recv_status.MPI_SOURCE);
      MPI_Recv(router_msg[0], recv_msgcount, MPI_BYTE, recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &recv_status);
      DBG("Router message received");
    }
    
    
    /* check type of message - command message or MPI message ? */
    switch (recv_status.MPI_TAG)
    {
      case MPIR_SEPARATE_MSG_TAG:
      {
	DBG("MPIR_SEPARATE_MSG_TAG");

	/* 
	 |  IMPI: this connection mapping would correspond to the IMPI host mapping, but currently we
	 |  maintain only one IMPI host per IMPI client!
	 */
//      conn = get_conn_for_dest( meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.dest_grank );
		 
	DBG4("Gateway-msg for [a%d] from [m%d], tag %d, MPI size %d",
	     meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.dest_grank,
	     meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.src_comm_lrank,
	     meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.tag,
	     meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.count );

	*(dest_grank)     = meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.dest_grank;
	*(src_comm_lrank) = meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.src_comm_lrank;
	*(tag)            = meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.tag;
	*(length)         = meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.count;
	*(buffer)         = (void*)(meta_msg_i[recv_status.MPI_SOURCE] + 1);

	gateway_flag = 1;
	meta_header_sent[recv_status.MPI_SOURCE] = 0;
	
	break;
      }
		
      case MPIR_SEPARATE_META_HEADER_TAG:
      {
	DBG("MPIR_SEPARATE_META_HEADER_TAG");
	/*
	 | this is a short message containing the meta data for a nonblocking msg
	 | that comes later on, therefore we must save the data here 
	 */
	DBG2("memcpy (%d) in buffer of %d", recv_msgcount, recv_status.MPI_SOURCE);
	memcpy( meta_msg_i[recv_status.MPI_SOURCE], router_msg[0], recv_msgcount );
	meta_header_sent[recv_status.MPI_SOURCE] = 1;
	DBG("Header stored");

	break;
      }
		 
      case MPIR_ROUTMSG_TAG:
      {
	DBG("MPIR_ROUTMSG_TAG");
	switch (((Meta_Header *)router_msg[0])->msg.Rout.command) {
	  case FINALIZE:
	  {
	    break;
	  }
	  default:
	  {
	    /* ERROR: got router message with wrong command */
	    break;
	  }
	}
	break;
      }
		 
      default:
      {
        /* ERROR: got message with wrong tag */
      }
    }
  } /* if (!iprobe_flag) ... */ 
    
  DBG1("Leaving Check_gateway with %d", gateway_flag);

  return gateway_flag;
}


int IMPI_Tunnel_import(int src_comm_lrank, int dest_grank, int tag, size_t length, void **buffer, int get_buffer_flag)
{
  int i;

  static Meta_Header **meta_msg_i;
  static size_t *meta_msg_i_size;
  static int *meta_header_sent;
  
  static MPI_Status  *snd_status;
  static MPI_Request *snd_request;
    
  static IntQueue *availQ;
  static IntQueue *pendingQ;

  static IntQueue _availQ;
  static IntQueue _pendingQ;

  static size_t size;
  static int req_id;

  static char   **router_msg;
  static size_t *bufsize;
  static int    flag;

  static int procs_on_metahost = 3;

  static int firstcall=1;

  DBG("This is IMPI_Import_msgs");

  if(firstcall)
  {
    
    /* !!! This must be determined !!!*/
    procs_on_metahost = 3;

    meta_msg_i = (Meta_Header **) malloc( procs_on_metahost * sizeof( Meta_Header * ) );
    meta_msg_i_size = (int *) malloc( procs_on_metahost * sizeof( int ) );
    meta_header_sent = (int *) malloc( procs_on_metahost * sizeof( int ) );

    DBG("Import_msgs: meta_msg buffers allocated");
    
    for( i = 0; i < procs_on_metahost; i++ )
    {
      if( !(MPIR_meta_cfg.isRouter[i]) )
      {
	meta_msg_i[i] = (Meta_Header *) malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char) );
	meta_msg_i_size[i] = INIT_ROUTER_BUFFER_SIZE * sizeof(char);
	meta_header_sent[i] = 0;
      }
    }

    snd_status  = (MPI_Status *) malloc (MPIR_RouterConfig.isend_num * sizeof (MPI_Status));
    snd_request = (MPI_Request *) malloc (MPIR_RouterConfig.isend_num * sizeof (MPI_Request));
      
    Qinit (&_availQ, MPIR_RouterConfig.isend_num, 1);
    Qinit (&_pendingQ, MPIR_RouterConfig.isend_num, 0);

    availQ = &_availQ;
    pendingQ = &_pendingQ;

    if( !(router_msg = (char **)malloc( MPIR_RouterConfig.isend_num * sizeof(char *))) ) {
      PRERROR( "Could not allocate enough local memory" );
      ROUTER_ABORT;
    }	
    bufsize = (int *)malloc( MPIR_RouterConfig.isend_num * sizeof(int) );
    for (i = 0; i < MPIR_RouterConfig.isend_num; i++) {
      if( !( router_msg[i]  = (char *)malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char))) ) {
	PRERROR( "Could not allocate enough local memory" );
	ROUTER_ABORT;
      }
      bufsize[i]   = INIT_ROUTER_BUFFER_SIZE;
    }
    
    DBG("Import_msgs: router_msg buffer allocated");
    
    firstcall = 0;

    DBG("Import_msg --> first call");
  }

  /* size of the meta packet to be tunneld: */
  size = length + sizeof(Meta_Header);

  if(get_buffer_flag)
  {
    DBG("This is a 'get_buffer' call to IMPI_Send_tunnel");

    /* before we get req_id for this transaction (id of buffer to be used), we must make shure that
       not all buffers are full; if this is the case, we block until at least one buffer is available */
    while (Qfull (pendingQ))
    {
      for (i = Qfirst(pendingQ); i >= 0; i = Qnext(pendingQ))
      {	
	MPI_Test(&snd_request[i], &flag, &snd_status[i]);
	
	if (flag)
	{
	  /* message has been sent */
	  Qput (availQ, i);
	  Qremove (pendingQ, i);
	}
      }
    }

    /* get id for this transaction */
    req_id = Qget (availQ);
    Qput (pendingQ, req_id);
  
    router_msg[req_id] = IMPI_adjustbuffer (router_msg[req_id], bufsize[req_id], size);
    if( bufsize[req_id] < size )  bufsize[req_id] = size;
  
    *buffer = (Meta_Header *)router_msg[req_id]+1;

    DBG("Leaving Send_tunnel");
    return 0;
  }
  else
  {     
    DBG("This is a 'send_call' to IMPI_Send_tunnel");

    /*
     |   This is a Send_Call!
     */
    int dest = dest_grank;
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    MPIR_SHANDLE             *shandle;
    static char myname[] = "MPI_ISSEND";
    int mpi_errno = MPI_SUCCESS;
    int my_all_rank, my_all_size;

    /* Create MetaHeader: */
    memset((Meta_Header *)router_msg[req_id], 0, sizeof(Meta_Header));
    ((Meta_Header *)router_msg[req_id])->msg.MPI.dest_grank     = dest;
    ((Meta_Header *)router_msg[req_id])->msg.MPI.src_comm_lrank = src_comm_lrank;
    ((Meta_Header *)router_msg[req_id])->msg.MPI.tag            = tag;
    ((Meta_Header *)router_msg[req_id])->msg.MPI.count          = length;
    ((Meta_Header *)router_msg[req_id])->msg.MPI.msgrep         = 1;
    
#if 0
    /* even more to fake ??? */
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
    
    typedef struct _Meta_Header {
      MPIR_GW_mode mode;
      union {
	GW_MPI_msg     MPI;
	GW_Router_msg  Rout;
      } msg;
      unsigned char dummychar;
    } Meta_Header;

#endif
    
    DBG4("Gateway-msg for [a%d] from [m%d], tag %d, MPI size %d",
	 ((Meta_Header *)router_msg[req_id])->msg.MPI.dest_grank,
	 ((Meta_Header *)router_msg[req_id])->msg.MPI.src_comm_lrank,
	 ((Meta_Header *)router_msg[req_id])->msg.MPI.tag,
	 ((Meta_Header *)router_msg[req_id])->msg.MPI.count);
    
    TR_PUSH(myname);
      
    MPI_Comm_rank(MPI_COMM_ALL, &my_all_rank);
    MPI_Comm_size(MPI_COMM_ALL, &my_all_size);
      
    comm_ptr = MPIR_GET_COMM_PTR(MPI_COMM_ALL);	  
    dtype_ptr = MPIR_GET_DTYPE_PTR(MPI_BYTE);

    MPIR_ALLOCFN(shandle, MPID_Send_alloc, comm_ptr, MPI_ERR_EXHAUSTED, myname);
      
    snd_request[req_id] = (MPI_Request)shandle;
    MPID_Request_init( (MPI_Request)shandle, MPIR_SEND );
	  
    /* we need the rank of dest in MPI_COMM_ALL in MPID_Gateway_SendCancelPacket(),
       so we save it here */
    shandle->partner_grank = comm_ptr->lrank_to_grank[dest];
      
    MPIR_REMEMBER_SEND( shandle, router_msg[req_id], size, MPI_BYTE, dest, MPIR_MPIMSG_TAG, comm_ptr);
      
    if (dest == MPI_PROC_NULL)
    {
      shandle->is_complete = 1;
    }
    else
    {
      DBG("Going to tunnel the msg..");
      MPID_IsendDatatype( comm_ptr, router_msg[req_id], size, dtype_ptr, 
			  comm_ptr->local_rank, MPIR_MPIMSG_TAG, 
			  comm_ptr->send_context, 
			  comm_ptr->lrank_to_grank[dest], 
			  snd_request[req_id], &mpi_errno, 0 );
      DBG("Msg tunneld!");
    }
  }

  /* wait for completion of pending sends */
  DBG("Waiting for pending sends...");
  if (!Qempty(pendingQ))
  {
    for (i = Qfirst(pendingQ); i >= 0; i = Qnext(pendingQ))
    {
      MPI_Test(&snd_request[i], &flag, &snd_status[i]);
      if (flag)
      {
	/* message has been sent */
	Qput (availQ, i);
	Qremove (pendingQ, i);
      }
    }
  }

  DBG("Leaving Send_tunnel");
  return 0;
}

static char *IMPI_adjustbuffer(char *buf, size_t actualsize, size_t newsize)
{
    if (buf == NULL) {
	DBG1("adjustbuffer: buffer pointer NULL, allocating %d bytes", newsize);
	buf = (char *) malloc (newsize);
    } else {
      if (actualsize < newsize)
      {
	DBG2("Realloc buffer: %d vs %d", actualsize, newsize);
	buf = (char *) realloc(buf, newsize);
      }
    }
    
    if (buf == NULL) {
	DBG("adjustbuffer: realloc failed");
	ROUTER_ABORT;
    }
    
    return buf;
}
