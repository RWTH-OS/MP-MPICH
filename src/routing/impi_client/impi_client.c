
#include "impi_client.h"
#include "impi_common.h"
#include "impi_tcp.h"
#include "impi_host.h"

#undef  IMPI_MODULE_NAME
#define IMPI_MODULE_NAME "Client"

IMPI_Data_struct IMPI_Data;

int IMPI_Client(int argc, char* argv[])
{
  int i, j, k, l;
  void *buffer;
  char *argument;

  /* always in network-byte-order: */
  in_addr_t server_host = 0;
  in_port_t server_port = 0;

  in_port_t dummy_port  = 0;

  in_addr_t local_host  = 0;
  in_port_t local_port  = 0;

  in_addr_t remote_host[32];
  in_port_t remote_port[32][32]; /* <-- ToDo: malloc this with the number of hosts !!! */
 
  IMPI_Conn server_conn;

  IMPI_Cmd command;
  IMPI_Coll client_coll;
  IMPI_Chdr server_coll;
  IMPI_Client_auth client_auth;
  IMPI_Server_auth server_auth;
  IMPI_Impi impi_env;

  IMPI_Version impi_version[32];
  IMPI_Uint4 num_hosts[32];

//  IMPI_Proc  **proc_world;
//  IMPI_Uint4 num_c_procs[32];

  IMPI_Uint4 num_h_procs[32][1];  /* <-- !!! there is only _one_ host per client !!! */

  IMPI_Uint4 datalen[32];
  IMPI_Int4 xsize[32];
  IMPI_Int4 maxlinear[32];

  unsigned char byte_dummy;

  /* rank and size in host-byte-order: */
  IMPI_Int4 impi_rank;
  IMPI_Int4 impi_size;
  IMPI_Int4 impi_local_np=2;

  local_host=0;   local_port=0;
  server_host=0;  server_port=0;

  printf(">>> %d args...\n", argc);
  for(i=0; i<argc; i++)
  {
    if(argv[i]) printf(">>> %s\n", argv[i]);
    fflush(stdout);
  }

  /* parse the command line: */
  argument = IMPI_Pop_argv2str(&argc, &argv, "-client", "-");

  if(argument)
  {
    /* this must be the client rank */
    impi_rank = atoi(argument);

    DBG1("My Client rank is: %d", impi_rank);
  }
  else
  {
    /* ERROR: the client ID and the server address must be supplied: */
    IMPI_Error("missing -client argument");
  }

  /* now, get the server address: */
  argument = IMPI_Pop_argv2str(&argc, &argv, "-client", "-");

  if(argument)
  {     
    /* check, if also the port is supplied: */
    if(index(argument, ':')!=NULL)
    {
      server_port = IMPI_TCP_Str2port(strchr(argument, ':')+1);
      *(strchr(argument, ':'))='\0';
    }
    else
    { 
      /* ERROR: the port number must be supplied: */
      IMPI_Error("missing server port in -client argument");
    }
  }
  else
  {
    /* ERROR: the client ID and the server address must be supplied: */
    IMPI_Error("missing server address in -client argument");
  }
  
  server_host = IMPI_TCP_Str2host(argument);

  /* setup the server address struct: */
  memset(&server_conn.remote_addr, 0, sizeof(server_conn.remote_addr));
      
  server_conn.remote_addr.sin_family = AF_INET;
  server_conn.remote_addr.sin_addr.s_addr = server_host;
  server_conn.remote_addr.sin_port = server_port;

  /* initialize this socket: */
  server_conn.socket = IMPI_TCP_Socket(PF_INET, SOCK_STREAM);

  /* if there is also a local IP supplied, then bind the local server port to this: */
  if(local_host!=0)
  {
    /* setup the local address struct: */
    memset(&server_conn.local_addr, 0, sizeof(server_conn.local_addr));
    server_conn.local_addr.sin_family = AF_INET;
    server_conn.local_addr.sin_addr.s_addr = local_host;
    if(local_port!=0) server_conn.local_addr.sin_port = local_port;
    
    IMPI_TCP_Bind(server_conn.socket, server_conn.local_addr);
  }

  /* connect to the server: */  
  IMPI_TCP_Connect(server_conn.socket, server_conn.remote_addr);

  DBG("Successfully connected to the server");

  /* send the AUTH command to the server: */
  command.cmd = IMPI_Int4_hton(IMPI_CMD_AUTH);
  command.len = IMPI_Int4_hton(4);
  IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));

  DBG("Successfully sent the AUTH command to the server");

  client_auth.auth_mask = IMPI_Int4_hton(0x0001); 
  IMPI_TCP_Send(server_conn.socket, &client_auth, sizeof(client_auth));

  DBG("Sent the AUTH_NONE mask to the server");

  IMPI_TCP_Recv(server_conn.socket, &server_auth, sizeof(server_auth));

  /* !!! we assume that there is no AUTH protocol !!! */
  DBG("Received the Server_auth -- assume that it is AUTH_NONE ...");

  /* send my rank to the server: */
  command.cmd = IMPI_Int4_hton(IMPI_CMD_IMPI);
  command.len = IMPI_Int4_hton(4);
  IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));
  impi_env.rank = IMPI_Int4_hton(impi_rank);
  IMPI_TCP_Send(server_conn.socket, &impi_env, sizeof(impi_env));

  /* receive the IMPI environment size: */
  IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));
  IMPI_TCP_Recv(server_conn.socket, &impi_env, sizeof(impi_env));
  
  /* store the IMPI size in host-byte-order: */
  impi_size = IMPI_Int4_ntoh(impi_env.size);

  DBG1("The number of clients is: %d", impi_size);

  /*
   | !!! in a first step, we want to link _one_ MetaMPICH metahost with _one_
   | !!! remote IMPI-client. So, there should only be those two clients!
   |
   */
  if(impi_size>2) IMPI_Error("currently, only the number of_two_ IMPI clients is supported");
    

  {/*-----------------------------------------------------------------------
    |
    |   Exchange of the PER-CLIENT information:
    |
    */
    
    /***********************
     **  IMPI_C_VERSION   **
     ***********************/
     
    /* tell the server the IMPI version that we support: */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(sizeof(client_coll)+sizeof(impi_version[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    
    client_coll.label = IMPI_Int4_hton(IMPI_C_VERSION);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));
    
    /* we support only version 0.0: */
    impi_version[impi_rank].major = 0;
    impi_version[impi_rank].minor = 0;
    IMPI_TCP_Send(server_conn.socket, &impi_version[impi_rank], sizeof(impi_version[impi_rank]));
    DBG2("Sent my IMPI version: %d.%d",  IMPI_Int4_ntoh(impi_version[impi_rank].major), IMPI_Int4_ntoh(impi_version[impi_rank].minor));
  
    /* try to receive the negotiated version: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
  	IMPI_Error("did not get the expected the COLL command from the server");
        
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_VERSION);

    /* determine the negotiated version: */
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(impi_version[i]), buffer+i*sizeof(impi_version[i]), sizeof(impi_version[i]));
	DBG3("Client %d wants the version %d.%d", i, IMPI_Int4_ntoh(impi_version[i].major), IMPI_Int4_ntoh(impi_version[i].minor));
	/* !!! since we only support version 0.0, this is version to be run !!! */
      }
      else
      {
	/* use the lowest IMPI version by default: */
	impi_version[i].major=0;
	impi_version[i].minor=0;
      }

      j=j<<1;
    }
    free(buffer);


    /***********************
     **  IMPI_C_NHOSTS    **
     ***********************/
    
    /* tell the server the number of hosts (alias routers) on this client (alias metahost):
     | 
     | !!! This number is actually alway 1, because we just want to link one MetaMPICH-Metahost
     | !!! with one remote IMPI-Client!
     |
     */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(num_hosts[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_C_NHOSTS);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    /* !!! just _one_ host per client: !!! */
    num_hosts[impi_rank] = IMPI_Int4_hton(1);
    IMPI_TCP_Send(server_conn.socket, &(num_hosts[impi_rank]), sizeof(num_hosts[impi_rank]));
    DBG1("Sent my number of hosts: %d",  IMPI_Int4_ntoh(num_hosts[impi_rank]));
 
    /* try to receive the number of hosts (alias routers) on other clients:
     |
     | !!! we assume that there is only _one_ host on one remote client !!!
     |
     */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_NHOSTS);

    /* get the number of hosts on each client: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(num_hosts[i]), buffer+i*sizeof(num_hosts[i]), sizeof(num_hosts[i]));
	DBG2("Number of hosts on client[%d]: %d",i,  IMPI_Int4_ntoh(num_hosts[i]));

	/* !!! we assume that there is only _one_ host on each client !!! */
	if(IMPI_Int4_ntoh(num_hosts[i])!=1)
	  IMPI_Error("we assume that there is only _one_ host on each client");
      }
      else num_hosts[i]=0;

      j=j<<1;
    }

    free(buffer);


    /***********************
     **  IMPI_C_NPROCS    **
     ***********************/
    
    /* tell the server the number of appliation processes on this client (alias metahost): */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(IMPI_Data.num_c_procs[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_C_NPROCS);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    IMPI_Data.num_c_procs[impi_rank] = IMPI_Int4_hton(impi_local_np);
    IMPI_TCP_Send(server_conn.socket, &(IMPI_Data.num_c_procs[impi_rank]), sizeof(IMPI_Data.num_c_procs[impi_rank]));
    DBG1("Sent my number of procs: %d",  IMPI_Int4_ntoh(IMPI_Data.num_c_procs[impi_rank]));
 
    /* try to receive the number of processes on the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_NPROCS);

    /* get the number of processes on each client: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(IMPI_Data.num_c_procs[i]), buffer+i*sizeof(IMPI_Data.num_c_procs[i]), sizeof(IMPI_Data.num_c_procs[i]));
	DBG2("Number of processes on client[%d]: %d",i,  IMPI_Int4_ntoh(IMPI_Data.num_c_procs[i]));
      }
      else IMPI_Data.num_c_procs[i]=0;

      j=j<<1;
    }

    free(buffer);

    /* allocate buffer for the proc_world information of each proc: */
    IMPI_Data.proc_world = (IMPI_Proc**)malloc(impi_size*sizeof(IMPI_Proc*));
    for(i=0, j=0; i<impi_size; i++) j += IMPI_Int4_ntoh(IMPI_Data.num_c_procs[i]);
    IMPI_Data.proc_world[0] = (IMPI_Proc*)malloc(j*sizeof(IMPI_Proc));
    for(i=1; i<impi_size; i++) IMPI_Data.proc_world[i] = IMPI_Data.proc_world[0] + IMPI_Int4_ntoh(IMPI_Data.num_c_procs[i-1]);
    

    /***********************
     **  IMPI_C_DATALEN   **
     ***********************/
    
    /* tell the server my DATALEN value: */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(datalen[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_C_DATALEN);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    /* */
    datalen[impi_rank] = IMPI_Int4_hton(1024);
    IMPI_TCP_Send(server_conn.socket, &(datalen[impi_rank]), sizeof(datalen[impi_rank]));
    DBG1("Sent my DATALEN: %d",  IMPI_Int4_ntoh(datalen[impi_rank]));
 
    /* try to receive the DATALEN values of the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_DATALEN);

    /* get the DATALEN values of the other clients: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(datalen[i]), buffer+i*sizeof(datalen[i]), sizeof(datalen[i]));
	DBG2("DATALEN of client[%d]: %d",i,  IMPI_Int4_ntoh(datalen[i]));

	/* check for the smallest datalen: */
	if(datalen[i]<datalen[impi_rank]) datalen[impi_rank]=datalen[i];
      }
      /* set the DATALEN to the minimum: */
      else datalen[i]=1;

      j=j<<1;
    }

    free(buffer);

    
    /***********************
     **  IMPI_C_XSIZE     **
     ***********************/
    
    /* tell the server my XSIZE value: (must be the same on all clients!) */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(xsize[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_C_COLL_XSIZE);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));    

    /* send the default value dummy (-1): */
    xsize[impi_rank] = IMPI_Int4_hton(-1);
    IMPI_TCP_Send(server_conn.socket, &(xsize[impi_rank]), sizeof(xsize[impi_rank]));
    DBG1("Sent my XSIZE: %d",  IMPI_Int4_ntoh(xsize[impi_rank]));
 
    /* try to receive the XSIZE values of the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_COLL_XSIZE);

    /* get the XSIZE value of the other clients: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(xsize[i]), buffer+i*sizeof(xsize[i]), sizeof(xsize[i]));
	DBG2("XSIZE of client[%d]: %d",i,  IMPI_Int4_ntoh(xsize[i]));

	/* all clients must contribute the same XSIZE: */
	if((xsize[i]!=IMPI_Int4_hton(-1))&&(xsize[i]!=xsize[impi_rank]))
	  IMPI_Error("did not receive same IMPI_COLL_XSIZE from all clients");
      }
      /* set the XSIZE default value: */
      else xsize[i]=IMPI_Int4_hton(1024);

      j=j<<1;
    }

    free(buffer);


    /***********************
     ** IMPI_C_MAXLINEAR  **
     ***********************/
    
    /* tell the server my MAXLINEAR value: (must be the same on all clients!) */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(maxlinear[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_C_COLL_MAXLINEAR);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    /* send the default dummy as my MAXLINEAR: */
    maxlinear[impi_rank] = IMPI_Int4_hton(-1);
    IMPI_TCP_Send(server_conn.socket, &(maxlinear[impi_rank]), sizeof(maxlinear[impi_rank]));
    DBG1("Sent my MAXLINEAR: %d",  IMPI_Int4_ntoh(maxlinear[impi_rank]));
 
    /* try to receive the MAXLINEAR values of the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_C_COLL_MAXLINEAR);

    /* get the number of processes on each client: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(maxlinear[i]), buffer+i*sizeof(maxlinear[i]), sizeof(maxlinear[i]));
	DBG2("MAXLINEAR of client[%d]: %d",i,  IMPI_Int4_ntoh(maxlinear[i]));

	/* all clients must contribute the same MAXLINEAR: */
	if((maxlinear[i]!=IMPI_Int4_hton(-1))&&(maxlinear[i]!=maxlinear[impi_rank]))
	  IMPI_Error("did not receive same IMPI_COLL_MAXLINEAR from all clients");
      }
      /* set the MAXLINEAR default value: */
      else maxlinear[i]=IMPI_Int4_hton(4);

      j=j<<1;
    }

    free(buffer);


  } /*--- End of PER-CLIENT information exchange ----------------------------*/

  
  {/*------------------------------------------------------------------------
    |
    |   Exchange of the PER-HOST information:
    |
    */

    /***********************
     **  IMPI_H_IPV6      **
     ***********************/
    
    /* tell the server the IPv6 address of the local router (alias host): */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4 + 16); /* !!!  16 * number of routers (alias hosts) on this client (actually only one router) !!! */
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_H_IPV6);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));
    
    /* send 10x the a zero dummy (80 bit): */
    byte_dummy = 0; /* it's size is 1 byte = 8 bit */
    for(i=0; i<10; i++) IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
    
    /* now, send the "do-not-support-IPv6" bit mask: */
    byte_dummy = 0xFF;
    IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
    IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
    
    local_host = IMPI_TCP_Str2host("134.130.62.70");

    /* finally, send the 32 bit IPv4 address: */
    IMPI_TCP_Send(server_conn.socket, &local_host, 4);
    
    /* try to receive the IP addresses of all the hosts: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh(server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_H_IPV6);

    /* get the IP addresses of all the routers (alias hosts): */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));


    for(i=0, j=1; i<impi_size; i++)
    {
      DBG2("XXX remote_host[%d] is %p", i, &(remote_host[i]));
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	memcpy((void*)&(remote_host[i]),(void*)&(((char*)buffer+i*16)[12]), sizeof(IMPI_Data.num_c_procs[i]));
	DBG2("Host of client[%d]: %s",i, inet_ntoa(*((struct in_addr*)&(remote_host[i]))));
      }
      else remote_host[i]=0;

      j=j<<1;
    }

    free(buffer);


    /***********************
     **  IMPI_H_PORT      **
     ***********************/
    
    /* tell the server the port of local router: */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(local_port));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_H_PORT);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));
    
    local_port = IMPI_Int4_hton(55555);
    IMPI_TCP_Send(server_conn.socket, &(local_port), sizeof(local_port));
    DBG1("Sent my portnumber: %d",  IMPI_Int4_ntoh(local_port));
 
    /* try to receive the port numbers of the other hosts: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_H_PORT);

#if 0
    for(i=0; i<IMPI_Int4_ntoh(command.len)-sizeof(server_coll); i++)
    {
      printf("%d\n", ((unsigned char*)buffer)[i]);
    }
#endif


    /* get the port number of the remote hosts: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1, l=0; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	for(k=0; k<IMPI_Int4_ntoh(num_hosts[i]); k++)
	{
	  memcpy((void*)&(remote_port[i][k]), buffer+2+l*sizeof(IMPI_Uint4), sizeof(remote_port[i][k]));
	  DBG3("Port number of host %d on client %d: %d", k, i,  ntohs(remote_port[i][k]));
	  l++;
	}
      }

      j=j<<1;
    }

    free(buffer);

    
    /***********************
     **  IMPI_H_NPROCS    **
     ***********************/
 
    /* tell the server the number of application processes my local hosts serve: */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+sizeof(num_h_procs[impi_rank][0]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_H_NPROCS);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));
    
    /* !!! since we assume only _one_ host per client, this is the number of local application procs !!! */
    num_h_procs[impi_rank][0] = IMPI_Int4_hton(impi_local_np);
    IMPI_TCP_Send(server_conn.socket, &(num_h_procs[impi_rank][0]), sizeof(num_h_procs[impi_rank][0]));
    DBG1("Sent my number of procs: %d",  IMPI_Int4_ntoh(num_h_procs[impi_rank][0]));
  
    /* try to receive the number of processes on the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_H_NPROCS);

    /* get the number of processes served by each host: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));
    for(i=0, j=1, l=0; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	for(k=0; k<IMPI_Int4_ntoh(num_hosts[i]); k++)
	{
	  memcpy((void*)&(num_h_procs[i][k]), buffer+l*sizeof(num_h_procs[i][k]), sizeof(num_h_procs[i][k]));
	  DBG3("Number of procs on host %d on client %d: %d", k, i,  IMPI_Int4_ntoh(num_h_procs[i][k]));
	  l++;

	  /* !!! we assume that there is only _one_ host on each client !!! */
	  /* !!! so the number of procs per host must be the number of procs per client !!! */
	  if( (num_h_procs[i][k]!=num_h_procs[i][k]) || (k>0) )
	    IMPI_Error("the number of procs per host must be the number of procs per client");
	}
      }

      j=j<<1;
    }

    free(buffer);


  } /*--- End of PER-HOST information exchange ------------------------------*/


  {/*------------------------------------------------------------------------
    |
    |   Exchange of the PER-PROC information:
    |
    */

    /***********************
     **  IMPI_P_IPV6      **
     ***********************/
    
    /* tell the server the IPv6 address the local procs (alias host): */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4 + 16*IMPI_Int4_ntoh(IMPI_Data.num_c_procs[impi_rank]));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_P_IPV6);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    for(i=0; i<impi_local_np; i++)
    {
      /* send 10x the a zero dummy (80 bit): */
      byte_dummy = 0; /* it's size is 1 byte = 8 bit */
      for(j=0; j<10; j++) IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
    
      /* now, send the "do-not-support-IPv6" bit mask: */
      byte_dummy = 0xFF;
      IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
      IMPI_TCP_Send(server_conn.socket, &byte_dummy, 1);
      
      /* the addresses must be determined by the configure-parser */
      /* for test purpose, all procs reside on the same host: */
      local_host  = IMPI_TCP_Str2host("134.130.62.70");

      /* finally, send the 32 bit IPv4 address: */
      IMPI_TCP_Send(server_conn.socket, &local_host, 4);
    }
      
    /* try to receive the IP addresses of all the hosts: */
    buffer=NULL;
    do
    {
      free(buffer);
      
      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh(server_coll.label), IMPI_Int4_ntoh(command.len));
      
      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));
      
    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_P_IPV6);

    /* get the IP addresses of all the other procs: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));

#if 0
    for(i=0; i<IMPI_Int4_ntoh(command.len)-sizeof(server_coll); i++)
    {
      printf("%d\n", ((unsigned char*)buffer)[i]);
    }
#endif

    for(i=0, j=1, l=0; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	for(k=0; k<IMPI_Int4_ntoh(IMPI_Data.num_c_procs[i]); k++)
	{
	  /* IMPI host identifiers are 16 bytes long: */	  
	  memcpy((void*)&(IMPI_Data.proc_world[i][k].p_hostid),(void*)((char*)buffer+l*16), 16);
	  DBG3("Host-ID of proc %d of client[%d]: %s", k, i, inet_ntoa(*((struct in_addr*)((char*)&(IMPI_Data.proc_world[i][k].p_hostid)+12))));
	  l++;
	}
      }
      else remote_host[i]=0;
      
      j=j<<1;
    }
    
    free(buffer);


    /***********************
     **  IMPI_P_PID       **
     ***********************/
    
    /* tell the server the PIDs of all my procs: */
    command.cmd = IMPI_Int4_hton(IMPI_CMD_COLL);
    command.len = IMPI_Int4_hton(4+IMPI_Int4_ntoh(IMPI_Data.num_c_procs[impi_rank])*sizeof(IMPI_Int8));
    IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));  
    client_coll.label = IMPI_Int4_hton(IMPI_P_PID);
    IMPI_TCP_Send(server_conn.socket, &client_coll, sizeof(client_coll));

    for(i=0; i<impi_local_np; i++)
    {
      /* IMPI PIDs are 8 bytes long, but the pid_t is only 8 bytes long: */
      byte_dummy=0;
      for(j=0; j<6; j++) IMPI_TCP_Send(server_conn.socket, &(byte_dummy), sizeof(byte_dummy));

      byte_dummy=(getpid()+i)/256;
      IMPI_TCP_Send(server_conn.socket, &(byte_dummy), sizeof(byte_dummy));
      byte_dummy=(getpid()+i)%256;
      IMPI_TCP_Send(server_conn.socket, &(byte_dummy), sizeof(byte_dummy));
      
      DBG2("Sent the PID of the local procs[%d]: %d", i, getpid()+i);
    }
 
    /* try to receive the number of processes on the other clients: */
    buffer=NULL;
    do
    {
      free(buffer);

      /* wait for COLL command from server: */
      IMPI_TCP_Recv(server_conn.socket, &command, sizeof(command));      
      if(IMPI_Int4_ntoh(command.cmd)!=IMPI_CMD_COLL)
	IMPI_Error("did not get the expected the COLL command from the server");
      
      /* receive the label and client-mask of this command: */
      IMPI_TCP_Recv(server_conn.socket, &server_coll, sizeof(server_coll));
      DBG2("Got the COLL command --> Label = %x, Payload = %d", IMPI_Int4_ntoh (server_coll.label), IMPI_Int4_ntoh(command.len));

      /* receive the rest of the payload: */
      buffer=malloc(IMPI_Int4_ntoh(command.len));
      IMPI_TCP_Recv(server_conn.socket, buffer, IMPI_Int4_ntoh(command.len)-sizeof(server_coll));

    } while(IMPI_Int4_ntoh(server_coll.label)!=IMPI_P_PID);

