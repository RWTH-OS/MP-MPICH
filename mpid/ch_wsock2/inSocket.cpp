
#include "insocket.h"
#include <stdio.h>
#include <iostream>

inSocket::inSocket(void)
{ 	
	addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
	fd=-1; 
	type = SOCK_STREAM;
}

inSocket::inSocket(unsigned int port,addrType ad, int theType)
{ 
	
	type = theType;
	create();
	if (ad==local) {
	    addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	}
	this->bind(port);
}

inSocket::inSocket(unsigned int localport,const char* host,unsigned int remoteport,int theType)
{ 
	type = theType; 
	create();
	this->bind(localport);
	this->connect(host,remoteport);
}  

inSocket::~inSocket(void)
{ 
	if(fd != -1) {
		shutdown(fd,SD_BOTH);
		closesocket(fd); 
	}
}

void inSocket::listen(int anz)
{ 
	DBG("listen: fd="<<fd<<" anz="<<anz);
	if (::listen(fd,anz)==-1)
		throw(socketException("listen failed"));
}

void inSocket::bind(int port)
{   
	int namelen=sizeof(addr);
	addr.sin_port=htons(port);
#ifndef _WIN32
	BOOL val=TRUE;
	if(port)
		setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const char FAR *)&val,sizeof(BOOL));
#endif
	if (::bind(fd,(sockaddr*)&addr,sizeof(addr))==-1)
		throw (socketException("bind failed"));
	getsockname(fd,(sockaddr*)&addr,&namelen);
	DBG("bind: fd="<<fd<<" Port="<<port);
}

void inSocket::connect(unsigned long host,unsigned int port) {
    addr.sin_addr.s_addr=host;
    addr.sin_port=htons(port);
    if(::connect(fd,(sockaddr*)&addr,sizeof(addr)) ==-1)
	throw (socketException("connect failed."));
}

void inSocket::connect(const char* host,unsigned int port)
{ 
	hostent* he;
	char er[255];
	
	DBG("Trying connection with "<<host<<" on port "<< port);
	if ((addr.sin_addr.s_addr=inet_addr(host))==-1) {
		if ((he=gethostbyname(host)) != 0) {
			memcpy((char*)&addr.sin_addr,he->h_addr,he->h_length);
			DBG("The IP is: "<<inet_ntoa(addr.sin_addr));
		} else {
			sprintf(er,"connect: gethostbyname(%s) failed.",host);
			throw (socketException((const char *)er));
		}
	}
		
	addr.sin_port=htons(port);
	
	if(::connect(fd,(sockaddr*)&addr,sizeof(addr)) ==-1)
		throw (socketException("connect failed."));
}

void inSocket::accept(inSocket &other) {
	int len=sizeof(addr);
	DBG("Starting accept");
	if((fd=::accept(other.fd,(sockaddr*)&addr,&len))==-1) {
		throw socketException("accept failed");
	}
	DBG("Got Connection");
}

void inSocket::create(void)
{
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_family=AF_INET;
	if ((fd=WSASocket(AF_INET,type,0,NULL,0,WSA_FLAG_OVERLAPPED))==SOCKET_ERROR) //wsock2function
	//if ((fd=socket(AF_INET,type,0))==SOCKET_ERROR)
		throw (socketException("socket creation failed."));
}     

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


int inSocket::write(void *buffer,unsigned size) { 
	int result;
	unsigned s=size;
	char *pos=(char*)buffer;
	do {
		result=send(fd,pos,s,0);
		if(result>0) {
			s -= result;
			pos += result;
		} else return result;
	} while (s>0);
	return size;

	
}