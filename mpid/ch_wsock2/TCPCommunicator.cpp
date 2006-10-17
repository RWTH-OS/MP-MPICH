#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctype.h>
#include "insocket.h"
#include <Ws2tcpip.h>
#ifndef NOSP4
#include <Iphlpapi.h>
#include <Iprtrmib.h>
#endif

#include "getopt.h"

#if defined(WIN32) && (_MSC_VER < 1100)
#define NOMINMAX 
#include <winsock2.h>

#include <bool>



#endif

#include "tcpcommunicator.h"

using namespace std;

#define HPART(IP)((IP)>>24)
#define CNET(IP) (((IP)&0x00FF0000L) >> 16)
#define BNET(IP) (((IP)&0x0000FF00L) >> 8)
#define ANET(IP)  ((IP)&0x000000FFL)
#define IP(m) ANET(m)<<'.'<<BNET(m)<<'.'<<CNET(m)<<'.'<<HPART(m)

#define VOLATILE volatile



extern "C" {
#include <mpid.h>
#include "..\ch_ntshmem\shpackets.h"
#include "..\ch_ntshmem\shdef.h"
}



// Align to the next 32-Bit value. SPARC likes that.
#define DWORD_ALIGN(x) { \
	if ((x) & 3) \
	x = ((((x) >> 2)+1) << 2); \
} 


extern "C" {
	/*! \var char *optarg 
	Belongs to getopt()	
	*/
	extern char *optarg;

	/*!
	belong to getopt()
	*/
	extern int optind, opterr, optopt;

	/*!
	\relates CTCPCommunicator
	Used to reset the getopt() function.
	*/
	void resetGetOpt();

	void wsock_error(const char*,int);
	void wsock_syserror(const char*,int);

	/* All the stuff from ch_lfshmem I need here */

	extern HANDLE MemHandle;
	extern HANDLE tmp_mutex;
	extern void *actual_mapping_address;
	extern unsigned int mem_size;
	/* MPID_child_pid: array with process ids of local clients (shmem) */
	extern HANDLE MPID_child_pid[MPID_MAX_PROCS];
	extern HANDLE *MPID_Events;



	void p2p_init(int maxprocs,int memsize);
	void p2p_wtime_init();
	int MPID_GetIntParameter( char *, int );
/* Global2Local: Array of indizes set by master if shmem is performed (MasterSMP) 
   Global2Local: At index GlobalId (rank) the LocalId (for shmem communication) is stored 
					local Ids start at 1 and are indizes for shmem arrays e.g. MPID_child_pid
					zero means this process is connected via TCP
*/
	unsigned long *Global2Local, *Local2Global;

}


#ifdef _MSC_VER
#pragma warning( disable : 4018)
#endif

#define ASSERT(m) 

#define MAX_TIMEOUT 120

/*! \file TCPCommunicator.cpp
\brief This file contains the implementation of CTCPCommunicator.
*/




/*! \relates CTCPCommunicator
Indicates weather the communicator is in the shutting down the connections.
*/
BOOL exiting=FALSE;

/*!
\relates CTCPCommunicator
Used to set several options on the sockets.
*/
int SetSocketOptions(SOCKET s) {
	BOOL val=TRUE;
	unsigned long val2=16384;
	linger l={1,10};
	int result=SOCKET_ERROR+1;

	result=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(const char FAR *)&val,sizeof(BOOL));
	if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_LINGER,(const char FAR *)&l,sizeof(linger));
	if(result!=SOCKET_ERROR) result=setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(const char FAR *)&val,sizeof(BOOL));
	if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_RCVBUF,(const char FAR *)&val2,sizeof(unsigned long));
	if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char FAR *)&val2,sizeof(unsigned long));
	return result==SOCKET_ERROR?-1:0;
}

/*!
The default constructor called by SVMInit().
It just initializes the member variables and calls WSAStartup().
*/
CTCPCommunicator::CTCPCommunicator() {
	DBG("Entering CTCPCommunicator()");
	WORD wVersionRequested; 
	WSADATA wsaData; 
	int err;
	wVersionRequested=2;
	err=WSAStartup(wVersionRequested,&wsaData);
	hosts=0;
	ShuttingDown=0;
	myId=numProcs=0;
	if(err) throw socketException("WSAStartup failed");
	LocalMaster = true;
	Global2Local=Local2Global=0;
}

/*!
The destructor calls CloseSockets().
After that all sockets should be destroyed.
*/
CTCPCommunicator::~CTCPCommunicator() {
	DBG("Entering ~CTCPCommunicator()");
	CloseSockets();
	if(WSACleanup()) throw socketException("WSACleanup failed");
	if(MPID_Events) {
		for(int i=0;i<numProcs;i++)
			CloseHandle(MPID_Events[i]);
		free(MPID_Events);
		MPID_Events=0;
	}
}


