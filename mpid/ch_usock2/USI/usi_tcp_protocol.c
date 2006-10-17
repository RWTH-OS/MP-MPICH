
#include "usi_tcp_protocol.h"

void USI_tcp_protocol_Error(char* string, int value)
{
  if(value!=0) fprintf(stderr, "### USI_tcp_client: Protocol ERROR -> %s (%d)\n", string, value);
  else fprintf(stderr, "### USI_tcp_client: Protocol ERROR -> %s\n", string);
  fflush(stderr);
}

void USI_tcp_protocol_Warning(char* string, int value)
{
  if(value!=0) fprintf(stderr, "### USI_tcp_client: Protocol WARNING -> %s (%d)\n", string, value);
  else fprintf(stderr, "### USI_tcp_client: Protocol WARNING -> %s\n", string);
  fflush(stderr);
}

int USI_tcp_Establish(int* usi_argc, char** usi_argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this)
{
  /*
   |   always in network byte order:
   |
   */

  fd_set fd_set_all;
  in_addr_t local_host, agent_host, partner_host, listen_host;
  in_port_t local_port, agent_port, partner_port, listen_port;
  struct sockaddr_in local_addr;
  
  USI_tcp_Socket agent_socket;
  USI_tcp_Socket listen_socket;
  USI_tcp_Socket *partner_socket;
  
  USI_rank_t partner_count, partner_iter, partner_listen;
  
  USI_tcp_Environ_t local_environ;
  USI_tcp_Environ_t partner_environ;

  USI_tcp_Command_t command;
  
  USI_rank_t* rank_map;

  char* argument;
  
  int exchange_argc;
  char** exchange_argv;
  char exchange_arg[USI_TCP_MAX_ARG_SIZE];

  exchange_argv=(char**)malloc(USI_TCP_MAX_ARG_NUM*sizeof(char*));

  /******************************
--**  INITIALIZATION:           **-----------------
   ******************************/


  /* is there a special local address to bind to ? */
  local_host=0;  local_port=0;
  if(argument=USI_basic_pop_argv2str(usi_argc, usi_argv, "-bind", "-"))
  {     
    /* is there also a port supplied? */
    if(index(argument, ':')!=NULL)
    {
      /*
       |   YES - print a warning, that an explicit connection
       |   port will be ignored.
       |   --> Set this port to zero!
       */
      local_port=USI_tcp_basic_str2port(strchr(argument, ':')+1);
      USI_tcp_protocol_Warning("excplicit connection ports are ignored!", ntohs(local_port));
      *(strchr(argument, ':'))='\0';

      /* the kernel will choose a suitable port, if it is set to zero:*/
      local_port=0;
    }
    
    /*
     |   YES - the "connection" sockets to to the agent and the 
     |   partners will later be bound to this IP, so store it:
     */
    local_host=USI_tcp_basic_str2host(argument);
  }

 
  /***********************************
--**  CHECK THE RANK/SIZE ARGUMENTS: **-----------------
   ***********************************
   |  
   |   A correct "size" and "rank" must be supplied to the clients.
   |   After the following check, every client has got its own rank
   |   and the number of processes (size) in this protocol environment.
   */

  if(USI_protocol_Establish(usi_argc, usi_argv, rank, size, handle, this))
  {
    /* this is the first protocol: */
    local_environ.rank=(*rank);  local_environ.size=(*size);
  }
  else
  {
    /* ! this is a secondary protocol! */  
    local_environ.rank=USI_basic_pop_argv2int(usi_argc, usi_argv, "-rank", "-");
    local_environ.size=USI_basic_pop_argv2int(usi_argc, usi_argv, "-size", "-");
    printf("Local rank=%d / Local size=%d\n",  local_environ.rank,  local_environ.size);

    /* ignore them: */
    local_environ.rank=(*rank);  local_environ.size=(*size);
  }

  /* create a rank map: (as big as the world size) */
  rank_map=(USI_rank_t*)malloc((*size)*sizeof(USI_rank_t));
  for(partner_iter=0; partner_iter<(*size); partner_iter++) rank_map[partner_iter]=-1;

  /* preset my own rank in this rank map: */
  rank_map[(*rank)]=(*size);


  /*******************************
--**  CLIENT-CLIENT CONNECTIONS: **-----------------
   *******************************
   |
   |  Now, the clients check for all "-connect" switches
   |  and connects to those partners.
   |  Afterwards, the client goes to listening status...
   */

  /* create an array of partern sockets: */
  partner_socket=(USI_tcp_Socket*)malloc(((*size)-1)*sizeof(USI_tcp_Socket));

  /* set the fd_all_set to zero: */
  FD_ZERO(&fd_set_all);
  
  /* check for the "-connect" flag and connect to the respective partner: */
  for(partner_count=0; argument=USI_basic_pop_argv2str(&(*usi_argc), usi_argv, "-connect", "-");  partner_count++)
  {
    /* check, if also the port is supplied: */
    if(index(argument, ':')!=NULL)
    {
      partner_port=USI_tcp_basic_str2port(strchr(argument, ':')+1);
      *(strchr(argument, ':'))='\0';
    }
    else
    { 
      /* ERROR: the port number must be supplied: */
      USI_tcp_protocol_Error("missing port in -connect argument", 0);
      exit(-1);
    }
    partner_host=USI_tcp_basic_str2host(argument);

    /* setup this partner address struct: */
    memset(&partner_socket[partner_count].addr, 0, sizeof(agent_socket.addr));
      
    partner_socket[partner_count].addr.sin_family=AF_INET;
    partner_socket[partner_count].addr.sin_addr.s_addr=partner_host;
    partner_socket[partner_count].addr.sin_port=partner_port;

    /* initialize this socket: */
    partner_socket[partner_count].socket=USI_tcp_basic_socket(PF_INET, SOCK_STREAM);

    /* remember this socket in the fd_set_all: */
    FD_SET( partner_socket[partner_count].socket, &fd_set_all);
    
    /* if there is also a local IP supplied, then bind the partner port to this: */
    if(local_host!=0)
    {
      /* setup the local address struct: */
      memset(&local_addr, 0, sizeof(local_addr));
      local_addr.sin_family=AF_INET;
      local_addr.sin_addr.s_addr=local_host;
      if(local_port!=0) local_addr.sin_port=htons(ntohs(local_port)+(*rank)+partner_count+1);

      USI_tcp_basic_bind(partner_socket[partner_count].socket, local_addr);
    }
    
#ifdef _USI_VERBOSE
    fprintf(USI_VERBOSE_STDOUT, "Connecting to partner: %s:%d", inet_ntoa(partner_socket[partner_count].addr.sin_addr), ntohs(partner_socket[partner_count].addr.sin_port));
    fflush(USI_VERBOSE_STDOUT);
#endif
    
    USI_tcp_basic_connect(partner_socket[partner_count].socket, partner_socket[partner_count].addr);
      
#ifdef _USI_VERBOSE  
    fprintf(USI_VERBOSE_STDOUT, " OK\n");
    fflush(USI_VERBOSE_STDOUT);
#endif

   /********************************************
---**  CLIENT-CLIENT STARTUP PROTOCOL - PART I **-----------------
    ********************************************
    |
    |
    */
    
    /* send the INIT command: */
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_INIT);
  
    if( (command=USI_tcp_recv_cmd(partner_socket[partner_count].socket)) != USI_TCP_CMD_CLNT)
    {
      USI_tcp_protocol_Error("got invalid command / SIGN was expected", command);
      USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }

    /***************************************************
     ** Send all the supplied arguments to the other client: **
     ***************************************************
     |
     |  "MASTER MODE"
     */
  
    /* firtsly, send the LSTN command to force the agent to listen: */
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_LSTN);
  
    /* wait for an acknowledgement of the other client: */
    if(USI_tcp_recv_cmd(partner_socket[partner_count].socket) != USI_TCP_CMD_ACKN)
    {
      USI_tcp_protocol_Error("got invalid acknowledgement", -1);
      USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }

    /*
     |   Now, send the needed arguments:
     | XXX  (May be, we want to send all supplied arguments?
     */
    
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ARGV);  
    USI_tcp_send_arg(partner_socket[partner_count].socket, "-rank");
    
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ARGV);    
    snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%d", (*rank));
    USI_tcp_send_arg(partner_socket[partner_count].socket, exchange_arg);
    
    /*
     |   The client has now sent all its current information.
     |   So send the DONE command and wait for the LSTN command:
     */
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_DONE);

    /****************
     ** Slave Mode **
     ****************/
    
    if((command=USI_tcp_recv_cmd(partner_socket[partner_count].socket)) != USI_TCP_CMD_LSTN)
    {
      USI_tcp_protocol_Error("got invalid command / LSTN was expected", command);
      USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }
    
    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ACKN);
    
    command=USI_tcp_recv_cmd(partner_socket[partner_count].socket);
    
    exchange_argc=0;
    while(command != USI_TCP_CMD_DONE)
    {
      if(command != USI_TCP_CMD_ARGV)
      {
	USI_tcp_protocol_Error("got invalid command / ARGV or DONE was expected", -1);
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	exit(-1);
      }  
      
      USI_tcp_recv_arg(partner_socket[partner_count].socket, &(exchange_argv[exchange_argc]));
      
      exchange_argc++;
      if(exchange_argc>=USI_TCP_MAX_ARG_NUM)
      {
	exchange_argc=USI_TCP_MAX_ARG_NUM;
#ifdef _USI_VERBOSE
	fprintf(USI_VERBOSE_STDOUT, "WARNING: Client sent too much arguments...\n");
	fflush(USI_VERBOSE_STDOUT);
#endif
      }
      
      command=USI_tcp_recv_cmd(partner_socket[partner_count].socket);
    }

    partner_environ.rank=-1;
    partner_environ.size=-1;
    
    /* Has this client supplied a rank ? */
    partner_environ.rank=USI_basic_pop_argv2int(&exchange_argc, &exchange_argv, "-rank", "-");
    if(partner_environ.rank<0)
    {
      printf("!!! FEHLER !!!\n");
    }
    
    if(rank_map[partner_environ.rank]==-1)
    {
      rank_map[partner_environ.rank]=partner_count;
      handle->protocol_table[partner_environ.rank]=this;

#ifdef _USI_VERBOSE
      fprintf(USI_VERBOSE_STDOUT, "This partner wanted to get rank %d -> OK\n", partner_environ.rank );
      fflush(USI_VERBOSE_STDOUT);
#endif
    }
    else
    {
      /* ERROR */
      USI_tcp_protocol_Error("Rank is already in use", partner_environ.rank);
      USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }
  }

  /* LISTEN: */
  
  if(partner_listen=USI_basic_pop_argv2int(usi_argc, usi_argv, "-listen", "-"))
  {
    if(argument=USI_basic_pop_argv2str(usi_argc, usi_argv, "-listen", "-"))
    {
      listen_host=0;  listen_port=0;
      
      /* is there also a port supplied? */
      if(index(argument, ':')!=NULL)
      {
	/* YES - store the port in local_port: */
	listen_port=USI_tcp_basic_str2port(strchr(argument, ':')+1);
	*(strchr(argument, ':'))='\0';
      }
      else
      {
	/* choose default port to listen: */
	listen_port=USI_tcp_basic_str2port(USI_TCP_LISTEN_PORT);
      }
      /* store the local IP in local_host: */
      listen_host=USI_tcp_basic_str2host(argument);
      
      memset(&(listen_socket.addr), 0, sizeof(listen_socket.addr));
    
      listen_socket.addr.sin_family=AF_INET;
      listen_socket.addr.sin_addr.s_addr=listen_host;
      listen_socket.addr.sin_port=listen_port;
      
      /* print the agent's address, so that the clients can read it ... */
      fprintf(USI_TCP_AGENT_STDOUT, "Listening on %s:%d for %d\n", inet_ntoa(listen_socket.addr.sin_addr), ntohs(listen_socket.addr.sin_port), partner_listen);
      fflush(USI_TCP_AGENT_STDOUT);
      
      listen_socket.socket=USI_tcp_basic_socket(PF_INET, SOCK_STREAM);
      
      USI_tcp_basic_bind(listen_socket.socket, listen_socket.addr);
      
      /* prepare to listen for the clients: */
      USI_tcp_basic_listen(listen_socket.socket, partner_listen);

      while(partner_listen>0)
      {     
	memset(&(partner_socket[partner_count].addr), 0, sizeof(partner_socket[partner_count].addr));
	
	partner_socket[partner_count].socket=USI_tcp_basic_accept(listen_socket.socket, &(partner_socket[partner_count].addr));
	
	
#ifdef _USI_VERBOSE
	fprintf(USI_VERBOSE_STDOUT, "Connection from: %s:%d\n", inet_ntoa(partner_socket[partner_count].addr.sin_addr), ntohs(partner_socket[partner_count].addr.sin_port));
	fflush(USI_VERBOSE_STDOUT);
#endif
      
     /*********************************************
-----**  CLIENT-CLIENT STARTUP PROTOCOL - PART II **-----------------
      *********************************************
      |
      |
      */

	if((command=USI_tcp_recv_cmd(partner_socket[partner_count].socket)) != USI_TCP_CMD_INIT)
	{
	  USI_tcp_protocol_Error("got invalid command / INIT was expected", command);
	  USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	  exit(-1);
	}
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_CLNT);
	
	if( (command=USI_tcp_recv_cmd(partner_socket[partner_count].socket)) != USI_TCP_CMD_LSTN)
	{
	  USI_tcp_protocol_Error("got invalid command / LSTN was expected", command);
	  USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	  exit(-1);
	}
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ACKN);
	
	command=USI_tcp_recv_cmd(partner_socket[partner_count].socket);
      
	exchange_argc=0;
	while(command != USI_TCP_CMD_DONE)
	{
	  if(command != USI_TCP_CMD_ARGV)
	  {
	    USI_tcp_protocol_Error("got invalid command / ARGV or DONE was expected", -1);
	    USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	    exit(-1);
	  }  
	
	  USI_tcp_recv_arg(partner_socket[partner_count].socket, &(exchange_argv[exchange_argc]));
	  
	  exchange_argc++;
	  if(exchange_argc>=USI_TCP_MAX_ARG_NUM)
	  {
	    exchange_argc=USI_TCP_MAX_ARG_NUM;
#ifdef _USI_VERBOSE
	    fprintf(USI_VERBOSE_STDOUT, "WARNING: Client sent too much arguments...\n");
	    fflush(USI_VERBOSE_STDOUT);
#endif
	  }
	  
	  command=USI_tcp_recv_cmd(partner_socket[partner_count].socket);
	}
	
	partner_environ.rank=-1;
	partner_environ.size=-1;
	
	/* Has this client supplied a rank ? */
	partner_environ.rank=USI_basic_pop_argv2int(&exchange_argc, &exchange_argv, "-rank", "-");
	if(partner_environ.rank<0)
	{
	  printf("!!! FEHLER !!!\n");
	}
	
	if(rank_map[partner_environ.rank]==-1)
	{
	  rank_map[partner_environ.rank]=partner_count;
	  handle->protocol_table[partner_environ.rank]=this;

#ifdef _USI_VERBOSE
	  fprintf(USI_VERBOSE_STDOUT, "This partner wanted to get rank %d -> OK\n", partner_environ.rank );
	  fflush(USI_VERBOSE_STDOUT);
#endif
	}
	else
	{
	  /* ERROR */
	  USI_tcp_protocol_Error("Rank is already in use", partner_environ.rank);
	  USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	  exit(-1);
	}
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_LSTN);
	
	/* wait for an acknowledgement of the other client: */
	if(USI_tcp_recv_cmd(partner_socket[partner_count].socket) != USI_TCP_CMD_ACKN)
	{
	  USI_tcp_protocol_Error("got invalid acknowledgement", -1);
	  USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_PERR);
	  exit(-1);
	}
	
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ARGV);  
	USI_tcp_send_arg(partner_socket[partner_count].socket, "-rank");
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_ARGV);    
	snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%d", (*rank));
	USI_tcp_send_arg(partner_socket[partner_count].socket, exchange_arg);
	
	USI_tcp_send_cmd(partner_socket[partner_count].socket, USI_TCP_CMD_DONE);

	partner_count++;
	partner_listen--;
      }
    }
  }
  else
  {
    /* ERROR -> listen arg: -listen number_to_listen_to listenIP:port */
  }

  for(partner_iter=0; partner_iter<(*size); partner_iter++)
  {
    partner_socket[partner_iter].active_send=0;
    partner_socket[partner_iter].active_recv=0;
  }

  /* store the rank map and the socket array: */
  this->private.size=sizeof(USI_tcp_Data);
  this->private.data=(USI_Pointer)malloc(sizeof(USI_tcp_Data));

  ((USI_tcp_Data*)(this->private.data))->rank_map=rank_map;
  ((USI_tcp_Data*)(this->private.data))->socket_array=partner_socket;
  ((USI_tcp_Data*)(this->private.data))->fd_set_all=fd_set_all;
    
  return 0;
}

