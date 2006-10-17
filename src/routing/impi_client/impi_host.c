
#include "impi_host.h"
#include "impi_common.h"

#include "impi_tcp.h"
#include "impi_client.h"
#include "impi_router.h"

#undef  IMPI_MODULE_NAME
#define IMPI_MODULE_NAME "Host"

int IMPI_Export_packets(IMPI_Conn conn)
{
  IMPI_Packet impi_packet;
//  IMPI_Packet impi_ackpkt;

  int  src_comm_lrank;
  int  dest_grank;
  int  tag;
  int  length;
  void *buffer;

  DBG("Entering IMPI_Export_packets...");

  if(IMPI_Gateway_check(&src_comm_lrank, &dest_grank, &tag, &length))
  {
    /*
     |   A gateway message is available.
     |   So, prepare the IMPI packet:
     */

    IMPI_Gateway_recv(&src_comm_lrank, &dest_grank, &tag, &length, &buffer);
    
    memset(&impi_packet, 0, sizeof(impi_packet));

    impi_packet.pk_type   = IMPI_Int4_hton(IMPI_PK_DATA);
    impi_packet.pk_src    = *( (*(IMPI_Data.proc_world)) + src_comm_lrank );
    impi_packet.pk_dest   = *( (*(IMPI_Data.proc_world)) + dest_grank );		 
    impi_packet.pk_len    = IMPI_Int4_hton(length);

    /* is this an IMPI short or long message? */
    impi_packet.pk_msglen = IMPI_Int8_hton(length);

    /* set the local communicator rank of the source proc: */
    impi_packet.pk_lsrank = IMPI_Int4_hton(src_comm_lrank);		 
    impi_packet.pk_tag    = IMPI_Int4_hton(tag);
		 
    /* send the packet to the remote IMPI host: */
    IMPI_TCP_Send(conn.socket, &impi_packet, sizeof(impi_packet));
    DBG("Sent packet to remote IMPI host");
		 
    IMPI_TCP_Send(conn.socket, buffer, IMPI_Int4_ntoh(impi_packet.pk_len));
    DBG("Sent message to remote IMPI host");
  }

  DBG("Leaving IMPI_Export_msg...");

  return 0;
}

typedef struct _IMPI_Recv_request
{
  int in_progress;

  void *buffer_start;
  void *buffer_pointer;

  IMPI_Uint8 actual_size;

  /* ToDo: we can store further information (dest, rank, final_size, etc) to chekc for validity !!! */

} IMPI_Recv_request;