/*!
This member function just deletes all inSockets in the array hosts .
*/
void CTCPCommunicator::CloseSockets() {

	DBG("Entering CTCPCommunicator::closeSockets()");
	if(!hosts) return;
	Sleep(5);
	ShuttingDown=true;
	for(DWORD i=0;i<numProcs;i++)
		if(hosts[i]){
			delete hosts[i];
			hosts[i]=0;
		}
		delete hosts;
		hosts=0;
}
/*!
\relates CTCPCommunicator
Displays a message if an illegal commandline has been detected.
*/
int usage() {
	std::cerr<<"Illegal commandline\n";
	std::cerr<<"Usage: -n <number of processes> -m <name of master node> [-p <baseportnumber>]\n";
	return -1;
}

/*!
\struct Options TCPCommunicator.cpp
\brief Used by CTCPCommunicator to parse the commandline.
*/
struct Options {
	//! The number of processes (Switch -n)
	int np;

	//! The master port (Switch -p)
	int port;

	//! The name of the master node (Switch -m)
	char MasterName[MAX_NAME_LEN];

	//! The name local address to bind to (Switch -b)
	char LocalAddr[MAX_NAME_LEN];
};

/*!
\relates CTCPCommunicator
This function parses the commandline using getopt().
It removes the parameters from \a argv and returns the parameters found in \a opt.
*/
int ParseCommandLine(int *argc, char*** argv,Options *opt) {
	int c,count=0;
	opt->np=opt->port=-1;
	opt->MasterName[0]=0;
	opt->LocalAddr[0]=0;
	opterr=0;
	while((c=getopt(*argc,*argv,"n:m:p:b:"))!=EOF) {
		switch(c) {
	case 'm':if(!optarg||!optarg[0]) return usage(); strcpy(opt->MasterName,optarg); count++; break;
	case 'n':if(sscanf(optarg,"%d",&(opt->np))!=1) return usage(); count++; break;
	case 'p':if(sscanf(optarg,"%d",&(opt->port))!=1) return usage(); count++; break;
	case 'b':if(!optarg||!optarg[0]) return usage(); strcpy(opt->LocalAddr,optarg); count++;
	case '?': break;
		}
	}
	if(optind>1) {
		optind--;
		*argc-=optind;
		(*argv)[optind]=(*argv)[0];
		*argv+=optind;
		resetGetOpt();
	}
	/*
	if(opt->np<=0) {
	opt->np = 1;
	}
	*/
	opterr=1;
	return 0;
}



#ifdef _MSC_VER
#pragma warning( disable : 4706)
#endif

#define servicename	"ch_wsock"

HANDLE *CreateEvents(DWORD size,DWORD NumEvents) {
	HANDLE* array;
	array = (HANDLE*)malloc(sizeof(HANDLE)*size);
	for(int i=0;i<size;i++) {
		//if(Global2Local[i]<NumEvents) {
		array[i]=CreateEvent(0,FALSE,FALSE,0);
		if(!array[i]) wsock_syserror("Cannot create event",GetLastError());
		//} else array[i]=0;

	}
	return array;
}

void DuplicateEvents(DWORD numEvents,HANDLE *Orig,HANDLE* New,HANDLE destProc) {
	for(int i=0;i<numEvents;i++) {
		if(Orig[i]) {
			if(!DuplicateHandle(GetCurrentProcess(),Orig[i],destProc,
				New+i,0,FALSE,DUPLICATE_SAME_ACCESS)) {
					wsock_syserror("Cannot duplicate event",GetLastError());
				}
		} else New[i]=0;
	}
}

#ifndef NOSP4

typedef struct {
	DWORD IP,speed;
} IFDesc;

int IFCmp( const void *arg1, const void *arg2 )
{ 
	if(((IFDesc*)arg1)->speed > ((IFDesc*)arg2)->speed)
		return 1;
	else if(((IFDesc*)arg1)->speed < ((IFDesc*)arg2)->speed)
		return -1;
	else return 0;
}


