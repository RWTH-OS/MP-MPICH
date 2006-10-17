/* $id$ */

#include <windows.h>
#include <iostream>
#include "insocket.h"

inSocket::inSocket(void)
{      
	addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
	fd=-1; 
	type = SOCK_STREAM;
}
  
inSocket::inSocket(unsigned int port,addrType ad, int theType)
{ hostent * he;
  type = theType;
  create();
  if (ad==local)
   { if ((he=gethostbyname("localhost")) != 0)
      memcpy((char*)&addr.sin_addr,he->h_addr,he->h_length);
     else 
      throw(socketException("inSocket:gethostbyname failed."));
	 DBG("Got local name");
   }
  this->bind(port);
}
  
inSocket::inSocket(unsigned int localport,const char* host,unsigned int remoteport,int theType)
{ type = theType; 
  create();
  this->bind(localport);
  this->connect(host,remoteport);
}  

inSocket::~inSocket(void)
{ if(fd != -1) closesocket(fd); }

void inSocket::listen(int anz)
{ 
	DBG("listen: fd="<<fd<<" anz="<<anz);
	if (::listen(fd,anz)==-1)
		throw(socketException("listen failed"));
}
   
void inSocket::bind(int port)
{     int namelen=sizeof(addr);
	   addr.sin_port=htons(port);
       if (::bind(fd,(sockaddr*)&addr,sizeof(addr))==-1)
         throw (socketException("bind failed"));
	   getsockname(fd,(sockaddr*)&addr,&namelen);
	   DBG("bind: fd="<<fd<<" Port="<<port);
         
}
  
void inSocket::connect(const char* host,unsigned int port)
{ hostent* he; 
   
   DBG("Trying connection with "<<host<<" on port "<< port);
   if ((addr.sin_addr.s_addr=inet_addr(host))==-1)
   if ((he=gethostbyname(host)) != 0)
     memcpy((char*)&addr.sin_addr,he->h_addr,he->h_length);
   else 
    throw(socketException("connect:gethostbyname failed."));  
    
  addr.sin_port=htons(port);
  
  if(::connect(fd,(sockaddr*)&addr,sizeof(addr)) ==-1)
    throw (socketException("connect failed."));
}
    
void inSocket::accept(inSocket &other) {
	int len=sizeof(addr);
	cout << "Starting accept" << endl;
	
	if((fd=::accept(other.fd,(sockaddr*)&addr,&len))==-1) {
	  cout <<"accept failed !\n";
		throw socketException("accept failed");
	}
	DBG("Got Connection");
}

void inSocket::create(void)
{   addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    if ((fd=socket(AF_INET,type,0))==-1)
     throw (socketException("socket creation failed."));
}     

#ifdef _DEBUG
int inSocket::read(void *buffer,unsigned size) {
	int result;
	unsigned s=size;
	char *pos=(char*)buffer;
	do {
		result=recv(fd,pos,s,0);
		if(result>0) {
			s -= result;
			pos += result;
		}
		else return result;
	} while (s>0);
	return size;
}
#endif