int USI_tcp_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, struct _USI_Protocol* this)
{
  USI_rank_t mapped_rank;
  USI_tcp_basic_sockfd_t sockfd;
  USI_basic_ssize_t result;

  mapped_rank=((USI_tcp_Data*)(this->private.data))->rank_map[dest];
  
  sockfd=(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).socket;

  /* check, if this socket is already sending: */
  if((((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_send) return USI_RETRY;

  /* indicate this socket as active: */
  (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_send=1;
  
  /* Initialize the USI_Request: */
  request->buffer=buffer;
  request->final_size=size;
  request->actual_size=0;
  request->rank=dest;
  request->handle=&handle;
  request->active_send=1;
  request->active_recv=0;

  /* try to send the buffer: */
  if(blocking)
  {
    result=send(sockfd, buffer, size, 0);
    if(result<0) return USI_ERROR;
  }
  else
  {
    result=send(sockfd, buffer, size, MSG_DONTWAIT);
    if(result<0)
    {
      /*
	if( (errno!=EAGAIN) && (errno!=EWOULDBLOCK) ) return USI_ERROR;
      */
      return USI_PENDING;
    }
  }

  request->actual_size += result;
  request->buffer += result;

  if(request->actual_size>=request->final_size)
  {
    (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_send=0;
    return USI_SUCCESS;
  }
  
  if(blocking) return USI_ERROR;
  else return USI_PENDING;
}

int USI_tcp_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, struct _USI_Protocol* this)
{
  USI_tcp_basic_sockfd_t recv_socket;
  USI_rank_t mapped_rank;
  USI_tcp_basic_sockfd_t sockfd;
  USI_basic_ssize_t result;

  mapped_rank=((USI_tcp_Data*)(this->private.data))->rank_map[src];
  
  sockfd=(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).socket;

  /* check, if this socket is already receiving: */
  if((((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv) return USI_RETRY;


  /* indicate this socket as active: */
  (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv=1;
  
  /* Initialize the USI_Request: */
  request->buffer=buffer;
  request->final_size=size;
  request->actual_size=0;
  request->rank=src;
  request->handle=&handle;
  request->active_send=0;
  request->active_recv=1;

  /* try to recv the message: */
  if(blocking)
  {
    result=recv(sockfd, buffer, size, 0);
    if(result<0) return USI_ERROR;
  }
  else
  {
    result=recv(sockfd, buffer, size, MSG_DONTWAIT);
    if(result<0)
    {
      /*
	if( (errno!=EAGAIN) && (errno!=EWOULDBLOCK) ) return USI_ERROR;
      */
      return USI_PENDING;
    }
  }

  request->actual_size += result;
  request->buffer += result;

  if(request->actual_size>=request->final_size)
  {
    (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv=0;
    return USI_SUCCESS;
  }

  if(blocking) return USI_ERROR;
  else return USI_PENDING;
}

int USI_tcp_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this)
{
  USI_rank_t mapped_rank;
  USI_tcp_basic_sockfd_t sockfd;
  USI_basic_ssize_t result;

  mapped_rank=((USI_tcp_Data*)(this->private.data))->rank_map[request->rank];
  
  sockfd=(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).socket;

  /* is this a send request? */
  if(request->active_send)
  {
    /* check, if the message is already sent: */
    if(request->actual_size>=request->final_size)
    {
      (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_send=0;
      return USI_SUCCESS;
    }
    
    /* not all of the message is sent, so try to send some more:*/
    if(blocking)
    {
      result=send(sockfd, request->buffer, request->final_size-request->actual_size, 0);
      if(result<0) return USI_ERROR;
    }
    else
    {
      result=send(sockfd, request->buffer, request->final_size-request->actual_size, MSG_DONTWAIT);
      if(result<0)
      {
	/*
	  if( (errno!=EAGAIN) && (errno!=EWOULDBLOCK) ) return USI_ERROR;
	*/
	return USI_PENDING;
      }
    }

    request->actual_size += result;
    request->buffer += result;
    
    if(request->actual_size>=request->final_size)
    {
      (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_send=0;
      return USI_SUCCESS;
    }
    
    if(blocking) return USI_ERROR;
    else return USI_PENDING;
  }
  else
  {
    /* is this a recv request? */
    if(request->active_recv)
    {
      /* check, if the message is already sent: */
      if(request->actual_size>=request->final_size)
      {
	(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv=0;
	return USI_SUCCESS;
      }
	
      /* not all of the message is sent, so try to send some more:*/
      if(blocking)
      {
	result=recv(sockfd, request->buffer, request->final_size-request->actual_size, 0);
	if(result<0) return USI_ERROR;
      }
      else
      {
	result=recv(sockfd, request->buffer, request->final_size-request->actual_size, MSG_DONTWAIT);
	if(result<0)
	{
	  /*
	    if( (errno!=EAGAIN) && (errno!=EWOULDBLOCK) ) return USI_ERROR;
	  */
	  return USI_PENDING;
	}
      }

      request->actual_size += result;
      request->buffer += result;
	
      if(request->actual_size>=request->final_size)
      {
	(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv=0;
	return USI_SUCCESS;
      }

      if(blocking) return USI_ERROR;
      else return USI_PENDING;
    }
    else return USI_ERROR;
  }
}

int USI_tcp_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle, USI_Protocol* this)
{
  int result;
  fd_set recv_set;
  struct timeval immediately={0,0};

  USI_rank_t mapped_rank;
  USI_tcp_basic_sockfd_t sockfd;

  mapped_rank=((USI_tcp_Data*)(this->private.data))->rank_map[src];
  sockfd=(((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).socket;

  FD_ZERO(&recv_set);
  FD_SET(sockfd, &recv_set);
  
  if(blocking) result=select(FD_SETSIZE, &recv_set, NULL, NULL, NULL);
  else result=select(FD_SETSIZE, &recv_set, NULL, NULL, &immediately);
  
  if(FD_ISSET(sockfd, &recv_set))
  {   
    (((USI_tcp_Data*)(this->private.data))->socket_array[mapped_rank]).active_recv=0;
    return USI_SUCCESS;
  }
  else
  {
    if(blocking) return USI_ERROR;
    else return USI_PENDING;
  }
}
