
#ifndef __INSOCKET_H__
#define __INSOCKET_H__

// This class encapsulates internet damain sockets.
// Sockets may be connection oriented (type SOCK_STREAM)
// (this is the default type) or connectionless (type SOCK_DGRAM).


#include <sys/types.h>
#include <winsock2.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "socketexception.h"

#define DBG(m) 

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
   void close() { if(fd != -1) {closesocket(fd); fd = -1;}}
   // Reads exactly size bytes from the socket.
   // If an error occurs -1 is returned else 0.
   // The data is returned in buffer.
   int read(void *buffer,unsigned size);

   int write(void *buffer,unsigned size);

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



#endif
