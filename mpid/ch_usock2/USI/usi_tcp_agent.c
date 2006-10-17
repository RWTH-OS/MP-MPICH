
#include "usi_tcp_agent.h"


void USI_tcp_agent_Error(char* string, int value)
{
  fprintf(stderr, "### USI_tcp_agent: Protocol ERROR -> %s (%d)\n", string, value);
  fflush(stderr);
}

static int USI_tcp_MasterMode(void);
static int USI_tcp_SlaveMode(void);

 int i;

  char* arg;
  USI_rank_t size, rank;
  USI_rank_t client_count, client_number, client_iter;
  USI_tcp_Socket listen_socket;
  USI_tcp_Socket *client_socket;
  in_addr_t agent_host; /* always in network byte order !*/
  in_port_t agent_port; /*               "               */
  in_addr_t client_host;
  in_port_t client_port;

  USI_tcp_Environ_t environ;
  USI_tcp_Command_t command;

  int cmd;
  int exchange_argc;
  char** exchange_argv;
  char exchange_arg[USI_TCP_MAX_ARG_SIZE];

  char address_table[32][24];
    
  USI_rank_t *rank_map;


int main(int argc, char* argv[])
{ 
  exchange_argv=(char**)malloc(USI_TCP_MAX_ARG_NUM*sizeof(char*));

  /* is the number of procs supplied? */
  size=USI_basic_pop_argv2int(&argc, &argv, "-size", "-");

  if(size<=0)
  {
    USI_tcp_agent_Error("AGENT: No or an illegal number of procs (-size) supplied", size);
    exit(-1);
  }

  /* is my address supplied? */
  if(arg=USI_basic_pop_argv2str(&argc, &argv, "-home", "-"))
  {
    /* -> YES: use this */
    if(index(arg, ':')!=NULL)
    {
      /* also a port number is supplied: */
      agent_port=USI_tcp_basic_str2port(strchr(arg, ':')+1);
      *(strchr(arg, ':'))='\0';
    }
    else
    {
      /* choose default port to listen: */
      agent_port=USI_tcp_basic_str2port(USI_TCP_LISTEN_PORT);
    }
    agent_host=USI_tcp_basic_str2host(arg);
  }
  else
  {
    /* else -> NO: determine my IP and choose a port to listen: */
    agent_host=USI_tcp_basic_local_IP();    
    agent_port=USI_tcp_basic_str2port(USI_TCP_LISTEN_PORT);
  }
  
  memset(&(listen_socket.addr), 0, sizeof(listen_socket.addr));
 
  listen_socket.addr.sin_family=AF_INET;
  listen_socket.addr.sin_addr.s_addr=agent_host;
  listen_socket.addr.sin_port=agent_port;

  /* print the agent's address, so that the clients can read it ... */
  fprintf(USI_TCP_AGENT_STDOUT, "%s:%d\n", inet_ntoa(listen_socket.addr.sin_addr), ntohs(listen_socket.addr.sin_port));
  fflush(USI_TCP_AGENT_STDOUT);


  /********************************
--**  LISTENING FOR THE CLIENTS:  **-----------------
   ********************************/
  
  listen_socket.socket=USI_tcp_basic_socket(PF_INET, SOCK_STREAM);

  USI_tcp_basic_bind(listen_socket.socket, listen_socket.addr);

  /* prepare to listen for the clients: */
  USI_tcp_basic_listen(listen_socket.socket, size);

  client_socket=(USI_tcp_Socket*)malloc(size*sizeof(USI_tcp_Socket));
  rank_map=(USI_rank_t*)malloc(size*sizeof(USI_rank_t));

  for(client_iter=0; client_iter<size; client_iter++) rank_map[client_iter]=-1;

  for(client_count=0; client_count<size; client_count++)
  {
    memset(&(client_socket[client_count].addr), 0, sizeof(client_socket[client_count].addr));
  
    client_socket[client_count].socket=USI_tcp_basic_accept(listen_socket.socket, &(client_socket[client_count].addr));

    client_host=client_socket[client_count].addr.sin_addr.s_addr;
    client_port=client_socket[client_count].addr.sin_port;

#ifdef _USI_VERBOSE
    fprintf(USI_VERBOSE_STDOUT, "Connection from: %s:%d\n", inet_ntoa(client_socket[client_count].addr.sin_addr), ntohs(client_socket[client_count].addr.sin_port));
    fflush(USI_VERBOSE_STDOUT);
#endif

    /* store the address of the client as given by this connection: */
    snprintf(address_table[client_count], 24, "%s:%d", inet_ntoa(client_socket[client_count].addr.sin_addr), ntohs(client_socket[client_count].addr.sin_port));


   /************************************
---**  AGENT-CLIENT STARTUP PROTOCOL:  **-----------------
    ************************************/

    if( (cmd=USI_tcp_recv_cmd(client_socket[client_count].socket)) != USI_TCP_CMD_INIT)
    {
      USI_tcp_agent_Error("got invalid command / INIT was expected", cmd);
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }
    
    /*
     |   This agent lets the clients talk at first.
     |   --> send ACKN and check if the client wants to be the master
     |       (in this case it replies with an LSTN)
     |   --> if the client does not want to be the master (replies DONE)
     |       the agent firstly has to act as the master
     */
    
    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_AGNT);

    USI_tcp_SlaveMode();
    USI_tcp_MasterMode();

    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_FINI);
    
    if( (cmd=USI_tcp_recv_cmd(client_socket[client_count].socket)) != USI_TCP_CMD_ACKN)
    {
      USI_tcp_agent_Error("got invalid command / ACKN was expected", cmd);
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }
  }
}

