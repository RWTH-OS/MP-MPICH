/* $id$ */
/* generally test if sockets work */

/* 
   HOWTO USE:
   start the demon on a host ( host1> ./test_sockets )
   start the ping process on another host ( hopst2> ./test_sockets host1 )

   on successfully completion 100 Messages will be displayed and the completion
   time is mentioned
*/

#include <windows.h>
#include "insocket.h"
#include <iostream>
using namespace std;

#define RECEIVEPORT 5043
#define ANZ 100
#define MESSAGESIZE 1024
#define REPEAT 2

char buf[MESSAGESIZE];

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tz)
{
	/*
	double i;
	clock_t system_time;

	system_time = clock();
	i = ((double) system_time) / ((double) CLOCKS_PER_SEC);
	tp->tv_sec = (int) i;
	tp->tv_usec = (int) ((i - tp->tv_sec) * 1000000.0);
	*/

	LARGE_INTEGER clocks;
	double secs, usecs;
	static LARGE_INTEGER freq;
	double dfreq;



	QueryPerformanceCounter(&clocks);
	QueryPerformanceFrequency(&freq);
	dfreq = (double)freq.QuadPart;

	secs = (double)clocks.QuadPart/dfreq;	
	usecs = ((double)clocks.QuadPart - ((double)((int)secs) * dfreq)) / (dfreq/1000000.0);

	tp->tv_sec = (long int)secs;
	tp->tv_usec	= (long int)usecs;

	  
	return(0);
}
#endif
inline int connectTo(sockaddr_in &addr)
{
int s=socket(AF_INET,SOCK_STREAM,0);
   addr.sin_port=htons(RECEIVEPORT);
   if(connect(s,(sockaddr*)&addr,sizeof(addr))==-1) {	
     cout<< "connect failed\n"; 
     exit(1);
   }
   return s;  
}     
   
int SetSocketOptions(SOCKET s) {
	BOOL val=TRUE;
	//unsigned long val2=1;
	linger l={1,10};
	int result;

	result=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(const char FAR *)&val,sizeof(BOOL));
	if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_LINGER,(const char FAR *)&l,sizeof(linger));
	if(result!=SOCKET_ERROR) result=setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(const char FAR *)&val,sizeof(BOOL));
	//result=ioctlsocket(s,FIONBIO,&val2);
	return result==SOCKET_ERROR?-1:0;
}
    


void pong()
{
  int conn;
#ifdef WIN32
int
#else
socklen_t 
#endif
  len=sizeof(sockaddr_in);
  
 char buf[MESSAGESIZE];
  timeval t1,t2;
 sockaddr_in addr;
 fd_set reads;
 int result;

  DBG("Entering Pong()");

  try{
    inSocket s(RECEIVEPORT,any);
    s.listen(1);
    DBG("accept...");
    conn=accept(s,(sockaddr*)&addr,&len);
    SetSocketOptions(conn);
    if(conn<0){cout<<"accept failed\n"; return;}
    	FD_ZERO(&reads);
  	FD_SET(conn,&reads);
  	DBG("starting select ..."<<conn);
	select(conn+1,&reads,0,0,0);
	DBG("... return");
	for(int j=0;j<REPEAT;j++) {
		gettimeofday(&t1,0);
		for(int i=0;i<ANZ;i++) {
			int toGet=MESSAGESIZE;
			select(conn+1,&reads,0,0,0);
			DBG("new mail.");
			while(toGet>0) {
				result=recv(conn,buf,toGet,0);
 				if(result==SOCKET_ERROR) break;
				toGet-=result;
				cerr<<"Read: "<<MESSAGESIZE-toGet<<" Bytes in "<<i<<"\n";
			}
			send(conn,buf,MESSAGESIZE,0);			
		}
		gettimeofday(&t2,0);
		if(t2.tv_sec>t1.tv_sec)
			cout<<"Pong: Time for "<<ANZ<<" Messages: "<<t2.tv_sec-t1.tv_sec<<"sec "<<t2.tv_usec<<" u_sec\n";
		else
			cout<<"Pong: Time for "<<ANZ<<" Messages: "<<(t2.tv_usec-t1.tv_usec)<<" u_sec\n";
	}
} catch (socketException e) {cout<<e<<endl; return;}  

  closesocket(conn);
  
}    
    
    
void ping(char* host)
{
  
  timeval t1,t2;
  fd_set reads;
  int result;
  strcpy(buf,"HALLO\n");
  DBG("Entering Ping()");
  try{
    inSocket s(0,host,RECEIVEPORT);
	SetSocketOptions(s);
	FD_SET(s,&reads);
	for(int j=0;j<REPEAT;j++) {
	    gettimeofday(&t1,0);
		for(int i=0;i<ANZ;i++) {  
			int toSend=MESSAGESIZE;
			while(toSend>0) {
			        DBG("send ...");
				result=send(s,buf,toSend,0);
				if(result==SOCKET_ERROR) {
				  DBG("send failed.");
				  break;
				}
				toSend-=result;
				cerr<<"Sent: "<<MESSAGESIZE-toSend<<" Bytes\n" ;
			}
			select(1,&reads,0,0,0);
			recv(s,buf,MESSAGESIZE,0);
		}
     gettimeofday(&t2,0);
     if(t2.tv_sec>t1.tv_sec)
		 cout<<"Ping: Time for "<<ANZ<<" Messages: "<<t2.tv_sec-t1.tv_sec<<" sec "<<t2.tv_usec<<" usec\n";
     else
		cout<<"Ping: Time for "<<ANZ<<" Messages: "<<(t2.tv_usec-t1.tv_usec)<<" usec\n";           
	}
   }catch(socketException e){
      cout<<e<<endl;
      return;
   }
}          
   



int main(int argc,char** argv)
{
	WORD wVersionRequested; 
	WSADATA wsaData; 
	int err;
	wVersionRequested=1;
	err=WSAStartup(wVersionRequested,&wsaData);

	if(err) {cerr<<"Startup failed \n"; return 1;}

	if(argc<2)
		pong();
	else
		ping(argv[1]);

	WSACleanup();
	return 1;
}    