int GetLocalIps(unsigned long *IPS, unsigned long *count,BOOL NoLoopback) {
	static IFDesc *buf=0;
	static DWORD returned=0;

	MIB_IPADDRTABLE *IpAddrTable=0;
	MIB_IFROW IfRow;
	DWORD size = 0;
	DWORD i,j;


	if(!buf) {
		GetIpAddrTable(IpAddrTable,&size,TRUE);
		if(size >0) {
			IpAddrTable = (MIB_IPADDRTABLE*)malloc(size);
			if(!IpAddrTable) {
				wsock_syserror("Out of memory in GetLocalIPs()",GetLastError());
				return -1;
			}
			if(GetIpAddrTable(IpAddrTable,&size,TRUE) != NO_ERROR) {
				wsock_syserror("GetIpAddrTable() (2) failed",GetLastError());
				return -1;
			}
		} else {
			wsock_syserror("GetIpAddrTable() (1) failed",GetLastError());
			return -1;
		}
		buf = (IFDesc *)realloc(buf,sizeof(IFDesc)*IpAddrTable->dwNumEntries);
		if(!buf) {
			wsock_syserror("Out of memory in GetLocalIPs()",GetLastError());
			return -1;
		}
		returned = IpAddrTable->dwNumEntries;
		for(j=0,i=0;i<returned;++i) {
			if(!IpAddrTable->table[i].dwAddr) continue;
			IfRow.dwIndex = IpAddrTable->table[i].dwIndex;
			GetIfEntry(&IfRow);
#if 0
			// This somehow doesn't work.
			// NT 4 and W2k both report MIB_STATUS_UNREACHABLE.
			// Don't ask me why...
			if(IfRow.dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL) {
				DBG("Interface "<<IP(IpAddrTable->table[i].dwAddr)<<" not operational "<<IfRow.dwOperStatus);
				continue;
			}
#endif
			buf[j].IP = IpAddrTable->table[i].dwAddr;
			buf[j].speed = IfRow.dwSpeed;
			++j;
		}
		returned = j;
		if(returned) 
			qsort(buf,returned,sizeof(IFDesc),IFCmp);
		free(IpAddrTable);
	}//if(!buf) 


	if(returned > *count) {
		*count = returned;
		return -1;
	}

	DBG("I have "<<returned<<" local IPs");
	for(j=0,i=0;i<returned;++i) {
		if(!NoLoopback || buf[i].IP != htonl(INADDR_LOOPBACK)) {
			IPS[j++] = buf[i].IP;
			DBG("Local IP["<<j-1<<"]=="<<IP(buf[i].IP))
		}
	}

	*count = j;   
	return 0;

}
#else
int GetLocalIps(unsigned long *IPS,unsigned long *count,BOOL NoLoopback) {
	static void *buf=0;
	static DWORD returned=0;

	DWORD size=4*sizeof(INTERFACE_INFO);
	SOCKET s;
	int res,j;
	INTERFACE_INFO *list;

	if(!buf) {
		s = WSASocket(AF_INET,SOCK_STREAM,0,0,0,0); //wsock2function
		if(s == INVALID_SOCKET) {
			wsock_syserror("WSASocket failed ",WSAGetLastError());
			*count = 0;
			return -1;
		}

		do {
			buf = realloc(buf,size);
			if(!buf) {
				*count = 0;
				return -1;
			}
			res = WSAIoctl(s,SIO_GET_INTERFACE_LIST,0,0,buf,size,&returned,0,0);  //wsock2function 
			size *= 2;
		} while(res == SOCKET_ERROR && (WSAGetLastError() == WSAENOBUFS || WSAGetLastError() ==WSAEFAULT ));

		if(res == SOCKET_ERROR) {
			wsock_syserror("WSAIoctl(SIO_GET_INTERFACE_LIST) failed ",WSAGetLastError()); //wsock2function
			closesocket(s);
			*count = 0;
			if(buf) free(buf);
			return -1;
		}

		closesocket(s);

		returned /= sizeof(INTERFACE_INFO);
	}

	if(returned > *count) {
		*count = returned;
		return -1;
	}


	list = (INTERFACE_INFO*)buf;

	j=0;
	for(int i=0;i<returned;++i) {
		if(!NoLoopback || list->iiAddress.AddressIn.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
			IPS[j++] = list->iiAddress.AddressIn.sin_addr.s_addr;
			DBG("Local IP["<<j-1<<"]=="<<IP(list->iiAddress.AddressIn.sin_addr.s_addr))
		}
		++list;
	}
	*count = j;
	DBG("Returning "<<j<<" local IPs");
	//if(buf) free(buf);
	return 0;
}
#endif

void ResolveAddress(const char *Host,unsigned long *Addr) {
	unsigned long Address;
	struct hostent *he;

	DBG("Entering ResolveAddress");

	if(!Host || !*Host) {
		*Addr = 0;
		return;
	}

	Address = inet_addr(Host);
	if(Address == (unsigned long)-1) {
		if ((he=gethostbyname(Host)) != 0) {
			memcpy((char*)&Address,he->h_addr,sizeof(Address));			
		} else Address = 0;
	}
	*Addr = Address;
	DBG("Resolved "<<Host<<" to "<<IP(*Addr));
}

int IsLocalAddress(unsigned long Addr) {
	unsigned long *localIps=0;
	DWORD size = 4*sizeof(unsigned long);

	DWORD oldSize;

	// 127.0.0.1 might or might not be contained in the array
	// of local IPs, so test it independently.
	if(Addr == htonl(INADDR_LOOPBACK)) return 1;


	do {
		localIps = (unsigned long*)realloc(localIps,size*sizeof(unsigned long));
		if(!localIps) return 0;
		oldSize = size;
		GetLocalIps(localIps,&size,TRUE);
	} while(size>oldSize);


	for(oldSize=0;oldSize<size;++oldSize) { 
		if(localIps[oldSize]==Addr) return 1;
	}
	free(localIps);
	return 0;
}