static int USI_tcp_MasterMode(void)
{

   /*********************************
---**  AGENT IS NOW IN MASTER MODE: **-----------------
    *********************************
    |
    |   Now it's time to talk to the client:
    |
    */

    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_LSTN);

    if( (cmd=USI_tcp_recv_cmd(client_socket[client_count].socket)) != USI_TCP_CMD_ACKN)
    {
      USI_tcp_agent_Error("got invalid command / ACKN was expected", cmd);
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }

    /* has this client supplied a home address?
       If not, send him its own address, as it will be toled to the other clients:
    */
    if(!USI_basic_check_arg(&exchange_argc, &exchange_argv, "-home", "-"))
    {
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      USI_tcp_send_arg(client_socket[client_count].socket, "-home");

      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);    
      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%s:%d", inet_ntoa(client_host), ntohs(client_port));
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);     
    }
    
    /* does this client need the information about the "size" ? */
    if(environ.size<0)
    {
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      USI_tcp_send_arg(client_socket[client_count].socket, "-size");
      
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);    
      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%d", size);
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);
    }
    
    /* does this client need the information about its rank ? */
    if(environ.rank<0)
    {
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      USI_tcp_send_arg(client_socket[client_count].socket, "-rank");
    
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);    
      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%d", rank);    
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);
    }

    for(client_iter=0; client_iter<client_count; client_iter++)
    {
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      USI_tcp_send_arg(client_socket[client_count].socket, "-connect");
      
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);

      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, address_table[client_iter]);
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);
    }

    if(client_count<size-1)
    {
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "-listen");
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);

      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ARGV);
      snprintf(exchange_arg, USI_TCP_MAX_ARG_SIZE, "%d", size-client_count-1);
      USI_tcp_send_arg(client_socket[client_count].socket, exchange_arg);
    }
    
    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_DONE);
}

