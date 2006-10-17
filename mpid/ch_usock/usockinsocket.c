/*
 |   CH_USOCK: usockinsocket.c
 |
 |   (formerly insocket.cpp)
 |
 */

#include "mydebug.h"
#include "usockinsocket.h"

/* prototype for external functions: */ 
void MPID_USOCK_Error(const char*,int);
void MPID_USOCK_SysError(const char*,int);

void inSocket_Constructor_0_(inSocket* inSocketPt)
{ 	
  DSECTION("inSocket_Constructor_0_");
  
  DSECTENTRYPOINT;
  
  inSocketPt->addr.sin_addr.s_addr=INADDR_ANY;
  inSocketPt->addr.sin_family=AF_INET;
  inSocketPt->fd=-1; 
  inSocketPt->type = SOCK_STREAM;
  
  DSECTLEAVE
    return;
}

void inSocket_Constructor_1_(unsigned int port, addrType ad, int theType, inSocket* inSocketPt)
{    
  DSECTION("inSocket_Constructor_1_");
  
  DSECTENTRYPOINT;
  
  inSocketPt->type = theType;
  inSocket_create(inSocketPt);
  if (ad==local) inSocketPt->addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  inSocket_bind(port, inSocketPt);
  
  DSECTLEAVE
    return;
}

void inSocket_Constructor_2_(unsigned int localport,const char* host,unsigned int remoteport,int theType, inSocket* inSocketPt)
{  
  DSECTION("inSocket_Constructor_2_");
  
  DSECTENTRYPOINT;
  
  inSocketPt->type = theType; 
  inSocket_create(inSocketPt);

  inSocket_bind(localport, inSocketPt);
  inSocket_connect_1_(host, remoteport, inSocketPt);
    
  DSECTLEAVE
    return;
}  

void inSocket_Destructor(inSocket* inSocketPt)
{ 
  DSECTION("inSocket_Destructor");
  
  DSECTENTRYPOINT;

  if(inSocketPt->fd != -1) {
    shutdown(inSocketPt->fd,SD_BOTH);
    closesocket(inSocketPt->fd); 
  }
  
  DSECTLEAVE
    return;
  
}

void inSocket_listen(int anz, inSocket* inSocketPt)
{ 
    DSECTION("inSocket_listen");

    DSECTENTRYPOINT;
    
    if (listen(inSocketPt->fd,anz)==-1)
    {
      MPID_USOCK_Error("inSocket_listen failed", -1);
    }

    DSECTLEAVE
      return;
}

void inSocket_bind(int port, inSocket* inSocketPt)
{
  DSECTION("inSocket_bind");

  BOOL val=TRUE;

#ifdef USE_NT2UNIX
  socklen_t namelen=sizeof(inSocketPt->addr);
#else
  int namelen=sizeof(inSocketPt->addr);
#endif

  DSECTENTRYPOINT;
  
  inSocketPt->addr.sin_port=htons(port);

#ifndef _WIN32
  /*#if !defined(_WIN32) && !defined(USE_NT2UNIX)*/
  if(port) setsockopt(inSocketPt->fd,SOL_SOCKET,SO_REUSEADDR,(const char FAR *)&val,sizeof(BOOL));
#endif

  if (bind(inSocketPt->fd,(struct sockaddr*)&(inSocketPt->addr),sizeof(inSocketPt->addr))==-1)
  {
    MPID_USOCK_Error("inSocket_bind failed", -1);
  }
  getsockname(inSocketPt->fd,(struct sockaddr*)&(inSocketPt->addr),&namelen);
  
  DSECTLEAVE
    return;
}

int inSocket_connect_0_(unsigned long host,unsigned int port, inSocket* inSocketPt)
{
  DSECTION("inSocket_connect(ulong,uint)");
  
  DSECTENTRYPOINT;
  
  inSocketPt->addr.sin_addr.s_addr=host;
  inSocketPt->addr.sin_port=htons(port);

  DSECTLEAVE
    return(connect(inSocketPt->fd,(struct sockaddr*)&(inSocketPt->addr),sizeof(inSocketPt->addr)));
}