int CheckMaster(const char *masterName,int NumProcs,unsigned long *MasterAddress) {
	unsigned long Address,*localIps=0;
	int res;

	// No -m switch, I must be the master.
	if(*masterName == 0) return 1;
	ResolveAddress(masterName,&Address);
	*MasterAddress = Address;
	if(!Address) return 1;

	// No -n switch, I must be a client.
	if(NumProcs<1) return 0;

	// O.K. We have both a -m and a -n switch
	// (old style commandline). Now we assume that all clients
	// use 127.0.0.1 as master and the master uses the official
	// hostname.
	if(Address == htonl(INADDR_LOOPBACK)) return 0;
	res = IsLocalAddress(Address);
	if(res) *MasterAddress = htonl(INADDR_LOOPBACK);
	return res;
}

/*!
This member function is called to initiate the communication.
It first parses the commandline by calling ParseCommandLine().
After that it either calls MasterConnect() or ClientConnect depending
on the value given by the switch -m.
*/
int CTCPCommunicator::Create(int *argc,char*** argv) {
	Options opt;
	int master=0;
	size_t i,j;
	DWORD dummy;
	HANDLE *TmpEvents;

	if(ParseCommandLine(argc,argv,&opt)<0)
		return -1;

	master = CheckMaster(opt.MasterName,opt.np,&masterAddress);

	// set flag if master, master creates socket for each client
	if(master) {
		if(opt.np<0) opt.np = 1;
		numProcs=opt.np;
		if(numProcs<2) {myId=0; return 0;}
		hosts=new inSocket*[numProcs];
		//all entrys to NULL
		memset(hosts,0,numProcs*sizeof(inSocket*));
	} else if(!masterAddress) {
		return usage();
	}

	DBG("Parsed: np="<<opt.np<<" Master="<<opt.MasterName);
	DBG("Master: "<<master);

	int result;
	// Get the port to use
	if(opt.port<0) {
		servent *serv;
		if(!(serv=getservbyname(servicename,0))) {
			Masterport=BASEPORT;
		} else Masterport=ntohs(serv->s_port);
	} else Masterport=opt.port;

	MPID_myid=0;
	MPID_numids=0;

	ResolveAddress(opt.LocalAddr,&localAddress);

	FD_ZERO(&backup);
	DBG("Port for "<<servicename<<"="<<Masterport);
	
	if(master) 
		/*The master waits for connections from all other nodes and sends them the names 
		of the other nodes and their ids */
		result=MasterConnect();
	else 
		/*
		The client connects to the master startup process an sends it a NodeDescription struct.
		When the answer has been received it starts connecting to all other nodes as described
		in the class description.
		*/
		result=ClientConnect(masterAddress);

	DBG("Net startup complete. Initializing SMP");

	if(MPID_numids) {
		p2p_create_procs(MPID_numids,*argc,*argv);
		MPID_numids++;
		MPID_lshmem.mypool = MPID_shmem->pool[MPID_myid];
		for (i=0; i<MPID_numids; i++) 
			MPID_lshmem.pool[i]   = MPID_shmem->pool[i];

#ifdef RNDV_STATIC
		for(i=0;i<MPID_numids;i++) {
			for(j=0;j<MPID_numids;j++) {
				MPID_lshmem.ActBuffers[i][j] = (volatile char**)&MPID_shmem->EagerBufs[i][j].ptr1;
			}
		}
#endif
		p2p_wtime_init();
		if(MPID_myid) { /* Startup Clients */
			MPID_Events = (HANDLE*)malloc(sizeof(HANDLE)*numProcs);
			for(i=0;i<numProcs;i++) {
				if(!Global2Local[i]) {
					hosts[i]->read(Global2Local,sizeof(DWORD)*numProcs);
					hosts[i]->read(MPID_Events,sizeof(HANDLE)*numProcs);
				}
				if(Global2Local[i]<MPID_numids && hosts[i]) {
					DBG("Shutting down net connection to "<<i);
					FD_CLR((int)*(hosts[i]),&backup);
					delete hosts[i];
					hosts[i]=0;
				}
			}
		} else { /* Startup Master */
			MPID_Events=CreateEvents(numProcs,MPID_numids);
			TmpEvents = (HANDLE*)malloc(sizeof(HANDLE)*numProcs);
			for(i=0;i<numProcs;i++) {
				if(Global2Local[i] && Global2Local[i]<MPID_numids) {
					/* duplicate each event handle for local process */ 
					DuplicateEvents(numProcs,MPID_Events,TmpEvents,MPID_child_pid[Global2Local[i]]);
					/* send information about global/local rank and event handles */
					hosts[i]->write(Global2Local,sizeof(DWORD)*numProcs);
					hosts[i]->write(TmpEvents,sizeof(HANDLE)*numProcs);
					DBG("Shutting down net connection to "<<i);
					FD_CLR((int)*(hosts[i]),&backup);
					shutdown(*hosts[i],SD_SEND);
					while(hosts[i]->read(&dummy,sizeof(dummy))>0) ;
					delete hosts[i];
					hosts[i]=0;
				}
			}
			free(TmpEvents);
		}
	}

	if(MPID_numids) {
		Local2Global = new DWORD[MPID_numids];
		for(i=0;i<numProcs;i++) {
			if(Global2Local[i] < MPID_numids) 
				Local2Global[Global2Local[i]] = i;
		}
	} else MPID_Events=CreateEvents(numProcs,MPID_numids);
	DBG("wsock2 startup finished, my cluster id: "<<SMPId);

	return result;
}