#if 0
    for(i=0; i<IMPI_Int4_ntoh(command.len)-sizeof(server_coll); i++)
    {
      printf("%d\n", ((unsigned char*)buffer)[i]);
    }
#endif

    /* get the PIDs of all the other procs: */
    DBG1("Client-Mask is: 0x%x",  IMPI_Int4_ntoh(server_coll.client_mask));

    for(i=0, j=1, l=0; i<impi_size; i++)
    {
      if( j&(IMPI_Int4_ntoh(server_coll.client_mask)) )
      {
	for(k=0; k<IMPI_Int4_ntoh(IMPI_Data.num_c_procs[i]); k++)
	{
	  memcpy((void*)&(IMPI_Data.proc_world[i][k].p_pid), (void*)((char*)buffer+l*8), 8);

	  /* IMPI PIDs are 8 bytes long, but the pid_t is only 8 bytes long: */
	  memcpy((void*)&dummy_port, ((char*)&(IMPI_Data.proc_world[i][k].p_pid)+6), 2);
	  DBG3("PID of proc %d of client[%d]: %d", k, i, ntohs(dummy_port));

	  l++;
	}
      }
      else remote_host[i]=0;
      
      j=j<<1;
    }

    free(buffer);


  } /*--- End of PER-PROC information exchange ------------------------------*/


  /* now, we are done with sending the COLL information: */
  command.cmd = IMPI_Int4_hton(IMPI_CMD_DONE);
  command.len = 0;
  IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));

  /* Setup the hosts: */
  for(i=0; i<32; i++)
  {
    memset(&(IMPI_Data.partner_conn[i].remote_addr), 0, sizeof(IMPI_Data.partner_conn[i].remote_addr));
    IMPI_Data.partner_conn[i].remote_addr.sin_family = AF_INET;
    IMPI_Data.partner_conn[i].remote_addr.sin_addr.s_addr = remote_host[i];
    IMPI_Data.partner_conn[i].remote_addr.sin_port = remote_port[i][0];
  }
  /* remmeber the global data in the IMPI_Data struct: */
  IMPI_Data.impi_rank = impi_rank;
  IMPI_Data.impi_size = impi_size;

   
#if 0
  /* send FINI, when all my hosts are ready: */
  command.cmd = IMPI_Int4_hton(IMPI_CMD_FINI);
  command.len = 0;
  IMPI_TCP_Send(server_conn.socket, &command, sizeof(command));
#endif

  return 0;
}

