
#include "usi_basic.h"

USI_basic_sockfd_t USI_basic_socket(int family, int type)
{
  USI_basic_sockfd_t result;

#ifndef _WIN32
  if( (result=socket(family, type, 0)) < 0)
#else
  if( (result=socket(family, type, 0)) == INVALID_SOCKET)
#endif
  {
    USI_basic_Error("socket()");
  }

  return result;
}

int USI_basic_connect(USI_basic_sockfd_t sockfd,  struct sockaddr *addr)
{
  int trials;

  for(trials=0; (trials<USI_MAX_TIMEOUT)||(USI_MAX_TIMEOUT<0); trials++)
  {
    if(connect(sockfd, addr, sizeof(addr)) >= 0 ) return 0;
    USI_basic_sleep(1);
  }
  
  USI_basic_Error("connect()");
  return -1;
}

int USI_basic_bind(USI_basic_sockfd_t sockfd, struct sockaddr *addr)
{
  if (bind(sockfd, addr, sizeof(addr)) < 0 )
  {
    USI_basic_Error("bind()");
    return -1;
  }

  return 0;
}

int USI_basic_listen(USI_basic_sockfd_t sockfd, int backlog)
{ 
  if(backlog==0) backlog=5;
  
  if( listen(sockfd, backlog) < 0 )
  {
    USI_basic_Error("listen()");
    return -1;
  }
  
  return 0;
}

USI_basic_sockfd_t USI_basic_accept(USI_basic_sockfd_t sockfd, struct sockaddr *addr, USI_basic_socklen_t* len)
{  
  USI_basic_sockfd_t result;
  
  if( (result=accept(sockfd, addr, len)) < 0 )
  {
    USI_basic_Error("accept()");  
  }
  
  return result;
}

USI_basic_ssize_t USI_basic_recv(USI_basic_sockfd_t socket, USI_basic_pointer_t buf, USI_basic_size_t len)
{
  USI_basic_pointer_t pos = buf;
  USI_basic_size_t    size = len;
  USI_basic_ssize_t   result = 0;

  do {
    result=recv(socket, pos, size, 0);
    if(result>=0) {
      size -= result;
      pos += result;
    }
    else
    {
      USI_basic_Error("recv()");
      return result;
    }
  } while (size>0);
  
  return len;
}

USI_basic_ssize_t USI_basic_send(USI_basic_sockfd_t socket, USI_basic_pointer_t buf, USI_basic_size_t len)
{
  USI_basic_pointer_t pos = buf;
  USI_basic_size_t    size = len;
  USI_basic_ssize_t   result = 0;

  do {
    result=send(socket, pos, size, 0);
    if(result>=0)
    {
      size -= result;
      pos += result;
    } 
    else 
    {
      USI_basic_Error("send()");
      return result;
    }
  } while (size>0);
    
  return len;
}

char* USI_basic_pop_argv2str(int* argc, char** argv[], char* string, char* end)
{
  int i, j, k;
  char* ret;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return NULL;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j])
	  {
	    if(strncmp((*argv)[j], end, strlen(end))==0)
	    {
	      (*argv)[i] = 0;
	      return NULL;
	    }
	    ret=(*argv)[j];
	    (*argv)[j] = 0;

	    for(k=j+1; k<(*argc); k++)
	    {
	      if((*argv)[k])
	      {
		if(strncmp((*argv)[k], end, strlen(end))==0) (*argv)[i] = 0;
		break;
	      }
	    }
	    return ret;
	  }
	}
      }
    }
  }

  return NULL;
}

char* USI_basic_get_argv2str(int* argc, char** argv[], char* string, char* end)
{
  int i, j, k;
  char* ret;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return NULL;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
	*/
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j]) return((*argv)[j]);
	}
      }
    }
  }

  return NULL;
}

int USI_basic_pop_argv2int(int* argc, char** argv[], char* string, char* end)
{
  int i, j, k;
  int ret;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return -1;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j])
	  {
	    if(strncmp((*argv)[j], end, strlen(end))==0)
	    {
	      (*argv)[i] = 0;
	      return -1;
	    }
	    ret=atoi((*argv)[j]);
	    (*argv)[j] = 0;

	    for(k=j+1; k<(*argc); k++)
	    {
	      if((*argv)[k])
	      {
		if(strncmp((*argv)[k], end, strlen(end))==0) (*argv)[i] = 0;
		break;
	      }
	    }
	    return ret;
	  }
	}
      }
    }
  }

  return -1;
}

int USI_basic_get_argv2int(int* argc, char** argv[], char* string, char* end)
{
  int i, j, k;
  char* ret;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return -1;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
	*/
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j]) return atoi((*argv)[j]);
	}
      }
    }
  }

  return -1;
}

int USI_basic_check_arg(int* argc, char** argv[], char *string, char* end)
{
  int i;
  
  for(i=0; i<*argc; i++ )
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) break;
      if(strcmp((*argv)[i], string)==0) return 1;
    }
  }

  return 0;
}


void USI_basic_Error(const char *string)
{
  fprintf(stderr,"### USI_basic ERROR: %s failed -> ", string);
  fprintf(stderr, " %s\n", USI_basic_error_code);
  fflush(stderr);
}


