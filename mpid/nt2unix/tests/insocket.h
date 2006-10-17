/* $id$ */
 
#ifndef __INSOCKET_H__
#define __INSOCKET_H__

// This class encapsulates internet damain sockets.
// Sockets may be connection oriented (type SOCK_STREAM)
// (this is the default type) or connectionless (type SOCK_DGRAM).


#include <sys/types.h>
#include <winsock.h>

#include <errno.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "socketexception.h"
#include "debug.h"

enum addrType {local, any};

class inSocket
{ public:
   // Just initialize the addr structure, set type to SOCK_STREAM. 
   inSocket(void);

   // Creates a socket and binds it to port port. If port is 0
   // the constructor selects a port.
   inSocket(unsigned int port,addrType ad=local, int theType=SOCK_STREAM);

   // Creates a socket, binds it to port and connects it to host host.
   inSocket(unsigned int localport,const char* host,unsigned int remoteport, int theType=SOCK_STREAM);

   // Closes the socket
   virtual ~inSocket(void);

   // Creates a socket with inaddr_any	
   void create(void);

   // Makes the socket listen at the connected port.
   void listen(int anz=FD_SETSIZE);	   

   // Port=0 means a port is selected.
   void bind(int port=0);

   void connect(const char* host,unsigned int port);
 
   void accept(inSocket &other) ;   

   // Reads exactly size bytes from the socket.
   // If an error occurs -1 is returned else 0.
   // The data is returned in buffer.
   virtual int read(void *buffer,unsigned size);

   inline virtual int write(void *buffer,unsigned size) { return send(fd,(char*)buffer,size,0);}

   // Returns the fd of the socket.
   operator const int(void) {return fd;}

   // return the type of the socket
   int getType(void) {return type;}
	
   int getPort() {return ntohs(addr.sin_port);}
  protected:

   int fd;
   sockaddr_in addr;

   int type; // either SOCK_DGRAM or SOCK_STREAM (this is the default) 
};    


#ifndef _DEBUG
 inline int inSocket::read(void *buffer,unsigned size) {
	int result;
	unsigned s=size;
	char *pos=(char*)buffer;
	do {
		result=recv(fd,pos,s,0);
		if((result!=SOCKET_ERROR)&&(result>0)) {
			s -= result;
			pos += result;
		}
		else return -1;
	} while (s>0);
	return size;
   }
#endif



#endif