void inSocket_connect_1_(const char* host,unsigned int port, inSocket* inSocketPt)
{ 
  DSECTION("inSocket_connect(char*,uint)");
  struct hostent* he;
  
  DSECTENTRYPOINT;
    
  if ((inSocketPt->addr.sin_addr.s_addr=inet_addr(host))==-1)
  {
    if ((he=gethostbyname(host)) != 0)
    {
      memcpy((char*)&(inSocketPt->addr.sin_addr),he->h_addr,he->h_length);
    }
    else
    {
      MPID_USOCK_Error("inSocket_connect_by_name failed", -1);
    }
  }
  
  inSocketPt->addr.sin_port=htons(port);    
  if(connect(inSocketPt->fd,(struct sockaddr*)&(inSocketPt->addr),sizeof(inSocketPt->addr)) ==-1)
  {
    MPID_USOCK_Error("inSocket_connect_by_name failed", -1);
  }
  
  DSECTLEAVE
    return;
}

void inSocket_accept(inSocket* other, inSocket* inSocketPt)
{
  DSECTION("inSocket_accept");

#ifdef USE_NT2UNIX
  socklen_t len=sizeof(inSocketPt->addr);
#else
  int len=sizeof(inSocketPt->addr);
#endif
    
  DSECTENTRYPOINT;
    
  if((inSocketPt->fd=accept(other->fd,(struct sockaddr*)&(inSocketPt->addr),&len))==-1)
  {
    MPID_USOCK_Error("inSocket_accept failed", -1);
  }
  
  DSECTLEAVE
    return;
}

void inSocket_create(inSocket* inSocketPt)
{
    DSECTION("inSocket_create");
    
    DSECTENTRYPOINT;
    
    inSocketPt->addr.sin_addr.s_addr=INADDR_ANY;
    inSocketPt->addr.sin_family=AF_INET;
    if ((inSocketPt->fd=WSASocket(AF_INET,inSocketPt->type,0,NULL,0,WSA_FLAG_OVERLAPPED))==SOCKET_ERROR)
    {
      MPID_USOCK_Error("inSocket_create failed", -1);
    }

    DSECTLEAVE
      return;
}     

int inSocket_read(void *buffer,unsigned size, inSocket* inSocketPt)
{
  /*
   |  Reads exactly size bytes from the socket.
   |  If an error occurs -1 is returned else .
   |  The data is returned in buffer.
   */
  
    DSECTION("inSocket_read");

    int result;
    unsigned s=size;
    char *pos=(char*)buffer;
    
    DSECTENTRYPOINT;
    
    do {
      result=recv(inSocketPt->fd,pos,s,0);
      if(result>=0) {
	s -= result;
	pos += result;
      }
      else
      {
	DSECTLEAVE
	  return result;
      }
    } while (s>0);
    
    DSECTLEAVE
      return size;
}


int inSocket_write(void *buffer,unsigned size, inSocket* inSocketPt)
{
  DSECTION("inSocket_write");

  int result;
  unsigned s=size;
  char *pos=(char*)buffer;
    
  DSECTENTRYPOINT;
    
  do {
    result=send(inSocketPt->fd,pos,s,0);
    if(result>0)
    {
      s -= result;
      pos += result;
    } 
    else 
    {
      DSECTLEAVE
	return result;
    }
  } while (s>0);
    
  DSECTLEAVE
    return size;
}

/* returns the fd of the socket: */
int inSocket_getInt(inSocket* inSocketPt)
{
  return inSocketPt->fd;
}

/* return the type of the socket */
int inSocket_getType(inSocket* inSocketPt)
{
  return inSocketPt->type;
}
	
int inSocket_getPort(inSocket* inSocketPt)
{
  return ntohs(inSocketPt->addr.sin_port);
}
