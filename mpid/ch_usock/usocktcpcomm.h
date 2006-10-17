#ifndef __USOCK_TCP_COMMUNICATOR_H__
#define __USOCK_TCP_COMMUNICATOR_H__

/* A special Communicator implemented with TCP sockets. */

#include "usockinsocket.h"
#include "stl2c_bool.h"

#ifdef _MSC_VER
#pragma warning( 3: 4705 4706 )
#endif

#define MAX_NAME_LEN 255
#define BASEPORT 2112
#define DEV "ch_usock"

/* 
 |   Size of the socket buffers:
 |  (set by the SO_RCVBUF and SO_SNDBUF options)
 */
#define SOCKET_BUF_SIZE 1024*1024*4
/*---*/

#define ACK_MASTER 3
#define COMM_CONNECT 4
#define Start 5
#define Quit 6
#define AckQuit 7
#define ErrorExit 8
#define MSG_UNUSED 9

/*
 |  Each process sends one of these structs to the startup manager and gets
 |  back an array of these structs describing all nodes that contacted the manager
 |  earlier.
*/
typedef 
struct _NodeDescription {
  /* The MPI id hof that process */
  DWORD ID;

  /* The number of addresses in the array. */
  DWORD IPCount;

  /* The port this process accepts connections on. */
  unsigned int listeningPort;
  
  /* The IP addresses of the host. */
  unsigned long address[1];

} NodeDescription;

typedef
struct _MessageHeader {
  /* The size of the data following this header */
  DWORD BodySize;	

} MessageHeader;

/*
 |  Each message sent by CTCPCommunicator uses this struct as message body
 |  to exchange information.
 */
typedef 
struct _TCPMessageBody {
  /* The message type */
  DWORD type;
  
  /* If sent by the master this field contains
    the number of processes. If sent by the client
    it indicates if this is a connection to localhost */
  DWORD local;
  
  /* Used if the connection is local. */
  DWORD ProcessId;
  HANDLE Handles[2];
  
  /* An array of NodeDescrition structs */
  NodeDescription Data;
} TCPMessageBody;

/*
 |  Each message sent by CTCPCommunicator uses this struct
 |  to exchange information.
 */
typedef
struct _TCPCommMessage {
  /* The message header */
  MessageHeader header;
	
  /* And the body */
  TCPMessageBody body;

} TCPCommMessage;


/* The default constructor initializes the header */
void TCPCommMessage_Constructor(TCPCommMessage *TCPCommMessagePt);

/*
 |  This class is used to exchange messages between processes using Berkeley TCP sockets.
 |  It overrides several member functions of CCommunicator.<BR>
 |  At startup this class parses the commandline. Using this information one node
 |  becomes the \e startup \e master. All others become clients.
 |  Each client first connects to the master and gets its id and a list of nodes
 |  with lower ids. It then waits for all clients with higher id to connect. 
 |  If these connections are established the node connects to all clients with lower IDs.
 */

typedef
struct _CTCPCommunicator {
	
  /* An array that containes the communication endpoints to all other nodes. */
  inSocket **hosts;

  /* The port the strtup mster listens on. */
  short Masterport;
  
  /* The timeout value: */
  int timeout;

  /* Used at startup time */
  DWORD numConnections;

  /* Used by the MainLoop() for the select() call. */
  fd_set backup;

  /* A flag indicating that the system shuts down connections. Used by ShudDown(). */
  bool ShuttingDown;

  /* The global SVMlib process id. Used by GetMyId(). */
  DWORD myId;
	
  /* The total number of processes. Used by GetNumProcs(). */
  DWORD numProcs;

  /* Used by MagPie to determine clusters */
  DWORD SMPId;

  bool LocalMaster;

  unsigned long masterAddress,localAddress;

} CTCPCommunicator;

void CTCPCommunicator_Constructor(                   CTCPCommunicator* CTCPCommunicatorPt);
void CTCPCommunicator_Destructor(                    CTCPCommunicator* CTCPCommunicatorPt);
int  CTCPCommunicator_Create(int *argc,char*** argv, CTCPCommunicator* CTCPCommunicatorPt);
int  CTCPCommunicator_GetFDs(int **Dest,      CTCPCommunicator* CTCPCommunicatorPt);
 
DWORD CTCPCommunicator_GetMyId(    CTCPCommunicator* CTCPCommunicatorPt);
DWORD CTCPCommunicator_GetSMPId(   CTCPCommunicator* CTCPCommunicatorPt);
DWORD CTCPCommunicator_GetNumProcs(CTCPCommunicator* CTCPCommunicatorPt);

int  CTCPCommunicator_MasterConnect(                      CTCPCommunicator* CTCPCommunicatorPt);
int  CTCPCommunicator_ClientConnect(unsigned long Master, CTCPCommunicator* CTCPCommunicatorPt);
void CTCPCommunicator_CloseSockets(                       CTCPCommunicator* CTCPCommunicatorPt);

void CTCPCommunicator_MasterSMP(inSocket *Client,TCPMessageBody *msg, CTCPCommunicator* CTCPCommunicatorPt);
void CTCPCommunicator_ClientSMP(inSocket *Client,TCPMessageBody *msg, CTCPCommunicator* CTCPCommunicatorPt);

#endif