int IMPI_Import_packets(IMPI_Conn conn, int *lrank_to_grank)
{
  int i;
  
  int src_comm_lrank;
  int dest_grank;
  int tag;
  void *buffer;
  void *mem_pointer;

  IMPI_Uint4 length;
  IMPI_Uint8 long_length;

  int procs_on_metahost = 3;
  
  IMPI_Int4 impi_rank = IMPI_Data.impi_rank;

  IMPI_Packet impi_packet;
  IMPI_Packet impi_ackpkt;

  DBG("Entering IMPI_Import_packets...");
  
  DBG("Checking for remote IMPI packets...");
  if(!IMPI_TCP_Select(conn.socket, 0))
  {
    DBG("No packets available!");
    DBG("Leaving Import_Msg...");
    return 0;
  }

  DBG("Receiving packet...");
  IMPI_TCP_Recv(conn.socket, &impi_packet, sizeof(impi_packet));
  DBG1("Packet received %d!", impi_packet.pk_type);

  if( (IMPI_Int4_ntoh(impi_packet.pk_type)==IMPI_PK_DATASYNC) || (IMPI_Int4_ntoh(impi_packet.pk_type)==IMPI_PK_DATA) )
  {
    /*
     |  This is an IMPI data packet!
     |
     */
    
    /* search for the destination proc: */
    for(i=0; i<procs_on_metahost; i++)
    {
      if(memcmp(&((IMPI_Data.proc_world)[impi_rank][i]), &(impi_packet.pk_dest), sizeof(IMPI_Proc))==0) break;
    }
    /*
     |   The rank i is now similar to the COMM_LOCAL rank
     |   The local-to-global map is given by parameter:
     */
    dest_grank = lrank_to_grank[i];
    
    src_comm_lrank = IMPI_Int4_ntoh(impi_packet.pk_lsrank);
    tag            = IMPI_Int4_ntoh(impi_packet.pk_tag);

    /* the size of this message part is: */
    length = IMPI_Int4_ntoh(impi_packet.pk_len);

    /* the size of the whole message is: */
    long_length = IMPI_Int8_ntoh(impi_packet.pk_msglen);

    if(IMPI_Int4_ntoh(impi_packet.pk_type)==IMPI_PK_DATASYNC)
    {
      /*
       |   This is either a short IMPI message or the first packet
       |   of a IMPI long message (both start with DATASYNC)
       |
       */

      /* check for the message length: */
      if(length == long_length)
      {
	/*
	 |   This is a short IMPI message (only one packet).
	 |   
	 */

	DBG("IMPI short message");

	/* get an adequate buffer with a MetaHeader: */
	IMPI_Tunnel_buffer(src_comm_lrank, dest_grank, tag, length, &buffer);
      
	DBG1("Receiving msg of length %d", length);
	IMPI_TCP_Recv(conn.socket, buffer, length);
	DBG("Msg received!");
	
	/* tunnel the message to the receiver: */
	IMPI_Tunnel_send(src_comm_lrank, dest_grank, tag, length, buffer);
	
	/* if this an synchronous send, we must reply with a SYNCACK packet: */
	if(IMPI_Int4_ntoh(impi_packet.pk_type)==IMPI_PK_DATASYNC)
	{
	  memset(&impi_ackpkt, 0, sizeof(impi_ackpkt));
	  impi_ackpkt.pk_type  = IMPI_Int4_hton(IMPI_PK_SYNCACK);
	  impi_ackpkt.pk_srqid = impi_packet.pk_srqid;
	  impi_ackpkt.pk_dest  = impi_packet.pk_src;
	  impi_ackpkt.pk_src   = impi_packet.pk_dest;
	}
      }
      else
      {
	/*
	 |   This is the first part of a long IMPI message (multiple packets) 
	 |   So, we must create a buffer for the whole message and
	 |   receive the first data part:
	*/

	DBG("First packet of a IMPI long message");
      
	/* get an adequate buffer with a MetaHeader: */
	IMPI_Tunnel_buffer(src_comm_lrank, dest_grank, tag, long_length, &buffer);

	DBG2("Receiving the first part %d of %lld", length, long_length);
	IMPI_TCP_Recv(conn.socket, buffer, length);
	DBG("Firts part received!");

	/* prepare the ACK packet: */
	memset(&impi_ackpkt, 0, sizeof(impi_ackpkt));
	impi_ackpkt.pk_type  = IMPI_Int4_hton(IMPI_PK_SYNCACK);
	impi_ackpkt.pk_srqid = impi_packet.pk_srqid;
	impi_ackpkt.pk_dest  = impi_packet.pk_src;
	impi_ackpkt.pk_src   = impi_packet.pk_dest;

	/*
	 |   In order to remember the buffer pointer for the next data packet
	 |   of this message, we set the pk_drqid field of the ACK packet to
	 |   the pointer to the receive request
	 |   The sender will echo this pointer in the  next data packet header.
	 |   (Should we check its validity ?)
	 */
	
	mem_pointer = malloc(sizeof(IMPI_Recv_request));
	impi_ackpkt.pk_drqid = IMPI_Int8_hton((size_t)mem_pointer);

	/* set up the receive request struct: */
	((IMPI_Recv_request*)mem_pointer)->buffer_start = buffer + length;
	((IMPI_Recv_request*)mem_pointer)->buffer_pointer = buffer + length;
	((IMPI_Recv_request*)mem_pointer)->actual_size = length;
      }
    }
    else
    {
      /*
       |   This is a data packet within the flow of a long message.
       |   Hope, that the sender provides us with all information needed.
       */
	
      /* this is the memory pointer to the respective receive request: */
      mem_pointer = (void*)(IMPI_Int4)IMPI_Int8_ntoh(impi_ackpkt.pk_drqid);

      buffer = ((IMPI_Recv_request*)mem_pointer)->buffer_pointer;      
      
      DBG1("Receiving msg of length %d", length);
      IMPI_TCP_Recv(conn.socket, mem_pointer, length);
      DBG("Msg received!");

      ((IMPI_Recv_request*)mem_pointer)->buffer_pointer = buffer + length;
      ((IMPI_Recv_request*)mem_pointer)->actual_size += length;

      if( ((IMPI_Recv_request*)mem_pointer)->actual_size >= long_length )
      {
	/* 
	 |   This transmission is completed!
	 |
	 */

	/* tunnel the message: */
	IMPI_Tunnel_send(src_comm_lrank, dest_grank, tag, long_length, ((IMPI_Recv_request*)mem_pointer)->buffer_start);

	/* free the receive request: */
	free(mem_pointer);
      }

      /* TODO: 

         SEND THE ACK PACKETS !!!
	
         . . .
      
       */
    }
  }
  else
  {
    /* This is NO data packet! */
  }

  return 0;
}