#define HEADERSIZE (sizeof(TCPCommMessage)-sizeof(NodeDescription))
#define NEXTNODE(n) ((NodeDescription*)(((char*)(n+1))+(n->IPCount-1)*sizeof(unsigned long)))

/*!
The master startup process calls this member function. It
waits for connections from all other nodes and sends them the names 
of the other nodes and their ids.
For each client that connects a new inSocket class is created.
*/
int CTCPCommunicator::MasterConnect() {
	TCPCommMessage msg,recMsg;
	FD_SET allfds;
	DWORD actId,answerSize=0,sent;
	// top be removed?    char *actNodePos;
	timeval t = {MAX_TIMEOUT,0};
	int res;
	WSABUF *buffers;

	DBG("Entering MasterConnect");

	buffers = (LPWSABUF)malloc((numProcs+1)*sizeof(WSABUF));
	buffers[0].buf = (char*)&msg;
	buffers[0].len = sizeof(TCPCommMessage);
	msg.body.local = numProcs;

	LocalMaster = true;
	// Create the socket to accept connections
	inSocket masterSocket(Masterport,any);

	msg.body.type=ACK_MASTER;

	myId=0;
	SMPId = 0;
	masterSocket.listen(numProcs-1);
	hosts[0]=0;
	FD_SET(masterSocket,&backup);
	// Start accepting connections
	DBG("Number of processes = " << numProcs);
	for(actId=1;actId<(DWORD)numProcs;++actId) {
		hosts[actId]=new inSocket;
		allfds=backup;
		res=select(0,&allfds,0,0,&t);
		if(res<=0 || !FD_ISSET(masterSocket,&allfds) ) {
			return -1;
		}

		((inSocket*)hosts[actId])->accept(masterSocket); /* Block until the Client with ID actId connected */

		if(SetSocketOptions(*((inSocket*)hosts[actId]))<0) {
			wsock_syserror("SetSocketOptions failed ",WSAGetLastError());
			return -1;
		}


		// How long is the next message and of which type?	
		DBG("Read header of " << HEADERSIZE << " bytes");
		if(hosts[actId]->read(&recMsg,HEADERSIZE)<HEADERSIZE) {
			wsock_syserror("Cannot read from Socket", WSAGetLastError());
			return -1;
		}
		// Read the name of the connecting node
		// bodySize containes the size of the NodeDescrition
		// struct in the messagebody.

		if(recMsg.body.type != COMM_CONNECT) {
			// This was the wrong message...
			DBG("Wrong message type " << recMsg.body.type << "! Wait for COMM_CONNECT");
			delete hosts[actId];
			hosts[actId] = 0;
			actId--;
			continue;
		}
		// Read the NodeDescription struct...
		buffers[actId].buf = (char*)malloc(recMsg.header.BodySize);
		buffers[actId].len = recMsg.header.BodySize;
		DBG("Receiving body with the length of " << recMsg.header.BodySize << " byte");
		hosts[actId]->read(buffers[actId].buf,buffers[actId].len);
		FD_SET((int)*(hosts[actId]),&backup);

		// Send the node its id and the names of the nodes with lower ids
		msg.body.Data.ID=actId;
		msg.header.BodySize=answerSize;

		DBG("Sending message with Bodylength "<<msg.header.BodySize);
		if(WSASend(*hosts[actId],buffers,actId,&sent,0,0,0) == SOCKET_ERROR) //wsock2function
			return -1;		
		answerSize+=recMsg.header.BodySize;

		if(SetSocketOptions(*(hosts[actId]))<0) {
			wsock_syserror("SetSocketOptions failed ",WSAGetLastError());
			return -1;
		}
		if(recMsg.body.local) {
			DBG("This node is local");
			recMsg.body.Data.ID = actId;
			MasterSMP(hosts[actId],&recMsg.body);
		}

	}//end for each process
	if(numProcs>1) {	
		if(hosts[1]->read(&msg,sizeof(TCPCommMessage))<sizeof(TCPCommMessage)) {
			wsock_syserror("Read failed in MasterConnect",WSAGetLastError()); 
			return -1;
		}
		DBG("Receive message of type " << msg.body.type);
		DBG("Message size = " << sizeof(TCPCommMessage));
		DBG("BodySize = " << msg.header.BodySize);
		DBG("local = " << msg.body.local);
		if(msg.body.type != Start) {
			wsock_error("Wrong message type received",-1);
			return -1;
		} 
#ifdef _DEBUG		
		else DBG("StartMessage received");
#endif
	}

	for(actId=1;actId<numProcs;++actId) 
		free(buffers[actId].buf);
	free(buffers);

	return 0;
}

