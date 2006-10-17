
#include <fcntl.h>
#include "usi_tcp_basic.h"

static void USI_tcp_basic_setsockopt(USI_tcp_basic_sockfd_t socket);

in_addr_t USI_tcp_basic_str2host(char* string)
{
  in_addr_t result;

  if(inet_aton(string, &result) == 0)
  {
    USI_tcp_basic_Error("inet_addr()");
  }

  /* return in host byte order: */
  return result;
}

in_port_t USI_tcp_basic_str2port(char* string)
{
  return (in_port_t)htons(atoi(string));
}

in_addr_t USI_tcp_basic_local_IP(void)
{
  struct hostent *Hostent;
  char HostName[USI_NAME_LENGTH];
  struct in_addr Address;

  gethostname(HostName, USI_NAME_LENGTH-1);

  /* FAILED ? */

  Hostent = gethostbyname(HostName);

  /* FAILED ? */

  memcpy(&Address.s_addr, *Hostent->h_addr_list, sizeof(Address.s_addr));

  return Address.s_addr;
}

USI_tcp_basic_sockfd_t USI_tcp_basic_socket(int family, int type)
{
  USI_tcp_basic_sockfd_t result;

#ifndef _WIN32
  if( (result=socket(family, type, 0)) < 0)
#else
  if( (result=socket(family, type, 0)) == INVALID_SOCKET)
#endif
  {
    USI_tcp_basic_Error("socket()");
  }
  
  return result;
}

void USI_tcp_basic_connect(USI_tcp_basic_sockfd_t sockfd,  struct sockaddr_in addr)
{
  int trials;

  USI_tcp_basic_setsockopt(sockfd);

  for(trials=0; (trials<USI_MAX_TIMEOUT)||(USI_MAX_TIMEOUT<0); trials++)
  {
    if( connect(sockfd, (struct sockaddr*)&(addr), sizeof(addr)) >= 0 ) return;
    USI_basic_sleep(1);
  }
  
  USI_tcp_basic_Error("connect()");
}

void USI_tcp_basic_bind(USI_tcp_basic_sockfd_t sockfd, struct sockaddr_in addr)
{
  USI_tcp_basic_setsockopt(sockfd);

  if( bind(sockfd, (struct sockaddr*)&(addr), sizeof(addr)) < 0 )
  {
    USI_tcp_basic_Error("bind()");
  }
  
  /* getsockname ??? */
  return;
}

in_port_t USI_tcp_basic_portname(USI_tcp_basic_sockfd_t sockfd)
{
  struct sockaddr_in addr;

#ifndef _WIN32
  socklen_t len=sizeof(struct sockaddr_in);
#else
  int len=sizeof(struct sockaddr_in);
#endif

  getsockname(sockfd, (struct sockaddr*)&(addr), &len);
  
  return(addr.sin_port);
}


void USI_tcp_basic_listen(USI_tcp_basic_sockfd_t sockfd, int backlog)
{  
  if(backlog==0) backlog=5;
  
  if( listen(sockfd, backlog) < 0 )
  {
    USI_tcp_basic_Error("listen()");
  }
  
  return;
}

USI_tcp_basic_sockfd_t USI_tcp_basic_accept(USI_tcp_basic_sockfd_t sockfd, struct sockaddr_in *client_addr)
{
  USI_tcp_basic_sockfd_t result;

#ifndef _WIN32
  socklen_t len=sizeof(struct sockaddr_in);
#else
  int len=sizeof(struct sockaddr_in);
#endif

  if( (result=accept(sockfd, (struct sockaddr*) client_addr, &len)) < 0 )
  {
    USI_tcp_basic_Error("accept()");  
  }
  
  return result;
}

USI_tcp_basic_ssize_t USI_tcp_basic_recv(USI_tcp_basic_sockfd_t socket, USI_tcp_basic_pointer_t buf, USI_tcp_basic_size_t len)
{
  USI_tcp_basic_pointer_t pos = buf;
  USI_tcp_basic_size_t    size = len;
  USI_tcp_basic_ssize_t   result = 0;

  do {
    result=recv(socket, pos, size, 0);
    if(result>=0) {
      size -= result;
      pos += result;
    }
    else
    {
      USI_tcp_basic_Error("recv()");
      return result;
    }
  } while (size>0);
  
  return len;
}

USI_tcp_basic_ssize_t USI_tcp_basic_send(USI_tcp_basic_sockfd_t socket, USI_tcp_basic_pointer_t buf, USI_tcp_basic_size_t len)
{
  USI_tcp_basic_pointer_t pos = buf;
  USI_tcp_basic_size_t    size = len;
  USI_tcp_basic_ssize_t   result = 0;

  do {
    result=send(socket, pos, size, 0);
    if(result>=0)
    {
      size -= result;
      pos += result;
    } 
    else 
    {
      USI_tcp_basic_Error("send()");
      return result;
    }
  } while (size>0);
    
  return len;
}

