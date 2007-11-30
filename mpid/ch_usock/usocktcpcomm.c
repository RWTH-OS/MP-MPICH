/*
 |   CH_USOCK: usocktcpcomm.c
 |
 |   (formerly TCPCommunicator.cpp)
 |
 */

#include <stdlib.h>
#include <ctype.h>

#include "usockinsocket.h"
#include "usockgetopt.h"

//#define _MYDEBUG_ENABLED
#include "mydebug.h"
//#undef _MYDEBUG_ENABLED

#ifndef USE_NT2UNIX
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include <Iprtrmib.h>
#else
#include <iphlpapi.h>
#endif

#if defined(WIN32) && (_MSC_VER < 1100)
#define NOMINMAX 
#include <winsock2.h>
#endif

#include "usocktcpcomm.h"
#include "usockdev.h"
#include "mpid.h"

#define HPART(IP)((IP)>>24)
#define CNET(IP) (((IP)&0x00FF0000L) >> 16)
#define BNET(IP) (((IP)&0x0000FF00L) >> 8)
#define ANET(IP)  ((IP)&0x000000FFL)

#ifdef WIN32
#define sleep(n) Sleep(n*1000)
#endif

char IPdummy[]="xxx.xxx.xxx.xxx";

char* IP(unsigned long m)
{
  sprintf(IPdummy,"%d.%d.%d.%d\n", ANET(m), BNET(m), CNET(m), HPART(m));

  return IPdummy;
}

/* prototype for external functions: */ 
void MPID_USOCK_Error(const char*,int);
void MPID_USOCK_SysError(const char*,int);

#ifdef _MSC_VER
#pragma warning( disable : 4018)
#endif

#define MAX_TIMEOUT 120


/*
 |  Used by CTCPCommunicator to parse the commandline.
 */
typedef 
struct _Options {
  /* The number of processes (Switch -n) */
  int np;
  
  /* The number of processes (Switch -r) */
  int rank;
  
  /* The master port (Switch -p) */
  int port;

  /* The number of seconds to atimeout (Switch -t) */
  int timeout;
  
  /* The name of the master node (Switch -m) */
  char MasterName[MAX_NAME_LEN];

  /* The name local address to bind to (Switch -b) */
  char LocalAddr[MAX_NAME_LEN];

} Options;


/*
 |  Used to set several options on the sockets:
 */
int SetSocketOptions(SOCKET s)
{
  DSECTION("SetSocketOptions");
  int val1;
  unsigned long val2;
  struct linger l={1,10};
  int result=SOCKET_ERROR+1;
  
  DSECTENTRYPOINT;
  
  val1=1;
  result=setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(const char FAR *)&val1,sizeof(val1));
  if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(const char FAR *)&val1,sizeof(val1));
  if(result!=SOCKET_ERROR) result=setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(const char FAR *)&val1,sizeof(val1));
  if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_LINGER,(const char FAR *)&l,sizeof(struct linger));

  val2=SOCKET_BUF_SIZE;
  if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_RCVBUF,(const char FAR *)&val2,sizeof(unsigned long));
  if(result!=SOCKET_ERROR) result=setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char FAR *)&val2,sizeof(unsigned long));
  
  DSECTLEAVE;
  
#if 0
  return result==SOCKET_ERROR?-1:0;
#endif
  return 0;
}


void CTCPCommunicator_Constructor(CTCPCommunicator* CTCPCommunicatorPt)
{
  DSECTION("CTCPCommunicator_Constructor");
  WORD wVersionRequested; 
  WSADATA wsaData; 
  int err;
  
  DSECTENTRYPOINT;
  
  wVersionRequested=2;
  err=WSAStartup(wVersionRequested,&wsaData);
  CTCPCommunicatorPt->hosts=0;
  CTCPCommunicatorPt->ShuttingDown=0;
  CTCPCommunicatorPt->myId=0;
  CTCPCommunicatorPt->numProcs=0;
  CTCPCommunicatorPt->timeout=0;
  if(err)
  {
    /* ! WSAStartup failed ! */
  }
  CTCPCommunicatorPt->LocalMaster = TRUE;
  
  DSECTLEAVE
    return;
}

/*
 |  The destructor calls CloseSockets().
 |  After that all sockets should be destroyed.
 */
void CTCPCommunicator_Destructor(CTCPCommunicator* CTCPCommunicatorPt)
{
  MPID_USOCK_Data_global_type *global_data;
  
  DSECTION("CTCPCommunicator_Destructor");

  int i;
  
  DSECTENTRYPOINT;
  
  MPID_USOCK_Test_device(MPID_devset->active_dev, "CTCPCommunicator_Destructor");

  /* get the pointer to the global data struct of this device entity: */
  global_data = MPID_USOCK_Get_global_data(MPID_devset->active_dev);
    
  CTCPCommunicator_CloseSockets(CTCPCommunicatorPt);
  if(WSACleanup())
  {
    /* ! WSACleanup failed ! */
  }
  if(global_data->Events)
  {
    for(i=0; i<CTCPCommunicatorPt->numProcs; i++) CloseHandle(global_data->Events[i]);
    free(global_data->Events);
    global_data->Events=0;
  }
  
  DSECTLEAVE
    return;
}

DWORD CTCPCommunicator_GetMyId(CTCPCommunicator* CTCPCommunicatorPt)
{
  return CTCPCommunicatorPt->myId;
}