//wait a maximum of 2 minutes (60 * 2000ms)
#define MAXWAIT 60

/*!
This member function is used by all startup clients. It first of all
connects to the master startup process an sends it a NodeDescription struct.
When the answer has been received it starts connecting to all other nodes as described
in the class description.
*/
int CTCPCommunicator::ClientConnect(unsigned long Master) {
	TCPCommMessage msg; 
	NodeDescription *nodes=0,*actNode; 
	unsigned size;
	DWORD actId;
	inSocket masterSocket,*tmpSocket;
	timeval t = {MAX_TIMEOUT,0};
	int res,i;
	BOOL local;
	FD_SET allfds;
	DWORD offset=0,NumIps;
	unsigned long *IPs=0;
	bool failed=true;
	DWORD wait=0,ret,dummy;

	DBG("Entering ClientConnect");

	// Create a socket that will be used to accept
	// connections from hosts with higher id.
	masterSocket.create();
	if(SetSocketOptions(masterSocket) <0) {
		wsock_syserror("SetSocketOptions() failed in ClientConnect()",WSAGetLastError());
		return -1;
	}
	masterSocket.bind();
	DBG("Bound listening socket to port "<<masterSocket.getPort()); 
	masterSocket.listen(SOMAXCONN);


	msg.body.type=COMM_CONNECT;
	msg.body.Data.listeningPort=masterSocket.getPort();
	msg.body.local = local = IsLocalAddress(Master);
	msg.body.ProcessId = GetCurrentProcessId();

	size = sizeof(msg);

	if(!localAddress) {
		// Fill in all my local IP addresses
		NumIps = 0;
		GetLocalIps(0,&NumIps,TRUE);
		if(NumIps > 1) {
			IPs = (unsigned long*)malloc(NumIps*sizeof(unsigned long));
			GetLocalIps(IPs,&NumIps,TRUE);
			size -= sizeof(unsigned long);
		} 
		else 
		{  
			GetLocalIps(msg.body.Data.address,&NumIps,TRUE);
			//first call of GetLocalIps delivers number including loopback
			//next calls deliver number without loopback
			//when we are here NumIps is always 0 so set it to 1
			NumIps=1;
		}
	} 
	else {
		// We had a -b switch, so only use this address
		NumIps = 1;
		msg.body.Data.address[0]=localAddress;
	}
	DBG("NumIps = " << NumIps);
	msg.body.Data.IPCount = NumIps;
	msg.header.BodySize = sizeof(NodeDescription)+(NumIps-1)*sizeof(unsigned long);


	// Connect to master
	try {
		tmpSocket=new inSocket;
		tmpSocket->create();
	} 
	catch(socketException e) {
		printf("Caught exception %s\n",(const char*)e);
		return -1;

	}


	while(failed && wait<MAXWAIT) {
		DBG("Trying connection to "<<IP(Master));
		try {
			tmpSocket->connect(Master,Masterport);
			failed=false;
			DBG("Connected to "<<IP(Master));
		} catch(socketException &e) {
			failed=true;
			DBG("Connection failed with "<<e<<" Retrying in 2 sec....");
			if(e.GetError() != WSAECONNREFUSED) wait = MAXWAIT;
			if(++wait<MAXWAIT) Sleep(2000);
		}
	}

	if(failed) {
		wsock_syserror("Could not connect to master",WSAGetLastError());
		return -1;
	}
	// Send my name to the master
	DBG("Send my name to the master: size = " << size);
	SetSocketOptions(*tmpSocket);
	if(tmpSocket->write(&msg,size)<size) {
		wsock_syserror("Could not send my data to master",WSAGetLastError());
		return -1;
	}
	if(IPs) {
		size = NumIps*sizeof(unsigned long);
		DBG("Sending my IPs "<<size);
		if(tmpSocket->write(IPs,size)<size) {
			wsock_syserror("Could not send my IPs to master",WSAGetLastError());
			return -1;
		}
		free(IPs);
		IPs = 0;
	}
	// How large is the next message ?
	if(tmpSocket->read(&msg,sizeof(TCPCommMessage))<sizeof(TCPCommMessage)) {
		wsock_syserror("Read from master failed",WSAGetLastError());
		return -1;
	}
	DBG("Received AckMessage with length "<<msg.header.BodySize);
	// Read the names of all nodes with lower id and my id
	size=msg.header.BodySize;
	// Create an array of NodeDescriptions with the right size.
	if(size>0) {
		actNode=nodes=(NodeDescription *) malloc(size);
		if(tmpSocket->read(nodes,size)<size) {
			wsock_syserror("Could not read data from master",WSAGetLastError());
			return -1;	
		}
	}
	myId=msg.body.Data.ID;
	numProcs = msg.body.local;
	hosts=new inSocket*[numProcs];
	memset(hosts,0,numProcs*sizeof(inSocket*));
	hosts[0] = tmpSocket;
	hosts[myId]=0;
	DBG("MyId="<<myId<<" numProcs="<<numProcs);
	SMPId = myId;

	// Calculate the number of local processes with lower id
	for(i=1;i<myId;++i) {
		if(IsLocalAddress(actNode->address[0])) {
			++MPID_numids;
			if(SMPId>i) SMPId = i;
		}
		actNode=NEXTNODE(actNode);
	}

	actNode=nodes;

	if(MPID_numids || local) {
		Global2Local = new DWORD[numProcs];
		memset(Global2Local,255,numProcs*sizeof(DWORD));
	}

	if(local) {
		++MPID_numids;
		ClientSMP(hosts[0],&msg.body);
		Global2Local[0]=0;
		SMPId = 0;
	}

	LocalMaster = (MPID_numids==0);
	DBG("Local master: "<<LocalMaster);

	// Now we have all data
	// We can start connecting to other nodes.
	// First wait for connections from nodes with higher id.
	FD_SET((int)masterSocket,&backup);
	FD_SET((int)*(hosts[0]),&backup);
	for(i=0;(DWORD)i<numProcs-myId-1;i++) {
		tmpSocket =new inSocket;

		allfds=backup;
		DBG("Starting to wait for connections");
		res=select(0,&allfds,0,0,&t);
		if(res<=0 || !FD_ISSET(masterSocket,&allfds) ) {
			if(!res) res = WSAECONNRESET;
			else res = WSAGetLastError();
			wsock_syserror("Found dead connection while waiting for client connections",res);
			return -1;
		}
		tmpSocket->accept(masterSocket);
		SetSocketOptions(*tmpSocket);
		msg.body.type = -1; // Sanity check...
		// How long is the next message ?
		if(tmpSocket->read(&msg,sizeof(TCPCommMessage))<0) {
			if(nodes) free(nodes);
			delete tmpSocket;
			wsock_syserror("Read failed in client connect...",WSAGetLastError());
			return -1;
		}
		if(msg.body.type != COMM_CONNECT) {
			// Nice try ;-)
			delete tmpSocket;
			i--;
			DBG("CommConnect: Illegal message received");
			continue;
		}
		hosts[msg.body.Data.ID]=tmpSocket;
		FD_SET((int)*tmpSocket,&backup);
		DBG("Accepted connection from "<<msg.body.Data.ID);
		if(msg.body.local) {			
			if(LocalMaster) {
				MasterSMP(tmpSocket,&msg.body);
			} else {
				++MPID_numids;
				Global2Local[msg.body.Data.ID]=msg.body.ProcessId;
			}
		}
	}	

	if(myId>1) {	
		// Now connect to nodes with lower id
		msg.body.type=COMM_CONNECT;
		msg.header.BodySize=sizeof(TCPMessageBody);
		msg.body.Data.ID=myId;
		for(actId=1;actId<myId;actId++) {
			msg.body.local= IsLocalAddress(actNode->address[0]);
			if(msg.body.local) {
				actNode->address[0] = htonl(INADDR_LOOPBACK);
				NumIps = 1;
			} else NumIps = actNode->IPCount;
			if(MPID_myid)
				msg.body.ProcessId=MPID_myid;
			else 
				msg.body.ProcessId=GetCurrentProcessId();
			try {
				hosts[actId]=new inSocket;
				hosts[actId]->create();
				hosts[actId]->bind();
			} catch(socketException e) {
				wsock_error((const char*)e,WSAGetLastError());
				return -1;
			}

			failed = true;
			wait =0;
			while(failed&&wait<MAXWAIT&&NumIps) {
				failed=false;
				try {
#ifndef NOSP4
					ret= GetRTTAndHopCount(actNode->address[NumIps-1],&dummy,255,&offset); 
					//GetBestInterface(actNode->address[NumIps-1],&dummy);
					if(!ret) {
						ret = GetLastError();
						DBG("GetRTTAndHopCount for "<<IP(actNode->address[NumIps-1])<<" returned "<<ret);
						failed = true;
						--NumIps;
						continue;
					} 
#endif

					hosts[actId]->connect(actNode->address[NumIps-1],actNode->listeningPort);
				} catch(socketException &e) {
					DBG("Connection to "<<IP(actNode->address[NumIps-1])<<" failed\n"<<(const char*)e);
					failed=true;
					if(e.GetError() == WSAECONNREFUSED) {
						if(wait<MAXWAIT) Sleep(2000);
						++wait;	
					} else --NumIps;
				}
			}

			if(failed) {
				std::cerr<<"Connection to "<<IP(actNode->address[0])<<" failed\n";
				wsock_syserror("Could not connect to lower node",WSAGetLastError());
				return -1;
			}

			SetSocketOptions(*(inSocket*)hosts[actId]);
			hosts[actId]->write(&msg,sizeof(TCPCommMessage));
			DBG("Connected to "<<IP(actNode->address[NumIps-1]));
			if(msg.body.local && !MPID_myid) {
				ClientSMP(hosts[actId],&msg.body);
				Global2Local[actId]=0;
			}	    
			actNode=NEXTNODE(actNode);
		}

	}


	if(myId==1) {
		// Tell the nodes that connections are ready
		// node 1 should be the last one that comes up
		// because it has to wait for all others to connect with
		// it. So it can start all other nodes.
		msg.header.BodySize=sizeof(DWORD);
		msg.body.type=Start;
		for(actId=0;actId<(DWORD)numProcs;actId++) {
			if(actId!=1) {
				DBG("Sending StartMessage of " << sizeof(TCPCommMessage) << " byte to "<<actId);
				hosts[actId]->write(&msg,sizeof(TCPCommMessage));
			}
		}
	} else {	
		// read the start message from node 1
		if(hosts[1]->read(&msg,sizeof(TCPCommMessage))<0) {
			if(nodes) 
				free(nodes);
			return -1;
		}

		if(msg.body.type != Start) {
			wsock_error("Illegal message received",-1);
			if(nodes) 
				free(nodes);
			return -1;
		}
#ifdef _DEBUG		
		else DBG("StartMessage received");
#endif
	}

	if(nodes) 
		free(nodes);

	return 0;
}