static int USI_tcp_SlaveMode(void)
{
  /*********************************
--**  AGENT IS NOW IN SLAVE MODE:  **-----------------
   *********************************/

  if( (cmd=USI_tcp_recv_cmd(client_socket[client_count].socket)) != USI_TCP_CMD_LSTN)
  {
    USI_tcp_agent_Error("got invalid command / LSTN was expected", cmd);
    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
    exit(-1);
  }
  
    USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_ACKN);

    cmd=USI_tcp_recv_cmd(client_socket[client_count].socket);

    exchange_argc=0;
    while(cmd != USI_TCP_CMD_DONE)
    {
      if(cmd != USI_TCP_CMD_ARGV)
      {
	USI_tcp_agent_Error("got invalid command / ARGV or DONE was expected", -1);
	USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
	exit(-1);
      }  

      USI_tcp_recv_arg(client_socket[client_count].socket, &(exchange_argv[exchange_argc]));
     
      exchange_argc++;
      if(exchange_argc>=USI_TCP_MAX_ARG_NUM)
      {
	exchange_argc=USI_TCP_MAX_ARG_NUM;
#ifdef _USI_VERBOSE
	fprintf(USI_VERBOSE_STDOUT, "WARNING: Client sent too much arguments...\n");
	fflush(USI_VERBOSE_STDOUT);
#endif
      }
      
      cmd=USI_tcp_recv_cmd(client_socket[client_count].socket);
    }

    environ.rank=-1;
    environ.size=-1;
 
    /* Has this client supplied a rank ? */
    environ.rank=USI_basic_pop_argv2int(&exchange_argc, &exchange_argv, "-rank", "-");

    if(environ.rank>-1)
    {
      if(environ.rank<size)
      {
	if(rank_map[environ.rank]==-1)
	{
	  rank_map[environ.rank]=client_count;
#ifdef _USI_VERBOSE
	  fprintf(USI_VERBOSE_STDOUT, "This client wanted to get rank %d -> OK\n", environ.rank );
	  fflush(USI_VERBOSE_STDOUT);
#endif
	}
	else
	{
	  /* ERROR */
	  USI_tcp_agent_Error("Rank is already in use", environ.rank);
	  USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
	  exit(-1);
	}
      }
      else
      {
	/* ERROR */
	USI_tcp_agent_Error("Rank is bigger than size", environ.rank);
	USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
	exit(-1);
      }
    }
    else
    {
      /* find a rank: */
      for(client_iter=0; client_iter<size; client_iter++)
      {
	if(rank_map[client_iter]==-1)
	{
	  rank_map[client_iter]=client_count;
	  rank=client_iter;
	  break;
	}
      }
#ifdef _USI_VERBOSE
      fprintf(USI_VERBOSE_STDOUT, "This client got rank %d\n", rank );
      fflush(USI_VERBOSE_STDOUT);
#endif   
    }

    /* Has this client supplied a size ? */
    environ.size=USI_basic_pop_argv2int(&exchange_argc, &exchange_argv, "-size", "-");
    if( (environ.size>-1) && (environ.size!=size) )
    {
      /* ERROR */
      USI_tcp_agent_Error("Client supplied an illegal size", environ.size);
      USI_tcp_send_cmd(client_socket[client_count].socket, USI_TCP_CMD_PERR);
      exit(-1);
    }

    /* Has this client supplied a home address? */
    if(arg=USI_basic_pop_argv2str(&exchange_argc, &exchange_argv, "-home", "-"))
    {
      if(index(arg, ':')!=NULL)
      {
	/* also a port number is supplied: */
	client_port=USI_tcp_basic_str2port(strchr(arg, ':')+1);
	*(strchr(arg, ':'))='\0';      
      }
      else
      { 
	/* choose default port to connect the agent: */
	/* LISTEN PORTS MUST BE DIFFERENT, DUE TO SMP MACHINES: */
	client_port=htons(ntohs(USI_tcp_basic_str2port(USI_TCP_LISTEN_PORT))+environ.rank+1);
      }
      client_host=USI_tcp_basic_str2host(arg);
    }
    
    snprintf(address_table[client_count], 24, "%s:%d", inet_ntoa(client_host), ntohs(client_port));

    printf("CLIENT WILL LISTEN ON %s\n", address_table[client_count]);    
}