DWORD CTCPCommunicator_GetSMPId(CTCPCommunicator* CTCPCommunicatorPt)
{
  return CTCPCommunicatorPt->SMPId;
}

DWORD CTCPCommunicator_GetNumProcs(CTCPCommunicator* CTCPCommunicatorPt)
{
  return CTCPCommunicatorPt->numProcs;
}

/*
 | This member function just deletes all inSockets in the array hosts.
 */
void CTCPCommunicator_CloseSockets(CTCPCommunicator* CTCPCommunicatorPt)
{
  DWORD i;
  
  DSECTION("CTCPCommunicator_CloseSockets");
  
  if(!CTCPCommunicatorPt->hosts)
  {
    DSECTLEAVE
      return;
  }    
  sleep(5);
  CTCPCommunicatorPt->ShuttingDown=TRUE;
  
  for(i=0; i<CTCPCommunicatorPt->numProcs; i++)
  {
    if(CTCPCommunicatorPt->hosts[i])
    {
      free(CTCPCommunicatorPt->hosts[i]);
      CTCPCommunicatorPt->hosts[i]=0;
    }
  }
  free(CTCPCommunicatorPt->hosts);
  CTCPCommunicatorPt->hosts=0;
    
  DSECTLEAVE
    return;
}

/*
 |  This function parses the commandline using MPID_USOCK_getopt().
 |  It removes the parameters from argv and returns the parameters found in opt.
 */
int ParseCommandLine(int *argc, char*** argv, Options *opt)
{
  DSECTION("ParseCommandLine");

  int c;
  
  DSECTENTRYPOINT;
  
  /* setting default values for options */
  opt->np   = -1;
  opt->port = -1;
  opt->rank = 0;
  opt->timeout = 0;
  opt->MasterName[0] = 0;
  opt->LocalAddr[0]  = 0;
  
  /* do the parsing */
  opterr = 0;
  while((c=MPID_USOCK_getopt(*argc,*argv,"n:m:p:b:r:t:"))!=EOF) {
    switch(c) {
      case 'm':
	       if(!optarg||!optarg[0]) return -1;
	       strcpy(opt->MasterName,optarg);
	       break;
      case 'n':
	       if(sscanf(optarg,"%d",&(opt->np))!=1) return -1;
	       break;
      case 'r':
	       if(sscanf(optarg,"%d",&(opt->rank))!=1) return -1;
	       break;
      case 'p':
	       if(sscanf(optarg,"%d",&(opt->port))!=1) return -1;
	       break;
      case 't':
	       if(sscanf(optarg,"%d",&(opt->timeout))!=1) return -1;
	       break;	       
      case 'b':
	       if(!optarg||!optarg[0]) return -1;
	       strcpy(opt->LocalAddr,optarg); 
	       break;
	       
    }
  }
  
  if(optind>1)
  {
    optind--;
    *argc-=optind;
    (*argv)[optind]=(*argv)[0];
    *argv+=optind;
    MPID_USOCK_resetGetOpt();
  }  
  opterr=1;
  
  DSECTLEAVE
    return 0;
}

#ifdef _MSC_VER
#pragma warning( disable : 4706)
#endif

#define servicename	"ch_usock"

HANDLE *CreateEvents(DWORD size,DWORD NumEvents)
{
  DSECTION("CreateEvents");

  HANDLE* array;
  int i;
  
  DSECTENTRYPOINT;
  
  array = (HANDLE*)malloc(sizeof(HANDLE)*size);
  for(i=0;i<size;i++)
  {
    array[i]=CreateEvent(0,FALSE,FALSE,0);
    if(!array[i]) MPID_USOCK_SysError("Cannot create event",GetLastError());    
  }
    
  DSECTLEAVE
    return array;
}

void DuplicateEvents(DWORD numEvents,HANDLE *Orig,HANDLE* New,HANDLE destProc)
{
  DSECTION("DuplicateEvents");

  int i;
  
  DSECTENTRYPOINT;
  
  for(i=0;i<numEvents;i++)
  {
    if(Orig[i])
    {
      if(!DuplicateHandle(GetCurrentProcess(),Orig[i],destProc, New+i,0,FALSE,DUPLICATE_SAME_ACCESS))
      {
	MPID_USOCK_SysError("Cannot duplicate event",GetLastError());
      }
    } else New[i]=0;
  }

  DSECTLEAVE
    return;
}

typedef struct {
    DWORD IP,speed;
} IFDesc;

int IFCmp( const void *arg1, const void *arg2 )
{ 
  if(((IFDesc*)arg1)->speed > ((IFDesc*)arg2)->speed) return 1;
  else
    if(((IFDesc*)arg1)->speed < ((IFDesc*)arg2)->speed) return -1;
    else return 0;
}