static void USI_tcp_basic_setsockopt(USI_tcp_basic_sockfd_t sockfd)
{
  int value;
  struct linger lvalue={1,10};

  if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (USI_Pointer)&value, sizeof(value)) < 0 )
  {
    USI_tcp_basic_Error("setsockopt(SO_REUSEADDR)");
    return;
  }
  if( setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (USI_Pointer)&value, sizeof(value)) < 0 )
  {
    USI_tcp_basic_Error("setsockopt(SO_KEEPALIVE)");
    return;
  }      
    

  if( setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (USI_Pointer)&value, sizeof(value)) < 0 )
  {
    USI_tcp_basic_Error("setsockopt(TCP_NODELAY)");
    return;
  }
  if( setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (USI_Pointer)&lvalue, sizeof(struct linger)) < 0 )
  {
    USI_tcp_basic_Error("setsockopt(SO_LINGER)");
    return;
  }
  
  /* DON NOT set the socket to non-blocking mode: */
  /*
  if( (value=fcntl(sockfd, F_GETFL, 0)) < 0 )
  {
    USI_tcp_basic_Error("fcntl(F_GETFL)");
    return;
  }
  else
  {
    value = value | O_NONBLOCK;
    if( fcntl(sockfd, F_SETFL, value) < 0 )
    {
      USI_tcp_basic_Error("fcntl(F_GETFL)");
      return;
    }
  }
  */
}



#if 0
/* is this protocol independent ??? / is the recv-function protocol independent ??? */
char* USI_tcp_basic_argv2str(int* argc, char** argv[], char *string)
{
  int i, j;
  char* ret;

  for(i=0; i<(*argc)-1; i++)
  {
    if((*argv)[i])
    {
      if(strncmp((*argv)[i], "--", 2)==0) break;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(i++; i<(*argc); i++)
	{
	  if((*argv)[i])
	  {
	    if(strncmp((*argv)[i], "-", 1)==0)
	    {
	      i--;
	      break;;
	    }
	    /* XXX free(...) */
	    ret=(*argv)[i];
	    (*argv)[i] = 0;
	    return ret;
	  }
	}
      }
    }
  }

  return NULL;
}

char* USI_tcp_basic_argv2str2(int* argc, char** argv[], char *string)
{
  int i, j;
  char* ret;

  for(i=0; i<(*argc)-1; i++)
  {
    if((*argv)[i])
    {
      if(strncmp((*argv)[i], "---", 3)==0) break;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(i++; i<(*argc); i++)
	{
	  if((*argv)[i])
	  {
	    if(strncmp((*argv)[i], "--", 2)==0)
	    {
	      i--;
	      break;;
	    }
	    /* XXX free(...) */
	    ret=(*argv)[i];
	    (*argv)[i] = 0;
	    return ret;
	  }
	}
      }
    }
  }

  return NULL;
}


/* XXX !!! regard USI_MAX_ARG_NUM !!! */

int USI_tcp_basic_argv2int(int* argc, char** argv[], char *string)
{
  int i;
  int ret;
  
  for(i=0; i<(*argc)-1; i++)
  {
    if((*argv)[i])
    {
      if(strncmp((*argv)[i], "--", 2)==0) break;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(i++; i<(*argc); i++)
	{
	  if((*argv)[i])
	  {
	    if(strncmp((*argv)[i], "-", 1)==0)
	    {
	      i--;
	      break;;
	    }
	    /* XXX free(...) */
	    ret=atoi((*argv)[i]);
	    (*argv)[i] = 0;
	    return ret;
	  }
	}
      }
    }
  }

  return -1;
}

int USI_tcp_basic_argv2int2(int* argc, char** argv[], char *string)
{
  int i;
  int ret;
  
  for(i=0; i<(*argc)-1; i++)
  {
    if((*argv)[i])
    {
      if(strncmp((*argv)[i], "---", 3)==0) break;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(i++; i<(*argc); i++)
	{
	  if((*argv)[i])
	  {
	    if(strncmp((*argv)[i], "--", 2)==0)
	    {
	      i--;
	      break;;
	    }
	    /* XXX free(...) */
	    ret=atoi((*argv)[i]);
	    (*argv)[i] = 0;
	    return ret;
	  }
	}
      }
    }
  }

  return -1;
}

int USI_tcp_basic_get_arg(int* argc, char** argv[], char *string)
{
  int i;
  
  for(i=0; i<*argc; i++ )
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], "--")==0) break;
      if(strcmp((*argv)[i], string)==0) return 1;
    }
  }

  return 0;
}
#endif


void USI_tcp_basic_Error(const char *string)
{
  fprintf(stderr,"### USI_tcp_basic ERROR: %s failed -> ", string);
#ifndef _WIN32
  fprintf(stderr, " %s\n", strerror(errno));
#else
  fprintf(stderr, " %s\n", strerror(WSAGetLastError()));
#endif
  fflush(stderr);

  exit(-1);
}