#ifdef _MSC_VER
#pragma warning( 3 : 4706)
#endif


int CTCPCommunicator::GetFDs(int **Dest) {
	int res=0;
	*Dest=new int[numProcs];
	if(!*Dest) return 0;
	for(unsigned long i=0;i<numProcs;i++) {
		if(hosts&&hosts[i]) {
			(*Dest)[i]=(int)*(hosts[i]);
			res++;
		}
		else (*Dest)[i]=INVALID_SOCKET;
	}
	return res;
}


/* This is called if a client send state local to master */
void CTCPCommunicator::MasterSMP(inSocket *Client,TCPMessageBody *msg) {
	TCPCommMessage NewMsg;
	int memsize;
	HANDLE hProcess;
	NewMsg.body.local=TRUE;
	if(!MemHandle){
		memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );
		p2p_init( 0, memsize );
		p2p_lock_init(&tmp_mutex);

		/* This lock will be release in p2p_create_processes() 
		After all the work has been done. This is needed to synchronize
		the initialization of the shared memory.
		*/
		p2p_lock( &tmp_mutex );
		Global2Local = new DWORD[numProcs];
		memset(Global2Local,255,numProcs*sizeof(DWORD));
		Global2Local[myId]=0;
	}

	hProcess=OpenProcess(
		PROCESS_DUP_HANDLE|SYNCHRONIZE|PROCESS_CREATE_THREAD|PROCESS_TERMINATE,
		FALSE,msg->ProcessId
		); /* get handle of local process */

	if(!hProcess) wsock_syserror("OpenProcess failed",GetLastError());
	if(!DuplicateHandle(GetCurrentProcess(),MemHandle,hProcess, ((HANDLE*) NewMsg.body.Handles),
		0,FALSE,DUPLICATE_SAME_ACCESS)) {
			wsock_syserror("DuplicateHandle(MemHandle) failed",GetLastError());
		}
		if(!DuplicateHandle(GetCurrentProcess(),tmp_mutex,hProcess, ((HANDLE*) NewMsg.body.Handles)+1,
			0,FALSE,DUPLICATE_SAME_ACCESS)) {
				wsock_syserror("DuplicateHandle(MemHandle) failed",GetLastError());
			}

			NewMsg.body.ProcessId=GetCurrentProcessId();
			NewMsg.body.Data.ID = ++MPID_numids;
			if(Client->write(&NewMsg,sizeof(NewMsg))<sizeof(NewMsg)) {
				wsock_syserror("Cannot write to child process",WSAGetLastError());
			}
			MPID_child_pid[MPID_numids]=hProcess;
			Global2Local[msg->Data.ID]=MPID_numids;


}

void CTCPCommunicator::ClientSMP(inSocket *Master,TCPMessageBody *msg) {
	char FatherId[24];
	TCPCommMessage NewMsg;
	int memsize;

	if(Master->read(&NewMsg,sizeof(NewMsg))<sizeof(NewMsg)) {
		wsock_syserror("Cannot read data from local master",WSAGetLastError());
	}
	MPID_myid = NewMsg.body.Data.ID;

	Global2Local[myId]=MPID_myid;
#ifdef _WIN64
	sprintf(FatherId,"%I64d",NewMsg.body.Handles[0]);
#else
	sprintf(FatherId,"%d",((HANDLE*) NewMsg.body.Handles)[0]);
#endif
	SetEnvironmentVariable("MPICH_SHMEM_HANDLE",FatherId);
	sprintf(FatherId,"%d",NewMsg.body.ProcessId);
	SetEnvironmentVariable("MPICH_SHMEM_FATHER",FatherId);
	tmp_mutex= ((HANDLE*) NewMsg.body.Handles)[1];
	if(!MemHandle){
		memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );
		p2p_init( 0, memsize );
	}
}


#ifdef _MSC_VER
#pragma warning( 3: 4705 4706 )
#endif