#include <mpi.h>
#include <metaconfig.h>

int IMPI_Host(int *lrank_to_grank)
{
  int i,j;

  IMPI_Impi impi_env;
  IMPI_Int4 impi_size = IMPI_Data.impi_size;
  IMPI_Int4 impi_rank = IMPI_Data.impi_rank;

  /* THE HOST DANCE: */
  
  DBG("Entering IMPI_Host...");

  /* let's dance: higher ranked hosts connects, lower ranked host accepts: */
  /* !!! we assume, that there is only _one_ host per client !!! */
  for(i=0; i<impi_size; i++)
  {
    if(i<impi_rank)
    {
      /* do connect: */

      DBG("Going to connect...");
      
      IMPI_Data.partner_conn[i].socket = IMPI_TCP_Socket(PF_INET, SOCK_STREAM);
      DBG("Got socket");
      
      DBG("CONNECT...");
      IMPI_TCP_Connect(IMPI_Data.partner_conn[i].socket, IMPI_Data.partner_conn[i].remote_addr);
      DBG("CONNECTED !!!");

      MPI_Barrier(MPI_COMM_HOST);

      /* send my rank to the partner: */
      impi_env.rank=IMPI_Int4_hton(impi_rank);
      IMPI_TCP_Send(IMPI_Data.partner_conn[i].socket, &impi_env.rank, sizeof(impi_env.rank));  

      break;
    }
    else
    {
      if(i>impi_rank)
      {
#if 0
	/* do accept: */
	IMPI_Conn listen_conn;      

	memset(&(listen_conn.local_addr), 0, sizeof(listen_conn.local_addr));
	listen_conn.remote_addr.sin_family = AF_INET;
	/* listen_conn.local_addr.sin_addr.s_addr = local_host; */
	listen_conn.local_addr.sin_addr.s_addr = local_host;
	listen_conn.local_addr.sin_port = local_port;

	listen_conn.socket = IMPI_TCP_Socket(PF_INET, SOCK_STREAM);
      
	IMPI_TCP_Bind(listen_conn.socket, listen_conn.local_addr);
      
	/* prepare to listen for the clients: */
	DBG("LSITEN...");
	IMPI_TCP_Listen(listen_conn.socket, 2);
	DBG("LISTEN !!!");

	memset(&(partner_conn[i].remote_addr), 0, sizeof(partner_conn[i].remote_addr));

	DBG("ACCEPT...");
	partner_conn[i].socket = IMPI_TCP_Accept(listen_conn.socket, &(partner_conn[i].remote_addr));
	DBG("ACCEPTED !!! :-)");     
#endif
      }
    }
  }

  for(j=0; j<5; j++)
  {
    IMPI_Import_packets(IMPI_Data.partner_conn[i], lrank_to_grank);
    sleep(1);
    IMPI_Export_packets(IMPI_Data.partner_conn[i]);
    sleep(1);
  }
 
  DBG("NO MORE IMPI ACTION --> Going to exit...");

  return 0;
}