int GetLocalIps(unsigned long *IPS, unsigned long *count,BOOL NoLoopback)
{
  DSECTION("GetLocalIps");

  /* 
   |  those static data need not to be coverd in the global device
   |  data struct for each dev-entity, because once the IPs are
   |  determined, they will not change for other entities:     
   */
  static IFDesc *buf=0;
  static DWORD returned=0;
    
  MIB_IPADDRTABLE *IpAddrTable=0;
  MIB_IFROW IfRow;
  DWORD size = 0;
  DWORD i,j;
  
  DSECTENTRYPOINT;
  
  if(!buf)
  {
    GetIpAddrTable(IpAddrTable,&size,TRUE);
    if(size >0)
    {
      IpAddrTable = (MIB_IPADDRTABLE*)malloc(size);
      if(!IpAddrTable)
      {
	MPID_USOCK_SysError("Out of memory in GetLocalIPs()",GetLastError());
	DSECTLEAVE return -1;
      }
      if(GetIpAddrTable(IpAddrTable,&size,TRUE) != 0)
      {
	MPID_USOCK_SysError("GetIpAddrTable() (2) failed",GetLastError());
	DSECTLEAVE return -1;
      }
    } 
    else
    {
      MPID_USOCK_SysError("GetIpAddrTable() (1) failed",GetLastError());
      DSECTLEAVE 
	return -1;
    }
    buf = (IFDesc *)realloc(buf,sizeof(IFDesc)*IpAddrTable->dwNumEntries);
    if(!buf)
    {
      MPID_USOCK_SysError("Out of memory in GetLocalIPs()",GetLastError());
      DSECTLEAVE
	return -1;
    }
    returned = IpAddrTable->dwNumEntries;
    for(j=0,i=0;i<returned;++i)
    {
      if(!IpAddrTable->table[i].dwAddr) continue;
      IfRow.dwIndex = IpAddrTable->table[i].dwIndex;
      GetIfEntry(&IfRow); /*iphlapi.h*/
#if 0
      /*
       |  This somehow doesn't work.
       |  NT 4 and W2k both report MIB_STATUS_UNREACHABLE.
       |  Don't ask me why...
      */
      if(IfRow.dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL)
      {
	DNOTICEI("Interface  not operational", IP(IpAddrTable->table[i].dwAddr);
	continue;
      }
#endif
      buf[j].IP = IpAddrTable->table[i].dwAddr;
      buf[j].speed = IfRow.dwSpeed;
      ++j;
    }
    returned = j;
    if(returned) qsort(buf,returned,sizeof(IFDesc),IFCmp);
    free(IpAddrTable);
  }
    
  if(returned > *count)
  {
    *count = returned;
    DSECTLEAVE return -1;
  }

  DNOTICEI("Number of local IPs", returned);

  for(j=0,i=0;i<returned;++i)
  {
	DNOTICES("IP: ", IP(buf[i].IP));
    if(!NoLoopback || buf[i].IP != htonl(INADDR_LOOPBACK))
    {  
      IPS[j++] = buf[i].IP;
    }
  }
    
  *count = j;   
    
  DSECTLEAVE
    return 0;
}


/* ResolveAddress() resolves the name of the host in 'Host', the resulting network address
   is put in 'Addr'. The hostname may be given as a name or in '.' notation. If the host name
   cannot be resolved, 0 is returned, otherwise 1 */
int ResolveAddress(const char *Host,unsigned long *Addr)
{
  DSECTION("ResolveAddress");

  unsigned long Address;
  struct hostent *he;
  
  DSECTENTRYPOINT;

  DNOTICES("Trying to resolve", Host);
  
  /* has a hostname been supplied? */
  if(!Host || !*Host)
  {
    *Addr = 0;
    DSECTLEAVE
      return 0;
  }
    
  /* we assume '.' notation first */
  Address = inet_addr(Host);
  
  /* if hostname was not in '.' notation, inet_addr returns -1 */
  if(Address == ((unsigned long long)256*256*256*256-1))
  {
    if ((he=gethostbyname(Host)) != 0)
    {
      memcpy((char*)&Address,he->h_addr,sizeof(Address));			
    }
    else
    {
      Address = 0;
      DSECTLEAVE
	return 0;
    }
  }
  
  *Addr = Address;

  DNOTICES("Resolved to", IP(*Addr));
  
  DSECTLEAVE
    return 1;
}


int IsLocalAddress(unsigned long Addr)
{
  DSECTION("IsLocalAddress");
  
  unsigned long *localIps=0;
  DWORD size = 4*sizeof(unsigned long);
  DWORD oldSize;
    
  DSECTENTRYPOINT;

  /*
   |  127.0.0.1 might or might not be contained in the array
   |  of local IPs, so test it independently.
   */
  if(Addr == htonl(INADDR_LOOPBACK))
  { 
    DSECTLEAVE
      return 1; 
  }

  do {
    localIps = (unsigned long*)realloc(localIps,size*sizeof(unsigned long));
    if(!localIps) { DSECTLEAVE; return 0 ;}
    oldSize = size;
    GetLocalIps(localIps,&size,TRUE);
  } while(size>oldSize);
  
  
  for(oldSize=0;oldSize<size;++oldSize)
  { 
    if(localIps[oldSize]==Addr) { DSECTLEAVE return 1;}
  }
  free(localIps);
  
  DSECTLEAVE
    return 0;
}


/* CheckMaster() decides if the current process is a master process or
   a client process. If it's a master, 1 is returned, if it's a client,
   0 is returned and the network address of the master processes' host
   is put in 'MasterAddress */
int CheckMaster(const char *masterName,int NumProcs,unsigned long *MasterAddress)
{
  DSECTION("CheckMaster");
    
  unsigned long Address,*localIps=0;
  /*    int res; */
  
  DSECTENTRYPOINT;
  
  /* No -m switch, I must be the master. */
  if(*masterName == 0) {DSECTLEAVE return 1;}
  
  /* we assume to be a client */
  /* resolve host name */
  if( ResolveAddress(masterName,&Address) == 1 ) *MasterAddress = Address;
  else
  {
    /* host name could not be resolved */
    DSECTLEAVE
      return 1;
  }

  /*
   |  O.K. We have both a -m and a -n switch
   |  (old style commandline). Now we assume that all clients
   |  use 127.0.0.1 as master and the master uses the official
   |  hostname.
   |
   |  everybody should use the right commandline arguments and mpirun does this, too
   |  I don't think this is necessary
   |  if(Address == htonl(INADDR_LOOPBACK)) {DSECTLEAVE return 0;}
   |     res = IsLocalAddress(Address);
   |     if(res) *MasterAddress = htonl(INADDR_LOOPBACK);
   */
    
  DSECTLEAVE
    return 0;
}


/*
 |  This member function is called to initiate the communication.
 |  It first parses the commandline by calling ParseCommandLine().
 |  After that it either calls MasterConnect() or ClientConnect depending
 |  on the value given by the switch -m.
 */
int CTCPCommunicator_Create(int *argc,char*** argv, CTCPCommunicator* CTCPCommunicatorPt)
{
  MPID_USOCK_Data_global_type *global_data;
  
  DSECTION("CTCPCommunicator_Create");

  Options opt;
  int master=0;
  int result;
  struct servent *serv;

  DSECTENTRYPOINT;
    
  MPID_USOCK_Test_device(MPID_devset->active_dev, "CTCPCommunicator_Create");

  /* get the pointer to the global data struct of this device entity: */
  global_data = MPID_USOCK_Get_global_data(MPID_devset->active_dev);

  if(ParseCommandLine(argc,argv,&opt)<0)
  {
    DSECTLEAVE
      return -1;
  }
  
  master = CheckMaster(opt.MasterName,opt.np,&(CTCPCommunicatorPt->masterAddress));
  
  CTCPCommunicatorPt->myId=opt.rank;

  if(opt.timeout==0) CTCPCommunicatorPt->timeout=MAX_TIMEOUT;
  else CTCPCommunicatorPt->timeout=opt.timeout;
  
  if(master)
  {
    if((CTCPCommunicatorPt->myId!=0)&&(CTCPCommunicatorPt->myId!=-1))
    {
      MPID_USOCK_Error("Master must be the process with rank 0!",-1);
      DSECTLEAVE
	return -1;
    }
    if(opt.np<0) opt.np = 1;
    CTCPCommunicatorPt->numProcs=opt.np;
    if(CTCPCommunicatorPt->numProcs<2)
    {
      CTCPCommunicatorPt->myId=0; 
      DSECTLEAVE 
	return 0;
    }
    CTCPCommunicatorPt->hosts = (inSocket**) malloc (CTCPCommunicatorPt->numProcs * sizeof(inSocket*));
    memset(CTCPCommunicatorPt->hosts,0,CTCPCommunicatorPt->numProcs*sizeof(inSocket*));
  }
  else
  {
    if(!CTCPCommunicatorPt->masterAddress)
    {
      DSECTLEAVE 
	return -1;
    }
  }

  DNOTICEI("Parsed: np =", opt.np);
  DNOTICES("Parsed: MasterName =", opt.MasterName);
  DNOTICEI("Master:", master);
  
  /* Get the port to use */
  if(opt.port<0)
  {   
    if(!(serv=getservbyname(servicename,0)))
    {
      CTCPCommunicatorPt->Masterport=BASEPORT;
    }
    else CTCPCommunicatorPt->Masterport=ntohs(serv->s_port);
  } else CTCPCommunicatorPt->Masterport=opt.port;
  
  ResolveAddress(opt.LocalAddr,&(CTCPCommunicatorPt->localAddress));
  
  FD_ZERO(&(CTCPCommunicatorPt->backup));

  DNOTICEI("Masterport:", CTCPCommunicatorPt->Masterport);

  if(master) result = CTCPCommunicator_MasterConnect(CTCPCommunicatorPt);
  else result = CTCPCommunicator_ClientConnect(CTCPCommunicatorPt->masterAddress,CTCPCommunicatorPt);

  DNOTICEI("usock startup finished, my cluster id: ", CTCPCommunicatorPt->SMPId);
  
  global_data->Events = (HANDLE*)malloc(sizeof(HANDLE)*CTCPCommunicatorPt->numProcs);
  global_data->Events = CreateEvents(CTCPCommunicatorPt->numProcs,CTCPCommunicatorPt->numProcs);
  
  DSECTLEAVE
    return result;
}


#define NEXTNODE(n) ((NodeDescription*)(((char*)(n+1))+(n->IPCount-1)*sizeof(unsigned long)))

/*
 |  The master startup process calls this member function. It
 |  waits for connections from all other nodes and sends them the names 
 |  of the other nodes and their ids.
 |  For each client that connects a new inSocket class is created.
 */
int CTCPCommunicator_MasterConnect(CTCPCommunicator* CTCPCommunicatorPt)
{
  DSECTION("CTCPCommunicator_MasterConnect");
  TCPCommMessage msg;
  NodeDescription *nodes=0, *tempNodes=0; 
  FD_SET allfds;
  DWORD rankCount,answerSize=0;
  struct timeval t = {MAX_TIMEOUT,0};
  int res=0, i=0;
  inSocket **tempHosts;
  int rankCount_flag=0;
  int *rankMap=0;
  inSocket masterSocket;
  
  DSECTENTRYPOINT;
  
  TCPCommMessage_Constructor(&msg);

  msg.body.local = CTCPCommunicatorPt->numProcs;

  rankMap   = (int*) malloc (CTCPCommunicatorPt->numProcs * sizeof(int));
  tempHosts = (inSocket**) malloc (CTCPCommunicatorPt->numProcs * sizeof(inSocket*));;
  nodes     = (NodeDescription*) calloc (CTCPCommunicatorPt->numProcs,sizeof(NodeDescription));
  tempNodes = (NodeDescription*) calloc (CTCPCommunicatorPt->numProcs,sizeof(NodeDescription));
  
  CTCPCommunicatorPt->LocalMaster = TRUE;
  
  /* Create the socket to accept connections */
  DNOTICE("Create the socket to accept connections");
  
  inSocket_Constructor_1_(CTCPCommunicatorPt->Masterport, any, SOCK_STREAM, &masterSocket);
  
  msg.body.type=ACK_MASTER;
  
  CTCPCommunicatorPt->myId=0; /* Master's ID is always 0 */
  CTCPCommunicatorPt->SMPId = 0;
  inSocket_listen(CTCPCommunicatorPt->numProcs-1, &masterSocket);
  CTCPCommunicatorPt->hosts[0]=0;
  FD_SET(inSocket_getInt(&masterSocket), &(CTCPCommunicatorPt->backup));
  DNOTICEI("FD_SET result:",res);
  
  /* Start accepting connections */
  DNOTICE("Start accepting connections");
  
  /* preset the rank map with invalid rank -1 */
  for(rankCount=1; rankCount<(DWORD)CTCPCommunicatorPt->numProcs; ++rankCount) rankMap[rankCount]=-1;
  
  /* Loop over all other processes */
  for(rankCount=1; rankCount<(DWORD)CTCPCommunicatorPt->numProcs; ++rankCount)
  { 
    tempHosts[rankCount] = (inSocket*) malloc (sizeof(inSocket));
    inSocket_Constructor_0_(tempHosts[rankCount]);
    allfds=CTCPCommunicatorPt->backup;
    DNOTICEI("select ", rankCount);
    res=select(FD_SETSIZE,&allfds,0,0,&t);
    if(res<0 || !FD_ISSET(inSocket_getInt(&masterSocket), &allfds) )
    {
      DSECTLEAVE
	return -1;
    }
    DNOTICEI("selected ", rankCount);

    DNOTICEI("accept ", rankCount);
    inSocket_accept(&masterSocket, tempHosts[rankCount]);
    DNOTICEI("accepted ", rankCount);

    if(SetSocketOptions(inSocket_getInt(tempHosts[rankCount]))<0)
    {
      MPID_USOCK_SysError("SetSocketOptions failed ",WSAGetLastError());
      DSECTLEAVE
	return -1;
    }

    /*   Now the master is connected to this client and we check for its MPI rank */
    if(inSocket_read(&msg, sizeof(TCPCommMessage), tempHosts[rankCount])<=0)
    {
      MPID_USOCK_SysError("Master cannot read from client",WSAGetLastError()); 
      DSECTLEAVE
	return -1;
    }

#define SHOW_NODE_INFO
#ifdef SHOW_NODE_INFO
	DNOTICEI("got message of node no: ",rankCount);
	DNOTICES("ID: ",msg.body.Data.ID);
	DNOTICEI("IPCount: ",msg.body.Data.IPCount);
	DNOTICES("IP[0]: ",IP(msg.body.Data.address[0]));	
	DNOTICEI("Port: ",msg.body.Data.listeningPort);

#endif

    /* if the rank of this client is not specified, the master determins the rank */
    if(msg.body.Data.ID==0)
    {
      if(rankCount_flag==-1)
      {
	MPID_USOCK_Error("You have to provide the [-r rank] option to all clients or to none of them!",-1);
	DSECTLEAVE
	  return -1;
      }
      rankCount_flag=1;
      msg.body.Data.ID=rankCount;
    }
    else
    {
      if(msg.body.Data.ID>=(DWORD)CTCPCommunicatorPt->numProcs)
      {
	MPID_USOCK_Error("Invalid client rank!!!",4);
	DSECTLEAVE
	  return -1;
      }
      if(rankCount_flag==1)
      {
	MPID_USOCK_Error("You have to provide the [-r rank] option to all clients or to none of them!",-1);
	DSECTLEAVE
	  return -1;
      }
      rankCount_flag=-1;
    }
    
    /* Check for a valid message */
    if(msg.body.type != COMM_CONNECT)
    {
      DNOTICE("This was the wrong message...");
      inSocket_Destructor(tempHosts[rankCount]);
      free(tempHosts[rankCount]);
      rankCount--;
      continue;
    }

    msg.body.local=CTCPCommunicatorPt->numProcs;

    if(inSocket_write(&msg, sizeof(TCPCommMessage), tempHosts[rankCount]) < sizeof(TCPCommMessage))
    {
      MPID_USOCK_SysError("Could not resend the message to client",WSAGetLastError());
      DSECTLEAVE
	return -1;
    }

    if(rankMap[msg.body.Data.ID]!=-1)
    {
      MPID_USOCK_Error("Dublicated client rank!",msg.body.Data.ID);
      DSECTLEAVE
	return -1;
    }
    rankMap[msg.body.Data.ID]=rankCount;
    tempNodes[rankCount]=msg.body.Data;
  }

  DNOTICE("master connected to all clients");
  
  /* Now the master is connected to all clients and we remap the ranks in a ascending order */
  for(rankCount=1; rankCount<(DWORD)CTCPCommunicatorPt->numProcs; ++rankCount)
  {
    CTCPCommunicatorPt->hosts[rankCount]=tempHosts[rankMap[rankCount]];
    nodes[rankCount]=tempNodes[rankMap[rankCount]];

  }
  free(tempHosts); 
  free(tempNodes);
  free(rankMap);

  /* Now the master distributes the NodeDescriptions to the clients */
  
  for(rankCount=1; rankCount<(DWORD)CTCPCommunicatorPt->numProcs; ++rankCount)
  {  
    for(i=0; (DWORD)i<rankCount; i++)
    {
      if(inSocket_write(&nodes[i], sizeof(nodes[i]), CTCPCommunicatorPt->hosts[rankCount]) < sizeof(nodes[i]))
      {
	MPID_USOCK_SysError("Could not send the data to the clients",WSAGetLastError());
	DSECTLEAVE
	  return -1;
      }       
    }
  }
  
  if(CTCPCommunicatorPt->numProcs>1)
  {
    if(inSocket_read(&msg, sizeof(TCPCommMessage), CTCPCommunicatorPt->hosts[1])<=0)
    {
      MPID_USOCK_SysError("Read failed in MasterConnect",WSAGetLastError()); 
      DSECTLEAVE
	return -1;
    }
    if(msg.body.type != Start)
    {
      MPID_USOCK_Error("Wrong message type received",-1);
      DSECTLEAVE
	return -1;
    } 
  }
  
  DSECTLEAVE
    return 0;
}


/*
 |  This member function is used by all startup clients. It first of all
 |  connects to the master startup process an sends it a NodeDescription struct.
 |  When the answer has been received it starts connecting to all other nodes as described
 |  in the class description.
 */
int CTCPCommunicator_ClientConnect(unsigned long Master, CTCPCommunicator* CTCPCommunicatorPt)
{
  DSECTION("CTCPCommunicator");

  TCPCommMessage msg;
  NodeDescription *nodes=0,*actNode; 
  unsigned size;
  DWORD actId;
  inSocket masterSocket, *tmpSocket;
  struct timeval t = {MAX_TIMEOUT,0};
  int res,i;
  BOOL local;
  FD_SET allfds;
  DWORD offset=0,NumIps;
  unsigned long *IPs=0;
  BOOL failed=TRUE;
  DWORD wait=0,ret,dummy;
  
  DSECTENTRYPOINT;
  
  TCPCommMessage_Constructor(&msg);
  inSocket_Constructor_0_(&masterSocket);

  /*
   |  Create a socket that will be used to accept
   |  connections from hosts with higher id.
   */

  DNOTICE("Create a socket that will be used to accept");
  inSocket_create(&masterSocket);
  if(SetSocketOptions(inSocket_getInt(&masterSocket)) <0)
  {
    MPID_USOCK_SysError("SetSocketOptions() failed in ClientConnect()",WSAGetLastError());
    DSECTLEAVE
      return -1;
  }
  inSocket_bind(0, &masterSocket);
  DNOTICEI("Bound listening socket to port ", inSocket_getPort(&masterSocket)); 
  inSocket_listen(SOMAXCONN, &masterSocket);
  
  msg.body.type=COMM_CONNECT;
  msg.body.Data.listeningPort=inSocket_getPort(&masterSocket);
  msg.body.local = local = IsLocalAddress(Master);
  msg.body.ProcessId = GetCurrentProcessId();
  msg.body.Data.ID=CTCPCommunicatorPt->myId;
  /* initialize IP info to prevent from submitting invalid information */
  msg.body.Data.IPCount=0;
  msg.body.Data.address[0]=-1;
  size = sizeof(msg);
    
  if(!CTCPCommunicatorPt->localAddress)
  {
    /* Fill in all my local IP addresses */
    DNOTICE("Fill in all my local IP addresses");
    NumIps = 0;
    GetLocalIps(0,&NumIps,TRUE);
    if(NumIps > 1)
    {
      IPs = (unsigned long*)malloc(NumIps*sizeof(unsigned long));
      GetLocalIps(IPs,&NumIps,TRUE);
	  /* in chwsock2: first call of GetLocalIps delivers number including loopback
	    next calls deliver number without loopback
	    when we are here NumIps is always 0 so set it to 1 
		in ch_usock: no further call is made -> choose last address manually!
		if call is repeated, then addresss will be reset in "else" */
		msg.body.Data.address[0]=IPs[NumIps-1];
		
	  /*
      size -= sizeof(unsigned long);
	  */
    }
    else 
		GetLocalIps(msg.body.Data.address,&NumIps,TRUE);
  }
  else 
  {
    /* We had a -b switch, so only use this address */
    DNOTICE("We had a -b switch, so only use this address");
    NumIps = 1;
    msg.body.Data.address[0]=CTCPCommunicatorPt->localAddress;
  }
  msg.body.Data.IPCount = NumIps;
  msg.header.BodySize = sizeof(NodeDescription)+(NumIps-1)*sizeof(unsigned long);

#define SHOW_NODE_INFO
#ifdef SHOW_NODE_INFO
  DNOTICE("Generated node information:");
  DNOTICES("ID: ",msg.body.Data.ID);
  DNOTICEI("IPCount: ",msg.body.Data.IPCount);
  DNOTICES("IP[0]: ",IP(msg.body.Data.address[0]));	
  DNOTICEI("Port: ",msg.body.Data.listeningPort);
#endif
  
  /* Connect to master */
  DNOTICE("Connect to master");

  tmpSocket = (inSocket*) malloc (sizeof(inSocket));
  inSocket_Constructor_0_(tmpSocket);
  inSocket_create(tmpSocket);

  wait=0;
  do 
  {
    DNOTICES("Trying connection to ", IP(Master));
    if(inSocket_connect_0_(Master, CTCPCommunicatorPt->Masterport, tmpSocket)<0)
    {
      sleep(1);
      wait++;
      failed=TRUE;
    }
    else
    {
      failed=FALSE;
      DNOTICES("Connected to ", IP(Master));
    }
  } 
  while(failed && wait<CTCPCommunicatorPt->timeout);
    
  if(failed)
  {
    MPID_USOCK_SysError("Could not connect to master", -1);
    DSECTLEAVE
      return -1;
  }
  
  /* Send my name and my ID (if valid) to the master */
  DNOTICE("Send my node information to the master");

  SetSocketOptions(inSocket_getInt(tmpSocket));
  if(inSocket_write(&msg, size, tmpSocket) < size)
  {
    MPID_USOCK_SysError("Could not send my data to master",WSAGetLastError());
    DSECTLEAVE
      return -1;
  }
  
  if(inSocket_read(&msg, sizeof(TCPCommMessage), tmpSocket) < sizeof(TCPCommMessage))
  {
    MPID_USOCK_SysError("Read from master failed",WSAGetLastError());
    DSECTLEAVE
      return -1;
  }
  
  /* overwrite the ID for the case, that the master has determined an ID for me: */
  CTCPCommunicatorPt->myId=msg.body.Data.ID;
  
  CTCPCommunicatorPt->numProcs = msg.body.local;
  CTCPCommunicatorPt->hosts = (inSocket**) malloc (CTCPCommunicatorPt->numProcs * sizeof(inSocket*));
  memset(CTCPCommunicatorPt->hosts,0,CTCPCommunicatorPt->numProcs*sizeof(inSocket*));
  CTCPCommunicatorPt->hosts[0] = tmpSocket;
  CTCPCommunicatorPt->hosts[CTCPCommunicatorPt->myId]=0;
  DNOTICEI("MyId=", CTCPCommunicatorPt->myId);
  DNOTICEI("NumProcs=", CTCPCommunicatorPt->numProcs);
  CTCPCommunicatorPt->SMPId = CTCPCommunicatorPt->myId;
  
  nodes = (NodeDescription*) malloc (CTCPCommunicatorPt->numProcs * sizeof(NodeDescription));
  
  for(i=0; (DWORD)i<CTCPCommunicatorPt->myId; i++)
  {
    if(inSocket_read(&nodes[i], sizeof(nodes[i]), tmpSocket)<=0)
    {
      MPID_USOCK_SysError("Could not read the NodeDescription array",WSAGetLastError());
      DSECTLEAVE
	return -1;
    }   
  }
  
  nodes[CTCPCommunicatorPt->myId]=msg.body.Data;
  
  /*
   |  Now we have all data
   |  We can start connecting to other nodes.
   |  First wait for connections from nodes with higher id.
   */
  DNOTICE("First wait for connections from nodes with higher id.");
  FD_SET(inSocket_getInt(&masterSocket), &(CTCPCommunicatorPt->backup));
  FD_SET(inSocket_getInt(CTCPCommunicatorPt->hosts[0]),&(CTCPCommunicatorPt->backup));

  for(i=0; (DWORD)i<CTCPCommunicatorPt->numProcs-CTCPCommunicatorPt->myId-1;i++)
  {
    tmpSocket = (inSocket*) malloc (sizeof(inSocket));
    inSocket_Constructor_0_(tmpSocket);
    
    allfds=CTCPCommunicatorPt->backup;
    DNOTICE("Starting to wait for connections");
    res=select(FD_SETSIZE,&allfds,0,0,&t);
    if(res<0 || !FD_ISSET(inSocket_getInt(&masterSocket), &allfds) )
    {
      if(!res) res = WSAECONNRESET;
      else res = WSAGetLastError();
      MPID_USOCK_SysError("Found dead connection while waiting for connections",res);
      DSECTLEAVE
	return -1;
    }
    inSocket_accept(&masterSocket, tmpSocket);
    SetSocketOptions(inSocket_getInt(tmpSocket));
    msg.body.type = MSG_UNUSED; /* Sanity check... */
    /* How long is the next message ? */
    DNOTICE("How long is the next message ?");
    if(inSocket_read(&msg, sizeof(TCPCommMessage), tmpSocket)<0)
    {
      if(nodes) free(nodes);
      inSocket_Destructor(tmpSocket);
      free(tmpSocket);
      MPID_USOCK_SysError("Read failed in client connect...",WSAGetLastError());
      DSECTLEAVE
	return -1;
    }
    if(msg.body.type != COMM_CONNECT)
    {
      /* Nice try ;-) */
      inSocket_Destructor(tmpSocket);
      free(tmpSocket);
      i--;
      DNOTICE("CommConnect: Illegal message received");
      continue;
    }
    CTCPCommunicatorPt->hosts[msg.body.Data.ID]=tmpSocket;
    FD_SET(inSocket_getInt(tmpSocket), &(CTCPCommunicatorPt->backup));
    DNOTICEI("Accepted connection from ", msg.body.Data.ID);
  }	
  
  if(CTCPCommunicatorPt->myId>1)
  {	
    /* Now connect to nodes with lower id */
    DNOTICE("Now connect to nodes with lower id");
    msg.body.type=COMM_CONNECT;
    msg.header.BodySize=sizeof(TCPMessageBody);
    msg.body.Data.ID=CTCPCommunicatorPt->myId;

    for(actId=1;actId<CTCPCommunicatorPt->myId;actId++)
    {
      /* actNode=&nodes[CTCPCommunicatorPt->myId-actId]; */
      actNode=&nodes[actId];
      
      msg.body.local= IsLocalAddress(actNode->address[0]);
      if(msg.body.local)
      {
	actNode->address[0] = htonl(INADDR_LOOPBACK);
	NumIps = 1;
	
      } else NumIps = actNode->IPCount;
      msg.body.ProcessId=GetCurrentProcessId(); 
      
      CTCPCommunicatorPt->hosts[actId] = (inSocket*) malloc (sizeof(inSocket));
      inSocket_Constructor_0_(CTCPCommunicatorPt->hosts[actId]);
      inSocket_create(CTCPCommunicatorPt->hosts[actId]);
      inSocket_bind(0, CTCPCommunicatorPt->hosts[actId]); 

      failed = TRUE; wait =0;
      while(failed && wait<CTCPCommunicatorPt->timeout && NumIps)
      {
	failed=FALSE;

#if !defined(USE_NT2UNIX)
	ret= GetRTTAndHopCount(actNode->address[NumIps-1],&dummy,255,&offset); 
	/* GetBestInterface(actNode->address[NumIps-1],&dummy); */
	   if(!ret)
	   {
	     ret = GetLastError();
	     DNOTICES("GetRTTAndHopCount for ", IP(actNode->address[NumIps-1]));
	     DNOTICEI("GetRTTAndHopCount returned ", ret);
	     failed = TRUE;
	     --NumIps;
	     continue;
	   } 
#endif	 
	   
	   DNOTICES("Trying connection to ", IP(Master));
	   if(inSocket_connect_0_(actNode->address[NumIps-1], actNode->listeningPort, CTCPCommunicatorPt->hosts[actId])<0)
	   {
	     sleep(1);
	     wait++;
	     failed=TRUE;
	   }
	   else
	   {
	     failed=FALSE;
	     DNOTICES("Connected to ", IP(Master));
	   }

	   /*
	   if(e.GetError() == WSAECONNREFUSED) {
	     if(wait<MAXWAIT) Sleep(2000);
	     ++wait;	
	   } else --NumIps;
	   */

	   
      }
	    
      if(failed)
      {
	MPID_USOCK_SysError("Could not connect to lower node",WSAGetLastError());
	DSECTLEAVE
	  return -1;
      }
      
      SetSocketOptions(inSocket_getInt(CTCPCommunicatorPt->hosts[actId]));
      inSocket_write(&msg, sizeof(TCPCommMessage), CTCPCommunicatorPt->hosts[actId]);
      DNOTICES("Connected to ", IP(actNode->address[NumIps-1]));
      /* actNode=NEXTNODE(actNode); */
    }
  }
   
  if(CTCPCommunicatorPt->myId==1)
  {
    /*
     |  Tell the nodes that connections are ready
     |  node 1 should be the last one that comes up
     |  because it has to wait for all others to connect with
     |  it. So it can start all other nodes.
    */
    msg.header.BodySize=sizeof(DWORD);
    msg.body.type=Start;
    for(actId=0;actId<(DWORD)CTCPCommunicatorPt->numProcs;actId++)
    {
      if(actId!=1)
      {
	DNOTICEI("Sending StartMessage to ", actId);
	inSocket_write(&msg, sizeof(TCPCommMessage), CTCPCommunicatorPt->hosts[actId]);
      }
    }
  } 
  else 
  {
    /* read the start message from node 1 */
    if(inSocket_read(&msg, sizeof(TCPCommMessage), CTCPCommunicatorPt->hosts[1])<0) {
      if(nodes) free(nodes);
      DSECTLEAVE
	return -1;
    }
    if(msg.body.type != Start) {
      MPID_USOCK_Error("Illegal message received",-1);
      if(nodes) free(nodes);
      DSECTLEAVE
	return -1;
    }    
  }
  if(nodes) free(nodes);

  DSECTLEAVE
    return 0;
}

#ifdef _MSC_VER
#pragma warning( 3 : 4706)
#endif


int CTCPCommunicator_GetFDs(int **Dest,CTCPCommunicator* CTCPCommunicatorPt)
{
  DSECTION("CTCPCommunicator_GetFDs");
  int res=0;
  unsigned long i;
  
  DSECTENTRYPOINT;

  *Dest = (int*) malloc (CTCPCommunicatorPt->numProcs * sizeof(int));
  
  if(!*Dest) {DSECTLEAVE return 0;}
  for(i=0; i<CTCPCommunicatorPt->numProcs; i++)
  {
    if(CTCPCommunicatorPt->hosts && CTCPCommunicatorPt->hosts[i])
    {
      (*Dest)[i]=inSocket_getInt(CTCPCommunicatorPt->hosts[i]);
      res++;
    }
    else (*Dest)[i]=INVALID_SOCKET;
  }

  DSECTLEAVE
    return res;
}


/* The default constructor initializes the header */
void TCPCommMessage_Constructor(TCPCommMessage *TCPCommMessagePt)
{
  TCPCommMessagePt->header.BodySize=sizeof(TCPCommMessagePt->body); 
  TCPCommMessagePt->body.local = FALSE;
}


#ifdef _MSC_VER
#pragma warning( 3: 4705 4706 )
#endif
