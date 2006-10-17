
#ifndef __USOCK_INSOCKET_H__
#define __USOCK_INSOCKET_H__

/*
 |   CH_USOCK: usockinsocket.h
 |
 |   (formerly insocket.h)
 |
 */

#include <sys/types.h>

#if defined USE_NT2UNIX
#include "winsock2.h"
#else
#include <winsock2.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

   /*********************************
--- ** type definitions            ** -------------------------------------------------------
    *********************************/

typedef 
enum _addrType {local, any}
addrType;

typedef
struct _inSocket
{
  int fd;
  struct sockaddr_in addr;
  int type; /* either SOCK_DGRAM or SOCK_STREAM (this is the default) */
} 
inSocket;    


   /*********************************
--- ** exported functions          ** -------------------------------------------------------
    *********************************/

/* Just initialize the addr structure, set type to SOCK_STREAM: */
void inSocket_Constructor_0_(inSocket* inSocketPt);

/* Creates a socket and binds it to port port. If port is 0 the constructor selects a port: */
void inSocket_Constructor_1_(unsigned int port, addrType ad, int theType, inSocket* inSocketPt);

/* Creates a socket, binds it to port and connects it to host host: */
void inSocket_Constructor_2_(unsigned int localport,const char* host,unsigned int remoteport, int theType, inSocket* inSocketPt);

/* Closes the socket: */
void inSocket_Destructor(inSocket* inSocketPt);

/* Creates a socket with inaddr_any: */
void inSocket_create(inSocket* inSocketPt);

/* Makes the socket listen at the connected port: */
void inSocket_listen(int anz, inSocket* inSocketPt);	   

/* port=0 means a port is selected: */
void inSocket_bind(int port, inSocket* inSocketPt);

int inSocket_connect_0_(unsigned long host,unsigned int port, inSocket* inSocketPt);

void inSocket_connect_1_(const char* host,unsigned int port, inSocket* inSocketPt);
 
void inSocket_accept(inSocket* other, inSocket* inSocketPt);   

/* Reads exactly size bytes from the socket: */
int inSocket_read(void *buffer,unsigned size, inSocket* inSocketPt);

int inSocket_write(void *buffer,unsigned size, inSocket* inSocketPt);

/* returns the fd of the socket: */
int inSocket_getInt(inSocket* inSocketPt);

/* return the type of the socket: */
int inSocket_getType(inSocket* inSocketPt);
	
/* return the port of the socket: */
int inSocket_getPort(inSocket* inSocketPt);

#endif
