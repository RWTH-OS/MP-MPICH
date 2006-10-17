
#include "impi_tcp.h"

#undef  IMPI_MODULE_NAME
#define IMPI_MODULE_NAME "TCP"

static void IMPI_TCP_Setsockopt(int sockfd);
static void IMPI_TCP_Error(const char *string);

in_addr_t IMPI_TCP_Str2host(char* string)
{
  in_addr_t result;

  if(inet_aton(string, (struct in_addr*) &result) == 0)
  {
    IMPI_TCP_Error("inet_addr()");
  }

  /* return in host byte order: */
  return result;
}

in_port_t IMPI_TCP_Str2port(char* string)
{
  return (in_port_t)htons(atoi(string));
}

int IMPI_TCP_Socket(int family, int type)
{
  int result;

  if( (result=socket(family, type, 0)) < 0)
  {
    IMPI_TCP_Error("socket()");
  }
  
  return result;
}

void IMPI_TCP_Connect(int sockfd,  struct sockaddr_in addr)
{
  int trials;

  IMPI_TCP_Setsockopt(sockfd);

  for(trials=0; (trials<IMPI_MAX_TIMEOUT)||(IMPI_MAX_TIMEOUT<0); trials++)
  {
    if( connect(sockfd, (struct sockaddr*)&(addr), sizeof(addr)) >= 0 ) return;
    sleep(1);
  }
  
  IMPI_TCP_Error("connect()");
}

void IMPI_TCP_Bind(int sockfd, struct sockaddr_in addr)
{
  IMPI_TCP_Setsockopt(sockfd);

  if( bind(sockfd, (struct sockaddr*)&(addr), sizeof(addr)) < 0 )
  {
    IMPI_TCP_Error("bind()");
  }
  
  return;
}

void IMPI_TCP_Listen(int sockfd, int backlog)
{  
  if(backlog==0) backlog=5;
  
  if( listen(sockfd, backlog) < 0 )
  {
    IMPI_TCP_Error("listen()");
  }
  
  return;
}

int IMPI_TCP_Accept(int sockfd, struct sockaddr_in *client_addr)
{
  int result;

  int len=sizeof(struct sockaddr_in);

  if( (result=accept(sockfd, (struct sockaddr*) client_addr, &len)) < 0 )
  {
    IMPI_TCP_Error("accept()");  
  }
  
  return result;
}

static void IMPI_TCP_Setsockopt(int sockfd)
{
  int value;
  struct linger lvalue={1,10};

  if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0 )
  {
    IMPI_TCP_Error("setsockopt(SO_REUSEADDR)");
    return;
  }
  if( setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) < 0 )
  {
    IMPI_TCP_Error("setsockopt(SO_KEEPALIVE)");
    return;
  }      
    
  if( setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) < 0 )
  {
    IMPI_TCP_Error("setsockopt(TCP_NODELAY)");
    return;
  }
  if( setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lvalue, sizeof(struct linger)) < 0 )
  {
    IMPI_TCP_Error("setsockopt(SO_LINGER)");
    return;
  }
}

size_t IMPI_TCP_Recv(int socket, void *buf, size_t len)
{
  void  *pos = buf;
  size_t size= len;
  size_t result = 0;

  do {
    result=recv(socket, pos, size, 0);
    if(result>=0) {
      size -= result;
      pos += result;
    }
    else
    {
      IMPI_TCP_Error("recv()");
      return result;
    }
  } while (size>0);
  
  return len;
}

size_t IMPI_TCP_Send(int socket, void *buf, size_t len)
{
  void  *pos = buf;
  size_t size= len;
  size_t result = 0;

  do {
    result=send(socket, pos, size, 0);
    if(result>=0)
    {
      size -= result;
      pos += result;
    } 
    else 
    {
      IMPI_TCP_Error("send()");
      return result;
    }
  } while (size>0);
    
  return len;
}

int IMPI_TCP_Select(int socket, int blocking)
{
  int result;
  fd_set recv_set;
  struct timeval immediately={0,0};

  FD_ZERO(&recv_set);
  FD_SET(socket, &recv_set);
  
  if(blocking)
  {
    DBG("Blocking Select");
    result=select(FD_SETSIZE, &recv_set, NULL, NULL, NULL);
  }
  else
  {
    DBG("Non blocking Select...");
    result=select(FD_SETSIZE, &recv_set, NULL, NULL, &immediately);
    DBG1("Non blocking Select: %d", result);
  }
  
  if(FD_ISSET(socket, &recv_set)) return 1;

  return 0;
}


static void IMPI_TCP_Error(const char *string)
{
  fprintf(stderr,"### IMPI: TCP-ERROR -- %s failed -> ", string);
  fprintf(stderr, " %s\n", strerror(errno));
  fflush(stderr);

  exit(-1);
}
